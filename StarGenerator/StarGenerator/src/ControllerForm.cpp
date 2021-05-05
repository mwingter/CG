///////////////////////////////////////////////////////////////////////////////
// ControllerForm.cpp
// ==================
// Derived Controller class for dialog window
//
//  AUTHOR: Song Ho Ahn (song.ahn@gamil.com)
// CREATED: 2016-02-10
// UPDATED: 2016-02-25
///////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <string>
#include <sstream>
#include "ControllerForm.h"
#include "wcharUtil.h"
#include "resource.h"
#include "Log.h"
using namespace Win;

// constants
const int   STAR_POINTS = 5;  // default 5-pointed star
const int   STAR_RADIUS = 10;
const int   STAR_MIN_POINTS = 10;
const int   STAR_MAX_POINTS = 500;
const int   STAR_MIN_RADIUS = 10;
const int   STAR_MAX_RADIUS = 100;

// global vars
static int starMaxPoints = 100;    // initial values
static int starMaxRadius = STAR_RADIUS;

INT_PTR CALLBACK aboutDialogProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void initAboutDialogControls(HWND hwnd);
void getSettingsFromAboutDialog(HWND dialogHandle);



///////////////////////////////////////////////////////////////////////////////
// default contructor
///////////////////////////////////////////////////////////////////////////////
ControllerForm::ControllerForm(ModelGL* model, ViewForm* view, ControllerGL* ctrl) : model(model), view(view), ctrlGL(ctrl)
{
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_DESTROY
///////////////////////////////////////////////////////////////////////////////
int ControllerForm::destroy()
{
    Win::log("Form dialog is destroyed.");
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_CREATE
///////////////////////////////////////////////////////////////////////////////
int ControllerForm::create()
{
    // initialize all controls
    view->initControls(handle);

    // load default star
    Win::log(L"Generating a default 5 pointed star ...");
    model->setStar(STAR_POINTS, (float)STAR_RADIUS);
    view->setTrackbarInnerRadius(model->getStarInnerRadius());
    view->setListPoints();
    ctrlGL->paint();    // update OpenGL canvas


    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_COMMAND
///////////////////////////////////////////////////////////////////////////////
int ControllerForm::command(int id, int command, LPARAM msg)
{
    switch(id)
    {
    case IDC_CHECK_GRID:
        if(command == BN_CLICKED)
        {
            view->toggleGrid();
        }
        break;
    case IDC_CHECK_FILL:
        if(command == BN_CLICKED)
        {
            view->toggleFill();
        }
        break;
    case IDC_CHECK_EDGE:
        if(command == BN_CLICKED)
        {
            view->toggleEdge();
        }
        break;
    case IDC_CHECK_POINT:
        if(command == BN_CLICKED)
        {
            view->togglePoint();
        }
        break;
    case IDC_LIST_POINTS:
        if(command == LBN_SELCHANGE)
        {
            view->selectPoint();
        }
        break;
    case IDC_BUTTON_RESET:
        if(command == BN_CLICKED)
        {
            model->resetCamera();
            paint();
        }
        break;
    case IDC_BUTTON_ABOUT:
        if(command == BN_CLICKED)
        {
            // open About dialog
            ::DialogBox((HINSTANCE)::GetWindowLongPtr(handle, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_DIALOG_ABOUT), handle, aboutDialogProcedure);

            // exitthe dialog, update the form controls
            view->setMaxTrackbars(starMaxPoints, starMaxRadius);
            model->setGridSize((float)starMaxRadius);
        }
        break;
    }

    ctrlGL->paint();
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle horizontal scroll notification
///////////////////////////////////////////////////////////////////////////////
int ControllerForm::hScroll(WPARAM wParam, LPARAM lParam)
{
    // check if the message comming from trackbar
    HWND trackbarHandle = (HWND)lParam;

    int position = HIWORD(wParam);              // current tick mark position
    if(trackbarHandle)
    {
        // get control ID
        int trackbarId = ::GetDlgCtrlID(trackbarHandle);

        switch(LOWORD(wParam))
        {
        case TB_THUMBTRACK:     // user dragged the slider
            //Win::log("trackbar: %d", position);
            view->updateTrackbars(trackbarHandle, position);
            if(trackbarId == IDC_SLIDER_POINTS)
            {
                model->setStarPointCount(position);
                view->setTrackbarInnerRadius(model->getStarInnerRadius());
            }
            else if(trackbarId == IDC_SLIDER_OUTRADIUS)
            {
                float radiusScale = view->getRadiusScale();
                model->setStarOuterRadius(position * radiusScale);
                view->setTrackbarInnerRadius(model->getStarInnerRadius());
            }
            else if(trackbarId == IDC_SLIDER_INRADIUS)
            {
                float radiusScale = view->getRadiusScale();
                model->setStarInnerRadius(position * radiusScale);
            }
            view->setListPoints();
            break;

        case TB_THUMBPOSITION:  // by WM_LBUTTONUP
            break;

        case TB_LINEUP:         // by VK_RIGHT, VK_DOWN
            break;

        case TB_LINEDOWN:       // by VK_LEFT, VK_UP
            break;

        case TB_TOP:            // by VK_HOME
            break;

        case TB_BOTTOM:         // by VK_END
            break;

        case TB_PAGEUP:         // by VK_PRIOR (User click the channel to prior.)
            break;

        case TB_PAGEDOWN:       // by VK_NEXT (User click the channel to next.)
            break;

        case TB_ENDTRACK:       // by WM_KEYUP (User release a key.)
            position = (int)::SendMessage(trackbarHandle, TBM_GETPOS, 0, 0);
            view->updateTrackbars(trackbarHandle, position);
            if(trackbarId == IDC_SLIDER_POINTS)
            {
                model->setStarPointCount(position);
                view->setTrackbarInnerRadius(model->getStarInnerRadius());
            }
            else if(trackbarId == IDC_SLIDER_OUTRADIUS)
            {
                float radiusScale = view->getRadiusScale();
                model->setStarOuterRadius(position * radiusScale);
                view->setTrackbarInnerRadius(model->getStarInnerRadius());
            }
            else if(trackbarId == IDC_SLIDER_INRADIUS)
            {
                float radiusScale = view->getRadiusScale();
                model->setStarInnerRadius(position * radiusScale);
            }
            view->setListPoints();
            break;
        }
    }

    ctrlGL->paint();
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_NOTIFY
// The id is not guaranteed to be unique, so use NMHDR.hwndFrom and NMHDR.idFrom.
///////////////////////////////////////////////////////////////////////////////
int ControllerForm::notify(int id, LPARAM lParam)
{
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// handle WM_SIZE
///////////////////////////////////////////////////////////////////////////////
int ControllerForm::size(int w, int h, WPARAM wParam)
{
    view->resizeListPoints(w, h);
    return 0;
}



///////////////////////////////////////////////////////////////////////////////
// dialog procedure for About window
///////////////////////////////////////////////////////////////////////////////
INT_PTR CALLBACK aboutDialogProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_INITDIALOG:
        initAboutDialogControls(hwnd);
        break;

    case WM_CLOSE:
        {
            // before close dialog, get the current values of spiner
            getSettingsFromAboutDialog(hwnd);
            ::EndDialog(hwnd, 0);
        }
        break;

    case WM_COMMAND:
        if(LOWORD(wParam) == IDOK && HIWORD(wParam) == BN_CLICKED)
        {
            getSettingsFromAboutDialog(hwnd);
            ::EndDialog(hwnd, 0);
        }
        break;

    case WM_NOTIFY:
        NMHDR* nmhdr = (NMHDR*)lParam;
        HWND from = nmhdr->hwndFrom;
        // UpDownBox notifications ========================
        if(nmhdr->code == UDN_DELTAPOS)
        {
            // unfocus editbox
            ::SetFocus(hwnd);
        }
        break;
    }

    return false;
}

void initAboutDialogControls(HWND hwnd)
{
    EditBox editPoints;
    editPoints.set(hwnd, IDC_EDIT_MAXPOINTS);

    EditBox editRadius;
    editRadius.set(hwnd, IDC_EDIT_MAXRADIUS);

    UpDownBox spinPoints;
    spinPoints.set(hwnd, IDC_SPIN_MAXPOINTS);
    spinPoints.setRange(STAR_MIN_POINTS, STAR_MAX_POINTS);
    spinPoints.setBuddy(editPoints.getHandle());
    spinPoints.setPos(starMaxPoints);

    UpDownBox spinRadius;
    spinRadius.set(hwnd, IDC_SPIN_MAXRADIUS);
    spinRadius.setRange(STAR_MIN_RADIUS, STAR_MAX_RADIUS);
    spinRadius.setBuddy(editRadius.getHandle());
    spinRadius.setPos(starMaxRadius);
}

void getSettingsFromAboutDialog(HWND hwnd)
{
    HWND spinPoints = ::GetDlgItem(hwnd, IDC_SPIN_MAXPOINTS);
    HWND spinRadius = ::GetDlgItem(hwnd, IDC_SPIN_MAXRADIUS);

    starMaxPoints = (int)::SendMessage(spinPoints, UDM_GETPOS32, 0, 0);
    starMaxRadius = (int)::SendMessage(spinRadius, UDM_GETPOS32, 0, 0);
}
