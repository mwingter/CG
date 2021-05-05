///////////////////////////////////////////////////////////////////////////////
// main.cpp
// ========
// main driver
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2016-02-10
// UPDATED: 2016-02-22
///////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN             // exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <commctrl.h>                   // common controls
#include "Window.h"
#include "DialogWindow.h"
#include "ControllerMain.h"
#include "ControllerGL.h"
#include "ControllerForm.h"
#include "ModelGL.h"
#include "ViewGL.h"
#include "ViewForm.h"
#include "resource.h"
#include "Log.h"


// constants ==================================================================
const wchar_t* APP_NAME = L"Star Generator";
const int      GL_WIDTH = 500;
const int      GL_HEIGHT = 500;
const int      FORM_WIDTH = 350;

// function declarations
int mainMessageLoop(HACCEL hAccelTable=0);




///////////////////////////////////////////////////////////////////////////////
// main function of a windows application
///////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdArgs, int cmdShow)
{
    //Win::logMode(Win::LOG_MODE_DIALOG);
    //Win::logMode(Win::LOG_MODE_BOTH);

    // init comctl32.dll before creating windows
    INITCOMMONCONTROLSEX commonCtrls;
    commonCtrls.dwSize = sizeof(commonCtrls);
    commonCtrls.dwICC = ICC_STANDARD_CLASSES | ICC_COOL_CLASSES;
    ::InitCommonControlsEx(&commonCtrls);

    Win::ControllerMain mainCtrl;
    Win::Window mainWin(hInst, APP_NAME, 0, &mainCtrl);

    // add menu to window class
    //mainWin.setMenuName(MAKEINTRESOURCE(IDR_MAIN_MENU));
    //mainWin.setWindowStyleEx(WS_EX_WINDOWEDGE);

    // create a window and show
    if(mainWin.create())
        Win::log("Main window is created.");
    else
        Win::log("[ERROR] Failed to create main window.");

    // create model and view components for controller
    ModelGL modelGL;
    Win::ViewGL viewGL;

    // create OpenGL rendering window as a child
    Win::ControllerGL glCtrl(&modelGL, &viewGL);
    Win::Window glWin(hInst, L"WindowGL", mainWin.getHandle(), &glCtrl);
    glWin.setWindowStyle(WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    glWin.setWindowStyleEx(WS_EX_WINDOWEDGE);
    glWin.setClassStyle(CS_OWNDC);
    //@@ MS ArticleID: 272222
    // There is a clipping bug when the window is resized larger.
    // Create a window with the max size initially to avoid this clipping bug.
    // Subsequent SetWindowPos() calls to resize the window dimension do not
    // generate the clipping issue.
    int fullWidth = ::GetSystemMetrics(SM_CXSCREEN);    // primary display only
    int fullHeight = ::GetSystemMetrics(SM_CXSCREEN);
    glWin.setWidth(fullWidth);
    glWin.setHeight(fullHeight);
    //Win::log("%d, %d", fullWidth, fullHeight);
    if(glWin.create())
        Win::log("OpenGL rendering window is created.");
    else
        Win::log("[ERROR] Failed to create OpenGL window.");

    // create a child dialog box contains controls
    Win::ViewForm viewForm(&modelGL);
    Win::ControllerForm formCtrl(&modelGL, &viewForm, &glCtrl);
    Win::DialogWindow formDialog(hInst, IDD_FORMVIEW, mainWin.getHandle(), &formCtrl);
    formDialog.setWidth(FORM_WIDTH);
    formDialog.setHeight(GL_HEIGHT);
    if(formDialog.create())
        Win::log("Form dialog is created.");
    else
        Win::log("[ERROR] Failed to create Form dialog.");

    // send window handles to mainCtrl, they are used for resizing window
    mainCtrl.setGLHandle(glWin.getHandle());
    mainCtrl.setFormHandle(formDialog.getHandle());

    // place windows in the right position ================
    RECT rect;

    // compute dim of dialog window (for higher DPI display)
    ::GetWindowRect(formDialog.getHandle(), &rect);
    int dialogWidth = rect.right - rect.left;
    int dialogHeight = rect.bottom - rect.top;

    // set dim of opengl window
    int glWidth = dialogHeight;
    int glHeight = dialogHeight;
    ::SetWindowPos(glWin.getHandle(), 0, 0, 0, glWidth, glHeight, SWP_NOZORDER);

    // place the form dialog at the right of the opengl rendering window
    ::SetWindowPos(formDialog.getHandle(), 0, glWidth, 0, dialogWidth, dialogHeight, SWP_NOZORDER);

    // set dim of main window
    rect.left = 0;
    rect.right = glWidth + dialogWidth;
    rect.top = 0;
    rect.bottom = glHeight;
    DWORD style = (DWORD)::GetWindowLongPtr(mainWin.getHandle(), GWL_STYLE);
    DWORD styleEx = (DWORD)::GetWindowLongPtr(mainWin.getHandle(), GWL_EXSTYLE);
    ::AdjustWindowRectEx(&rect, style, FALSE, styleEx);
    ::SetWindowPos(mainWin.getHandle(), 0, 0, 0, (rect.right-rect.left), (rect.bottom-rect.top), SWP_NOZORDER);

    // show all windows
    glWin.show();
    formDialog.show();
    mainWin.show();


    // main message loop //////////////////////////////////////////////////////
    int exitCode;
    HACCEL hAccelTable = 0;
    //hAccelTable = ::LoadAccelerators(hInst, MAKEINTRESOURCE(ID_ACCEL));
    exitCode = mainMessageLoop(hAccelTable);
    Win::log("Application is terminated.");

    return exitCode;
}



///////////////////////////////////////////////////////////////////////////////
// main message loop
///////////////////////////////////////////////////////////////////////////////
int mainMessageLoop(HACCEL hAccelTable)
{
    HWND activeHandle;
    MSG msg;

    while(::GetMessage(&msg, 0, 0, 0) > 0)  // loop until WM_QUIT(0) received
    {
        // determine the activated window is dialog box
        // skip if messages are for the dialog windows
        activeHandle = GetActiveWindow();
        if(::GetWindowLongPtr(activeHandle, GWL_EXSTYLE) & WS_EX_CONTROLPARENT) // WS_EX_CONTROLPARENT is automatically added by CreateDialogBox()
        {
            if(::IsDialogMessage(activeHandle, &msg))
                continue;   // message handled, back to while-loop
        }

        // now, handle window messages
        if(!::TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;                 // return nExitCode of PostQuitMessage()
}
