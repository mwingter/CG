///////////////////////////////////////////////////////////////////////////////
// Star.h
// ======
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

#ifndef STAR_H_DEF
#define STAR_H_DEF

#include <vector>
#include "Vectors.h"

class Star
{
public:
    // ctor/dtor
    Star(unsigned int pointCount=5, float radius=1);
    ~Star();

    // setters/getters
    void set(unsigned int count, float radius);

    void setPointCount(unsigned int count);     // set # of of outer points
    unsigned int getPointCount() const { return pointCount; }

    void setRadius(float radius=1);             // generate new star with radius
    float getRadius() const { return radius; }

    void setInnerRadius(float radius=1);        // generate new star with radius
    float getInnerRadius() const { return innerRadius; }

    const std::vector<Vector2>& getPoints() const { return points; }
    const std::vector<Vector3>& getPrints3D() const;

    const Vector2& getPoint(int index) const;   // return a single point corresponding index
    const Vector3& getPoint3D(int index) const;

    // round the coordinates to the nearest integer
    void roundInt();

    // debug
    void printSelf();

protected:

private:
    // re-generate outer/inner points
    void generatePoints();

    // compute modular arithmetic (It always returns unsigned int)
    unsigned int modulo(int dividend, int divisor);

    unsigned int pointCount;
    float radius;                   // for outer points
    float innerRadius;              // for inner points
    std::vector<Vector2> points;    // 2*N points to make star contour
    std::vector<Vector3> points3D;  // as 3D
};
#endif
