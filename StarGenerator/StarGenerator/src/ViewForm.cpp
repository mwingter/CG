///////////////////////////////////////////////////////////////////////////////
// ViewForm.cpp
// ============
// View component of dialog window
//
//  AUTHORL Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2016-02-16
// UPDATED: 2016-02-24
///////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <iomanip>
#include <cmath>
#include "ViewForm.h"
#include "resource.h"
#include "Log.h"
#include "wcharUtil.h"
using namespace Win;

// constants
const int POINT_START = 4;
const int POINT_END = 100;
const int POINT_DEFAULT = 5;
const int RADIUS_START = 1;     // ticks, not values
const int RADIUS_END = 100;



///////////////////////////////////////////////////////////////////////////////
// default ctor
///////////////////////////////////////////////////////////////////////////////
ViewForm::ViewForm(ModelGL* model) : model(model), parentHandle(0), radiusScale(0.1f)
{
}


///////////////////////////////////////////////////////////////////////////////
// default dtor
///////////////////////////////////////////////////////////////////////////////
ViewForm::~ViewForm()
{
}



///////////////////////////////////////////////////////////////////////////////
// initialize all controls
///////////////////////////////////////////////////////////////////////////////
void ViewForm::initControls(HWND handle)
{
    // remember the handle to parent window
    parentHandle = handle;

    // set all controls
    checkGrid.set(handle, IDC_CHECK_GRID);
    checkGrid.check();
    checkFill.set(handle, IDC_CHECK_FILL);
    checkFill.check();
    checkEdge.set(handle, IDC_CHECK_EDGE);
    checkEdge.check();
    checkPoint.set(handle, IDC_CHECK_POINT);
    checkPoint.check();

    sliderPoints.set(handle, IDC_SLIDER_POINTS);
    sliderPoints.setRange(POINT_START, POINT_END);
    sliderPoints.setPos(POINT_DEFAULT);
    sliderOutRadius.set(handle, IDC_SLIDER_OUTRADIUS);
    sliderOutRadius.setRange(RADIUS_START, RADIUS_END);
    sliderOutRadius.setPos(RADIUS_END);
    sliderInRadius.set(handle, IDC_SLIDER_INRADIUS);
    sliderInRadius.setRange(RADIUS_START, RADIUS_END);
    sliderInRadius.setPos(RADIUS_END);
    textPoints.set(handle, IDC_STATIC_POINTS);
    textPoints.setText(toWchar((long)POINT_DEFAULT));
    textOutRadius.set(handle, IDC_STATIC_OUTRADIUS);
    textOutRadius.setText(toWchar(RADIUS_END * radiusScale));
    textInRadius.set(handle, IDC_STATIC_INRADIUS);
    textInRadius.setText(toWchar(RADIUS_END * radiusScale));

    listPoints.set(handle, IDC_LIST_POINTS);
    buttonReset.set(handle, IDC_BUTTON_RESET);
    buttonAbout.set(handle, IDC_BUTTON_ABOUT);
    buttonAbout.setImage(::LoadIcon(0, IDI_INFORMATION));
}



///////////////////////////////////////////////////////////////////////////////
// reset trackbar controls with new max values
///////////////////////////////////////////////////////////////////////////////
void ViewForm::setMaxTrackbars(int maxPoints, int maxRadius)
{
    int position = sliderPoints.getPos();
    sliderPoints.setRange(POINT_START, maxPoints);
    sliderPoints.setPos(position);
    textPoints.setText(toWchar((long)position));

    // get prev positions & values
    int outPos = sliderOutRadius.getPos();
    float outRadius = (int)(outPos * radiusScale * 10.0f + 0.5f) * 0.1f;
    int inPos = sliderInRadius.getPos();
    float inRadius = (int)(inPos * radiusScale * 10.0f + 0.5f) * 0.1f;

    // map new tick to value
    radiusScale = maxRadius / 100.0f;

    sliderOutRadius.setRange(RADIUS_START, RADIUS_END);
    sliderOutRadius.setPos((int)(outRadius / radiusScale + 0.5f));
    textOutRadius.setText(toWchar(outRadius));

    sliderInRadius.setRange(RADIUS_START, RADIUS_END);
    sliderInRadius.setPos((int)(inRadius / radiusScale * 0.5f));
    textInRadius.setText(toWchar(inRadius));
}



