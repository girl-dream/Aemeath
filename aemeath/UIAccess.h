#pragma once
#include <windows.h>

class UIAccess
{
public:
	/// <summary>
	/// 检查当前进程令牌是否具有UIAccess权限。
	/// </summary>
	/// <param name="pdwErr">可选参数，用于接收Win32错误码。</param>
	/// <param name="pfUIAccess">可选参数，用于接收UIAccess权限状态（TRUE表示具有权限）。</param>
	/// <returns>返回TRUE表示查询成功，pfUIAccess输出有效状态；FALSE表示查询失败。</returns>
	BOOL hasUIAccess(DWORD* pdwErr = nullptr, DWORD* pfUIAccess = nullptr);

	/// <summary>
	/// 主入口函数：检测并获取UIAccess权限。
	/// 若当前进程尚无UIAccess权限，则创建带有TokenUIAccess属性的令牌并重启自身。
	/// </summary>
	/// <returns>返回Win32错误码，ERROR_SUCCESS表示成功，其他值表示失败原因。</returns>
	DWORD prepare();

private:
	/// <summary>
	/// 遍历进程列表，查找同一Session下的winlogon.exe进程，
	/// 复制其令牌（具有SeTcbPrivilege权限），用于后续提权操作。
	/// </summary>
	/// <param name="dwSessionId">会话ID，用于定位同一会话中的winlogon.exe进程。</param>
	/// <param name="dwDesiredAccess">请求的访问权限，用于打开令牌。</param>
	/// <param name="phToken">输出参数，接收复制的令牌句柄。</param>
	/// <returns>返回Win32错误码，ERROR_SUCCESS表示成功。</returns>
	DWORD duplicateWinlogonToken(DWORD dwSessionId, DWORD dwDesiredAccess, PHANDLE phToken);

	/// <summary>
	/// 借用winlogon的System令牌模拟当前线程，
	/// 然后复制自身令牌并设置TokenUIAccess属性。
	/// </summary>
	/// <param name="phToken">输出参数，接收新创建的具有UIAccess属性的令牌句柄。</param>
	/// <returns>返回Win32错误码，ERROR_SUCCESS表示成功。</returns>
	DWORD createUIAccessToken(PHANDLE phToken);
};