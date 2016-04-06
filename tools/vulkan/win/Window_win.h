/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef Window_win_DEFINED
#define Window_win_DEFINED

#include <windows.h>
#include "../Window.h"

class Window_win : public Window {
public:
    Window_win() : Window() {}
    ~Window_win() override {}

    bool init(HINSTANCE instance);

    void setTitle(const char*) override;
    void show() override;

    // event callbacks
    bool onKeyboard(UINT nChar, bool bKeyDown, bool bAltDown);
    bool onMouse(bool bLeftButtonDown, bool bRightButtonDown, bool bMiddleButtonDown,
                 bool bSideButton1Down, bool bSideButton2Down, int nMouseWheelDelta,
                 int xPos, int yPos);

    bool attach(BackEndTypes attachType, int msaaSampleCount, AttachmentInfo*) override;

private:
    HINSTANCE fHInstance;
    HWND      fHWnd;
};

#endif
