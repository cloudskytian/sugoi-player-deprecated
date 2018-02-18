#include <Windows.h>
#include <Shlwapi.h>
#include <WtsApi32.h>
#include <UserEnv.h>
#include <iostream>
#include <atlstr.h>
#include <atlbase.h>
#include <tchar.h>

static TCHAR lpServiceName[] = TEXT("SPlayer Protect Service");
static SERVICE_STATUS_HANDLE hServiceStatus = NULL;
static SERVICE_STATUS	ServiceStatus = {0};
static TCHAR szCurDir[MAX_PATH+1] = {0};
static bool bRun = false;
static HANDLE hProcess = NULL;

bool InstallService();
VOID WINAPI ServiceMain(DWORD dwArgc,LPTSTR *lpszArgv);
VOID WINAPI HandlerFunc(DWORD dwControl);
HANDLE RunAsLoggedUser(CString lpPath, CString lpCmdLine);
void WorkFunc();

int main(int argc, char **argv)
{
    GetModuleFileName(NULL,szCurDir,MAX_PATH);
    TCHAR *pFind = _tcsrchr(szCurDir, '\\');
    if (pFind)
    {
        *pFind = '\0';
    }

    SERVICE_TABLE_ENTRY ServiceTable[2];
    ServiceTable[0].lpServiceName = lpServiceName;
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

    ServiceTable[1].lpServiceName = NULL;
    ServiceTable[1].lpServiceProc = NULL;

    if (!StartServiceCtrlDispatcher(ServiceTable))
    {
        std::cout<<"Program not run as service. Do you want to install and run this service? y/n:";
        if (getchar() == 'y')
        {
            if (!InstallService())
            {
                std::cout<<"Failed to install sercice."<<std::endl;
                return 1;
            }
        }

    }
    return 0;
}

bool InstallService()
{
    SC_HANDLE hSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
    if (!hSCManager)
    {
        return false;
    }

    SC_HANDLE hService = OpenService(hSCManager,lpServiceName,SERVICE_QUERY_CONFIG);
    if (hService)
    {
        CloseServiceHandle(hSCManager);
        CloseServiceHandle(hService);
        return false;
    }

    TCHAR szPath[MAX_PATH+1];
    GetModuleFileName(NULL,szPath,MAX_PATH);
    hService = CreateService(hSCManager,
        lpServiceName,
        lpServiceName,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        szPath,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);
    if (!hService)
    {
        CloseServiceHandle(hSCManager);
        return false;
    }

    if (!StartService(hService,0,NULL))
    {
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);

    return true;
}

VOID WINAPI ServiceMain( DWORD dwArgc,LPTSTR *lpszArgv )
{
    ServiceStatus.dwServiceType = SERVICE_WIN32;
    ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
    hServiceStatus = RegisterServiceCtrlHandler(lpServiceName,HandlerFunc);
    if (!hServiceStatus)
    {
        return;
    }
    if (!SetServiceStatus(hServiceStatus,&ServiceStatus))
    {
        return;
    }

    bRun = true;
    WorkFunc();
}

VOID WINAPI HandlerFunc( DWORD dwControl )
{
    if (dwControl == SERVICE_CONTROL_STOP || dwControl == SERVICE_CONTROL_SHUTDOWN)
    {
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        bRun = false;
        if (hProcess)
        {
            TerminateProcess(hProcess,0);
        }
    }

    SetServiceStatus(hServiceStatus,&ServiceStatus);
}

HANDLE RunAsLoggedUser(CString lpPath,CString lpCmdLine)
{
    if (!PathFileExists(lpPath))
    {
        return NULL;
    }

    DWORD dwSid = WTSGetActiveConsoleSessionId();

    HANDLE hExistingToken = NULL;
    if (!WTSQueryUserToken(dwSid,&hExistingToken))
    {
        return NULL;
    }

    HANDLE hNewToken = NULL;
    if (!DuplicateTokenEx(hExistingToken,MAXIMUM_ALLOWED,NULL,SecurityIdentification,TokenPrimary,&hNewToken))
    {
        CloseHandle(hExistingToken);
        return NULL;
    }
    CloseHandle(hExistingToken);

    LPVOID pEnv = NULL;
    DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
    if (CreateEnvironmentBlock(&pEnv,hNewToken,FALSE))
    {
        dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
    }

    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);
    si.lpDesktop = (LPWSTR)"WinSta0\\Default";

    if (!CreateProcessAsUser(hNewToken,lpPath.AllocSysString(),lpCmdLine.AllocSysString(),NULL,NULL,FALSE,dwCreationFlags,pEnv,szCurDir,&si,&pi))
    {
        if (pEnv)
        {
            DestroyEnvironmentBlock(pEnv);
        }
        return NULL;
    }

    if (pi.hThread && pi.hThread != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pi.hThread);
    }

    if (pEnv)
    {
        DestroyEnvironmentBlock(pEnv);
    }

    return pi.hProcess;
}

void WorkFunc()
{
    CString curDir = szCurDir;
    CString szProgPath = curDir + "\\SPlayer64.exe";
    CString szCmdLine = TEXT("--runinbackground");
    while (bRun)
    {
        hProcess = RunAsLoggedUser(szProgPath,szCmdLine);
        if (!hProcess)
        {
            //
        }
        WaitForSingleObject(hProcess,INFINITE);
        CloseHandle(hProcess);
        hProcess = NULL;
        Sleep(2000);
    }
}
