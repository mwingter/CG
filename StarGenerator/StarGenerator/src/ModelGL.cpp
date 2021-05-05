///////////////////////////////////////////////////////////////////////////////
// ModelGL.cpp
// ===========
// Model component of OpenGL
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2016-02-10
// UPDATED: 2016-02-25
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>    // include windows.h to avoid thousands of compile errors even though this class is not depending on Windows
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <cmath>
#include <sstream>
#include "ModelGL.h"
#include "glExtension.h"

// constants
const float GRID_SIZE = 10.0f;
const float GRID_STEP = 1.0f;
const float CAM_DIST = 20.0f;
const float DEG2RAD = 3.141593f / 180;
const float FOV_Y = 60.0f;
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 1000.0f;
const int   MAX_LOG_LENGTH = 4096;

// flat shading ===========================================
const char* vsSource1 = "\
void main() \
{ \
    gl_FrontColor = gl_Color; \
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; \
} \
";
const char* fsSource1 = "\
void main() \
{ \
    gl_FragColor = gl_Color; \
} \
";


// blinn shading ==========================================
const char* vsSource2 = "\
varying vec3 esVertex, esNormal; \
void main() \
{ \
    esVertex = vec3(gl_ModelViewMatrix * gl_Vertex); \
    esNormal = gl_NormalMatrix * gl_Normal; \
    gl_FrontColor = gl_Color; \
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; \
} \
";
const char* fsSource2 = "\
varying vec3 esVertex, esNormal; \
void main() \
{ \
    vec3 normal = normalize(esNormal); \
    vec3 light; \
    if(gl_LightSource[0].position.w == 0.0) \
    { \
        light = normalize(gl_LightSource[0].position.xyz); \
    } \
    else \
    { \
        light = normalize(gl_LightSource[0].position.xyz - esVertex); \
    } \
    vec3 view = normalize(-esVertex); \
    vec3 halfv = normalize(light + view); \
    vec4 color =  gl_FrontMaterial.ambient * gl_FrontLightProduct[0].ambient; \
    float dotNL = max(dot(normal, light), 0.0); \
    color += gl_FrontMaterial.diffuse * gl_FrontLightProduct[0].diffuse * dotNL; \
    float dotNH = max(dot(normal, halfv), 0.0); \
    /*vec4 specular = (vec4(1.0) - color) * gl_FrontMaterial.specular * gl_FrontLightProduct[0].specular * pow(dotNH, gl_FrontMaterial.shininess); \
    color += specular;*/ \
    color += gl_FrontMaterial.specular * gl_FrontLightProduct[0].specular * pow(dotNH, gl_FrontMaterial.shininess); \
    gl_FragColor = color; \
} \
";



///////////////////////////////////////////////////////////////////////////////
// default ctor
///////////////////////////////////////////////////////////////////////////////
ModelGL::ModelGL() : windowWidth(0), windowHeight(0), mouseLeftDown(false),
                     mouseRightDown(false), windowSizeChanged(false),
                     nearPlane(NEAR_PLANE), farPlane(FAR_PLANE),
                     fillEnabled(true), edgeEnabled(true), pointEnabled(true),
                     gridEnabled(true), gridSize(GRID_SIZE), gridStep(GRID_STEP),
                     vboSupported(false), vboVertex(0), glslSupported(false),
                     glslReady(false), progId1(0), progId2(0)
{
    bgColor.set(0, 0, 0, 0);

    // model, view, modelview matrices are fixed in this app
    matrixView.identity();
    matrixModel.identity();
    matrixModelView.identity();
    matrixProjection.identity();

    // init view matrix
    cameraPosition.set(0, 0, CAM_DIST);
    updateViewMatrix();

    // init default material
    defaultAmbient[0] = defaultAmbient[1] = defaultAmbient[2] = 0.3f; defaultAmbient[3] = 1.0f;
    defaultDiffuse[0] = defaultDiffuse[1] = defaultDiffuse[2] = 1.0f; defaultDiffuse[3] = 1.0f;
    defaultSpecular[0] = defaultSpecular[1] = defaultSpecular[2] = 0.5f; defaultSpecular[3] = 1.0f;
    defaultShininess = 128.0f;
}



///////////////////////////////////////////////////////////////////////////////
// destructor
///////////////////////////////////////////////////////////////////////////////
ModelGL::~ModelGL()
{
}



