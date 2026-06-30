#include "uiaccess.h"
#include <tlhelp32.h>
#include <tchar.h>

// 遍历进程列表，查找同一 Session 下的 winlogon.exe，
// 复制其令牌（具有 SeTcbPrivilege），用于后续提权操作
DWORD UIAccess::duplicateWinlogonToken(DWORD dwSessionId, DWORD dwDesiredAccess, PHANDLE phToken) 
{
	DWORD dwErr;
	PRIVILEGE_SET ps;

	ps.PrivilegeCount = 1;
	ps.Control = PRIVILEGE_SET_ALL_NECESSARY;

	if (LookupPrivilegeValue(NULL, SE_TCB_NAME, &ps.Privilege[0].Luid)) 
	{
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (INVALID_HANDLE_VALUE != hSnapshot) 
		{
			BOOL bCont, bFound = FALSE;
			PROCESSENTRY32 pe;

			pe.dwSize = sizeof(pe);
			dwErr = ERROR_NOT_FOUND;

			for (bCont = Process32First(hSnapshot, &pe); bCont; bCont = Process32Next(hSnapshot, &pe)) 
			{
				HANDLE hProcess;

				if (0 != _tcsicmp(pe.szExeFile, TEXT("winlogon.exe"))) 
				{
					continue;
				}

				hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
				if (hProcess) 
				{
					HANDLE hToken;
					DWORD dwRetLen, sid;

					if (OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_DUPLICATE, &hToken)) 
					{
						BOOL fTcb;

						if (PrivilegeCheck(hToken, &ps, &fTcb) && fTcb) 
						{
							if (GetTokenInformation(hToken, TokenSessionId, &sid, sizeof(sid), &dwRetLen) && sid == dwSessionId) 
							{
								bFound = TRUE;
								if (DuplicateTokenEx(hToken, dwDesiredAccess, NULL, SecurityImpersonation, TokenImpersonation, phToken)) 
								{
									dwErr = ERROR_SUCCESS;
								}
								else 
								{
									dwErr = GetLastError();
								}
							}
						}
						CloseHandle(hToken);
					}
					CloseHandle(hProcess);
				}

				if (bFound) break;
			}

			CloseHandle(hSnapshot);
		}
		else 
		{
			dwErr = GetLastError();
		}
	}
	else 
	{
		dwErr = GetLastError();
	}

	return dwErr;
}

// 借用 winlogon 的 System 令牌模拟当前线程，
// 然后复制自身令牌并设置 TokenUIAccess 属性
DWORD UIAccess::createUIAccessToken(PHANDLE phToken) 
{
	DWORD dwErr;
	HANDLE hTokenSelf;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &hTokenSelf)) {
		DWORD dwSessionId, dwRetLen;

		if (GetTokenInformation(hTokenSelf, TokenSessionId, &dwSessionId, sizeof(dwSessionId), &dwRetLen)) 
		{
			HANDLE hTokenSystem;

			dwErr = duplicateWinlogonToken(dwSessionId, TOKEN_IMPERSONATE, &hTokenSystem);
			if (ERROR_SUCCESS == dwErr) 
			{
				if (SetThreadToken(NULL, hTokenSystem)) 
				{
					if (DuplicateTokenEx(hTokenSelf, TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_DEFAULT, NULL, SecurityAnonymous, TokenPrimary, phToken)) 
					{
						BOOL bUIAccess = TRUE;

						if (!SetTokenInformation(*phToken, TokenUIAccess, &bUIAccess, sizeof(bUIAccess))) 
						{
							dwErr = GetLastError();
							CloseHandle(*phToken);
						}
					}
					else {
						dwErr = GetLastError();
					}
					RevertToSelf();
				}
				else 
				{
					dwErr = GetLastError();
				}
				CloseHandle(hTokenSystem);
			}
		}
		else 
		{
			dwErr = GetLastError();
		}

		CloseHandle(hTokenSelf);
	}
	else 
	{
		dwErr = GetLastError();
	}

	return dwErr;
}

// 检查当前进程令牌是否已经具有 UIAccess 权限
BOOL UIAccess::hasUIAccess(DWORD* pdwErr, DWORD* pfUIAccess) 
{
	HANDLE hToken;
	DWORD dwErr = ERROR_SUCCESS;
	DWORD fUIAccess = 0;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) 
	{
		DWORD dwRetLen;

		if (!GetTokenInformation(hToken, TokenUIAccess, &fUIAccess, sizeof(fUIAccess), &dwRetLen)) 
		{
			dwErr = GetLastError();
		}
		CloseHandle(hToken);
	}
	else 
	{
		dwErr = GetLastError();
	}

	if (pdwErr) *pdwErr = dwErr;
	if (pfUIAccess) *pfUIAccess = fUIAccess;

	return dwErr == ERROR_SUCCESS;
}

// 公开接口：检测并获取 UIAccess 权限
// 若当前进程尚无 UIAccess，则创建带 TokenUIAccess 的令牌并重启自身
DWORD UIAccess::prepare() 
{
	DWORD dwErr;
	DWORD fUIAccess;

	if (hasUIAccess(&dwErr, &fUIAccess)) 
	{
		if (fUIAccess) 
		{
			dwErr = ERROR_SUCCESS;
		}
		else {
			HANDLE hTokenUIAccess;
			dwErr = createUIAccessToken(&hTokenUIAccess);
			if (ERROR_SUCCESS == dwErr) 
			{
				STARTUPINFO si;
				PROCESS_INFORMATION pi;

				GetStartupInfo(&si);
				if (CreateProcessAsUser(hTokenUIAccess, NULL, GetCommandLine(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) 
				{
					CloseHandle(pi.hProcess), CloseHandle(pi.hThread);
					ExitProcess(0);
				}
				else 
				{
					dwErr = GetLastError();
				}

				CloseHandle(hTokenUIAccess);
			}
		}
	}

	return dwErr;
}
