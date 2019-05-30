#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAIN_CLASS_NAME "LLauncher"
#define TRAY_ICON_UID 100
#define WM_NOTIFYICON (WM_USER + 1)
#define IDM_TRAY_SETTING 998
#define IDM_TRAY_QUIT 999
#define TOOLTIP_TEXT "LLauncher\n"\
                     "ver 1.01"
#define PATH_SIZE 256
#define NAME_SIZE 128
#define INI_PATH ".\\LLauncher.ini"

// 関数形式マクロ
#define MALLOC(type, n) ((type *)malloc(sizeof(type) * (n)))

// プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int CreateWnd();
int LoadIniFileInfo();
void CreateMnu();
BOOL AddNotifyIcon(HWND, UINT, HICON, LPSTR);
BOOL DeleteNotifyIcon(HWND, UINT);
int GetIniFileInt(const char *, const char *, int *);
int GetIniFileString(const char *, const char *, char *);
void ShowMenu();

// iniファイル情報構造体
typedef struct _INI_INF {
    char MenuTitle[NAME_SIZE]; // メニューのタイトル
    char FilePath[PATH_SIZE];  // 起動したいファイルパス
} INI_INF;

// グローバル変数
WNDCLASS wc;
MSG msg;
HINSTANCE hIns;
HWND hwnd;
HMENU hpopmenu;
static INI_INF *iniInf;         // iniファイル情報構造体
static int launcher_cnt = 0;    // 起動したいものの数
static unsigned int hotKey = 0; // ホットキー

