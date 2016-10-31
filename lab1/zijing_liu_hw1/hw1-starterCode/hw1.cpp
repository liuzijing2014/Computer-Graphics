/*
 CSCI 420 Computer Graphics, USC
 Assignment 1: Height Fields
 C++ starter code
 
 Student username: <zijingli>
 */

#include <iostream>
#include <cstring>
#include <cstdio>
#include "openGLHeader.h"
#include "glutHeader.h"

#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"

#ifdef WIN32
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

typedef struct
{
	float XYZ[3];
	float RGBA[4];
} Vertex;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

typedef enum { POINTS, LINES, TRIANGLES } RENDER_TYPE;
RENDER_TYPE renderType = TRIANGLES;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

// Screenshot flag
bool screenshot = false;
int screenshotCount = 0;

// Image and the corresponding height data
ImageIO * heightmapImage;
ImageIO * textureImage = NULL;
GLuint height, width;
Vertex* vertices;
GLuint* indices = NULL;
int bytesPerPixel;

GLuint indexNum;
GLuint
	programId,
	VaoId,
	BufferId,
	IndexBufferId,
	vertexNum;

float sFactor = 1.0f;

// Helper lib pointers
BasicPipelineProgram* pipelineProgram;
OpenGLMatrix* openGLMatrix;

// Functions decleared
void initBuffer();
void initIndex();
void initImageData();
void initPerspective(int w, int h);
void setupMatrex();

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
	unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 4];
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGBA, GL_UNSIGNED_BYTE, screenshotData);
	
	ImageIO screenshotImg(windowWidth, windowHeight, 4, screenshotData);
	
	if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
		cout << "File " << filename << " saved successfully." << endl;
	else cout << "Failed to save file " << filename << '.' << endl;
	
	delete [] screenshotData;
}

void displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(VaoId);
	setupMatrex();
	if(renderType == TRIANGLES)
	{
		glDrawElements(GL_TRIANGLES, indexNum, GL_UNSIGNED_INT, (GLvoid*)0);
	}
	else if(renderType == LINES)
	{
		glDrawElements(GL_LINES, indexNum, GL_UNSIGNED_INT, (GLvoid*)0);
	}
	else if(renderType == POINTS)
	{
		glDrawElements(POINTS, indexNum, GL_UNSIGNED_INT, (GLvoid*)0);
	}
	glFlush();
	glutSwapBuffers();
	glBindVertexArray(0);
}

// Setup modelview and upload the matrix to GPU
void setupMatrex()
{
	//change mode to modelview
	openGLMatrix->SetMatrixMode(OpenGLMatrix::ModelView);
	openGLMatrix->LoadIdentity();
	openGLMatrix->LookAt(0.0f, 1.5f, 1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	openGLMatrix->Rotate(landRotate[0], 1.0, 0.0, 0.0);
	openGLMatrix->Rotate(landRotate[1], 0.0, 1.0, 0.0);
	openGLMatrix->Rotate(landRotate[2], 0.0, 0.0, 1.0);
	openGLMatrix->Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
	openGLMatrix->Scale(landScale[0]*sFactor, landScale[1]*sFactor, landScale[2]*sFactor);
	
	// Setup modelview matrix
	float m[16];
	openGLMatrix->GetMatrix(m);
	pipelineProgram->SetModelViewMatrix(m);
}

void idleFunc()
{
	// do some stuff...
	
	// for example, here, you can save the screenshots to disk (to make the animation)
	if(screenshot)
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
}

void mouseMotionDragFunc(int x, int y)
{
	// mouse has moved and one of the mouse buttons is pressed (dragging)
	
	// the change in mouse position since the last invocation of this function
	int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };
	
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
		case 27: // ESC key
			delete [] indices;
			delete [] vertices;
			exit(0); // exit the program
			break;
			
		case ' ':
			if(screenshot)
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
		case 'l':
			// render as lines
			renderType = LINES;
			initIndex();
			glBindVertexArray(VaoId);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexNum, indices, GL_STATIC_DRAW);
			glBindVertexArray(0);
			break;
		case 't':
			// render as triangles
			renderType = TRIANGLES;
			initIndex();
			glBindVertexArray(VaoId);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexNum, indices, GL_STATIC_DRAW);
			glBindVertexArray(0);
			break;
		case 'p':
			// render as points
			renderType = POINTS;
			initIndex();
			glBindVertexArray(VaoId);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexNum, indices, GL_STATIC_DRAW);
			glBindVertexArray(0);
			break;
	}
}


// Initilize each vertex data (position and color) based on the given heightmap image
void initImageData()
{
	// Initilize image data
	height = heightmapImage->getHeight();
	width = heightmapImage->getWidth();
	float halfh = height/2.0f;
	float halfw = width/2.0f;
	vertices = new Vertex[height * width];
	
	int i = 0;
	for(int x = 0; x < width; x++)
	{
		for(int y = 0; y < height; y++)
		{
			// Compute the color value at the point, which also determines the y value
			float color = 0;
			for(int c = 0; c < bytesPerPixel; c++)
			{
				color += (float)heightmapImage->getPixel(x, y, c);
			}
			color /= bytesPerPixel;
			
			vertices[i].XYZ[0] = (((float)x) - halfw)/200.0f;
			vertices[i].XYZ[1] = color/800.0f;
			vertices[i].XYZ[2] = (((float)y) - halfh)/200.0f;

			// Assign color based on if a texture image is given.
			if(textureImage == NULL)
			{
				vertices[i].RGBA[0] = vertices[i].RGBA[1] = vertices[i].RGBA[2] = color/255.0f;
			}
			else
			{
				vertices[i].RGBA[0] = textureImage->getPixel(y,x,0)/255.0f;
				vertices[i].RGBA[1] = textureImage->getPixel(y,x,1)/255.0f;
				vertices[i].RGBA[2] = textureImage->getPixel(y,x,2)/255.0f;
			}
			vertices[i].RGBA[3] = 1.0f;
			i++;
		}
	}
	vertexNum = i;
}


