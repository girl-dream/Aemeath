#include "TrayIcon.h"
#include "resource.h"
#include "Config.h"


extern AppConfig g_config; 
extern int g_scaleIndex;
extern int g_transparencyIndex;
extern int g_petIdleIndex;

void TrayIcon::Init(HINSTANCE hInst, HWND Hwnd_)
{
    Hwnd = Hwnd_;

    nid.cbSize = sizeof(nid);
    nid.hWnd = Hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APPICON));
    lstrcpy(nid.szTip, "飞行雪绒"); 

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void TrayIcon::ShowMenu()
{
    POINT pt;
    GetCursorPos(&pt);

    HMENU menu = CreatePopupMenu();

    // 子菜单：缩放
    HMENU scaleMenu = CreatePopupMenu();
    const wchar_t* scaleText[] = { L"30%",L"50%",L"70%",L"90%",L"110%",L"130%",L"150%",L"170%",L"190%" };
    for (int i = 0; i < 9; i++)
        AppendMenuW(scaleMenu,
            (i == g_scaleIndex ? MF_CHECKED : MF_UNCHECKED),
            2000 + i,
            scaleText[i]);

    // 子菜单：透明度
    HMENU alphaMenu = CreatePopupMenu();
    PCSTR alphaText[] = { "100%","90%","80%","70%","60%","50%","40%","30%" };
    for (int i = 0; i < 8; i++)
        AppendMenu(alphaMenu,
            (i == g_transparencyIndex ? MF_CHECKED : MF_UNCHECKED),
            2100 + i,
            alphaText[i]);
    // 子菜单：选择静止动画
    HMENU gifMenu = CreatePopupMenu();
    PCSTR gifText[] = { "1","2","3","4","随机" };
    for (int i = 0; i < 5; i++)
        AppendMenu(gifMenu,
            (i == g_petIdleIndex ? MF_CHECKED : MF_UNCHECKED),
            2400 + i,
            gifText[i]);

    AppendMenu(menu, MF_POPUP, (UINT_PTR)scaleMenu, "缩放");
    AppendMenu(menu, MF_POPUP, (UINT_PTR)alphaMenu, "透明度");
    AppendMenu(menu, MF_POPUP, (UINT_PTR)gifMenu, "静止动画");

    AppendMenu(menu, MF_SEPARATOR, 0, nullptr);

    AppendMenu(menu,
        g_config.followMouse ? MF_CHECKED : MF_UNCHECKED,
        2200, "跟随鼠标");

    AppendMenu(menu,
        g_config.clickThrough ? MF_CHECKED : MF_UNCHECKED,
        2201, "鼠标穿透");
    AppendMenu(menu,
        g_config.defaultState ? MF_CHECKED : MF_UNCHECKED,
        2202, "默认静止");

    AppendMenu(menu, MF_SEPARATOR, 0, nullptr);

    AppendMenu(menu, MF_STRING, 2300, "静止/飞行");
    AppendMenu(menu, MF_STRING, 2301, "退出");

    SetForegroundWindow(Hwnd);

    int cmd = TrackPopupMenu(
        menu,
        TPM_RETURNCMD | TPM_NONOTIFY,
        pt.x, pt.y,
        0, Hwnd, nullptr);

    DestroyMenu(menu);

    if (cmd != 0)
        PostMessage(Hwnd, WM_COMMAND, cmd, 0);
}

