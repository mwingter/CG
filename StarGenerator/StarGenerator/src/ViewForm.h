///////////////////////////////////////////////////////////////////////////////
// ViewForm.h
// ==========
// View component of dialog window
//
//  AUTHORL Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2016-02-16
// UPDATED: 2016-02-24
///////////////////////////////////////////////////////////////////////////////

#ifndef VIEW_FORM_H
#define VIEW_FORM_H

#include <windows.h>
#include "Controls.h"
#include "ModelGL.h"

namespace Win
{
    class ViewForm
    {
    public:
        ViewForm(ModelGL* model);
        ~ViewForm();

        void initControls(HWND handle);         // init all controls
        void setListPoints();
        void clearListPoints();
        void resizeListPoints(int parentW, int parentH);
        void selectPoint();
        void unselectPoint();
        void updateTrackbars(HWND handle, int position);
        void setTrackbarInnerRadius(float radius);
        void setMaxTrackbars(int maxPoints, int maxRadius);

        void toggleGrid();
        void toggleFill();
        void toggleEdge();
        void togglePoint();

        float getRadiusScale() const { return radiusScale; }

    protected:

    private:
        ModelGL* model;
        HWND parentHandle;
        float radiusScale;                      // slider tick to actual value

        // controls
        Win::CheckBox checkGrid;
        Win::CheckBox checkFill;
        Win::CheckBox checkEdge;
        Win::CheckBox checkPoint;

        Win::EditBox textPoints;
        Win::EditBox textOutRadius;
        Win::EditBox textInRadius;
        Win::Trackbar sliderPoints;
        Win::Trackbar sliderOutRadius;
        Win::Trackbar sliderInRadius;

        Win::ListBox listPoints;
        Win::Button buttonAbout;
        Win::Button buttonReset;
    };
}

#endif
