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

// �֐��`���}�N��
#define MALLOC(type, n) ((type *)malloc(sizeof(type) * (n)))

// �v���g�^�C�v�錾
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int CreateWnd();
int LoadIniFileInfo();
void CreateMnu();
BOOL AddNotifyIcon(HWND, UINT, HICON, LPSTR);
BOOL DeleteNotifyIcon(HWND, UINT);
int GetIniFileInt(const char *, const char *, int *);
int GetIniFileString(const char *, const char *, char *);
void ShowMenu();

// ini�t�@�C�����\����
typedef struct _INI_INF {
    char MenuTitle[NAME_SIZE]; // ���j���[�̃^�C�g��
    char FilePath[PATH_SIZE];  // �N���������t�@�C���p�X
} INI_INF;

// �O���[�o���ϐ�
WNDCLASS wc;
MSG msg;
HINSTANCE hIns;
HWND hwnd;
HMENU hpopmenu;
static INI_INF *iniInf;         // ini�t�@�C�����\����
static int launcher_cnt = 0;    // �N�����������̂̐�
static unsigned int hotKey = 0; // �z�b�g�L�[

// ���C���֐��i��ԍŏ��ɌĂ΂��j
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    int i;
    hIns = hInstance;

    // �E�B���h�E�쐬
    CreateWnd();

    // �ݒ�t�@�C���̓ǂݍ���
    if(LoadIniFileInfo() != 0)
    {
        return -1;
    }
    
    // ���j���[�쐬
    CreateMnu();

    // �A�C�R���i�[
    AddNotifyIcon(hwnd, TRAY_ICON_UID, LoadIcon(NULL, IDI_APPLICATION), TOOLTIP_TEXT);

    // �z�b�g�L�[�o�^
    RegisterHotKey(hwnd, 0, MOD_CONTROL | MOD_SHIFT, hotKey);

    // ���b�Z�[�W���[�v
    while(GetMessage(&msg, NULL, 0, 0))
    {
        // ���b�Z�[�W���E�B���h�E�v���V�[�W���ɂ��n��
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    int retBtn;
    int result;
    int i;

    switch(msg)
    {
        // �N����
        case WM_CREATE:
            break;
        // �A�C�R���̃C�x���g���m
        case WM_NOTIFYICON:
        switch(wp)
        {
            // �f�t�H���g�A�C�R��
            case TRAY_ICON_UID:
                switch(lp)
                {
                    // �A�C�R�����E�N���b�N��
                    case WM_RBUTTONUP:
                        // ���j���[�\��
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
    // �z�b�g�L�[������
    case WM_HOTKEY:
        // ���j���[�\��
        ShowMenu();
        break;
    // ���j���[�I����
    case WM_COMMAND:
        switch(LOWORD(wp))
        {
            // �u�ݒ�v������
            case IDM_TRAY_SETTING:
                // �ݒ�t�@�C�����N��
                ShellExecute(NULL, NULL, INI_PATH, NULL, NULL, SW_SHOWNORMAL);
                break;
            // �u�I���v������
            case IDM_TRAY_QUIT:
                DestroyWindow(hwnd);
                return 0;
            // �e���j���[�^�C�g���I����
            default:
                // �t�@�C���p�X�����ɋN��
                ShellExecute(NULL, NULL, (iniInf + wp)->FilePath, NULL, NULL, SW_SHOWNORMAL);
                break;
        }
        return 0;
    // �E�B���h�E�j����
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

// �i�����Ȃ��j�E�B���h�E�쐬
int CreateWnd()
{
    // ���C���E�B���h�E�N���X�̒�`
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

    // �E�B���h�E�N���X�̓o�^
    if (!RegisterClass(&wc))
    {
        return -1;
    }

    // �E�B���h�E�쐬�֐�
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

// �ݒ�t�@�C���̓ǂݍ���
int LoadIniFileInfo()
{
    int i, j;
    int ret;
    char sec_buf[10]; // �Z�N�V����
    char key_buf[256]; // �L�[
    char hotKey_buf[10];

    memset(hotKey_buf, '\0', sizeof(hotKey_buf));

    // LAUNCHER_COUNT�擾
    GetIniFileInt("COMMON", "LAUNCHER_COUNT", &launcher_cnt);

    // HOT_KEY�擾
    ret = GetIniFileString("COMMON", "HOT_KEY", hotKey_buf);
    if(ret != 0)
    {
        return(ret);
    }
    hotKey = (UINT)hotKey_buf[0];

    // ini�t�@�C�����\���́@�������m��
    iniInf = MALLOC(INI_INF, launcher_cnt);
    if(iniInf == NULL)
    {
        return -1;
    }

    for(i = 0; i < launcher_cnt; i++)
    {
        // �Z�b�V�������ҏW
        sprintf(sec_buf, "%d", i);

        // ���j���[�^�C�g���擾
        ret = GetIniFileString(sec_buf, "MENU_TITLE", (iniInf + i)->MenuTitle);
        if(ret != 0)
        {
            return(ret);
        }

        // �t�@�C���p�X�擾
        ret = GetIniFileString(sec_buf, "FILE_PATH", (iniInf + i)->FilePath);
        if(ret != 0)
        {
            return(ret);
        }
    }

    return 0;
}

// ���j���[�̍쐬
void CreateMnu()
{
    int i;
    char buf[256];

    memset(buf, '\0', sizeof(buf));

    // ���j���[�쐬
    hpopmenu = CreatePopupMenu();

    for(i = 0; i < launcher_cnt; i++) 
    {
        // ���j���[�^�C�g���̌��ɃV���[�g�J�b�g�L�[��t�^�iA�`Z�j
        sprintf(buf, "%s(&%c)", (iniInf + i)->MenuTitle, 65 + i);

        // �ݒ�t�@�C�������Ƀ��j���[���쐬
        AppendMenu(hpopmenu, MF_POPUP | MF_STRING, i, buf);
    }

    // �u�ݒ�v�u�I���v���j���[���쐬
    AppendMenu(hpopmenu, MFT_SEPARATOR, 0, NULL);
    AppendMenu(hpopmenu, MF_POPUP | MF_STRING, IDM_TRAY_SETTING, "�ݒ�(&0)");
    AppendMenu(hpopmenu, MF_POPUP | MF_STRING, IDM_TRAY_QUIT, "�I��(&1)");

    // ���j���[��ݒ�
    SetMenu(hwnd, hpopmenu);
}

// �A�C�R���̒ǉ�
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

// �A�C�R���̍폜
BOOL DeleteNotifyIcon(HWND hWnd, UINT uID)
{
    NOTIFYICONDATA nid;

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = uID;

    return Shell_NotifyIcon(NIM_DELETE, &nid);
}

// ini�t�@�C���̏��擾�i���l�j
// SECTION��KEY�����ɐ��l���擾
int GetIniFileInt(const char *sec, const char *key, int *ret)
{
    *ret = GetPrivateProfileInt(sec, key, 0, INI_PATH);

    return(*ret);
}

// ini�t�@�C���̏��擾�i������j
// SECTION��KEY�����ɕ�������擾
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

// ���j���[�̕\��
void ShowMenu()
{
    POINT pt;

    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);
    TrackPopupMenu(hpopmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);
}