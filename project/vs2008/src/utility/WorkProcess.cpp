#include "WorkProcess.h"
#include <new>
#include <iostream>


/**************************************************************
*  @brief : CWorkProcess::CWorkProcess
*     
*  @param : 
*     
*    -char* szApp    exe��ִ���ļ�����·����ϵͳ������Բ�������·��
*     
*    -char* szArg    ���������п���
*     
*  @author : Jimmy
*     
*  @date : 2019/3/9 ������
*     
*  @note : 
***************************************************************/
CWorkProcess::CWorkProcess(const char* szApp, char* szArg):
m_pszCmd(NULL),
m_bInherit(FALSE)
{
    ZeroMemory(&m_pi, sizeof(m_pi));
    ZeroMemory(&m_sa,sizeof(m_sa));

    m_pi.hProcess = NULL;
    m_pi.hThread = NULL;

    unsigned usArgLen = (szArg == NULL ? 0 : strlen(szArg)) ;

    if ( szApp != NULL)
    {
        //�����в���
        unsigned usCmdLen = strlen(szApp) + sizeof(' ') + usArgLen;

        //��'\n'����
        unsigned usBufferSize = usCmdLen + 1;

        m_pszCmd = new (std::nothrow) char[usBufferSize];
        memset(m_pszCmd, 0, usBufferSize);

        //��װ������ 0 1 2 3
        strcpy_s(m_pszCmd, usBufferSize, szApp);

        if (szArg != NULL && strlen(szArg) != 0)
        {
            m_pszCmd[strlen(szApp)] = ' ';
            strcat_s(m_pszCmd,usBufferSize, szArg);
        }
    }
}

/**************************************************************
*  @brief : CWorkProcess::~CWorkProcess
*     
*  @param : -none
*     
*  @author : Jimmy
*     
*  @date : 2019/3/9 ������
*     
*  @note : ������Դ
***************************************************************/
CWorkProcess::~CWorkProcess()
{
    if (m_pi.hProcess != NULL)
    {
        CloseHandle(m_pi.hProcess);
    }

    if (m_pi.hThread != NULL)
    {
        CloseHandle(m_pi.hThread);
    }

    if (m_pszCmd != NULL)
    {
        delete[] m_pszCmd;
        m_pszCmd = NULL;
    }

}

void  CWorkProcess::SetStartupInfo(const STARTUPINFOA& si)
{
    m_sa = si;
    m_sa.cb = sizeof(si);
}

void CWorkProcess::SetInheritanceOpt(bool bInherit)
{
    m_bInherit = (bInherit ? TRUE : FALSE);
}


/**************************************************************
*  @brief : CWorkProcess::LaunchProcess
*     
*  @param : -none
*     
*  @return : bool
*     
*  @author : Jimmy
*     
*  @date : 2019/3/9 ������
*     
*  @note : ����һ���ӽ���
***************************************************************/
bool CWorkProcess::LaunchProcess(bool showWnd)
{
    if (m_pszCmd == NULL || strlen(m_pszCmd) == 0)
    {
        return false;
    }

    DWORD dwCreateFlag = (showWnd ? CREATE_NEW_CONSOLE : 0);

    // Start the child process.
    if( !CreateProcessA(
        NULL,            // app path
        m_pszCmd,        // Command line (needs to include app path as first argument. args seperated by whitepace)
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        m_bInherit,          // Set handle inheritance to FALSE
        dwCreateFlag,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &m_sa,            // Pointer to STARTUPINFO structure
        &m_pi)           // Pointer to PROCESS_INFORMATION structure
        )
    {
        //std::cout << "CreateProcess failed " << GetLastError() << std::endl;
        return false;
    }
    else
    {
        //std::cout << "Successfully launched child process" << std::endl;
    }

    return true;
}


/**************************************************************
*  @brief : CWorkProcess::CheckIfProcessIsActive
*     
*  @param : -none
*     
*  @return : bool
*     
*  @author : Jimmy
*     
*  @date : 2019/3/9 ������
*     
*  @note : �������ͷŴ��
***************************************************************/
bool CWorkProcess::CheckIfProcessIsActive()
{
    // Check if handle is closed
    if ( m_pi.hProcess == NULL )
    {
        printf( "Process handle is closed or invalid (%d).\n");
        return false;
    }

    // If handle open, check if process is active
    DWORD lpExitCode = 0;
    if( GetExitCodeProcess(m_pi.hProcess, &lpExitCode) == 0)
    {
        printf( "Cannot return exit code (%d).\n", GetLastError() );
        return false;
    }

    if (lpExitCode == STILL_ACTIVE)
    {
        return true;
    }

    return false;
}


