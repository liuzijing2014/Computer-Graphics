/*
 CSCI 420 Computer Graphics, USC
 Assignment 2: Roller Coaster
 C++ starter code
 
 Student username: Zijing Liu
 */

#include <iostream>
#include <cstring>
#include <cstdio>
#include <vector>
#include <math.h>
#include <limits>
#include "openGLHeader.h"
#include "glutHeader.h"

#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"
#include <glm/glm.hpp>

#define DEBUG true

#ifdef WIN

#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#endif

#ifdef WIN32
char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

// Represent a vertex
typedef struct
{
    float XYZ[3];
    float RGBA[4];
    float TEX[2];
} Vertex;

// Data structure that stores all information that camera needs
// to simulate spline movement.
typedef struct
{
    glm::vec3 position;
    glm::vec3 tangent;
    glm::vec3 normal;
    glm::vec3 binormal;
} AnimationPoint;

// represents one control point along the spline
struct Point
{
    double x;
    double y;
    double z;
};

// spline struct
// contains how many control points the spline has, and an array of control points
struct Spline
{
    int numControlPoints;
    Point *points;
};

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0;   // 1 if pressed, 0 if not
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0;  // 1 if pressed, 0 if not

typedef enum { ROTATE,
               TRANSLATE,
               SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = {0.0f, 0.0f, 0.0f};
float landTranslate[3] = {0.0f, 0.0f, 0.0f};
float landScale[3] = {1.0f, 1.0f, 1.0f};
float cameraForward[3] = {0.0f, 0.0f, 0.0f};
float cameraUp[3] = {0.0f, 1.0f, 0.0f};

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

// Screenshot flag
bool screenshot = false;
int screenshotCount = 0;

// Texture handler
const char *mainImageFilename = "./textures/empty.jpg";
GLuint mTextureHandler;

const char *gdImageFilename = "./textures/sand.jpg";
GLuint gdTextureHandler;

const char *cbImageFilename = "./textures/wood.jpg";
GLuint cbTextureHandler;

const GLchar *faces[6] = {
    "./textures/right.jpg",
    "./textures/left.jpg",
    "./textures/bottom.jpg",
    "./textures/top.jpg",
    "./textures/back.jpg",
    "./textures/front.jpg"};

GLuint skyboxHandler;

// Vertiex array for the rails
vector<Vertex> verticesVector;
// Index array for the rails
vector<GLuint> indicesVector;

// Vertex array for the cross bars
vector<Vertex> crossbarVertices;
// Index array for the cross bars
vector<GLuint> crossbarIndices;

// AnimationPoint vector
vector<AnimationPoint> aniArray;
// Index counter for aniArray; 
GLint aniCounter = 0;

// Ground vertice
Vertex groundVertices[4] = {
    {{-64.0, 0.0, -64.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 0.0}},
    {{-64.0, 0.0, 64.0}, {1.0, 1.0, 1.0, 1.0}, {0.0, 32.0}},
    {{64.0, 0.0, 64.0}, {1.0, 1.0, 1.0, 1.0}, {32.0, 32.0}},
    {{64.0, 0.0, -64.0}, {1.0, 1.0, 1.0, 1.0}, {32.0, 0.0}},
};

// Ground indices
GLuint groundIndices[6] = {
    0, 1, 2,
    2, 3, 0};

// Skybox vertice
Vertex skyVertices[8] = {
    {{-128.0, 128.0, 128.0}, {1.0, 1.0, 1.0, -1.0}, {0.0, 0.0}},
    {{-128.0, -128.0, 128.0}, {1.0, 1.0, 1.0, -1.0}, {0.0, 0.0}},
    {{128.0, -128.0, 128.0}, {1.0, 1.0, 1.0, -1.0}, {0.0, 0.0}},
    {{128.0, 128.0, 128.0}, {1.0, 1.0, 1.0, -1.0}, {0.0, 0.0}},

    {{128.0, 128.0, -128.0}, {1.0, 1.0, 1.0, -1.0}, {0.0, 0.0}},
    {{-128.0, 128.0, -128.0}, {1.0, 1.0, 1.0, -1.0}, {0.0, 0.0}},
    {{-128.0, -128.0, -128.0}, {1.0, 1.0, 1.0, -1.0}, {0.0, 0.0}},
    {{128.0, -128.0, -128.0}, {1.0, 1.0, 1.0, -1.0}, {0.0, 0.0}},
};

// Skybox indices
GLuint skyboxIndices[36] = {
    5, 0, 3,
    3, 4, 5,
    0, 1, 2,
    2, 3, 0,
    6, 1, 2,
    2, 7, 6,
    5, 6, 1,
    1, 0, 5,
    5, 6, 7,
    4, 7, 2,
    7, 4, 5,
    2, 3, 4,
};

GLuint IndexRail; // rail index buffer handler
GLuint IndexGd;  // ground index buffer handler
GLuint IndexSky; // sky index buffer handler
GLuint IndexCb;  // cross bar index buffer handler

GLuint programId;
GLuint VaoId;
GLuint VaoGd;  // ground VAO
GLuint VaoSky; // skybox VAO
GLuint VaoCb; // cross bar VAO
GLuint BufferId;
GLuint BufferGd;  // ground buffer
GLuint BufferSky; // skybox buffer
GLuint BufferCb; // cross bar buffer

float sFactor = 1.0f;
float splineScale = 1.5f;
float railScale = 0.1f;

// Helper lib pointers
BasicPipelineProgram *pipelineProgram;
OpenGLMatrix *openGLMatrix;

// the spline array
Spline *splines;

// total number of splines
int numSplines;

// Base Matrix
glm::mat4x4 basisMatrix = glm::mat4x4(-0.5, 1.0, -0.5, 0.0,
                                      1.5, -2.5, 0.0, 1.0,
                                      -1.5, 2.0, 0.5, 0.0,
                                      0.5, -0.5, 0.0, 0.0);

// Max distance
float maxDistance = 0.05f;
// lowest/highest spline altitude
float lowest = FLT_MAX;
float highest = -FLT_MAX;

// physics related variables
int oldTime = 0;
float velocity = 1.0f;

// Functions decleared
void initBuffer();
void initVertex();
void initPerspective(int, int);
void setupMatrex();
void subDivideSegment(float, float, glm::mat4x4&);
GLuint loadSkybox(const GLchar);
void addAnimationPoint(float, glm::vec3&, glm::mat4x4&);
void addRailVertices(glm::vec3&, glm::vec3&, glm::vec3&);
void addRailIndices(GLuint);
void addCrossbarVertices(glm::vec3&, glm::vec3&, glm::vec3&, glm::vec3&);
void updateGroundHeight();

// write a screenshot to the specified filename
void saveScreenshot(const char *filename)
{
    unsigned char *screenshotData = new unsigned char[windowWidth * windowHeight * 4];
    glReadPixels(0, 0, windowWidth, windowHeight, GL_RGBA, GL_UNSIGNED_BYTE, screenshotData);

    ImageIO screenshotImg(windowWidth, windowHeight, 4, screenshotData);

    if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
        cout << "File " << filename << " saved successfully." << endl;
    else
        cout << "Failed to save file " << filename << '.' << endl;

    delete[] screenshotData;
}

void displayFunc()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Draw skybox
    pipelineProgram->Bind();
    setupMatrex();

    glDepthMask(GL_FALSE);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxHandler);
    glBindVertexArray(VaoSky);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLvoid *)0);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);

    // Draw rails
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, mTextureHandler);
    glBindVertexArray(VaoId);
    glDrawElements(GL_TRIANGLES, indicesVector.size(), GL_UNSIGNED_INT, (GLvoid *)0);
    glBindVertexArray(0);

    // Draw cross bar
    glBindTexture(GL_TEXTURE_2D, cbTextureHandler);
    glBindVertexArray(VaoCb);
    glDrawElements(GL_TRIANGLES, crossbarIndices.size(), GL_UNSIGNED_INT, (GLvoid *)0);
    glBindVertexArray(0);   

    glBindTexture(GL_TEXTURE_2D, gdTextureHandler);
    glBindVertexArray(VaoGd);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (GLvoid*)0);
    glBindVertexArray(0);

    glFlush();
    glutSwapBuffers();
}

