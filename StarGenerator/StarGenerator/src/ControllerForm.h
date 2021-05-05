///////////////////////////////////////////////////////////////////////////////
// ControllerForm.h
// ================
// Derived Controller class for dialog window
//
//  AUTHOR: Song Ho Ahn (song.ahn@gamil.com)
// CREATED: 2016-02-10
// UPDATED: 2016-02-25
///////////////////////////////////////////////////////////////////////////////

#ifndef WIN_CONTROLLER_FORM_H
#define WIN_CONTROLLER_FORM_H

#include "Controller.h"
#include "ViewForm.h"
#include "ModelGL.h"
#include "ControllerGL.h"


namespace Win
{
    class ControllerForm : public Controller
    {
    public:
        ControllerForm(ModelGL* model, ViewForm* view, ControllerGL* ctrl);
        ~ControllerForm() {};

        int command(int id, int cmd, LPARAM msg);   // for WM_COMMAND
        int create();                               // for WM_CREATE
        int destroy();                              // for WM_DESTROY
        int hScroll(WPARAM wParam, LPARAM lParam);  // for WM_HSCROLL
        int notify(int id, LPARAM lParam);          // for WM_NOTIFY
        int size(int w, int h, WPARAM wParam);      // for WM_SIZE: width, height, type(SIZE_MAXIMIZED...)

    private:
        ModelGL* model;                             // pointer to model component
        ViewForm* view;                             // pointer to view component
        ControllerGL* ctrlGL;                       // pointer to OpenGL controller compoment
    };
}

#endif