/**************************************************************
*  @brief : CWorkProcess::GetExitCode
*     
*  @param : -none
*     
*  @return : DWORD
*     
*  @author : Jimmy
*     
*  @date : 2019/8/9 ������
*     
*  @note : ����WaitForEnded�ӿڣ�
          ȷ�����̽�����Ȼ��ͨ��GetExitCode��ȡ�˳��룿
***************************************************************/
DWORD CWorkProcess::GetExitCode()
 {
     // Check if handle is closed
     if ( m_pi.hProcess == NULL )
     {
         printf( "Process handle is closed or invalid (%d).\n");
         return -1;
     }

     DWORD lpExitCode = -1;
     if( GetExitCodeProcess(m_pi.hProcess, &lpExitCode) == 0)
     {
         printf( "get exit code error (%d).\n", ::GetLastError() );
         return -1;
     }

    return lpExitCode;
 }
/**************************************************************
*  @brief : CWorkProcess::StopProcess
*     
*  @param : -none
*     
*  @return : bool
*     
*  @author : Jimmy
*     
*  @date : 2019/3/9 ������
*     
*  @note :  ����һ������
***************************************************************/
bool CWorkProcess::StopProcess()
{
    // Check if handle is invalid or has allready been closed
    if ( m_pi.hProcess == NULL )
    {
        printf( "Process handle invalid. Possibly allready been closed (%d).\n");
        return false;
    }

    // Terminate Process
    //DLLs are not notified when a process is terminated by TerminateProcess. 
    if( !TerminateProcess(m_pi.hProcess,1))
    {
        printf( "ExitProcess failed (%d).\n", GetLastError() );
        return false;
    }

    if (m_pi.hProcess != NULL)
    {
        CloseHandle(m_pi.hProcess);
        m_pi.hProcess = NULL;
    }

    if (m_pi.hThread != NULL)
    {
        CloseHandle(m_pi.hThread);
        m_pi.hProcess = NULL;
    }

    return true;
}

/**************************************************************
*  @brief : CWorkProcess::WaitForEnded
*     
*  @param : 
*     
*    -int nTimeout  ��
*     
*  @return : bool
*     
*  @author : Jimmy
*     
*  @date : 2019/3/9 ������
*     
*  @note : �ȴ����̽���
***************************************************************/
bool CWorkProcess::WaitForEnded(int nTimeout)
{
    if (m_pi.hProcess == NULL)
    {
        return false;
    }

    DWORD dwMilliSeconds = (nTimeout == INFINITE ? INFINITE : nTimeout* 1000);  

    if (WaitForSingleObject(m_pi.hProcess, dwMilliSeconds) != WAIT_OBJECT_0)
    {   
        return   false;
    }

    return true;
}



/**************************************************************
*  @brief : RunSubProcess
*     
*  @param : 
*     
*    -_TCHAR* szCmd
*     
*    -int nTimeout
*     
*  @return : bool
*     
*  @author : Jimmy
*     
*  @date : 2019/3/8 ������
*     
*  @note :  ����һ���ӽ���
***************************************************************/
bool  RunSubProcess(char* szCmd, int nTimeout)
{

    DWORD exitCode = (DWORD)-1;
    bool bRet = false;

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
    if( !CreateProcessA( NULL,   // No module name (use command line)
        szCmd,          // Command line ���� + �ո� + ����
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
        ) 
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
        return false;
    }

    //�ȴ��ʽ�ؽ���
    if (WaitForSingleObject(pi.hProcess, nTimeout * 1000) != WAIT_OBJECT_0 )
    {
        printf( "run cmd timt out...\n");
        bRet = false;
        //ExitProcess(-1);
        //The preferred way to shut down a process is by using the ExitProcess function, 
        //because this function sends notification of approaching termination to all DLLs attached to the process.
        TerminateProcess(pi.hProcess, (DWORD)-1);
    }
    else
    {
        //��ȡ�ӽ��̵��˳�״̬
        GetExitCodeProcess(pi.hProcess, &exitCode);
        if (exitCode == 0)
        {
            printf("exit exe ok\n");
            bRet = true;
        } 
        else
        {
            printf("exit exe error\n");
            bRet = false;
        }
    }
    //�ر�ϵͳ��Դ
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return bRet;
}

//for example:
// CWorkProcess StopSrv("ping", "www.baidu.com");
// 
// 
// if (!StopSrv.LaunchProcess(true))
// {
//     return 0;
// }
// 
// if (!StopSrv.WaitForEnded(1))
// {
//     StopSrv.StopProcess();
// }