// Setup modelview and upload the matrix to GPU
void setupMatrex()
{
    static float preHeight;
    glm::vec3 &position = aniArray[aniCounter].position;
    glm::vec3 &tangent = aniArray[aniCounter].tangent;
    glm::vec3 &normal = aniArray[aniCounter].normal;

    // if(aniCounter == 0)
    // {
    //     preHeight = position.y;
    //     aniCounter += 2;
    // }
    // else if(position.y > preHeight && (aniCounter + 1 < aniArray.size())) 
    // {
    //     aniCounter++;
    // }
    // else if(position.y == preHeight && (aniCounter + 2 < aniArray.size()))
    // {
    //     aniCounter += 2;
    // }
    // else if(position.y < preHeight && (aniCounter + 3 < aniArray.size()))
    // {
    //     aniCounter += 3;
    // }

    if((aniCounter + 2) < aniArray.size())
    {
        aniCounter += 2;
    }

    //change mode to modelview
    openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
    openGLMatrix->LoadIdentity();
    openGLMatrix->LookAt(position, position + tangent, normal);
    //openGLMatrix->LookAt(0.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    openGLMatrix->Rotate(landRotate[0], 1.0, 0.0, 0.0);
    openGLMatrix->Rotate(landRotate[1], 0.0, 1.0, 0.0);
    openGLMatrix->Rotate(landRotate[2], 0.0, 0.0, 1.0);
    openGLMatrix->Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
    openGLMatrix->Scale(landScale[0] * sFactor, landScale[1] * sFactor, landScale[2] * sFactor);

    // Setup modelview matrix
    float m[16];
    openGLMatrix->GetMatrix(m);
    pipelineProgram->SetModelViewMatrix(m);
}