///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL states and scene
///////////////////////////////////////////////////////////////////////////////
void ModelGL::init()
{
    glShadeModel(GL_SMOOTH);                        // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);          // 4-byte pixel alignment

    // enable /disable features
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    //glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);   // background color
    glClearStencil(0);                              // clear stencil buffer
    glClearDepth(1.0f);                             // 0 is near, 1 is far
    glDepthFunc(GL_LEQUAL);

    initLights();

    // check extensions
    glExtension& extension = glExtension::getInstance();
    glslSupported = extension.isSupported("GL_ARB_shader_objects");
    if(glslSupported)
        createShaderPrograms();

    vboSupported = extension.isSupported("GL_ARB_vertex_buffer_object");
    if(vboSupported)
        createVertexBufferObjects();
}



///////////////////////////////////////////////////////////////////////////////
// clean up OpenGL objects
///////////////////////////////////////////////////////////////////////////////
void ModelGL::quit()
{
}



///////////////////////////////////////////////////////////////////////////////
// initialize lights
///////////////////////////////////////////////////////////////////////////////
void ModelGL::initLights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightKa[] = {.2f, .2f, .2f, 1.0f};      // ambient light
    GLfloat lightKd[] = {.8f, .8f, .8f, 1.0f};      // diffuse light
    GLfloat lightKs[] = {1, 1, 1, 1};               // specular light
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightKa);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightKd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightKs);

    // position the light in eye space
    float lightPos[4] = {0, 0, 1, 0};               // directional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);                            // MUST enable each light source after configuration
}



///////////////////////////////////////////////////////////////////////////////
// set camera position and lookat direction
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setCamera(float posX, float posY, float posZ, float targetX, float targetY, float targetZ)
{
    float forward[4];
    float up[4];
    float left[4];
    float position[4];
    float invLength;

    // determine forward vector (direction reversed because it is camera)
    forward[0] = posX - targetX;    // x
    forward[1] = posY - targetY;    // y
    forward[2] = posZ - targetZ;    // z
    forward[3] = 0.0f;              // w
    // normalize it without w-component
    invLength = 1.0f / sqrtf(forward[0]*forward[0] + forward[1]*forward[1] + forward[2]*forward[2]);
    forward[0] *= invLength;
    forward[1] *= invLength;
    forward[2] *= invLength;

    // assume up direction is straight up
    up[0] = 0.0f;   // x
    up[1] = 1.0f;   // y
    up[2] = 0.0f;   // z
    up[3] = 0.0f;   // w

    // compute left vector with cross product
    left[0] = up[1]*forward[2] - up[2]*forward[1];  // x
    left[1] = up[2]*forward[0] - up[0]*forward[2];  // y
    left[2] = up[0]*forward[1] - up[1]*forward[0];  // z
    left[3] = 1.0f;                                 // w

    // re-compute orthogonal up vector
    up[0] = forward[1]*left[2] - forward[2]*left[1];    // x
    up[1] = forward[2]*left[0] - forward[0]*left[2];    // y
    up[2] = forward[0]*left[1] - forward[1]*left[0];    // z
    up[3] = 0.0f;                                       // w

    // camera position
    position[0] = -posX;
    position[1] = -posY;
    position[2] = -posZ;
    position[3] = 1.0f;

    // copy axis vectors to matrix
    matrixView.identity();
    matrixView.setColumn(0, left);
    matrixView.setColumn(1, up);
    matrixView.setColumn(2, forward);
    matrixView.setColumn(3, position);
}



///////////////////////////////////////////////////////////////////////////////
// set rendering window size
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setWindowSize(int width, int height)
{
    // assign the width/height of viewport
    windowWidth = width;
    windowHeight = height;
    windowSizeChanged = true;
}



///////////////////////////////////////////////////////////////////////////////
// configure projection and viewport
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setViewport(int x, int y, int w, int h)
{
    // set viewport to be the entire window
    glViewport((GLsizei)x, (GLsizei)y, (GLsizei)w, (GLsizei)h);

    // set perspective viewing frustum
    setFrustum(FOV_Y, (float)(w)/h, nearPlane, farPlane); // FOV, AspectRatio, NearClip, FarClip
}



///////////////////////////////////////////////////////////////////////////////
// configure star
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setStar(int pointCount, float radius)
{
    star.set(pointCount, radius);
}

void ModelGL::setStarPointCount(int count)
{
    if(count != star.getPointCount())
        star.setPointCount(count);
}

void ModelGL::setStarOuterRadius(float radius)
{
    if(radius != star.getRadius())
        star.setRadius(radius);
}

void ModelGL::setStarInnerRadius(float radius)
{
    if(radius != star.getInnerRadius())
        star.setInnerRadius(radius);
}

