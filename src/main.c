#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include "input.h"
#include "commands.h"
#include "manager.h"

// receive leader key presses
HHOOK keyboardHook;
KeyCode keyLeader = KEY_SUPER_LEFT;
InputBuffer hookBuf = {0};
// receive other input
HWND windowManagerHwnd = NULL;
InputBuffer inputBuf = {0};

typedef KeyCode KeyMap[COMMAND_ENUM_COUNT];
typedef unsigned char byte;

static KeyCode MapVk(byte vk) {
    switch (vk) {
        case '0': return KEY_0;
        case '1': return KEY_1;
        case '2': return KEY_2;
        case '3': return KEY_3;
        case '4': return KEY_4;
        case '5': return KEY_5;
        case '6': return KEY_6;
        case '7': return KEY_7;
        case '8': return KEY_8;
        case '9': return KEY_9;
        case 'Q': return KEY_Q;
        case 'W': return KEY_W;
        case 'H': return KEY_H;
        case 'J': return KEY_J;
        case 'K': return KEY_K;
        case 'L': return KEY_L;
        case 'S': return KEY_S;
        case 'V': return KEY_V;
        case VK_LWIN: return KEY_SUPER_LEFT;
        default: return KEY_NONE;
    }
}

static void ForceWindowIntoFocus(HWND hwnd) {
    HWND fg = GetForegroundWindow();
    DWORD fgThread = GetWindowThreadProcessId(fg, NULL);
    DWORD thisThread = GetCurrentThreadId();

    AttachThreadInput(thisThread, fgThread, TRUE);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    AttachThreadInput(thisThread, fgThread, FALSE);
}