// メイン関数（一番最初に呼ばれる）
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    int i;
    hIns = hInstance;

    // ウィンドウ作成
    CreateWnd();

    // 設定ファイルの読み込み
    if(LoadIniFileInfo() != 0)
    {
        return -1;
    }
    
    // メニュー作成
    CreateMnu();

    // アイコン格納
    AddNotifyIcon(hwnd, TRAY_ICON_UID, LoadIcon(NULL, IDI_APPLICATION), TOOLTIP_TEXT);

    // ホットキー登録
    RegisterHotKey(hwnd, 0, MOD_CONTROL | MOD_SHIFT, hotKey);

    // メッセージループ
    while(GetMessage(&msg, NULL, 0, 0))
    {
        // メッセージをウィンドウプロシージャにも渡す
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    int retBtn;
    int result;
    int i;

    switch(msg)
    {
        // 起動時
        case WM_CREATE:
            break;
        // アイコンのイベント感知
        case WM_NOTIFYICON:
        switch(wp)
        {
            // デフォルトアイコン
            case TRAY_ICON_UID:
                switch(lp)
                {
                    // アイコンを右クリック時
                    case WM_RBUTTONUP:
                        // メニュー表示
                        ShowMenu();
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        return 0;
    // ホットキー押下時
    case WM_HOTKEY:
        // メニュー表示
        ShowMenu();
        break;
    // メニュー選択時
    case WM_COMMAND:
        switch(LOWORD(wp))
        {
            // 「設定」押下時
            case IDM_TRAY_SETTING:
                // 設定ファイルを起動
                ShellExecute(NULL, NULL, INI_PATH, NULL, NULL, SW_SHOWNORMAL);
                break;
            // 「終了」押下時
            case IDM_TRAY_QUIT:
                DestroyWindow(hwnd);
                return 0;
            // 各メニュータイトル選択時
            default:
                // ファイルパスを元に起動
                ShellExecute(NULL, NULL, (iniInf + wp)->FilePath, NULL, NULL, SW_SHOWNORMAL);
                break;
        }
        return 0;
    // ウィンドウ破棄時
    case WM_DESTROY:
        DeleteNotifyIcon(hwnd, TRAY_ICON_UID);
        DestroyMenu(hpopmenu);
        UnregisterHotKey(hwnd, hotKey);
        free(iniInf);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}

// （見えない）ウィンドウ作成
int CreateWnd()
{
    // メインウィンドウクラスの定義
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hIns;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = MAIN_CLASS_NAME;

    // ウィンドウクラスの登録
    if (!RegisterClass(&wc))
    {
        return -1;
    }

    // ウィンドウ作成関数
    hwnd = CreateWindowEx(WS_EX_TOOLWINDOW,
                          MAIN_CLASS_NAME, MAIN_CLASS_NAME,
                          WS_POPUP | WS_DISABLED,
                          CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                          (HWND)NULL, (HMENU)NULL, hIns, (LPVOID)NULL);

    if(hwnd == NULL)
    {
        return -1;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    return 0;
}

// 設定ファイルの読み込み
int LoadIniFileInfo()
{
    int i, j;
    int ret;
    char sec_buf[10]; // セクション
    char key_buf[256]; // キー
    char hotKey_buf[10];

    memset(hotKey_buf, '\0', sizeof(hotKey_buf));

    // LAUNCHER_COUNT取得
    GetIniFileInt("COMMON", "LAUNCHER_COUNT", &launcher_cnt);

    // HOT_KEY取得
    ret = GetIniFileString("COMMON", "HOT_KEY", hotKey_buf);
    if(ret != 0)
    {
        return(ret);
    }
    hotKey = (UINT)hotKey_buf[0];

    // iniファイル情報構造体　メモリ確保
    iniInf = MALLOC(INI_INF, launcher_cnt);
    if(iniInf == NULL)
    {
        return -1;
    }

    for(i = 0; i < launcher_cnt; i++)
    {
        // セッション名編集
        sprintf(sec_buf, "%d", i);

        // メニュータイトル取得
        ret = GetIniFileString(sec_buf, "MENU_TITLE", (iniInf + i)->MenuTitle);
        if(ret != 0)
        {
            return(ret);
        }

        // ファイルパス取得
        ret = GetIniFileString(sec_buf, "FILE_PATH", (iniInf + i)->FilePath);
        if(ret != 0)
        {
            return(ret);
        }
    }

    return 0;
}

// メニューの作成
void CreateMnu()
{
    int i;
    char buf[256];

    memset(buf, '\0', sizeof(buf));

    // メニュー作成
    hpopmenu = CreatePopupMenu();

    for(i = 0; i < launcher_cnt; i++) 
    {
        // メニュータイトルの後ろにショートカットキーを付与（A～Z）
        sprintf(buf, "%s(&%c)", (iniInf + i)->MenuTitle, 65 + i);

        // 設定ファイルを元にメニューを作成
        AppendMenu(hpopmenu, MF_POPUP | MF_STRING, i, buf);
    }

    // 「設定」「終了」メニューを作成
    AppendMenu(hpopmenu, MFT_SEPARATOR, 0, NULL);
    AppendMenu(hpopmenu, MF_POPUP | MF_STRING, IDM_TRAY_SETTING, "設定(&0)");
    AppendMenu(hpopmenu, MF_POPUP | MF_STRING, IDM_TRAY_QUIT, "終了(&1)");

    // メニューを設定
    SetMenu(hwnd, hpopmenu);
}

// アイコンの追加
BOOL AddNotifyIcon(HWND hWnd, UINT uID, HICON hIcon, LPSTR lpszTip)
{
    NOTIFYICONDATA nid;

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = uID;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_NOTIFYICON;
    nid.hIcon = hIcon;
    if(lpszTip == NULL)
    {
        nid.szTip[0] = '\0';
    }
    else
    {
        strncpy(nid.szTip, lpszTip, sizeof(nid.szTip));
    }

    return Shell_NotifyIcon(NIM_ADD, &nid);
}

// アイコンの削除
BOOL DeleteNotifyIcon(HWND hWnd, UINT uID)
{
    NOTIFYICONDATA nid;

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = uID;

    return Shell_NotifyIcon(NIM_DELETE, &nid);
}

// iniファイルの情報取得（数値）
// SECTIONとKEYを元に数値を取得
int GetIniFileInt(const char *sec, const char *key, int *ret)
{
    *ret = GetPrivateProfileInt(sec, key, 0, INI_PATH);

    return(*ret);
}

// iniファイルの情報取得（文字列）
// SECTIONとKEYを元に文字列を取得
int GetIniFileString(const char *sec, const char *key, char *ret)
{
    char buff[128];

    memset(buff, '\0', sizeof(buff));
    GetPrivateProfileString(sec, key, "default", buff, sizeof(buff), INI_PATH);
    if(strcmp(buff, "default") == 0)
    {
        return -1;
    }
    strcpy(ret, buff);

    return 0;
}

// メニューの表示
void ShowMenu()
{
    POINT pt;

    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);
    TrackPopupMenu(hpopmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);
}