void idleFunc()
{
    // do some stuff...

    // for example, here, you can save the screenshots to disk (to make the animation)
    if (screenshot)
    {
        std::string fname = "frame" + std::to_string(screenshotCount++) + ".jpg";
        saveScreenshot(fname.c_str());
    }

    // make the screen update
    glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
    glViewport(0, 0, w, h);

    // setup perspective matrix...
    initPerspective(w, h);
}

void initPerspective(int w, int h)
{
    openGLMatrix->SetMatrixMode(OpenGLMatrix::Projection);
    openGLMatrix->LoadIdentity();
    float aspect = (float)w / (float)h;
    openGLMatrix->Perspective(60.0f, aspect, 0.01, 1000.0);

    // Setup projection matrix
    float p[16];
    openGLMatrix->GetMatrix(p);
    pipelineProgram->SetProjectionMatrix(p);
    openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);

    // Reset aniCounter
    aniCounter = 0;
}

void mouseMotionDragFunc(int x, int y)
{
    // mouse has moved and one of the mouse buttons is pressed (dragging)

    // the change in mouse position since the last invocation of this function
    int mousePosDelta[2] = {x - mousePos[0], y - mousePos[1]};

    switch (controlState)
    {
    // translate the landscape
    case TRANSLATE:
        if (leftMouseButton)
        {
            // control x,y translation via the left mouse button
            landTranslate[0] += mousePosDelta[0] * 0.01f;
            landTranslate[1] -= mousePosDelta[1] * 0.01f;
        }
        if (middleMouseButton)
        {
            // control z translation via the middle mouse button
            landTranslate[2] += mousePosDelta[1] * 0.01f;
        }
        break;

    // rotate the landscape
    case ROTATE:
        if (leftMouseButton)
        {
            // control x,y rotation via the left mouse button
            landRotate[0] += mousePosDelta[1];
            landRotate[1] += mousePosDelta[0];
        }
        if (middleMouseButton)
        {
            // control z rotation via the middle mouse button
            landRotate[2] += mousePosDelta[1];
        }
        break;

    // scale the landscape
    case SCALE:
        if (leftMouseButton)
        {
            // control x,y scaling via the left mouse button
            landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
            landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
        }
        if (middleMouseButton)
        {
            // control z scaling via the middle mouse button
            landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
        }
        break;
    }

    // store the new mouse position
    mousePos[0] = x;
    mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
    // mouse has moved
    // store the new mouse position
    mousePos[0] = x;
    mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
    // a mouse button has has been pressed or depressed

    // keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        leftMouseButton = (state == GLUT_DOWN);
        break;

    case GLUT_MIDDLE_BUTTON:
        middleMouseButton = (state == GLUT_DOWN);
        break;

    case GLUT_RIGHT_BUTTON:
        rightMouseButton = (state == GLUT_DOWN);
        break;
    }

    // keep track of whether CTRL and SHIFT keys are pressed
    switch (glutGetModifiers())
    {
    case GLUT_ACTIVE_CTRL:
        controlState = TRANSLATE;
        break;

    case GLUT_ACTIVE_SHIFT:
        controlState = SCALE;
        break;

    // if CTRL and SHIFT are not pressed, we are in rotate mode
    default:
        controlState = ROTATE;
        break;
    }

    // store the new mouse position
    mousePos[0] = x;
    mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:     // ESC key
        exit(0); // exit the program
        break;
    case ' ':
        if (screenshot)
        {
            screenshot = false;
            cout << "Stop recording frames" << endl;
        }
        else
        {
            screenshot = true;
            cout << "Start recording frames" << endl;
        }
        break;
    case 'x':
        // take a screenshot
        saveScreenshot("screenshot.jpg");
        break;
    }
}

