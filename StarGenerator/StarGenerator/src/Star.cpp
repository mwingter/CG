///////////////////////////////////////////////////////////////////////////////
// Star.cpp
// ========
// class for a N-pointed star on 2D plane. A star consists of N outer points
// plus N inner points (where N >= 4).
// The default star will be constructed 5-pointed and radius=1.
// Star object can be created with given radius. Then, the top point will be
// (0, radius).
//
// Dependencies: Vector2, Vector3
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2016-01-13
// UPDATED: 2016-03-14
///////////////////////////////////////////////////////////////////////////////

#include "Star.h"
#include "Line.h"
#include <cmath>
#include <cstdlib>
#include <iostream>



///////////////////////////////////////////////////////////////////////////////
// ctor
///////////////////////////////////////////////////////////////////////////////
Star::Star(unsigned int pointCount, float radius) : innerRadius(0)
{
    set(pointCount, radius);
}



///////////////////////////////////////////////////////////////////////////////
// dtor
///////////////////////////////////////////////////////////////////////////////
Star::~Star()
{
}


///////////////////////////////////////////////////////////////////////////////
// define a star with the # of outer points and radius
///////////////////////////////////////////////////////////////////////////////
void Star::set(unsigned int pointCount, float radius)
{
    // set radius first
    if(radius < 0)
        return;

    this->radius = radius;
    setPointCount(pointCount);
}



///////////////////////////////////////////////////////////////////////////////
// set # of outer and inner points
// The total number of points are 2*N (outer + inner).
///////////////////////////////////////////////////////////////////////////////
void Star::setPointCount(unsigned int pointCount)
{
    // min count is 4
    if(pointCount < 4)
        pointCount = 4;
    else
        this->pointCount = pointCount;

    points.resize(this->pointCount * 2);
    points3D.resize(this->pointCount * 2);

    generatePoints();
}



///////////////////////////////////////////////////////////////////////////////
// set outer radius
///////////////////////////////////////////////////////////////////////////////
void Star::setRadius(float radius)
{
    if(radius < 0)
        return;

    this->radius = radius;
    generatePoints();
}



///////////////////////////////////////////////////////////////////////////////
// generate N-pointed star with given radius
//      0
//   9 / \ 1
//8---+---+---2
//  \ |   | /
//   \|   |/
//   7+   +3
//    |\ /|
//    | + |
//    |/5\|
//    6   4
///////////////////////////////////////////////////////////////////////////////
void Star::generatePoints()
{
    const float PI = 3.141592f / 180.0f;    // step angle in radian

    float angle = 360.0f / pointCount * PI; // angle segment in radian;
    float x;
    float y;
    unsigned i, j, k;

    // compute outer points first (even index) ======================
    for(i = 0; i <= pointCount; i += 2)
    {
        j = i / 2;
        x = radius * sinf(j * angle);
        y = radius * cosf(j * angle);

        points[i].x = x;
        points[i].y = y;
        points3D[i].x = x;
        points3D[i].y = y;
        points3D[i].z = 0;

        // set the point reflected over y-axis
        if(i > 0 && i < pointCount)
        {
            j = 2 * pointCount - i;
            points[j].x = -x;
            points[j].y = y;
            points3D[j].x = -x;
            points3D[j].y = y;
            points3D[j].z = 0;
        }
    }

    // compute inner intersect points (odd index) ===================
    Vector3 p;              // for finding intersection point
    Line l1, l2;            // for lines
    for(i = 1; i <= pointCount; i += 2)
    {
        // find line1 with 2 points at i-1 and i+3
        j = i - 1;
        k = i + 3;
        l1.set(points[k] - points[j], points[j]);

        // find line2 with 2 points at i+1 and i-3
        j = i + 1;
        k = modulo(i - 3, pointCount*2);
        l2.set(points[k] - points[j], points[j]);

        // find intersection point
        p = l1.intersect(l2);
        points[i].x = p.x;
        points[i].y = p.y;

        // set the point reflected over y-axis
        if(i < pointCount)
        {
            j = 2 * pointCount - i;
            points[j].x = -p.x;
            points[j].y = p.y;
            points3D[j].x = -p.x;
            points3D[j].y = p.y;
            points3D[j].z = 0;
        }
    }

    // compute radius for inner points
    innerRadius = points[1].length();
    if(pointCount == 4)
    {
        // special case where pointCount=4, use 1/5 of outer radius
        innerRadius = radius * 0.2f;
        float side = sqrtf(innerRadius * 0.5f);
        points3D[1].x = points3D[3].x = points[1].x = points[3].x = side;
        points3D[1].y = points3D[7].y = points[1].y = points[7].y = side;
        points3D[5].x = points3D[7].x = points[5].x = points[7].x = -side;
        points3D[3].y = points3D[5].y = points[3].y = points[5].y = -side;
    }
}



///////////////////////////////////////////////////////////////////////////////
// override radius for inner points
///////////////////////////////////////////////////////////////////////////////
void Star::setInnerRadius(float radius)
{
    if(radius < 0)
        return;

    innerRadius = radius;
    unsigned int count = 2 * pointCount;
    for(unsigned int i = 1; i < count; i += 2)
    {
        points[i] = radius * points[i].normalize();
        points3D[i] = radius * points3D[i].normalize();
    }
}



///////////////////////////////////////////////////////////////////////////////
// getters
///////////////////////////////////////////////////////////////////////////////
const Vector2& Star::getPoint(int index) const
{
    return points.at(index);
}

const Vector3& Star::getPoint3D(int index) const
{
    return points3D.at(index);
}



///////////////////////////////////////////////////////////////////////////////
// round all point values to the nearest int
///////////////////////////////////////////////////////////////////////////////
void Star::roundInt()
{
    unsigned int count = 2 * pointCount;
    for(unsigned int i = 0; i < count; ++i)
    {
        points3D[i].x = points[i].x = (float)round(points[i].x);
        points3D[i].y = points[i].y = (float)round(points[i].y);
    }
}



///////////////////////////////////////////////////////////////////////////////
// debug
///////////////////////////////////////////////////////////////////////////////
void Star::printSelf()
{

    std::cout << "Star\n"
              << "====\n";
    for(unsigned int i = 0; i < pointCount * 2; ++i)
    {
        std::cout << i << ": " << points[i] << std::endl;
    }
    std::cout << std::endl;
}



///////////////////////////////////////////////////////////////////////////////
// compute modular (clock) arithmetic
// C's % operator may return negative modulus, but it returns unsigned int.
///////////////////////////////////////////////////////////////////////////////
unsigned int Star::modulo(int dividend, int divisor)
{
    // validate divisor
    if(divisor == 0)
        return abs(dividend);
    else if(divisor < 0)
        return modulo(-dividend, -divisor);

    // calculate positive modulus
    int modulus = dividend % divisor;
    if(modulus < 0)
        modulus += divisor;

    return (unsigned int)modulus;
}
