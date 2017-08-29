/*
*	Copyright (C) 2017  BensonLaur
*	note: Looking up header file for license detail
*/

// LyricMaker.cpp :  实现  LyricMaker类 的接口	
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "LyricMaker.h"
#include "../utility/FileHelper.h"
#include "../utility/WinFile.h"
#include "Windows.h"
#include <mmsystem.h> 
#include "BSMessageBox.h"
#pragma comment (lib, "winmm.lib")

LyricMaker::LyricMaker():
		 m_bLastLineSpace(false),
		 m_nCurLine(0),
		 m_nTotalLine(0)
{
	m_szMusicPathName[0]= _T('\0');		
	m_szLyricPathName[0]= _T('\0');
	m_szOutputPath[0]= _T('\0');
	m_szOutputPathName[0] = _T('\0');
}

void LyricMaker::Init(CSettingPage *pSettingPage)
{
	SASSERT(NULL != pSettingPage);

	//保存主窗口对象
	m_pSettingPage = pSettingPage;
}

//设置各个路径
//设置音乐路径时，传入播放音乐需要的 消息宿主窗口的句柄
void LyricMaker::setMusicPath(LPCTSTR pathName, HWND hostWnd)
{
	_tcscpy(m_szMusicPathName,pathName);
	m_musicPlayer.init(m_szMusicPathName,hostWnd);
}
	
void LyricMaker::setLyricPath(LPCTSTR pathName)
{
	_tcscpy(m_szLyricPathName,pathName);
}
	
void LyricMaker::setm_szOutputPath(LPCTSTR pathName)
{
	_tcscpy(m_szOutputPath,pathName);
}

//重置 LyricMaker的 歌词数据为空, 生成输出的文件名
void LyricMaker::reloadMaker()
{
	m_vLyricOrigin.clear();
	m_vLyricOutput.clear();

	//生成输出的文件名
	generateOutputFileName();
}

//制作开始,记录开始制作的时间
//成功播放返回true，需要转换格式返回false
bool LyricMaker::makingStart()
{
	SYSTEMTIME		startPoint;				/* 记录开始的时间点 */
	GetLocalTime(&startPoint);

	SystemTimeToFileTime(&startPoint,(FILETIME*)&startPointF); 

	//更新基本的行数记录的数据
	m_nTotalLine = m_vLyricOrigin.size();
	m_nCurLine = 0;

	//异步播放音乐
	//PlaySound(m_szMusicPathName,NULL,SND_FILENAME|SND_ASYNC); //不支持MP3

	return playMusic();
}

//为下一行歌词 标记上 网易云音乐要求的 时间轴格式，写入m_vLyricOutput中
void LyricMaker::markNextLine()
{
	SYSTEMTIME		currentPoint;				/* 记录当前的时间点 */
	ULARGE_INTEGER  currentPointF;				/* 对应的 FILETIME ，为了得到时间差，使用FILETIME*/ 

	GetLocalTime(&currentPoint);
	SystemTimeToFileTime(&currentPoint,(FILETIME*)&currentPointF); 
	
	unsigned __int64 dft=currentPointF.QuadPart-startPointF.QuadPart; 
	int ms = (int)(dft/10000);//得到相差的毫秒数

	//根据用户设置的时间偏移，修改得到实际记录的毫秒数
	ms -= m_pSettingPage->m_nTimeShift;
	if(ms < 0 )
		ms = 0;

	//得到[00:33.490] 形式的时间串
	TCHAR timeBuf[255];
	msToLyricTimeString(ms, timeBuf);

	//构建新的一行加入m_vLyricOutput中
	SStringT newLine(timeBuf);
	newLine.Append(m_vLyricOrigin.at(m_nCurLine-1));
	newLine.Append(SStringT(_T("\n")));
	m_vLyricOutput.push_back(newLine);

	//MB(SStringT().Format(_T("%s"),m_vLyricOutput.at(m_nCurLine-1)));

	setLastLineSpace(false);
}