// Initilize the vertices
void initVertex()
{
    for (int i = 0; i < numSplines; i++)
    {
        int splineLength = splines[i].numControlPoints;
        Spline &segment = splines[i];
        for (int j = 0; j < splineLength - 3; j++)
        {
            glm::mat4x4 controlMatrix = glm::mat4x4(segment.points[j].x, segment.points[j + 1].x, segment.points[j + 2].x, segment.points[j + 3].x,
                                                    segment.points[j].y, segment.points[j + 1].y, segment.points[j + 2].y, segment.points[j + 3].y,
                                                    segment.points[j].z, segment.points[j + 1].z, segment.points[j + 2].z, segment.points[j + 3].z,
                                                    0.0, 0.0, 0.0, 0.0);
            glm::mat4x4 combinedMatrix = basisMatrix * controlMatrix;
            subDivideSegment(0.0, 1.0, combinedMatrix);
        }
    }
}
/*
 * Based on u and given combined matrix of control matrix and basis matrix,
 * calculate the current AnimationPoint at u.
 */
void addAnimationPoint(float u, glm::vec3& v, glm::mat4x4 &matrix)
{
    static bool firstPoint = true;
    // The first time calling this function, we need to generate the first point
    // and its associated tangent, normal, and binormal.
    if(firstPoint)
    {
        #ifdef DEBUG
            printf("Generate the first point\n");
        #endif
        AnimationPoint p;
        p.tangent = glm::normalize(glm::vec3(glm::vec4(0.0, 0.0, 1, 0) * matrix));
        p.normal = glm::normalize(glm::cross(p.tangent, glm::vec3(1.0, 0.0, 0.0)));
        p.binormal = glm::normalize(glm::cross(p.tangent, p.normal));
        p.position = glm::vec3(glm::vec4(0.0, 0.0, 0.0, 1.0) * matrix) * splineScale;
        aniArray.push_back(p);
        firstPoint = false;
    }
#ifdef DEBUG
    //printf("Added Animation Point at U = %f\n", u);
#endif
    AnimationPoint p;

    // Calculate the Tangent t(u) = p'(u) = [3u^2 2u 1 0] M C
    p.tangent = glm::normalize(glm::vec3(glm::vec4(3 * pow(u, 2.0), 2 * u, 1, 0) * matrix));

    // Calculate normal, N = cross(T, B)
    p.normal = glm::normalize(glm::cross(aniArray[aniCounter].binormal, p.tangent));

    // Now calculate binormal by crossing tangent and normal
    p.binormal = glm::normalize(glm::cross(p.tangent, p.normal));

    // Set position
    p.position = v;

    aniArray.push_back(p);
    aniCounter++;
}

// Subdivide the spline segment to generate vertices
void subDivideSegment(float u0, float u1, glm::mat4x4 &matrix)
{
    static bool firstPoint = true;
    static GLuint drawCrossbar = 0;
    static GLuint crossbarDis = 35;

    float uMid = (u0 + u1) / 2.0;
    glm::vec3 v0 = glm::vec3(glm::vec4(pow(u0, 3.0), pow(u0, 2.0), u0, 1.0) * matrix);
    glm::vec3 v1 = glm::vec3(glm::vec4(pow(u1, 3.0), pow(u1, 2.0), u1, 1.0) * matrix);

    if (glm::distance(v1, v0) > maxDistance)
    {
        subDivideSegment(u0, uMid, matrix);
        subDivideSegment(uMid, u1, matrix);
    }
    else
    {


        v1 *= splineScale;
        // update lowest/highest
        if(v1.z < lowest)
        {
            lowest = v1.z;
        }
        else if(v1.z > highest)
        {
            highest = v1.z;
        }

        addAnimationPoint(u1, v1, matrix);

        if(firstPoint)
        {
            #ifdef DEBUG
                printf("first point to add position, u = %f\n", u0);
            #endif

            v0 *= splineScale;
            if(v0.z < lowest)
            {
                lowest = v0.z;
            }
            else if(v0.z > highest)
            {
                highest = v0.z;
            }
            addRailVertices(v0, aniArray[0].normal, aniArray[0].binormal);
            firstPoint = false;
        }

        GLuint index0 = verticesVector.size();
        addRailVertices(v1, aniArray[aniCounter].normal, aniArray[aniCounter].binormal);
        addRailIndices(index0);

        // Check if we need to add a cross bar
        if(drawCrossbar >= crossbarDis)
        {
            addCrossbarVertices(v1, aniArray[aniCounter].normal, aniArray[aniCounter].binormal, aniArray[aniCounter].tangent);
            drawCrossbar = 0;
        }
        else
        {
            drawCrossbar++;
        }
    }
}

