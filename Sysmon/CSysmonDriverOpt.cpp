#include "StdAfx.h"
#include "CSysmonDriverOpt.h"


CSysmonDriverOpt::CSysmonDriverOpt(void)
{
}


CSysmonDriverOpt::~CSysmonDriverOpt(void)
{
}


//======================================== ��̬����/ж��sys���� ======================================
// SYS�ļ����������ͬ��Ŀ¼��
// ���������SYS��ΪHelloDDK.sys,��ô��װ����InstallDriver("HelloDDK",".\\HelloDDK.sys","370030"/*Altitude*/);
// ������������ StartDriver("HelloDDK");
// ֹͣ�������� StopDriver("HelloDDK");
// ж��SYSҲ�����Ƶĵ��ù��̣� DeleteDriver("HelloDDK");
//====================================================================================================

BOOL CSysmonDriverOpt::InstallDriver(
							LPCTSTR lpszDriverName,
							LPCTSTR lpszDriverPath,
							LPCTSTR lpszAltitude)
{
	TCHAR   szTempStr[MAX_PATH];
	HKEY    hKey;
	DWORD    dwData;
	TCHAR    szDriverImagePath[MAX_PATH];    

	if( NULL == lpszDriverName || NULL == lpszDriverPath )
	{
		return FALSE;
	}

	//�õ�����������·��
	GetFullPathName(
			lpszDriverPath,
			MAX_PATH,
			szDriverImagePath,
			NULL);

	SC_HANDLE hServiceMgr = NULL;
	SC_HANDLE hService = NULL;

	//�򿪷�����ƹ�����
	hServiceMgr = OpenSCManager( 
							NULL,
							NULL, 
							SC_MANAGER_ALL_ACCESS );

	if( hServiceMgr == NULL ) 
	{
		CloseServiceHandle(hServiceMgr);
		return FALSE;        
	}

	//������������Ӧ�ķ���
	hService = CreateService( hServiceMgr,
		lpszDriverName,             // �����������ע����е�����
		lpszDriverName,             // ע������������DisplayName ֵ
		SERVICE_ALL_ACCESS,         // ������������ķ���Ȩ��
		SERVICE_FILE_SYSTEM_DRIVER, // ��ʾ���صķ������ļ�ϵͳ��������
		SERVICE_DEMAND_START,       // ע������������Start ֵ
		SERVICE_ERROR_IGNORE,       // ע������������ErrorControl ֵ
		szDriverImagePath,          // ע������������ImagePath ֵ
		_T("FSFilter Activity Monitor"),// ע������������Group ֵ
		NULL, 
		_T("FltMgr"),                   // ע������������DependOnService ֵ
		NULL, 
		NULL);

	if( hService == NULL ) 
	{        
		if( GetLastError() == ERROR_SERVICE_EXISTS ) 
		{
			//���񴴽�ʧ�ܣ������ڷ����Ѿ�������
			CloseServiceHandle(hService);       // ������
			CloseServiceHandle(hServiceMgr);    // SCM���
			return TRUE; 
		}
		else 
		{
			CloseServiceHandle(hService);       // ������
			CloseServiceHandle(hServiceMgr);    // SCM���
			return FALSE;
		}
	}
	CloseServiceHandle(hService);       // ������
	CloseServiceHandle(hServiceMgr);    // SCM���

	//-------------------------------------------------------------------------------------------------------
	// SYSTEM\\CurrentControlSet\\Services\\DriverName\\Instances�ӽ��µļ�ֵ�� 
	//-------------------------------------------------------------------------------------------------------
	lstrcpy(szTempStr,_T("SYSTEM\\CurrentControlSet\\Services\\"));
	lstrcat(szTempStr,lpszDriverName);
	lstrcat(szTempStr,_T("\\Instances"));
	if(RegCreateKeyEx(
				HKEY_LOCAL_MACHINE,
				szTempStr,
				0,
				_T(""),
				TRUE,
				KEY_ALL_ACCESS,
				NULL,
				&hKey,
				(LPDWORD)&dwData)!=ERROR_SUCCESS)
	{
		return FALSE;
	}

	// ע������������DefaultInstance ֵ 
	lstrcpy(szTempStr,lpszDriverName);
	lstrcat(szTempStr,_T("Instance"));
	if(RegSetValueEx(
					hKey,
					_T("DefaultInstance"),
					0,
					REG_SZ,
					(CONST BYTE*)
					szTempStr,
					(DWORD)lstrlen(szTempStr)*sizeof(TCHAR))!=ERROR_SUCCESS)
	{
		return FALSE;
	}

	RegFlushKey(hKey);//ˢ��ע���
	RegCloseKey(hKey);
	//-------------------------------------------------------------------------------------------------------

	//-------------------------------------------------------------------------------------------------------
	// SYSTEM\\CurrentControlSet\\Services\\DriverName\\Instances\\DriverName Instance�ӽ��µļ�ֵ�� 
	//-------------------------------------------------------------------------------------------------------
	lstrcpy(szTempStr,_T("SYSTEM\\CurrentControlSet\\Services\\"));
	lstrcat(szTempStr,lpszDriverName);
	lstrcat(szTempStr,_T("\\Instances\\"));
	lstrcat(szTempStr,lpszDriverName);
	lstrcat(szTempStr,_T("Instance"));

	if(RegCreateKeyEx(
			HKEY_LOCAL_MACHINE,
			szTempStr,
			0,
			_T(""),
			TRUE,
			KEY_ALL_ACCESS,
			NULL,
			&hKey,
			(LPDWORD)&dwData)!=ERROR_SUCCESS)
	{
		return FALSE;
	}

	// ע������������Altitude ֵ
	lstrcpy(szTempStr,lpszAltitude);
	if(RegSetValueEx(
				hKey,
				_T("Altitude"),
				0,
				REG_SZ,
				(CONST BYTE*)szTempStr,
				(DWORD)
				lstrlen(szTempStr)*sizeof(TCHAR))!=ERROR_SUCCESS)
	{
		return FALSE;
	}

	// ע������������Flags ֵ
	dwData = 0x0;
	if(RegSetValueEx(
					hKey,
					_T("Flags"),
					0,
					REG_DWORD,
					(CONST BYTE*)&dwData,
					sizeof(DWORD))!=ERROR_SUCCESS)
	{
		return FALSE;
	}

	RegFlushKey(hKey);//ˢ��ע���
	RegCloseKey(hKey);
	//-------------------------------------------------------------------------------------------------------

	return TRUE;
}