static void FocusWindowManager() {
    ForceWindowIntoFocus(windowManagerHwnd);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0) {
        return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
    }

    KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
    KeyCode key = MapVk(pKeyboard->vkCode);

    if (HAS_BITS(keyLeader, key)) {
        if (wParam == WM_KEYDOWN) {
            SetKeyDown(key, &hookBuf);
        } else if (wParam == WM_KEYUP) {
            SetKeyUp(keyLeader, &hookBuf);
        }
    }

    if (IsKeyPressed(keyLeader, &hookBuf)) {
        ResetInputBuffer(&hookBuf);
        FocusWindowManager();
        return 1;
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

static void SetKeyboardHook() {
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (keyboardHook == NULL) {
        exit(1);
    }
}

static void RemoveKeyboardHook() {
    if (keyboardHook != NULL) {
        UnhookWindowsHookEx(keyboardHook);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_KEYDOWN: {
            KeyCode key = MapVk(wParam);
            SetKeyDown(key, &inputBuf);
            break;
        }
        case WM_KEYUP: {
            KeyCode key = MapVk(wParam);
            SetKeyUp(key, &inputBuf);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

HWND CreateWindowManagerWindow() {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "manager-of-windows-class";

    if (!RegisterClass(&wc)) {
        return NULL;
    }

    HWND hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        "manager-of-windows",
        0,
        0, 0, 0, 0,
        NULL,
        NULL,
        wc.hInstance,
        NULL
    );

    if (!hwnd) {
        return NULL;
    }

    ShowWindow(hwnd, SW_HIDE);
    return hwnd;
}

static inline bool IsCommandRequested(CommandType cmdType, InputBuffer* buf, KeyCode* keyMap) {
    return IsKeyPressed(keyMap[cmdType], buf);
}

static bool HandleSelectWorkspaceMode(InputBuffer* buf, KeyMap keyMap, CommandModeType* mode) {
    bool didConsumeKey = false;

    int workspace = -1;
    for (int i = 0; i <= 9; i++) {
        CommandType cmd = COMMAND_SELECT_WORKSPACE_0 + i;
        if (IsCommandRequested(cmd, buf, keyMap)) {
            workspace = i;
            SelectWorkspace(workspace);
            didConsumeKey = true;
            break;
        }
    }

    if (IsCommandRequested(COMMAND_MODE_SELECT_WINDOW, buf, keyMap)) {
        printf("select window mode\n");
        *mode = MODE_SELECT_WINDOW;
        didConsumeKey = true;
    } else if (IsCommandRequested(COMMAND_SPLIT_HORIZONTAL, buf, keyMap)) {
        printf("split horizontal\n");
        didConsumeKey = true;
    } else if (IsCommandRequested(COMMAND_SPLIT_VERTICAL, buf, keyMap)) {
        printf("split vertical\n");
        didConsumeKey = true;
    }


    return didConsumeKey;
}

static bool HandleSelectWindowMode(InputBuffer* buf, KeyMap keyMap, CommandModeType* mode) {
    bool didConsumeKey = false;
    if (IsCommandRequested(COMMAND_SELECT_WINDOW_LEFT, buf, keyMap)) {
        printf("window left\n");
        didConsumeKey = true;
        *mode = MODE_SELECT_WORKSPACE;
    } else if (IsCommandRequested(COMMAND_SELECT_WINDOW_RIGHT, buf, keyMap)) {
        printf("window right\n");
        didConsumeKey = true;
        *mode = MODE_SELECT_WORKSPACE;
    } else if (IsCommandRequested(COMMAND_SELECT_WINDOW_UP, buf, keyMap)) {
        printf("window up\n");
        didConsumeKey = true;
        *mode = MODE_SELECT_WORKSPACE;
    } else if (IsCommandRequested(COMMAND_SELECT_WINDOW_DOWN, buf, keyMap)) {
        printf("window down\n");
        didConsumeKey = true;
        *mode = MODE_SELECT_WORKSPACE;
    }

    return didConsumeKey;
}

bool IsActualWindow(HWND hwnd) {
    if (!IsWindowVisible(hwnd)) {
        return false;
    }

    int length = GetWindowTextLength(hwnd);
    if (length == 0) {
        return false;
    }

    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TOOLWINDOW) {
        return false;
    }

    return true;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    if (!IsActualWindow(hwnd)) {
        return TRUE;
    }

    AddWindow(hwnd);
    return TRUE;
}

static void ShowWindowWin32(void* handle) {
    ShowWindow(handle, SW_SHOW);
}

static void HideWindowWin32(void* handle) {
    ShowWindow(handle, SW_HIDE);
}

static void MoveResizeWindowWin32(void* handle, int x, int y, int width, int height) {
    MoveWindow(handle, x, y, width, height, TRUE);
}

int main() {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    InitWindowManager(screenWidth, screenHeight);

    WindowPlatform windowPlatform = {
        .ShowWindow = &ShowWindowWin32,
        .HideWindow = &HideWindowWin32,
        .MoveResizeWindow = &MoveResizeWindowWin32,
    };
    InitWindowPlatform(windowPlatform);

    windowManagerHwnd = CreateWindowManagerWindow();
    assert(windowManagerHwnd && "Failed to create window manager window.");

    BOOL enumWinResult = EnumWindows(&EnumWindowsProc, 0);
    assert(enumWinResult && "Failed to enumerate windows.");

    SetKeyboardHook();

    KeyMap keyMap = {
        [COMMAND_QUIT] = KEY_Q,
        [COMMAND_SELECT_WORKSPACE_0] = KEY_0,
        [COMMAND_SELECT_WORKSPACE_1] = KEY_1,
        [COMMAND_SELECT_WORKSPACE_2] = KEY_2,
        [COMMAND_SELECT_WORKSPACE_3] = KEY_3,
        [COMMAND_SELECT_WORKSPACE_4] = KEY_4,
        [COMMAND_SELECT_WORKSPACE_5] = KEY_5,
        [COMMAND_SELECT_WORKSPACE_6] = KEY_6,
        [COMMAND_SELECT_WORKSPACE_7] = KEY_7,
        [COMMAND_SELECT_WORKSPACE_8] = KEY_8,
        [COMMAND_SELECT_WORKSPACE_9] = KEY_9,
        [COMMAND_MODE_SELECT_WINDOW] = KEY_W,
        [COMMAND_SELECT_WINDOW_LEFT] = KEY_H,
        [COMMAND_SELECT_WINDOW_RIGHT] = KEY_L,
        [COMMAND_SELECT_WINDOW_UP] = KEY_K,
        [COMMAND_SELECT_WINDOW_DOWN] = KEY_J,
        [COMMAND_SPLIT_HORIZONTAL] = KEY_S,
        [COMMAND_SPLIT_VERTICAL]= KEY_V,
    };

    CommandModeType mode = MODE_SELECT_WORKSPACE;

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_QUIT) {
            break;
        }

        if (msg.message != WM_KEYDOWN) {
            continue;
        }

        if (IsCommandRequested(COMMAND_QUIT, &inputBuf, keyMap)) {
            printf("quit\n");
            break;
        }

        bool didConsumeKey = false;
        switch (mode) {
            case MODE_SELECT_WORKSPACE:
                didConsumeKey = HandleSelectWorkspaceMode(&inputBuf, keyMap, &mode);
                break;
            case MODE_SELECT_WINDOW:
                didConsumeKey = HandleSelectWindowMode(&inputBuf, keyMap, &mode);
                break;
            default:
                break;
        }

        if (didConsumeKey) {
            ResetInputBuffer(&inputBuf);
        }
    }

    RemoveKeyboardHook();

    return 0;
}