void addCrossbarVertices(glm::vec3 &position, glm::vec3 &normal, glm::vec3 &binormal, glm::vec3& tangent)
{
    glm::vec3 curPos;
    GLuint index0 = crossbarVertices.size();
    Vertex v;
    v.RGBA[0] = v.RGBA[1] = v.RGBA[2] = v.RGBA[3] = 1.0;

    // top four vertices
    curPos = position - 3 * railScale * binormal - 2.0f * railScale * normal - 0.75f * railScale * tangent;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    v.TEX[0] = 0.0f;
    v.TEX[1] = 0.0f;
    crossbarVertices.push_back(v);

    curPos = position - 3 * railScale * binormal - 2.0f * railScale * normal + 0.75f * railScale * tangent;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    v.TEX[0] = 0.0f;
    v.TEX[1] = 1.0f;
    crossbarVertices.push_back(v);

    curPos = position + 3 * railScale * binormal - 2.0f * railScale * normal + 0.75f * railScale * tangent;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    v.TEX[0] = 1.0f;
    v.TEX[1] = 1.0f;
    crossbarVertices.push_back(v);

    curPos = position + 3 * railScale * binormal - 2.0f * railScale * normal - 0.75f * railScale * tangent;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    v.TEX[0] = 1.0f;
    v.TEX[1] = 0.0f;
    crossbarVertices.push_back(v);

    // bottom four vertices
    curPos = position - 3 * railScale * binormal - 2.5f * railScale * normal - 0.75f * railScale * tangent;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    v.TEX[0] = 1.0f;
    v.TEX[1] = 1.0f;
    crossbarVertices.push_back(v);

    curPos = position - 3 * railScale * binormal - 2.5f * railScale * normal + 0.75f * railScale * tangent;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    v.TEX[0] = 1.0f;
    v.TEX[1] = 0.0f;
    crossbarVertices.push_back(v);

    curPos = position + 3 * railScale * binormal - 2.5f * railScale * normal + 0.75f * railScale * tangent;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    v.TEX[0] = 0.0f;
    v.TEX[1] = 0.0f;
    crossbarVertices.push_back(v);

    curPos = position + 3 * railScale * binormal - 2.5f * railScale * normal - 0.75f * railScale * tangent;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    v.TEX[0] = 0.0f;
    v.TEX[1] = 1.0f;
    crossbarVertices.push_back(v);

    // Add indices
    crossbarIndices.push_back(index0);
    crossbarIndices.push_back(index0+1);
    crossbarIndices.push_back(index0+5);

    crossbarIndices.push_back(index0+5);
    crossbarIndices.push_back(index0+4);
    crossbarIndices.push_back(index0);

    crossbarIndices.push_back(index0);
    crossbarIndices.push_back(index0+1);
    crossbarIndices.push_back(index0+2);

    crossbarIndices.push_back(index0+2);
    crossbarIndices.push_back(index0+3);
    crossbarIndices.push_back(index0);

    crossbarIndices.push_back(index0+3);
    crossbarIndices.push_back(index0+2);
    crossbarIndices.push_back(index0+6);
    
    crossbarIndices.push_back(index0+6);
    crossbarIndices.push_back(index0+7);
    crossbarIndices.push_back(index0+3);

    crossbarIndices.push_back(index0+4);
    crossbarIndices.push_back(index0+5);
    crossbarIndices.push_back(index0+6);

    crossbarIndices.push_back(index0+6);
    crossbarIndices.push_back(index0+7);
    crossbarIndices.push_back(index0+4);

    crossbarIndices.push_back(index0);
    crossbarIndices.push_back(index0+3);
    crossbarIndices.push_back(index0+7);

    crossbarIndices.push_back(index0+7);
    crossbarIndices.push_back(index0+4);
    crossbarIndices.push_back(index0);

    crossbarIndices.push_back(index0+1);
    crossbarIndices.push_back(index0+2);
    crossbarIndices.push_back(index0+6);

    crossbarIndices.push_back(index0+6);
    crossbarIndices.push_back(index0+5);
    crossbarIndices.push_back(index0+1);
}

/*
 * Calculate the indices for triangles
 */