BOOL CSysmonDriverOpt::StartDriver(LPCTSTR lpszDriverName)
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;

	if( NULL == lpszDriverName)
	{
		return FALSE;
	}

	schManager = OpenSCManager(
							NULL,
							NULL,
							SC_MANAGER_ALL_ACCESS);

	if(NULL == schManager)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}

	schService = OpenService(
					schManager,
					lpszDriverName,
					SERVICE_ALL_ACCESS);

	if(NULL == schService)
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	if(!StartService(
			schService,
			0,
			NULL))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);

		DWORD dwError = GetLastError();
		if( dwError == ERROR_SERVICE_ALREADY_RUNNING ) 
		{             
			return TRUE;
		}

		TCHAR szError[50];
		wsprintf(szError,_T("����%d"),dwError);
		MessageBox(NULL,szError,szError,MB_OK);

		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

BOOL CSysmonDriverOpt::StopDriver(LPCTSTR lpszDriverName)
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;
	bool            bStopped=false;

	schManager = OpenSCManager(
							NULL,
							NULL,
							SC_MANAGER_ALL_ACCESS);
	if(NULL == schManager)
	{
		return FALSE;
	}

	schService = OpenService(
						schManager,
						lpszDriverName,
						SERVICE_ALL_ACCESS);
	if(NULL == schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}

	if(!ControlService(
					schService,
					SERVICE_CONTROL_STOP,
					&svcStatus) &&
					(svcStatus.dwCurrentState != SERVICE_STOPPED))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}

BOOL CSysmonDriverOpt::DeleteDriver(LPCTSTR lpszDriverName)
{
	SC_HANDLE        schManager;
	SC_HANDLE        schService;
	SERVICE_STATUS    svcStatus;

	schManager = OpenSCManager(
							NULL,
							NULL,
							SC_MANAGER_ALL_ACCESS);
	if(NULL == schManager)
	{
		return FALSE;
	}

	schService = OpenService(
						schManager,
						lpszDriverName,
						SERVICE_ALL_ACCESS);

	if(NULL == schService)
	{
		CloseServiceHandle(schManager);
		return FALSE;
	}

	ControlService(
				schService,
				SERVICE_CONTROL_STOP,
				&svcStatus);

	if(!DeleteService(schService))
	{
		CloseServiceHandle(schService);
		CloseServiceHandle(schManager);
		return FALSE;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schManager);

	return TRUE;
}