void ModelGL::setStarSelectedPoint(int index)
{
    int pointCount = star.getPointCount() * 2; // both inner/outer points
    if(index < 0 || index >= pointCount)
        selectedPoint = -1;

    selectedPoint = index;
}



///////////////////////////////////////////////////////////////////////////////
// draw 2D/3D scene
///////////////////////////////////////////////////////////////////////////////
void ModelGL::draw()
{
    preFrame();

    // clear buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // pass projection matrix to OpenGL
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(matrixProjection.get());
    glMatrixMode(GL_MODELVIEW);

    // draw grid and axis
    if(gridEnabled)
    {
        // copy  the current ModelView matrix to OpenGL after transpose
        glLoadMatrixf(matrixView.get());
        drawGrid(gridSize, gridStep);
    }

    // draw star
    glLoadMatrixf(matrixModelView.get());
    drawStar();

    postFrame();
}



///////////////////////////////////////////////////////////////////////////////
// pre-frame
///////////////////////////////////////////////////////////////////////////////
void ModelGL::preFrame()
{
    if(windowSizeChanged)
    {
        setViewport(0, 0, windowWidth, windowHeight);
        windowSizeChanged = false;
    }
}


///////////////////////////////////////////////////////////////////////////////
// post-frame
///////////////////////////////////////////////////////////////////////////////
void ModelGL::postFrame()
{
}



///////////////////////////////////////////////////////////////////////////////
// rotate the camera
///////////////////////////////////////////////////////////////////////////////
void ModelGL::rotateCamera(int x, int y)
{
    cameraAngle.y -= (x - mouseX) * 0.5f;
    cameraAngle.x -= (y - mouseY) * 0.5f;
    mouseX = x;
    mouseY = y;

    // constrain x angle -89 < x < 89
    if(cameraAngle.x < -89.0f)
        cameraAngle.x = -89.0f;
    else if(cameraAngle.x > 89.0f)
        cameraAngle.x = 89.0f;

    updateViewMatrix();
}



///////////////////////////////////////////////////////////////////////////////
// zoom the camera
///////////////////////////////////////////////////////////////////////////////
void ModelGL::zoomCamera(int y)
{
    cameraPosition.z -= (y - mouseY) * 0.2f;
    mouseY = y;

    // constrain min and max
    if(cameraPosition.z < nearPlane)
        cameraPosition.z = nearPlane;
    else if(cameraPosition.z > farPlane)
        cameraPosition.z = farPlane;

    updateViewMatrix();
}
void ModelGL::zoomCameraDelta(int delta)
{
    cameraPosition.z += delta;

    // constrain min and max
    if(cameraPosition.z < nearPlane)
        cameraPosition.z = nearPlane;
    else if(cameraPosition.z > farPlane)
        cameraPosition.z = farPlane;

    updateViewMatrix();
}



///////////////////////////////////////////////////////////////////////////////
// set a perspective frustum with 6 params similar to glFrustum()
// (left, right, bottom, top, near, far)
// Note: this is for row-major notation. OpenGL needs transpose it
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setFrustum(float l, float r, float b, float t, float n, float f)
{
    matrixProjection.identity();
    matrixProjection[0]  =  2 * n / (r - l);
    matrixProjection[5]  =  2 * n / (t - b);
    matrixProjection[8]  =  (r + l) / (r - l);
    matrixProjection[9]  =  (t + b) / (t - b);
    matrixProjection[10] = -(f + n) / (f - n);
    matrixProjection[11] = -1;
    matrixProjection[14] = -(2 * f * n) / (f - n);
    matrixProjection[15] =  0;
}



///////////////////////////////////////////////////////////////////////////////
// set a symmetric perspective frustum with 4 params similar to gluPerspective
// (vertical field of view, aspect ratio, near, far)
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setFrustum(float fovY, float aspectRatio, float front, float back)
{
    float tangent = tanf(fovY/2 * DEG2RAD);   // tangent of half fovY
    float height = front * tangent;           // half height of near plane
    float width = height * aspectRatio;       // half width of near plane

    // params: left, right, bottom, top, near, far
    setFrustum(-width, width, -height, height, front, back);
}



///////////////////////////////////////////////////////////////////////////////
// set a orthographic frustum with 6 params similar to glOrtho()
// (left, right, bottom, top, near, far)
// Note: this is for row-major notation. OpenGL needs transpose it
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setOrthoFrustum(float l, float r, float b, float t, float n, float f)
{
    matrixProjection.identity();
    matrixProjection[0]  =  2 / (r - l);
    matrixProjection[5]  =  2 / (t - b);
    matrixProjection[10] = -2 / (f - n);
    matrixProjection[12]  =  -(r + l) / (r - l);
    matrixProjection[13]  =  -(t + b) / (t - b);
    matrixProjection[14] = -(f + n) / (f - n);
}