void addRailIndices(GLuint index0)
{
    // Add indices
    // leftmost face
    indicesVector.push_back(index0 - 8);
    indicesVector.push_back(index0);
    indicesVector.push_back(index0 + 4);

    indicesVector.push_back(index0 + 4);
    indicesVector.push_back(index0 - 4);
    indicesVector.push_back(index0 - 8);

    // left top face
    indicesVector.push_back(index0 - 8);
    indicesVector.push_back(index0);
    indicesVector.push_back(index0 + 1);

    indicesVector.push_back(index0 + 1);
    indicesVector.push_back(index0 - 7);
    indicesVector.push_back(index0 - 8);

    // 2nd leftmost face
    indicesVector.push_back(index0 - 7);
    indicesVector.push_back(index0 + 1);
    indicesVector.push_back(index0 + 5);

    indicesVector.push_back(index0 + 5);
    indicesVector.push_back(index0 - 3);
    indicesVector.push_back(index0 - 7);

    // left bottom face
    indicesVector.push_back(index0 - 4);
    indicesVector.push_back(index0 + 4);
    indicesVector.push_back(index0 + 5);

    indicesVector.push_back(index0 + 5);
    indicesVector.push_back(index0 - 3);
    indicesVector.push_back(index0 - 4);

    // 2nd rightmost face
    indicesVector.push_back(index0 - 6);
    indicesVector.push_back(index0 + 2);
    indicesVector.push_back(index0 + 6);

    indicesVector.push_back(index0 + 6);
    indicesVector.push_back(index0 - 2);
    indicesVector.push_back(index0 - 6);

    // right top face
    indicesVector.push_back(index0 - 6);
    indicesVector.push_back(index0 + 2);
    indicesVector.push_back(index0 + 3);

    indicesVector.push_back(index0 + 3);
    indicesVector.push_back(index0 - 5);
    indicesVector.push_back(index0 - 6);

    // rightmost face
    indicesVector.push_back(index0 - 5);
    indicesVector.push_back(index0 + 3);
    indicesVector.push_back(index0 + 7);

    indicesVector.push_back(index0 + 7);
    indicesVector.push_back(index0 - 1);
    indicesVector.push_back(index0 - 5);

    // right bottom face
    indicesVector.push_back(index0 - 2);
    indicesVector.push_back(index0 + 6);
    indicesVector.push_back(index0 + 7);

    indicesVector.push_back(index0 + 7);
    indicesVector.push_back(index0 - 1);
    indicesVector.push_back(index0 - 2);
}

/*
 * Given a point on the spline, generate 8 vertices around
 * this point to form double rail.
 *          top left        2nd top left        2nd top right       top right
 *          bottom left     2nd bottom left     2nd bottom right    bottom right
 */
void addRailVertices(glm::vec3 &position, glm::vec3 &normal, glm::vec3 &binormal)
{
    glm::vec3 curPos;
    Vertex v;
    v.RGBA[0] = v.RGBA[1] = v.RGBA[2] = v.RGBA[3] = 1.0;
    v.TEX[0] = 0.1f;
    v.TEX[1] = 0.8f;

    // top left
    curPos = position - 2 * railScale * binormal - 1.5f * railScale * normal;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    verticesVector.push_back(v);

    // 2nd top left
    curPos = position - 1 * railScale * binormal - 1.5f * railScale * normal;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    verticesVector.push_back(v);

    // 2nd top right
    curPos = position + 1 * railScale * binormal - 1.5f * railScale * normal;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    verticesVector.push_back(v);

    // top right
    curPos = position + 2 * railScale * binormal - 1.5f * railScale * normal;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    verticesVector.push_back(v);

    // bottom left
    curPos = position - 2 * railScale * binormal - 2.0f * railScale * normal;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    verticesVector.push_back(v);

    // 2nd bottom left
    curPos = position - 1 * railScale * binormal - 2.0f * railScale * normal;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    verticesVector.push_back(v);

    // 2nd bottom right
    curPos = position + 1 * railScale * binormal - 2.0f * railScale * normal;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    verticesVector.push_back(v);

    // bottom right
    curPos = position + 2 * railScale * binormal - 2.0f * railScale * normal;
    v.XYZ[0] = curPos.x;
    v.XYZ[1] = curPos.y;
    v.XYZ[2] = curPos.z;
    verticesVector.push_back(v);
}

void initBuffer()
{
    const size_t BufferSize = sizeof(Vertex) * verticesVector.size();
    const size_t VertexSize = sizeof(verticesVector[0]);
    const size_t RgbOffset = sizeof(verticesVector[0].XYZ);
    const size_t TexOffset = sizeof(verticesVector[0].XYZ) + sizeof(verticesVector[0].RGBA);

    GLuint pos = glGetAttribLocation(programId, "position");
    GLuint col = glGetAttribLocation(programId, "color");
    GLuint tex = glGetAttribLocation(programId, "texcoord");

    // Generate rail VBO and VAO
    glGenVertexArrays(1, &VaoId);
    glBindVertexArray(VaoId);

    glGenBuffers(1, &BufferId);
    glBindBuffer(GL_ARRAY_BUFFER, BufferId);
    glBufferData(GL_ARRAY_BUFFER, BufferSize, verticesVector.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, VertexSize, 0);
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(col, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid *)RgbOffset);
    glEnableVertexAttribArray(col);
    glVertexAttribPointer(tex, 2, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid *)TexOffset);
    glEnableVertexAttribArray(tex);

    glGenBuffers(1, &IndexRail);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexRail);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indicesVector.size(), indicesVector.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    // Generate the crossbar VAO and VBO
    glGenVertexArrays(1, &VaoCb);
    glBindVertexArray(VaoCb);

    glGenBuffers(1, &BufferCb);
    glBindBuffer(GL_ARRAY_BUFFER, BufferCb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * crossbarVertices.size(), crossbarVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, VertexSize, 0);
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(col, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid *)RgbOffset);
    glEnableVertexAttribArray(col);
    glVertexAttribPointer(tex, 2, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid *)TexOffset);
    glEnableVertexAttribArray(tex);

    glGenBuffers(1, &IndexCb);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexCb);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * crossbarIndices.size(), crossbarIndices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    //Generate the ground VAO
    glGenVertexArrays(1, &VaoGd);
    glBindVertexArray(VaoGd);

    glGenBuffers(1, &BufferGd);
    glBindBuffer(GL_ARRAY_BUFFER, BufferGd);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 4, groundVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, VertexSize, 0);
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(col, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid *)RgbOffset);
    glEnableVertexAttribArray(col);
    glVertexAttribPointer(tex, 2, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid *)TexOffset);
    glEnableVertexAttribArray(tex);

    glGenBuffers(1, &IndexGd);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexGd);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 6, groundIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);

    // Initiliaize skybox
    glGenVertexArrays(1, &VaoSky);
    glBindVertexArray(VaoSky);

    glGenBuffers(1, &BufferSky);
    glBindBuffer(GL_ARRAY_BUFFER, BufferSky);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 8, skyVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, VertexSize, 0);
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(col, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid *)RgbOffset);
    glEnableVertexAttribArray(col);
    glVertexAttribPointer(tex, 2, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid *)TexOffset);
    glEnableVertexAttribArray(tex);

    glGenBuffers(1, &IndexSky);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexSky);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * 36, skyboxIndices, GL_STATIC_DRAW);

    glBindVertexArray(0);
}

