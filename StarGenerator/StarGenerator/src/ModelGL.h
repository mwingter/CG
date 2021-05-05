///////////////////////////////////////////////////////////////////////////////
// ModelGL.h
// =========
// Model component of OpenGL
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2016-02-10
// UPDATED: 2016-02-25
///////////////////////////////////////////////////////////////////////////////

#ifndef MODEL_GL_H
#define MODEL_GL_H

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#ifdef _WIN32
#include "glExtension.h"
#endif

#include <vector>
#include <string>
#include "Matrices.h"
#include "Vectors.h"
#include "Star.h"
#include "Line.h"

class ModelGL
{
public:
    ModelGL();
    ~ModelGL();

    void init();                            // initialize OpenGL states
    void quit();                            // clean up OpenGL objects
    void setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ);
    void draw();

    void setMouseLeft(bool flag)            { mouseLeftDown = flag; };
    void setMouseRight(bool flag)           { mouseRightDown = flag; };
    void setMousePosition(int x, int y)     { mouseX = x; mouseY = y; };
    void setWindowSize(int width, int height);

    void rotateCamera(int x, int y);
    void zoomCamera(int dist);
    void zoomCameraDelta(int delta);        // for mousewheel
    void resetCamera();

    bool isShaderSupported() const          { return glslSupported; }
    bool isShaderReady() const              { return glslReady; }
    bool isVboSupported() const             { return vboSupported; }

    // toggle options
    void enableGrid()                       { gridEnabled = true; }
    void enableFill()                       { fillEnabled = true; }
    void enableEdge()                       { edgeEnabled = true; }
    void enablePoint()                      { pointEnabled = true; }

    void disableGrid()                      { gridEnabled = false; }
    void disableFill()                      { fillEnabled = false; }
    void disableEdge()                      { edgeEnabled = false; }
    void disablePoint()                     { pointEnabled = false; }

    // update star properties
    void setStar(int pointCount, float radius);
    void setStarPointCount(int count);
    void setStarOuterRadius(float radius);
    void setStarInnerRadius(float radius);
    void setStarSelectedPoint(int index);
    float getStarInnerRadius()              { return star.getInnerRadius(); }
    const std::vector<Vector2>& getStarPoints() { return star.getPoints(); }

    // for grid
    void setGridSize(float radius);

protected:

private:
    // member functions
    void initLights();                              // add a white light ti scene
    void setViewport(int x, int y, int width, int height);
    void preFrame();
    void postFrame();
    void drawStar();
    void drawStarWithVbo();
    void drawGrid(float size, float step);          // draw a grid on XZ plane
    void setFrustum(float l, float r, float b, float t, float n, float f);
    void setFrustum(float fovy, float ratio, float n, float f);
    void setOrthoFrustum(float l, float r, float b, float t, float n=-1, float f=1);
    void updateModelMatrix();
    void updateViewMatrix();
    bool createShaderPrograms();
    bool createVertexBufferObjects();
    void logShaders();

    // members
    int windowWidth;
    int windowHeight;
    bool windowSizeChanged;
    bool mouseLeftDown;
    bool mouseRightDown;
    int mouseX;
    int mouseY;
    float nearPlane;
    float farPlane;
    Vector4 bgColor;
    bool gridEnabled;
    bool fillEnabled;
    bool edgeEnabled;
    bool pointEnabled;

    float gridSize;         // half length of grid
    float gridStep;         // step for next grid line

    // star
    Star star;
    int selectedPoint;

    Vector3 cameraPosition;
    Vector3 cameraAngle;
    Vector3 modelCenter;
    Vector3 modelAngle;

    // 4x4 transform matrices
    Matrix4 matrixView;
    Matrix4 matrixModel;
    Matrix4 matrixModelView;
    Matrix4 matrixProjection;

    // vbo extensions
    bool vboSupported;
    GLuint vboVertex;               // vbo for OBJ vertices
    std::vector<GLuint> vboIndices; // vbo for OBJ indices

    // glsl extensions
    bool glslSupported;
    bool glslReady;
    GLhandleARB progId1;            // shader program with color
    GLhandleARB progId2;            // shader program with color + lighting

    // default material
    float defaultAmbient[4];
    float defaultDiffuse[4];
    float defaultSpecular[4];
    float defaultShininess;

};
#endif