// Initilize the indice data based on which way (point, line, triangle) we use to render the terrian.
void initIndex()
{
	int i = 0;
	if(indices != NULL)
	{
		delete [] indices;
	}
	// Render as solid triangles.
	if(renderType == TRIANGLES)
	{
		indices = new GLuint[(width - 1) * (height - 1) * 6];
		for(int x = 0; x < width-1; x++)
		{
			for(int y = 0; y < height-1; y++)
			{
				int bottomLeft = x * height + y;
				// First triangle
				indices[i] = bottomLeft;
				indices[++i] = bottomLeft + 1;
				indices[++i] = bottomLeft + 1 + height;
				// Second triangle
				indices[++i] = bottomLeft;
				indices[++i] = bottomLeft + height;
				indices[++i] = bottomLeft + height + 1;
				i++;
			}
		}
	}
	// Render as lines
	else if(renderType == LINES)
	{
		indices = new GLuint[(height - 1) * width * 2 * 2];
		for(int x = 0; x < width; x++)
		{
			for(int y = 0; y < height; y++)
			{
				int firstPos = x * height + y;
				if(y < height - 1)
				{
					// Vertical line
					indices[i] = firstPos;
					indices[++i] = firstPos + 1;
					i++;
				}
				if(x < width - 1)
				{
					// Horizontal line
					indices[i] = firstPos;
					indices[++i] = firstPos + height;
					i++;
				}
			}
		}
	}
	// Render as points
	else if(renderType == POINTS)
	{
		indices = new GLuint[height * width];
		for(i = 0 ; i < height * width; i++)
		{
			indices[i] = i;
		}
	}
	indexNum = i;
}


void initBuffer()
{
	const size_t BufferSize = sizeof(Vertex) * vertexNum;
	const size_t VertexSize = sizeof(vertices[0]);
	const size_t RgbOffset = sizeof(vertices[0].XYZ);
	
	GLuint pos = glGetAttribLocation(programId, "position");
	GLuint col = glGetAttribLocation(programId, "color");
	
	
	// Generate VAO and bind it
	glGenVertexArrays(1, &VaoId);
	glBindVertexArray(VaoId);
	
	// Generate VBOs and bind them
	glGenBuffers(1, &BufferId);
	glBindBuffer(GL_ARRAY_BUFFER, BufferId);
	glBufferData(GL_ARRAY_BUFFER, BufferSize, vertices, GL_STATIC_DRAW);
	
	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(col, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
	
	glEnableVertexAttribArray(pos);
	glEnableVertexAttribArray(col);
	
	// Index buffer
	glGenBuffers(1, &IndexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indexNum, indices, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
}

void initScene(int argc, char *argv[])
{
	// load the image from a jpeg disk file to main memory
	heightmapImage = new ImageIO();
	if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
	{
		cout << "Error reading image " << argv[1] << "." << endl;
		exit(EXIT_FAILURE);
	}
	
	// load the image for texture
	if(argc == 3)
	{
		textureImage = new ImageIO();
		if (textureImage->loadJPEG(argv[2]) != ImageIO::OK)
		{
			cout << "Fail to read a texture image" << argv[2] << "." << endl;
		}
		else
		{
			int hoffset = textureImage->getHeight() - heightmapImage->getHeight();
			int woffset = textureImage->getWidth() - heightmapImage->getWidth();
			if(hoffset != 0 || woffset != 0)
			{
				cout << "Texturemap image must be the same size as the heightmap image" << endl;
				textureImage = NULL;
			}
		}
	}
	
	// Get bytes per per pixel
	bytesPerPixel = heightmapImage->getBytesPerPixel();
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	
	// Initialize openGLMatrix
	openGLMatrix = new OpenGLMatrix();
	
	// Initialize pipeline program
	pipelineProgram = new BasicPipelineProgram();
	pipelineProgram->Init("../openGLHelper-starterCode");
	pipelineProgram->Bind();
	programId = pipelineProgram->GetProgramHandle();
	
	// Default render type is solid triangles
	renderType = POINTS;
	
	// Initialize
	initImageData();
	initIndex();
	initBuffer();
	initPerspective(windowWidth, windowHeight);
}

int main(int argc, char *argv[])
{
	if (argc > 3)
	{
		cout << "The arguments are incorrect." << endl;
		cout << "usage: ./hw1 <heightmap file> <texturemap file>" << endl;
		exit(EXIT_FAILURE);
	}
	
	cout << "Initializing GLUT..." << endl;
	glutInit(&argc,argv);
	
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
	initScene(argc, argv);
	
	// sink forever into the glut loop
	glutMainLoop();
}