///////////////////////////////////////////////////////////////////////////////
// draw star
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawStar()
{
    if(glslReady)
        glUseProgramObjectARB(progId1);

    glDisable(GL_LIGHTING);

    int i, j, k;
    std::vector<Vector2> points = star.getPoints();
    int pointCount = (int)points.size();

    // draw triangles
    if(fillEnabled)
    {
        glColor3f(0.8f, 0.8f, 0.8f);
        glNormal3f(0, 0, 1);
        glBegin(GL_TRIANGLES);
        for(i = pointCount - 1; i > 0; i -= 2)
        {
            j = i - 1;
            k = i - 2;
            if(k < 0)
                k = pointCount - 1;

            glVertex2f(points[i].x, points[i].y);
            glVertex2f(points[j].x, points[j].y);
            glVertex2f(points[k].x, points[k].y);
        }
        glEnd();

        glBegin(GL_POLYGON);
        for(i = pointCount - 1; i > 0; i -= 2)
        {
            glVertex2f(points[i].x, points[i].y);
        }
        glEnd();
    }

    // draw edge lines
    if(edgeEnabled)
    {
        glDisable(GL_DEPTH_TEST);
        glLineWidth(3.0f);
        glColor3f(1.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);
        for(i = 0; i < pointCount - 1; ++i)
        {
            glVertex2f(points[i].x, points[i].y);
            glVertex2f(points[i+1].x, points[i+1].y);
        }
        glVertex2f(points[pointCount-1].x, points[pointCount-1].y);
        glVertex2f(points[0].x, points[0].y);
        glEnd();
    }

    // draw all points
    if(pointEnabled)
    {
        glPointSize(7);
        glDisable(GL_DEPTH_TEST);
        glBegin(GL_POINTS);
        glColor3f(0, 1, 0);
        glNormal3f(0, 0, 1);
        for(i = 0; i < pointCount; ++i)
        {
            glVertex2f(points[i].x, points[i].y);
        }
        glEnd();

        // selected point
        if(selectedPoint >= 0)
        {
            glColor3f(1, 0, 0);
            glPointSize(15);
            glBegin(GL_POINTS);
            glVertex2f(points[selectedPoint].x, points[selectedPoint].y);
            glEnd();
        }

        glPointSize(1);
        glEnable(GL_DEPTH_TEST);
    }

    glEnable(GL_LIGHTING);

    // reset shader to no-texturing & no-lighting
    if(glslReady)
        glUseProgramObjectARB(progId1);
}



///////////////////////////////////////////////////////////////////////////////
// draw star with VBOs
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawStarWithVbo()
{
    /*
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, vboVertex);

    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNormalPointer(GL_FLOAT, stride, (void*)(sizeof(float)*3));
    glVertexPointer(3, GL_FLOAT, stride, 0);
    if(glslReady)
        glUseProgramObjectARB(progId2);

    // material
    if(defaultLightingEnabled)
    {
        glMaterialfv(GL_FRONT, GL_AMBIENT, defaultAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, defaultDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, defaultSpecular);
        glMaterialf(GL_FRONT, GL_SHININESS, defaultShininess);
    }

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vboIndices[i]);
    glDrawElements(GL_TRIANGLES, obj.getIndexCount(i), GL_UNSIGNED_INT, 0);

    glDisableClientState(GL_VERTEX_ARRAY);  // disable vertex arrays
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

    // reset shader to no-texturing & no-lighting
    if(glslReady)
        glUseProgramObjectARB(progId1);
    */
}



///////////////////////////////////////////////////////////////////////////////
// draw a grid on the xy plane
///////////////////////////////////////////////////////////////////////////////
void ModelGL::drawGrid(float size, float step)
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(0.5f);

    glBegin(GL_LINES);

    glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
    for(float i=step; i <= size; i+= step)
    {
        glVertex3f(-size,  i, 0);   // lines parallel to X-axis
        glVertex3f( size,  i, 0);
        glVertex3f(-size, -i, 0);   // lines parallel to X-axis
        glVertex3f( size, -i, 0);

        glVertex3f( i, -size, 0);   // lines parallel to Y-axis
        glVertex3f( i,  size, 0);
        glVertex3f(-i, -size, 0);   // lines parallel to Y-axis
        glVertex3f(-i,  size, 0);
    }

    // x-axis
    glColor4f(1.0f, 0, 0, 0.5f);
    glVertex3f(-size, 0, 0);
    glVertex3f( size, 0, 0);

    // y-axis
    glColor4f(0, 0, 1.0f, 0.5f);
    glVertex3f(0, -size, 0);
    glVertex3f(0,  size, 0);

    glEnd();

    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}



