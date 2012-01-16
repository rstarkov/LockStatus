// Copyright 2012 Roman Starkov
//
// LockStatus is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// LockStatus is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.

#include "stdafx.h"

HINSTANCE hInstance;
HWND hMessageWnd;

namespace KeyIconModes { enum KeyIconMode { Never, OnlyOn, OnlyOff, Always }; }
using KeyIconModes::KeyIconMode;

class KeyInfo
{
    private:
        NOTIFYICONDATA _trayIcon;
        int _virtualKey;
        bool _state; // true if the icon corresponds to the "ON" state, false for "OFF"
        HICON _onIcon, _offIcon; // NULL if the user doesn't want an icon for this state

    public:
        KeyInfo() { }

        KeyInfo(int virtualKey, int onIconId, int offIconId, KeyIconMode mode, LPCWSTR tooltip):
                _trayIcon(),
                _virtualKey(virtualKey),
                _state(false),
                _onIcon(NULL), _offIcon(NULL)
        {
            if (mode == KeyIconModes::Never)
                return; // though the application will never construct this with ::Never anyway

            _trayIcon.cbSize = sizeof(NOTIFYICONDATA);
            _trayIcon.uVersion = NOTIFYICON_VERSION;
            Shell_NotifyIcon(NIM_SETVERSION, &_trayIcon);

            _trayIcon.hWnd = hMessageWnd;
            _trayIcon.uFlags = NIF_ICON | NIF_TIP | NIF_STATE;
            _trayIcon.uID = onIconId;
            StringCchCopy(_trayIcon.szTip, ARRAYSIZE(_trayIcon.szTip), tooltip);
            if (mode == KeyIconModes::OnlyOn || mode == KeyIconModes::Always)
                _onIcon = LoadIcon(hInstance, MAKEINTRESOURCE(onIconId));
            if (mode == KeyIconModes::OnlyOff || mode == KeyIconModes::Always)
                _offIcon = LoadIcon(hInstance, MAKEINTRESOURCE(offIconId));

            refresh(true);
        }

        void Refresh()
        {
            refresh(false);
        }

        void DeleteFromTray()
        {
            Shell_NotifyIcon(NIM_DELETE, &_trayIcon);
        }

    private:
        void refresh(bool first)
        {
            if (_onIcon == NULL && _offIcon == NULL)
                return;

            bool on = (GetKeyState(_virtualKey) & 1) != 0;
            if (on == _state && !first)
                return; // didn't change
            _state = on;

            HICON icon = on ? _onIcon : _offIcon;
            if (icon == NULL)
            {
                _trayIcon.dwState = NIS_HIDDEN;
                _trayIcon.dwStateMask = NIS_HIDDEN;
            }
            else
            {
                _trayIcon.dwState = 0;
                _trayIcon.dwStateMask = NIS_HIDDEN;
                _trayIcon.hIcon = icon;
            }
            Shell_NotifyIcon(first ? NIM_ADD : NIM_MODIFY, &_trayIcon);
        }
};

KeyInfo *keyCapsLock = NULL;
KeyInfo *keyNumLock = NULL;
KeyInfo *keyScrollLock = NULL;

LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
    // Invoked on every key press, so had better be quick
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT *data = (KBDLLHOOKSTRUCT*) lParam;
        if (keyCapsLock != NULL && data->vkCode == VK_CAPITAL)
            keyCapsLock->Refresh();
        if (keyNumLock != NULL && data->vkCode == VK_NUMLOCK)
            keyNumLock->Refresh();
        if (keyScrollLock != NULL && data->vkCode == VK_SCROLL)
            keyScrollLock->Refresh();
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

VOID CALLBACK RefreshStates(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    // Invoked once every second to cheaply keep in sync when not changed by a key press
    if (keyCapsLock != NULL)
        keyCapsLock->Refresh();
    if (keyNumLock != NULL)
        keyNumLock->Refresh();
    if (keyScrollLock != NULL)
        keyScrollLock->Refresh();
}

void WINAPI Entry()
{
    KeyIconMode capsMode = KeyIconModes::OnlyOn;
    KeyIconMode numMode = KeyIconModes::Never;
    KeyIconMode scrollMode = KeyIconModes::OnlyOn;

    // Poor man's command line parsing: must be three characters long, one for each key (caps/num/scroll).
    // Each character may be a '0' (only show if off), a '1' (only show if on), an 'N' (never show); otherwise the
    // icon is always shown for that key.
    LPTSTR cmdLine = GetCommandLine();
    if (wcslen(cmdLine) == 3)
    {
        switch (cmdLine[0])
        {
            case L'0': capsMode = KeyIconModes::OnlyOff; break;
            case L'1': capsMode = KeyIconModes::OnlyOn; break;
            case L'n': case L'N': capsMode = KeyIconModes::Never; break;
            default: capsMode = KeyIconModes::Always; break;
        }
        switch (cmdLine[1])
        {
            case L'0': numMode = KeyIconModes::OnlyOff; break;
            case L'1': numMode = KeyIconModes::OnlyOn; break;
            case L'n': case L'N': numMode = KeyIconModes::Never; break;
            default: numMode = KeyIconModes::Always; break;
        }
        switch (cmdLine[2])
        {
            case L'0': scrollMode = KeyIconModes::OnlyOff; break;
            case L'1': scrollMode = KeyIconModes::OnlyOn; break;
            case L'n': case L'N': scrollMode = KeyIconModes::Never; break;
            default: scrollMode = KeyIconModes::Always; break;
        }
    }

    // Grab a message-only window, which is required by pretty much every piece of functionality this program has
    hInstance = GetModuleHandle(NULL);
    hMessageWnd = CreateWindow(L"Message", L"Dummy", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    // Initialize the key descriptions and show the icons
    KeyInfo capsInfo(VK_CAPITAL, IDI_CAPS_ON, IDI_CAPS_OFF, capsMode, L"Caps Lock status");
    KeyInfo numInfo(VK_NUMLOCK, IDI_NUM_ON, IDI_NUM_OFF, numMode, L"Num Lock status");
    KeyInfo scrollInfo(VK_SCROLL, IDI_SCROLL_ON, IDI_SCROLL_OFF, scrollMode, L"Scroll Lock status");
    keyCapsLock = capsMode == KeyIconModes::Never ? NULL : &capsInfo;
    keyNumLock = numMode == KeyIconModes::Never ? NULL : &numInfo;
    keyScrollLock = scrollMode == KeyIconModes::Never ? NULL : &scrollInfo;

    // Refresh the icons whenever certain keys are pressed
    SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, hInstance, 0);
    // Refresh the icons periodically
    SetTimer(hMessageWnd, 6483215, 750, RefreshStates);

    // Message loop: required not just for the timer, but also for the keyboard hook
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Don't bother deleting the pointers since we're about to quit anyway, but do bother to remove the
    // icons because that's one resource the OS fails to clean up properly on process exit - *still*.
    if (keyCapsLock != NULL)
        keyCapsLock->DeleteFromTray();
    if (keyNumLock != NULL)
        keyNumLock->DeleteFromTray();
    if (keyScrollLock != NULL)
        keyScrollLock->DeleteFromTray();

    ExitProcess(0);
}
