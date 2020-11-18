#include <windows.h>
#include <iostream>
#include <shellapi.h>
#include <string>

#define CAPI extern "C" __declspec(dllexport)

struct tray_menu
{
    char *text;
};

/*句柄, 图标位置, 菜单结构体 */
CAPI void *tray_create(HWND hwnd, LPCSTR icon_path);
typedef void *mtray;
#define WM_TRAY_CALLBACK_MESSAGE 11685

/* 消息处理  */
LRESULT CALLBACK _tray_wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class tray
{
private:
    /* data */
    int hIcon;
    HWND hwnd;
    NOTIFYICONDATA notify_data;
    LONG cbProc;
    struct tray_menu *m_menu;
    void (* traycb)(int id);

public:
    tray(/* args */);
    bool create(HWND hwnd, LPCSTR path, struct tray_menu *arg_menu, void (*cb)(int))
    {
        if (IsWindow(hwnd) == TRUE)
        {
            this->hwnd = hwnd;
            HICON icon;
            ExtractIconExA(path, 0, NULL, &icon, 1);
            // NIF_MESSAGE 必填, 很重要
            this->notify_data = {
                .cbSize = 488,
                .hWnd = hwnd,
                .uID = 1,
                .uFlags = NIF_ICON | NIF_TIP | NIF_INFO | NIF_MESSAGE,
                .uCallbackMessage = WM_TRAY_CALLBACK_MESSAGE,
                .hIcon = icon,
                .uTimeout = 10000};
            lstrcpy(this->notify_data.szTip, "test");
            Shell_NotifyIcon(NIM_ADD, &this->notify_data);
            this->cbProc = SetWindowLongA(hwnd, -4, (LONG)&_tray_wnd_proc);
            SetPropA(hwnd, "CallbackMessage", (HANDLE)WM_TRAY_CALLBACK_MESSAGE);
            SetPropA(hwnd, "WinProc", (HANDLE)this->cbProc);
            SetPropA(hwnd, "TrayClass", (HANDLE)this);
            SetPropA(hwnd, "TrayCb",  (HANDLE)cb);

            /* 绑定 */
            this->m_menu = arg_menu;
            this->traycb = cb;

            return true;
        }
        else
        {
            std::cout << "创建失败" << std::endl;
            return false;
        }
    }

    void pop_menu()
    {
        HMENU hmenu = CreateMenu();
        // 循环加入菜单
        for (int id = 0; this->m_menu[id].text != NULL; id++)
        {
            AppendMenuA(hmenu, 0, id, m_menu[id].text);
        }

        HMENU mainHMenu = CreateMenu();
        AppendMenuA(mainHMenu, 16, (UINT_PTR)hmenu, "托盘菜单");

        POINT p;
        GetCursorPos(&p);
        WORD cmd = TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                                  p.x, p.y, 0, this->hwnd, NULL);
        SendMessage(hwnd, WM_COMMAND, cmd, 0);
    }

    ~tray();
};

tray::tray(/* args */)
{
    RegisterWindowMessageA("TaskbarCreated");
}

tray::~tray()
{
}

LRESULT CALLBACK _tray_wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    WNDPROC cbProc = (WNDPROC)GetPropA(hWnd, "WinProc");
    int trayMsg = (int)GetPropA(hWnd, "CallbackMessage");
    tray *trayCls = static_cast<tray *>(GetPropA(hWnd, "TrayClass"));
    void(* traycb)(int id) = (void(* )(int id))GetPropA(hWnd, "TrayCb");
    
    switch (uMsg)
    {
    case WM_TRAY_CALLBACK_MESSAGE:
        // MessageBoxA(hWnd, "123", std::to_string(trayMsg).c_str(), 0);
        if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
        {
            trayCls->pop_menu();
            return 0;
        }
        return 0;
        break;
    case WM_COMMAND:
        if (uMsg == 273 && lParam == 0)
        {
            MessageBoxA(hWnd, "被点击", std::to_string(wParam).c_str(), 0);
            traycb(int(wParam));
        }
        break;
    default:
        break;
    }

    return CallWindowProcA(cbProc, hWnd, uMsg, wParam, lParam);
}

inline void aaaa(int id){
    MessageBoxA(NULL, std::to_string(id).c_str(), "回调函数", 0);
}

CAPI mtray tray_create(HWND hwnd, LPCSTR icon_path)
{
    static struct tray_menu m_menu[] = {
        {.text = "menu1"},
        {.text = "menu2"},
        {.text = NULL}};

    mtray mtray_ptr = new tray();
    bool result = static_cast<tray *>(mtray_ptr)->create(hwnd, icon_path, m_menu, aaaa);
    if (result)
    {
        return mtray_ptr;
    }
    else
    {
        return NULL;
    }
}
