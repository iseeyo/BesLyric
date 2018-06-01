/*
	BesLyric  һ�� �����򵥡�����ʵ�õ� ר�������������������ֹ�����ʵ� �������������
    Copyright (C) 2017  
	Author: BensonLaur <BensonLaur@163.com>
	Author:

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
* @file       LoadAndCheckNcmIDThread.h
* 
* Describe    ����һ���̣߳���������ncm ID ��������� ID�ĺϷ��� (֮����ʹ���̣߳�����Ϊ���������ܱȽϺ�ʱ��������ʱ���ᵼ�½��������Ӻ�)
*/

#pragma once
#include "stdafx.h"
#include "../utility/SSingleton.h"


class  CLoadAndCheckNcmIDThread : public Singleton<CLoadAndCheckNcmIDThread>
{
public:
	// �������������
	CLoadAndCheckNcmIDThread():m_handleThread(NULL){}

	//��ʼ�߳�
	bool Start();

	//�����߳�
	void Stop();
	
private:

	// �߳�ִ�е�ַ
	static DWORD WINAPI ProcLoadAndCheckNcmID(LPVOID pParam);

private:
	HANDLE m_handleThread;
};