///////////////////////////////////////////////////////////////////////////////
// reset camera
///////////////////////////////////////////////////////////////////////////////
void ModelGL::resetCamera()
{
    cameraPosition.set(0, 0, gridSize * 2);
    cameraAngle.set(0, 0, 0);
    updateViewMatrix();
}



///////////////////////////////////////////////////////////////////////////////
// update matrix
///////////////////////////////////////////////////////////////////////////////
void ModelGL::updateViewMatrix()
{
    // transform the camera (viewing matrix) from world space to eye space
    // Notice all values are negated, because we move the whole scene with the
    // inverse of camera transform matrix
    matrixView.identity();
    matrixView.translate(-modelCenter);
    matrixView.rotateZ(-cameraAngle.z);     // roll
    matrixView.rotateY(-cameraAngle.y);     // heading
    matrixView.rotateX(-cameraAngle.x);     // pitch
    matrixView.translate(-cameraPosition);

    matrixModelView = matrixView * matrixModel;
}

void ModelGL::updateModelMatrix()
{
    // transform objects from object space to world space
    // it rotates at the center of the model
    matrixModel.identity();
    matrixModel.translate(-modelCenter);
    matrixModel.rotateX(modelAngle.x);
    matrixModel.rotateY(modelAngle.y);
    matrixModel.rotateZ(modelAngle.z);
    matrixModel.translate(modelCenter);

    matrixModelView = matrixView * matrixModel;
}



///////////////////////////////////////////////////////////////////////////////
// compute grid size and step
///////////////////////////////////////////////////////////////////////////////
void ModelGL::setGridSize(float size)
{
    gridSize = size;
    gridStep = 1;

    // adjust viewing frustum
    cameraPosition.set(0, 0, gridSize * 2);
    updateViewMatrix();
}



///////////////////////////////////////////////////////////////////////////////
// create VBOs
///////////////////////////////////////////////////////////////////////////////
bool ModelGL::createVertexBufferObjects()
{
    if(!vboVertex)
        glGenBuffersARB(1, &vboVertex);

    return true;
}



///////////////////////////////////////////////////////////////////////////////
// create glsl programs
///////////////////////////////////////////////////////////////////////////////
bool ModelGL::createShaderPrograms()
{
    // create 1st shader and program
    GLhandleARB vsId1 = glCreateShaderObjectARB(GL_VERTEX_SHADER);
    GLhandleARB fsId1 = glCreateShaderObjectARB(GL_FRAGMENT_SHADER);
    progId1 = glCreateProgramObjectARB();

    // load shader sources: flat shader
    glShaderSourceARB(vsId1, 1, &vsSource1, NULL);
    glShaderSourceARB(fsId1, 1, &fsSource1, NULL);

    // compile shader sources
    glCompileShaderARB(vsId1);
    glCompileShaderARB(fsId1);

    // attach shaders to the program
    glAttachObjectARB(progId1, vsId1);
    glAttachObjectARB(progId1, fsId1);

    // link program
    glLinkProgramARB(progId1);

    // create 2nd shader and program
    GLhandleARB vsId2 = glCreateShaderObjectARB(GL_VERTEX_SHADER);
    GLhandleARB fsId2 = glCreateShaderObjectARB(GL_FRAGMENT_SHADER);
    progId2 = glCreateProgramObjectARB();

    // load shader sources: 
    glShaderSourceARB(vsId2, 1, &vsSource2, NULL);
    glShaderSourceARB(fsId2, 1, &fsSource2, NULL);

    // compile shader sources
    glCompileShaderARB(vsId2);
    glCompileShaderARB(fsId2);

    // attach shaders to the program
    glAttachObjectARB(progId2, vsId2);
    glAttachObjectARB(progId2, fsId2);

    // link program
    glLinkProgramARB(progId2);

    glUseProgramObjectARB(progId2);

    // check status
    int linkStatus1, linkStatus2;
    glGetObjectParameterivARB(progId1, GL_OBJECT_LINK_STATUS_ARB, &linkStatus1);
    glGetObjectParameterivARB(progId2, GL_OBJECT_LINK_STATUS_ARB, &linkStatus2);
    if(linkStatus1 == GL_TRUE && linkStatus2 == GL_TRUE)
    {
        glslReady = true;
    }
    else
    {
        glslReady = false;
    }

    return glslReady;
}