///////////////////////////////////////////////////////////////////////////////
// print all coords of outer/inner points
///////////////////////////////////////////////////////////////////////////////
void ViewForm::setListPoints()
{
    unselectPoint();
    listPoints.resetContent();

    std::wstringstream wss;
    wss << std::fixed << std::setprecision(1);

    // print all points
    std::vector<Vector2> points = model->getStarPoints();
    for(unsigned int i = 0; i < points.size(); ++i)
    {
        wss.str(L"");
        wss << i+1 << L": (" << points[i].x << L", " << points[i].y << L")" << std::ends;
        listPoints.addString(wss.str().c_str());
    }

    // unset floating format
    wss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
}



///////////////////////////////////////////////////////////////////////////////
// clear listbox
///////////////////////////////////////////////////////////////////////////////
void ViewForm::clearListPoints()
{
    listPoints.resetContent();
}



///////////////////////////////////////////////////////////////////////////////
// resize listbox height
///////////////////////////////////////////////////////////////////////////////
void ViewForm::resizeListPoints(int parentWidth, int parentHeight)
{
    RECT rect;

    // get parent's position (x,y) in window coords
    ::GetWindowRect(parentHandle, &rect);
    int parentX = rect.left;
    int parentY = rect.top;
    int parentRight = rect.right;
    int parentBottom = rect.bottom;

    // compute new listbox's position, width and height
    ::GetWindowRect(listPoints.getHandle(), &rect);
    int x = rect.left - parentX;
    int y = rect.top - parentY;
    int spacing = parentRight - rect.right;
    int width = rect.right - rect.left;
    int height = (parentBottom - spacing) - rect.top;
    if(height < 0)
        height = 0;

    ::SetWindowPos(listPoints.getHandle(), 0, x, y, width, height, SWP_NOZORDER);
}



///////////////////////////////////////////////////////////////////////////////
// toggle on/off options
///////////////////////////////////////////////////////////////////////////////
void ViewForm::toggleGrid()
{
    if(checkGrid.isChecked())
        model->enableGrid();
    else
        model->disableGrid();
}
void ViewForm::toggleFill()
{
    if(checkFill.isChecked())
        model->enableFill();
    else
        model->disableFill();
}
void ViewForm::toggleEdge()
{
    if(checkEdge.isChecked())
        model->enableEdge();
    else
        model->disableEdge();
}
void ViewForm::togglePoint()
{
    if(checkPoint.isChecked())
        model->enablePoint();
    else
        model->disablePoint();
}



///////////////////////////////////////////////////////////////////////////////
// handle listbox event
///////////////////////////////////////////////////////////////////////////////
void ViewForm::selectPoint()
{
    int index = (int)::SendMessage(listPoints.getHandle(), LB_GETCURSEL, 0, 0);
    model->setStarSelectedPoint(index);
}

void ViewForm::unselectPoint()
{
    model->setStarSelectedPoint(-1);
}


///////////////////////////////////////////////////////////////////////////////
// update trackbars
///////////////////////////////////////////////////////////////////////////////
void ViewForm::updateTrackbars(HWND handle, int position)
{
    if(handle == sliderPoints.getHandle())
    {
        sliderPoints.setPos(position);
        textPoints.setText(toWchar((long)position));
    }
    else if(handle == sliderOutRadius.getHandle())
    {
        sliderOutRadius.setPos(position);
        textOutRadius.setText(toWchar(position * radiusScale));
    }
    else if(handle == sliderInRadius.getHandle())
    {
        sliderInRadius.setPos(position);
        textInRadius.setText(toWchar(position * radiusScale));
    }
}

void ViewForm::setTrackbarInnerRadius(float radius)
{
    int position = (int)(radius / radiusScale);
    sliderInRadius.setPos(position);
    textInRadius.setText(toWchar(position * radiusScale));
}