//如果上一行不是空白行的话,添加
void LyricMaker::markSpaceLine()
{
	if(!isLastLineSpace())
	{
		SYSTEMTIME		currentPoint;				/* 记录当前的时间点 */
		ULARGE_INTEGER  currentPointF;				/* 对应的 FILETIME ，为了得到时间差，使用FILETIME*/ 
		GetLocalTime(&currentPoint);
	
		SystemTimeToFileTime(&currentPoint,(FILETIME*)&currentPointF); 
	
		unsigned __int64 dft=currentPointF.QuadPart-startPointF.QuadPart; 
		int ms = (int)(dft/10000);//得到相差的毫秒数

		//根据用户设置的时间偏移，修改得到实际记录的毫秒数
		ms -= m_pSettingPage->m_nTimeShift;
		if(ms < 0 )
			ms = 0;

		//得到[00:33.490] 形式的时间串
		TCHAR timeBuf[255];
		msToLyricTimeString(ms, timeBuf);

		//构建新的一行加入m_vLyricOutput中 (只有时间的空白行)
		SStringT newLine(timeBuf);
		newLine.Append(SStringT(_T("\n")));

		//if( m_nCurLine >= m_nTotalLine )//此时添加的空行在文件结尾，而原文件可能没有回车，所以这里多加个回车前缀
		//	newLine.Insert(0,_T("\n"));

		m_vLyricOutput.push_back(newLine);

		setLastLineSpace(true);
	}
}

//将 m_vLyricOutput 写入输出文件m_szOutputPathName 中
void LyricMaker::makingEnd()
{
	bool bRet = FileOperator::WriteToUtf8File(m_szOutputPathName,m_vLyricOutput);

	if(!bRet)
	{
		BSMessageBox m;
		m.MessageBoxW(NULL,SStringT().Format(_T("文件写入失败:\\n【%s】\\n!请确保文件有效"),m_szOutputPathName),
			_T("失败提示"),MB_OK|MB_ICONWARNING);
		return;
	}

	//停止播放音乐
	stopMusic();
}

//获得当前的输出 文件名
void LyricMaker::getOutputFileName(TCHAR* name, int lenth)
{
	_tcscpy(name,outputFileName);
}

//获得当前的输出 路径文件名
void LyricMaker::getm_szOutputPathName(TCHAR* name, int lenth)
{
	_tcscpy(name,m_szOutputPathName);
}

//结束音乐播放
void LyricMaker::stopMusic()
{
	m_musicPlayer.closeStop();
}

void LyricMaker::setLastLineSpace(bool value)
{
	this->m_bLastLineSpace = value;
}

//上一行是否为空白行
bool LyricMaker::isLastLineSpace()
{
	return this->m_bLastLineSpace;
}

//从文件获取每行歌词的集合向量
vector<SStringT> LyricMaker::getLyricOrigin(File& encodingFile)
{
	vector<SStringW> lyrics;

	FileOperator::ReadAllLinesW(encodingFile, &lyrics);
	
	return lyrics;
}


//根据m_szMusicPathName 的文件名得到歌词文件名，并更新outputFileName 和 m_szOutputPathName的值
void LyricMaker::generateOutputFileName()
{
	int len = _tcslen(m_szMusicPathName);
	if(len==0)
		return;

	// 保存最后一个'\\'的位置
	int i,j;
	int pos; 
	for(i=0;i< len;i++)
		if(m_szMusicPathName[i]==_T('\\'))
			pos = i;

	//复制歌名到 outputFileName ，不包括后缀
	for(i=pos+1,j=0;m_szMusicPathName[i]!=_T('.');i++,j++)
	{
		outputFileName[j] = m_szMusicPathName[i];
	}
	outputFileName[j] = _T('\0');

	//补充完输出完整的文件名
	_tcscat(outputFileName,_T(".lrc"));

	//初始化 m_szOutputPathName
	_tcscpy(m_szOutputPathName,m_szOutputPath);
	_tcscat(m_szOutputPathName,_T("\\"));
	_tcscat(m_szOutputPathName,outputFileName);
}

//将毫秒差值时间 转换为歌词时间格式 “[00:33.490] Look at the stars”
//以 [00:33.490] 格式输出到 timeBuf
void LyricMaker::msToLyricTimeString(int ms, LPTSTR timeBuf)
{
	int minutes = ms/60000;
	ms = ms%60000;
	double seconds = ms*1.0/1000;
	_stprintf(timeBuf,_T("[%02d:%06.3lf]"),minutes,seconds);
}

//播放音乐
bool LyricMaker::playMusic()
{
	if(m_musicPlayer.openTest() == false)
	{
		return false;
	}

	m_musicPlayer.openStart();
	return true;
}

