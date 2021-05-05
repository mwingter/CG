///////////////////////////////////////////////////////////////////////////////
// ControllerMain.cpp
// ==================
// Derived Controller class for main window
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2016-02-10
// UPDATED: 2016-02-24
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <commctrl.h>                   // common controls
#include "ControllerMain.h"
#include "resource.h"
#include "Log.h"

using namespace Win;


ControllerMain::ControllerMain() : glHandle(0), formHandle(0)
{
}



int ControllerMain::command(int id, int cmd, LPARAM msg)
{
    switch(id)
    {
    case ID_FILE_EXIT:
        ::PostMessage(handle, WM_CLOSE, 0, 0);
        break;

    }
    return 0;
}



int ControllerMain::close()
{
    Win::log("Closing the application...");
    ::DestroyWindow(handle);    // destroy this window and children
    return 0;
}



int ControllerMain::destroy()
{
    ::PostQuitMessage(0);       // exit the message loop
    Win::log("Main windows is destroyed.");
    return 0;
}



int ControllerMain::create()
{
    return 0;
}



int ControllerMain::size(int w, int h, WPARAM wParam)
{
    RECT rect;

    // get client dim of mainWin
    ::GetClientRect(handle, &rect);
    int mainClientWidth = rect.right - rect.left;
    int mainClientHeight = rect.bottom - rect.top;

    // get dim of glWin
    ::GetWindowRect(glHandle, &rect);
    int glWidth = rect.right - rect.left;
    int glHeight = rect.bottom - rect.top;

    // get dim of form dialog
    ::GetWindowRect(formHandle, &rect);
    int formWidth = rect.right - rect.left;

    // resize glWin and reposition form
    glWidth = mainClientWidth - formWidth;
    glHeight = mainClientHeight;
    ::SetWindowPos(glHandle, 0, 0, 0, glWidth, glHeight, SWP_NOZORDER);
    ::SetWindowPos(formHandle, 0, glWidth, 0, formWidth, glHeight, SWP_NOZORDER);
    ::InvalidateRect(formHandle, 0, TRUE);      // force to repaint

    return 0;
}



int ControllerMain::mouseWheel(int state, int delta, int x, int y)
{
    // send wm_mouse event to glWin
    ::SendMessage(glHandle, WM_MOUSEWHEEL, MAKEWPARAM((short)state, (short)delta*WHEEL_DELTA), MAKELPARAM((short)x, (short)y));
    return 0;
}



