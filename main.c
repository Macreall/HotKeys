#define UNICODE
#include <windows.h>
#include <shellapi.h>
#include <psapi.h>
#include "resource.h"
#include <stdio.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_SETTINGS  1002
#define ID_TRAY_ABOUT     1003

HWND hSettingsWnd = NULL;
HINSTANCE g_hInstance = NULL;
HWND hTab = NULL;
HWND hPagePC = NULL;
HWND hPageApps = NULL;

void OpenSettingsWindow(HINSTANCE hInstance, HWND hwndParent);



NOTIFYICONDATA nid = {};

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    HWND hList = (HWND)lParam;
    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);



    if (IsWindowVisible(hwnd)) {
        TCHAR title[256];
        GetWindowText(hwnd, title, sizeof(title)/sizeof(TCHAR));
        if (title[0] != 0) {
            SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)title);
        }
    }

    return TRUE;
}

void ListRunningApps() {
    EnumWindows(EnumWindowsProc, 0);
}

LRESULT CALLBACK SettingsWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            hSettingsWnd = NULL;
            break;
        case WM_NOTIFY:
            if (((LPNMHDR)lParam)->code == TCN_SELCHANGE) {
                int iPage = TabCtrl_GetCurSel(hTab);
                
                if (iPage == 0) { 
                    ShowWindow(hPagePC, SW_SHOW);
                    ShowWindow(hPageApps, SW_HIDE);
                } else { 
                    ShowWindow(hPagePC, SW_HIDE);
                    ShowWindow(hPageApps, SW_SHOW);
                }
            }
            break;
        case WM_COMMAND:

            // Add a function here that gets all of the parameters for the app
            // we want things like you can add hot key shortcuts to global space or app specific
            // we also want a visualization of all current bound keys per app
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        case WM_TRAYICON:
            if (lParam == WM_LBUTTONDOWN) {
                HMENU menu = CreatePopupMenu();

                AppendMenu(menu, MF_STRING, ID_TRAY_SETTINGS, L"Settings");
                AppendMenu(menu, MF_STRING, ID_TRAY_ABOUT, L"About");
                AppendMenu(menu, MF_SEPARATOR, 0, NULL);
                AppendMenu(menu, MF_STRING, ID_TRAY_EXIT, L"Exit");

                POINT p;
                GetCursorPos(&p);
                SetForegroundWindow(hwnd);
                TrackPopupMenu(menu, TPM_RIGHTBUTTON, p.x, p.y, 0, hwnd, NULL);
                DestroyMenu(menu);
            }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
            case ID_TRAY_EXIT:
                    DestroyWindow(hwnd);
                    break;
            case ID_TRAY_SETTINGS:
                    OpenSettingsWindow(g_hInstance, hwnd);
                    break;
            case ID_TRAY_ABOUT:
                    MessageBox(hwnd, L"TrayApp v1.0", L"About", MB_OK);
                    break;
            }
            break;


        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            DestroyIcon(nid.hIcon);
            hSettingsWnd = NULL;
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);

    }
    return 0;
}

void OpenSettingsWindow(HINSTANCE hInstance, HWND hwndParent) {
    if (hSettingsWnd != NULL) {

        SetForegroundWindow(hSettingsWnd);
        return;
    }

    const wchar_t CLASS_NAME[] = L"SettingsWindowClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = SettingsWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    hSettingsWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Settings",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        400, 400, 700, 750,
        hwndParent,
        NULL,
        hInstance,
        NULL
    );

    HWND hTab = CreateWindowEx(
    0,
    WC_TABCONTROL,
    NULL,
    WS_CHILD | WS_VISIBLE,
    5, 5, 300, 200,
    hSettingsWnd,
    (HMENU)4001,
    hInstance,
    NULL
);

//     HWND hList = CreateWindowEx(
//     0,
//     L"LISTBOX",
//     NULL,
//     WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_STANDARD,
//     10, 10, 520, 450,
//     hSettingsWnd,
//     (HMENU)3001,
//     hInstance,
//     NULL
// );

    RECT rcClient;
    GetClientRect(hTab, &rcClient);

    HWND hPagePC = CreateWindowEx(
        0,
        L"STATIC",
        L"PC Info Here",
        WS_CHILD | WS_VISIBLE,
        rcClient.left + 20, rcClient.top + 20, rcClient.right, rcClient.bottom,
        hSettingsWnd,
        NULL,
        hInstance,
        NULL
    );

    // HWND hPageApps = CreateWindowEx(
    //     0,
    //     L"LISTBOX",
    //     NULL,
    //     WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_STANDARD,
    //     rcClient.left, rcClient.top, rcClient.right, rcClient.bottom,
    //     hSettingsWnd,
    //     (HMENU)3001, 
    //     hInstance,
    //     NULL
    // );




    TCITEM tie;
    tie.mask = TCIF_TEXT;

    tie.pszText = L"PC";
    TabCtrl_InsertItem(hTab, 0, &tie);

    tie.pszText = L"Current Apps";
    TabCtrl_InsertItem(hTab, 1, &tie);




    // SendMessage(hList, LB_RESETCONTENT, 0, 0);
    EnumWindows(EnumWindowsProc, (LPARAM)hPageApps);
    if (hSettingsWnd) {
        ShowWindow(hSettingsWnd, SW_SHOW);



        UpdateWindow(hSettingsWnd);
    }
}


int WINAPI WinMain(

    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
) {
  

    const wchar_t CLASS_NAME[] = L"TrayAppClass";
    g_hInstance = hInstance;



    HICON hIcon = (HICON)LoadImage(
    hInstance,
    MAKEINTRESOURCE(IDI_TRAY),
    IMAGE_ICON,
    32,
    32,
    LR_DEFAULTCOLOR
);

    if (!hIcon) {
        MessageBox(NULL, L"Icon has failed to load", L"Error", MB_OK);
    }



    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TrayAppClass";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
    0,
    CLASS_NAME,
    L"LISTBOX",

    WS_OVERLAPPEDWINDOW,
    0, 0, 0, 0,
    NULL,
    NULL,
    hInstance,
    NULL
);

    HWND hSettingsWnd = CreateWindowEx(
    0,
    L"STATIC",
    L"My Small Window",
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
    100, 100, 300, 150,
    hwnd,
    NULL,
    hInstance,
    NULL
    );


    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = hIcon;
    wcscpy_s(nid.szTip, sizeof(nid.szTip), L"Your Tooltip Text Here");

    Shell_NotifyIcon(NIM_ADD, &nid);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