GLuint loadSkybox()
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    for (GLuint i = 0; i < 6; i++)
    {
        // read the texture image
        ImageIO img;
        ImageIO::fileFormatType imgFormat;
        ImageIO::errorType err = img.load(faces[i], &imgFormat);

        if (err != ImageIO::OK)
        {
            printf("Loading texture from %s failed.\n", faces[i]);
            return -1;
        }

        // check that the number of bytes is a multiple of 4
        if (img.getWidth() * img.getBytesPerPixel() % 4)
        {
            printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", faces[i]);
            return -1;
        }

        // allocate space for an array of pixels
        int width = img.getWidth();
        int height = img.getHeight();
        unsigned char *pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

        // fill the pixelsRGBA array with the image pixels
        memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
        for (int h = 0; h < height; h++)
        {
            for (int w = 0; w < width; w++)
            {
                // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
                pixelsRGBA[4 * (h * width + w) + 0] = 0;   // red
                pixelsRGBA[4 * (h * width + w) + 1] = 0;   // green
                pixelsRGBA[4 * (h * width + w) + 2] = 0;   // blue
                pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

                // set the RGBA channels, based on the loaded image
                int numChannels = img.getBytesPerPixel();
                for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
                    pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
            }
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

        delete[] pixelsRGBA;
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}

void updateGroundHeight()
{
    for (int i = 0; i < 4; i++)
    {
        groundVertices[i].XYZ[1] = lowest - 10.0f;
    }
}

int initTexture(const char *imageFilename, GLuint textureHandle)
{

    // read the texture image
    ImageIO img;
    ImageIO::fileFormatType imgFormat;
    ImageIO::errorType err = img.load(imageFilename, &imgFormat);

    if (err != ImageIO::OK)
    {
        printf("Loading texture from %s failed.\n", imageFilename);
        return -1;
    }

    // check that the number of bytes is a multiple of 4
    if (img.getWidth() * img.getBytesPerPixel() % 4)
    {
        printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
        return -1;
    }

    // allocate space for an array of pixels
    int width = img.getWidth();
    int height = img.getHeight();
    unsigned char *pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

    // fill the pixelsRGBA array with the image pixels
    memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
    for (int h = 0; h < height; h++)
        for (int w = 0; w < width; w++)
        {
            // assign some default byte values (for the case where img.getBytesPerPixel() < 4)
            pixelsRGBA[4 * (h * width + w) + 0] = 0;   // red
            pixelsRGBA[4 * (h * width + w) + 1] = 0;   // green
            pixelsRGBA[4 * (h * width + w) + 2] = 0;   // blue
            pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

            // set the RGBA channels, based on the loaded image
            int numChannels = img.getBytesPerPixel();
            for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
                pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
        }

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    // initialize the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);

    // generate the mipmaps for this texture
    glGenerateMipmap(GL_TEXTURE_2D);

    // set the texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // query support for anisotropic texture filtering
    GLfloat fLargest;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
    printf("Max available anisotropic samples: %f\n", fLargest);
    // set anisotropic texture filtering
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

    // query for any errors
    GLenum errCode = glGetError();
    if (errCode != 0)
    {
        printf("Texture initialization error. Error code: %d.\n", errCode);
        return -1;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    // de-allocate the pixel array -- it is no longer needed
    delete[] pixelsRGBA;

    return 0;
}

int loadSplines(char *argv)
{
    char *cName = (char *)malloc(128 * sizeof(char));
    FILE *fileList;
    FILE *fileSpline;
    int iType, i = 0, j, iLength;

    // load the track file
    fileList = fopen(argv, "r");
    if (fileList == NULL)
    {
        printf("can't open file\n");
        exit(1);
    }

    // stores the number of splines in a global variable
    fscanf(fileList, "%d", &numSplines);

    splines = (Spline *)malloc(numSplines * sizeof(Spline));

    // reads through the spline files
    for (j = 0; j < numSplines; j++)
    {
        i = 0;
        fscanf(fileList, "%s", cName);
        fileSpline = fopen(cName, "r");

        if (fileSpline == NULL)
        {
            printf("can't open file\n");
            exit(1);
        }

        // gets length for spline file
        fscanf(fileSpline, "%d %d", &iLength, &iType);

        // allocate memory for all the points
        splines[j].points = (Point *)malloc(iLength * sizeof(Point));
        splines[j].numControlPoints = iLength;

        // saves the data to the struct
        while (fscanf(fileSpline, "%lf %lf %lf",
                      &splines[j].points[i].x,
                      &splines[j].points[i].y,
                      &splines[j].points[i].z) != EOF)
        {
            i++;
        }
    }

    free(cName);

    return 0;
}

void initScene()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Initialize openGLMatrix
    openGLMatrix = new OpenGLMatrix();

    // Initialize pipeline program
    pipelineProgram = new BasicPipelineProgram();
    pipelineProgram->Init("../openGLHelper-starterCode");
    programId = pipelineProgram->GetProgramHandle();
    pipelineProgram->Bind();

    // Initialize skybox
    skyboxHandler = loadSkybox();

    // Initialize textures
    glGenTextures(1, &mTextureHandler);
    if (initTexture(mainImageFilename, mTextureHandler) != 0)
    {
        cout << "Error reading main image texture." << endl;
        exit(EXIT_FAILURE);
    }

    glGenTextures(1, &gdTextureHandler);
    if (initTexture(gdImageFilename, gdTextureHandler) != 0)
    {
        cout << "Error reading ground image texture." << endl;
        exit(EXIT_FAILURE);
    }

    glGenTextures(1, &cbTextureHandler);
    if (initTexture(cbImageFilename, cbTextureHandler) != 0)
    {
        cout << "Error reading cross bar image texture." << endl;
        exit(EXIT_FAILURE);
    }

    // Initialize
    initVertex();
    updateGroundHeight();
    initBuffer();
    initPerspective(windowWidth, windowHeight);

    glLinkProgram(programId); //Initial link

    GLint textureLoc = glGetUniformLocation(programId, "textureImage");
    GLint skyboxLoc = glGetUniformLocation(programId, "skyboxMap");

    glUseProgram(programId);
    glUniform1i(textureLoc, 0); //Texture unit 0 is for base images.
    glUniform1i(skyboxLoc, 1);  //Texture unit 2 is for normal maps.
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("usage: %s <trackfile>\n", argv[0]);
        exit(0);
    }

    // load the splines from the provided filename
    loadSplines(argv[1]);

    printf("Loaded %d spline(s).\n", numSplines);
    for (int i = 0; i < numSplines; i++)
    {
        printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);
    }

    // OpenGL initilization
    cout << "Initializing GLUT..." << endl;
    glutInit(&argc, argv);

    cout << "Initializing OpenGL..." << endl;

#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
#else
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#endif

    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(windowTitle);

    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
    cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

    // tells glut to use a particular display function to redraw
    glutDisplayFunc(displayFunc);
    // perform animation inside idleFunc
    glutIdleFunc(idleFunc);
    // callback for mouse drags
    glutMotionFunc(mouseMotionDragFunc);
    // callback for idle mouse movement
    glutPassiveMotionFunc(mouseMotionFunc);
    // callback for mouse button changes
    glutMouseFunc(mouseButtonFunc);
    // callback for resizing the window
    glutReshapeFunc(reshapeFunc);
    // callback for pressing the keys on the keyboard
    glutKeyboardFunc(keyboardFunc);
// init glew
#ifdef __APPLE__
// nothing is needed on Apple
#else
    // Windows, Linux
    GLint result = glewInit();
    if (result != GLEW_OK)
    {
        cout << "error: " << glewGetErrorString(result) << endl;
        exit(EXIT_FAILURE);
    }
#endif

    // Enable Z-buffer
    glEnable(GL_DEPTH_TEST);

    // do initialization
    initScene();

    // sink forever into the glut loop
    glutMainLoop();
}
