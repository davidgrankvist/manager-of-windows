#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "input.h"
#include "commands.h"

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
        case VK_LWIN: return KEY_SUPER_LEFT;
        default: return KEY_NONE;
    }
}

void FocusWindowManager();

HHOOK keyboardHook;
KeyCode keyLeader = KEY_SUPER_LEFT;
InputBuffer hookBuf = {0};

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

void SetKeyboardHook() {
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (keyboardHook == NULL) {
        exit(1);
    }
}

void RemoveKeyboardHook() {
    if (keyboardHook != NULL) {
        UnhookWindowsHookEx(keyboardHook);
    }
}

InputBuffer inputBuf = {0};

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

void ForceWindowIntoFocus(HWND hwnd) {
    HWND fg = GetForegroundWindow();
    DWORD fgThread = GetWindowThreadProcessId(fg, NULL);
    DWORD thisThread = GetCurrentThreadId();

    AttachThreadInput(thisThread, fgThread, TRUE);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    AttachThreadInput(thisThread, fgThread, FALSE);
}

HWND windowManagerHwnd = NULL;

void FocusWindowManager() {
    ForceWindowIntoFocus(windowManagerHwnd);
}

static bool IsCommandRequested(CommandType cmdType, InputBuffer* buf, KeyCode* keyMap) {
    return IsKeyPressed(keyMap[cmdType], buf);
}

int main() {
    windowManagerHwnd = CreateWindowManagerWindow();
    assert(windowManagerHwnd && "Failed to create window manager window.");
    SetKeyboardHook();

    KeyMap keyMap = {
        [COMMAND_QUIT] = KEY_Q,
        [COMMAND_SELECT_WORKSPACE_1] = KEY_1,
        [COMMAND_SELECT_WORKSPACE_2] = KEY_2,
    };

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_QUIT) {
            break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (IsCommandRequested(COMMAND_QUIT, &inputBuf, keyMap)) {
            printf("quit\n");
            break;
        }

        if (IsCommandRequested(COMMAND_SELECT_WORKSPACE_1, &inputBuf, keyMap)) {
            printf("workspace 1\n");
            ResetInputBuffer(&inputBuf);
        }

        if (IsCommandRequested(COMMAND_SELECT_WORKSPACE_2, &inputBuf, keyMap)) {
            printf("workspace 2\n");
            ResetInputBuffer(&inputBuf);
        }
    }

    RemoveKeyboardHook();

    return 0;
}
