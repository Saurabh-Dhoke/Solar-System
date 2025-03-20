
/******************************************* 
	Texture + light = Done + Moving 
	Date : 15, 16, 17 .08 .2024 
*******************************************/

// Common Header Files
#include<windows.h>
#include<stdio.h> 
#include<stdlib.h> // for exit() 

#include"OGL.h"

// for PlaySound() api
#include <mmsystem.h>	

// opengl header Files
#include<gl/glew.h> // this must be before gl/GL.h
#include<gl/GL.h>

#include"vmath.h"
using namespace vmath;

// for image loading ..(png, bmp, jpeg, jpg)
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

// Macrose
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// link with OpenGL library
#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"OpenGL32.lib")

// openGl related global variables
HDC ghdc = NULL;
HGLRC ghrc = NULL;

GLuint shaderProgramObject  = 0;

enum
{
	AMC_ATTRIBUTE_POSITION = 0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_TEXCOORD,
	AMC_ATTRIBUTE_NORMAL
};

GLuint mvpMatrixUniform      = 0;

GLuint textureSamplerUniform = 0;
GLuint texture_Sphere[9] = { 0 };

mat4 PerspectiveProjectionMatrix; // mat4 in vmath.h 

// Global Function Declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Variable declarations
HWND ghwnd = NULL;  
DWORD dwStyle = 0;

WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
BOOL gbfullscreen = FALSE; 

FILE* gpFILE = NULL;
BOOL gbActive = FALSE;

//------------------------------------------
// For Light 
GLuint modelMatrixUniform = 0;
GLuint viewMatrixUniform = 0;
GLuint projectionMatrixUniform = 0;

GLuint lightDiffuseUniform = 0;
GLuint lightAmbiantUniform = 0;  
GLuint lightSpecularUniform = 0; 
GLuint lightPositionUniform = 0; 

GLuint materialAmbiantUniform = 0;
GLuint materialDiffuseUniform = 0;
GLuint materialSpecularUniform = 0;
GLuint materialShinninesUniform = 0;

GLuint keyPressedUniform = 0;
BOOL bLightingEnabled = TRUE;
GLfloat PI = 22.0f / 7.0f;
GLfloat lightAngle_Y = 3.14f;

GLfloat lightAmbiant[] = { 1.0f, 1.0f, 1.0f, 1.0f }; 
GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f }; 
GLfloat lightSpecular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat lightPosition[] = { 0.0f, 0.0f, 0.0f, 0.0f }; 

GLfloat materialAmbiant[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat materialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialShinniness =  128.0f ;

//------------------------------------------
// for Sphere
#include <vector>			// for vector
using namespace std;

// vectors for sphere data 
// TODO - struct
vector <float> vertices;
vector <float> normals;
vector <float> texcoords;
vector <int> indices;

// vao_sphere and vbo
GLuint vao_sphere;
GLuint vbo_position_sphere;
GLuint vbo_element_sphere;
GLuint vbo_normal_sphere;
GLuint vbo_texture_sphere;

float roate_Sphere = 0.0f;

GLuint change_Count = 0;
//------------------------------------------
// for sound play 
HINSTANCE hInst = NULL;
//------------------------------------------

// Entry point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{ 
	// Function Declarations 
	int initialize(void);
	void uninitialize(void);
	void display(void); 
	void update(void);

	// local variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("SVDWindow"); 
	int iResult = 0;  // for base code
	BOOL bDone = FALSE; 

	// Local Variable for finding the center of the window
	int SW = 0;   // Screen width
	int SH = 0;   // Screen Height

	float x = 0.0f;
	float y = 0.0f;

	// Code
	//gpFILE = fopen("log.txt","w");
	//if (gpFILE == NULL)
	
	if(fopen_s(&gpFILE, "log.txt", "w") != 0) // success zal tr 0 return krt nhitr non zero return krt 
	{
		MessageBox(NULL, TEXT("Log File cannot be opend..."), TEXT("Error"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	else
	{
		fprintf(gpFILE, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
	}

	fprintf(gpFILE , "Program Started Successfully... \n");
	fprintf(gpFILE, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n\n");


	// WNDCLASSEX initialization  
	// structer ch inline initialization

	wndclass.cbSize			= sizeof(WNDCLASSEX);
	wndclass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC; 
	wndclass.cbWndExtra		= 0;
	wndclass.cbClsExtra		= 0;
	wndclass.lpfnWndProc	= WndProc;
	wndclass.hInstance		= hInstance;
	wndclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor		= LoadCursor(NULL,IDC_ARROW);
	wndclass.lpszClassName	= szAppName;
	wndclass.lpszMenuName	= NULL;
	wndclass.hIconSm		= LoadIcon(hInstance,MAKEINTRESOURCE(MYICON));


	// Register WNDCLASSEX 
	RegisterClassEx(&wndclass);

	// Centering of window code
	SW = GetSystemMetrics(SM_CXSCREEN); 
	SH = GetSystemMetrics(SM_CYSCREEN); 

	x = (SW / 2.0) - (WIN_WIDTH / 2.0); 
	y = (SH / 2.0) - (WIN_HEIGHT / 2.0); 


	// Create Window
	hwnd = CreateWindowEx( WS_EX_APPWINDOW,
						 szAppName,
						 TEXT("Saurabh Dhoke"),
						 WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE, 
						 x,
				   		 y,
		                 WIN_WIDTH,  
		                 WIN_HEIGHT,  
						 NULL,
						 NULL,
						 hInstance,
						 NULL );

	// gloabal handel madhi value takli 
	ghwnd = hwnd; 

	// .rc madhun song play karnyasathi hInstance Copy krun ghetla initialize() la access krta yava mhanun
	hInst = hInstance;

	//initialization
	iResult = initialize();

	if (iResult != 0)
	{
		MessageBox(hwnd, TEXT("initialize() Failed..."), TEXT("Error"), MB_OK | MB_ICONERROR);
		DestroyWindow(hwnd);

		//uninitialize();
		//exit(0);	//  yala call kela tri chalel
	}

	// Show the Window
	ShowWindow(hwnd,iCmdShow); 
	
	SetForegroundWindow(hwnd); 
	SetFocus(hwnd); 

	// Game Loop
	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			if (msg.message == WM_QUIT)
				bDone = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActive == TRUE)  // setfocus mdhi true zalelay he gbActive 
			{
				// Render
				display();

				// Update 
				update();

			}
		}
	}

	// uninitialization
	uninitialize();
	
	return((int)msg.wParam);  
}

// CALLBACK function
LRESULT CALLBACK WndProc(HWND   hwnd, UINT   iMsg, WPARAM wParam, LPARAM lParam)
{
	// Function declarations
	void ToggleFullscreen(void);
	void resize(int,int); 


	// Code

	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActive = TRUE; 
		break;

	case WM_KILLFOCUS:
		gbActive = FALSE; 
		break;

	case WM_SIZE: //  Width           Height
		resize(LOWORD(lParam), HIWORD(lParam)); 
		break;

	case WM_ERASEBKGND:  // os ch WS_Paint vaparayach nahiy mhanun he vapralay 
		return(0);

	case WM_KEYDOWN :
		switch (LOWORD(wParam))
		{
		case VK_ESCAPE :
			DestroyWindow(hwnd);
			break;
		}
		break;
	
	case WM_CHAR :
		switch (LOWORD(wParam))
		{
		case 'L':
		case 'l':
			if (bLightingEnabled == TRUE)
			{
				bLightingEnabled = FALSE;
			}
			else
			{
				bLightingEnabled = TRUE;
			}
			break;

		case 'F' :
		case 'f' :
			if (gbfullscreen == FALSE)
			{
				//ToggleFullscreen();
				gbfullscreen = TRUE;
			}
			else
			{
				//ToggleFullscreen();
				gbfullscreen = FALSE; 
			}
			break;
		}
		break;

	case WM_DESTROY : 
		PostQuitMessage(0);
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	default :
		break;
	}

	return(DefWindowProc(hwnd,iMsg,wParam,lParam));
}
void ToggleFullscreen(void)
{
	// Local Variable Declarations
	MONITORINFO mi = { sizeof(MONITORINFO) };

	// Code
	if (gbfullscreen == FALSE)
	{	
		dwStyle = GetWindowLong(ghwnd,GWL_STYLE);
		
		if (dwStyle & WS_OVERLAPPEDWINDOW) 
		{	
			if (GetWindowPlacement(ghwnd,&wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{    
				SetWindowLong(ghwnd,GWL_STYLE,dwStyle & ~WS_OVERLAPPEDWINDOW); // dwStyle shodh aani WS_OVERLAPPEDWINDOW kadhun tak (~ remove karto)                             
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);			
			}
		}
		ShowCursor(FALSE);
	}
	else
	{
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPos(ghwnd, HWND_TOP, 0,0,0,0,SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED); // window placement allredy set zali first line la mhanun 0 dilet
		ShowCursor(TRUE); 
	}
}
int initialize(void)
{
	ToggleFullscreen();

	// Function Declarations
	void printGLInfo(void);
	void uninitialize();
	void resize(int, int);
	int loadImages();
	void generateSphereVertices(float, float, float);

	// Code
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;
	BOOL bResult; // for texture

	ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));

	// step 1 - initialization of pixel format descriptor
	pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion   = 1;
	pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;  /// ???????????? PFD_RGBA
	pfd.cColorBits = 32;
	pfd.cRedBits   = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits  = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	// step 2 - get the DC (device context)
	ghdc = GetDC(ghwnd);
	if (ghdc == 0)
	{
		fprintf(gpFILE, "GetDC() failed.. \n\n");
		return -1;
	}

	// step 3
	iPixelFormatIndex = ChoosePixelFormat(ghdc,&pfd);
	if (iPixelFormatIndex == 0)
	{
		fprintf(gpFILE, "ChoosePixelFormat() failed.. \n\n");
		return -2;
	}

	// Step 4
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		fprintf(gpFILE, "SetPixelFormat() failed.. \n\n");
		return -3;
	}

	// tell wgl library to give opengl compatable device context to this device context
	// create opengl context from device context 
	ghrc = wglCreateContext(ghdc);
	if (ghrc == 0)
	{
		fprintf(gpFILE, "wglCreateContext() failed.. \n\n");
		return -4;
	}

	// Make rendering Context current 
	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		fprintf(gpFILE, "wglMakeCurrent() failed.. \n\n");
		return -5;
	}

	//----------------------------------------------------
	// PP cha code ya shivay eligible hot nahi 
	// Initialize GLEW
	if (glewInit() != GLEW_OK)
	{
		fprintf(gpFILE, "glewInint() failed.. \n\n");
		return -6;
	}

	//-----------------------------------
	// Print OpenGL info 
	// printGLInfo();
	//-----------------------------------
	
	 //------------ Vertex Shader --------------- 
	 const GLchar* vertexShaderSourceCode =
		 "#version 460 core" \
		 "\n " \
		 "in vec4 aPosition; " \
		 "in vec3 aNormal; " \
		 "in vec4 aColor; " \
		 "out vec4 oColor; " \
		 "in vec2 aTexCoord; " \
		 "out vec2 oTexCoord; " \

		 "uniform mat4 uModelMatrix; " \
		 "uniform mat4 uViewMatrix; " \
		 "uniform mat4 uProjectionMatrix; " \
		 "uniform vec4 ulightPosition; " \
		 "uniform int uKeyPressed; " \

		 "out vec3 oTransformedNormals; " \
		 "out vec3 oLightDirection; " \
		 "out vec3 oViewerVector; " \

		 "void main(void) " \
		 "{ " \

			 "if(uKeyPressed == 1) " \
			 "{ " \
				 "vec4 iCoordinates       = uViewMatrix * uModelMatrix * aPosition; " \
				 "oTransformedNormals     = mat3(uViewMatrix * uModelMatrix) * aNormal;" \
				 "oLightDirection         = vec3(ulightPosition - iCoordinates);" \
				 "oViewerVector           = (- iCoordinates.xyz);" \
			 "} " \
			 "else " \
			 "{ " \
				 "oTransformedNormals = vec3(0.0, 0.0, 0.0); " \
				 "oLightDirection     = vec3(0.0, 0.0, 0.0); " \
				 "oViewerVector       = vec3(0.0, 0.0, 0.0); " \
			 "} " \
			 "gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * aPosition; " \
			 "oTexCoord = aTexCoord ; " \
		 "} ";
	 
	 // 2: Create vertex shadder object
	 GLuint vertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	
	 // 3: give vertex shader source code to opengl
	 glShaderSource(vertexShaderObject,1,(const GLchar**)&vertexShaderSourceCode,NULL);

	 // 4: let opengl compile vertex shader
	 glCompileShader(vertexShaderObject);

	 // 5.c
	 GLint status        = 0;
	 GLint infoLogLength = 0;
	 GLchar * szInfoLog   = NULL;

	 // 5.a
	 glGetShaderiv(vertexShaderObject, GL_COMPILE_STATUS, &status);
	 if (status == GL_FALSE)
	 {	// 5.b
		 glGetShaderiv(vertexShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength); 

		 if (infoLogLength > 0)
		 {   // 5.c
			 szInfoLog = (GLchar *)malloc(infoLogLength+1);

			 if (szInfoLog != NULL)
			 {   // 5.d
				 glGetShaderInfoLog(vertexShaderObject, infoLogLength+1, NULL, szInfoLog);  
				 // 5.e
				 fprintf(gpFILE, "vertex shadder Compilation Error Log : %s \n", szInfoLog);
				 // 5.f
				 free(szInfoLog);
				 szInfoLog = NULL;
				 // 5.g
			 }
		 }
		 uninitialize();
	 }

	 //-------------- Fragment Shader ---------------
	 const GLchar* fragmentShaderSourceCode =
		 "#version 460 core" \
		 "\n" \
		"in vec4 oColor;                    \n" \
		 "in vec3 oTransformedNormals;       \n" \
		 "in vec3 oLightDirection;           \n" \
		 "in vec3 oViewerVector;             \n" \
		 "out vec4 FragColor;                \n" \

		 "uniform vec3 uLightAmbiant;   \n" \
		 "uniform vec3 uLightDiffuse;   \n" \
		 "uniform vec3 uLightSpecular;   \n" \
		 "uniform vec3 uMaterialAmbiant;   \n" \
		 "uniform vec3 uMaterialDiffuse;   \n" \
		 "uniform vec3 uMaterialSpecular;   \n" \
		 "uniform float uMaterialShinniness;   \n" \
		 "uniform int uKeyPressed;  \n" \

		 "in vec2 oTexCoord;  \n" \
		 "uniform sampler2D uTextureSampler;  \n" \

		 "void main(void)  \n" \
		 "{  \n" \
			 "vec3 Phong_ADS_Light = vec3(0.0, 0.0, 0.0);  \n" \
			 "vec4 lightColor   = vec4(0.0, 0.0, 0.0, 1.0);  \n" \
			 "vec4 textureColor = vec4(0.0, 0.0, 0.0, 1.0);  \n" \
			 
			"if(uKeyPressed == 1)  \n" \
			 "{  \n" \
				 "vec3 normalisedTransformedNormals =  normalize(oTransformedNormals);   \n" \
				 "vec3 normalisedLightDirection     =  normalize(oLightDirection);   \n" \
				 "vec3 normalisedViewerVector       =  normalize(oViewerVector);   \n" \
				 "vec3 ambiantLight           = uLightAmbiant * uMaterialAmbiant;   \n" \
				 "vec3 diffuseLight           = uLightDiffuse * uMaterialDiffuse * max(dot(normalisedLightDirection,normalisedTransformedNormals),0.0);   \n" \
				 "vec3 reflectionVector       = reflect(-normalisedLightDirection,normalisedTransformedNormals);   \n" \
				 "vec3 lightSpecular          = uLightSpecular * uMaterialSpecular * pow(max(dot(reflectionVector,normalisedViewerVector), 0.0), uMaterialShinniness );   \n" \
				 "Phong_ADS_Light = ambiantLight + diffuseLight+lightSpecular;  \n" \
				 "lightColor = vec4(Phong_ADS_Light, 1.0);  \n" \
			 "}  \n" \
			 "else  \n" \
			 "{  \n" \
				 "lightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);  \n" \
			 "}  \n" \
			
		    "textureColor = texture(uTextureSampler, oTexCoord);   \n" \

			"FragColor = textureColor * lightColor;   \n" \
		 "}";

	 GLuint fragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	 glShaderSource(fragmentShaderObject, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);
	 glCompileShader(fragmentShaderObject);

	 status = 0;
	 infoLogLength = 0;
	 szInfoLog = NULL;

	 glGetShaderiv(fragmentShaderObject, GL_COMPILE_STATUS, &status);
	 if (status == GL_FALSE)
	 {
		 glGetShaderiv(fragmentShaderObject, GL_INFO_LOG_LENGTH, &infoLogLength);
		 if (infoLogLength > 0)
		 {
			 szInfoLog = (GLchar*)malloc(infoLogLength);
			 if (szInfoLog != NULL)
			 {
				 glGetShaderInfoLog(fragmentShaderObject, infoLogLength, NULL,szInfoLog);
				 fprintf(gpFILE,"Fragment shader Compilation Error Log : %s \n", szInfoLog); 

				 free(szInfoLog);
				 szInfoLog = NULL;
			 }
		 }
		 uninitialize();
	 }

	 //------------ Shader Progarm --------------
	 // 7: create shader program
	 shaderProgramObject = glCreateProgram();

	 // 8 : Attach both shadders to this program
	 glAttachShader(shaderProgramObject, vertexShaderObject);
	 glAttachShader(shaderProgramObject, fragmentShaderObject);

	 // 9 : bind attribute locations with the shader program objects
	 glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_POSITION,"aPosition");
	 glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_TEXCOORD, "aTexCoord");
	 glBindAttribLocation(shaderProgramObject, AMC_ATTRIBUTE_NORMAL, "aNormal"); 

	 // 10: link the shader program
	 glLinkProgram(shaderProgramObject);

	 // 11:

	 status = 0;
	 infoLogLength = 0;
	 szInfoLog = NULL;

	 glGetProgramiv(shaderProgramObject,GL_LINK_STATUS,&status); 
	 if (status == GL_FALSE)
	 {
		 glGetProgramiv(shaderProgramObject,GL_INFO_LOG_LENGTH, &infoLogLength);
		 if (infoLogLength > 0)
		 {
			 szInfoLog = (GLchar*)malloc(infoLogLength);
			 if (szInfoLog != NULL)
			 {
				 glGetProgramInfoLog(shaderProgramObject, infoLogLength, NULL, szInfoLog);
				 fprintf(gpFILE, "Shadder program linking error Log : %s \n", szInfoLog);

				 free(szInfoLog);
				 szInfoLog = NULL; 
			 }
		 }
		 uninitialize();
	 }

	  // get shader uniform locations 
	 //                      kunakadun gheu , kontya uniform location ch gheu  
	 modelMatrixUniform       = glGetUniformLocation(shaderProgramObject, "uModelMatrix");
	 viewMatrixUniform        = glGetUniformLocation(shaderProgramObject, "uViewMatrix");
	 projectionMatrixUniform  = glGetUniformLocation(shaderProgramObject, "uProjectionMatrix");
	 lightAmbiantUniform      = glGetUniformLocation(shaderProgramObject, "uLightAmbiant");
	 lightDiffuseUniform	  = glGetUniformLocation(shaderProgramObject, "uLightDiffuse");
	 lightSpecularUniform     = glGetUniformLocation(shaderProgramObject, "uLightSpecular");
	 lightPositionUniform     = glGetUniformLocation(shaderProgramObject, "ulightPosition");
	 materialAmbiantUniform   = glGetUniformLocation(shaderProgramObject, "uMaterialAmbiant");
	 materialDiffuseUniform   = glGetUniformLocation(shaderProgramObject, "uMaterialDiffuse");
	 materialSpecularUniform  = glGetUniformLocation(shaderProgramObject, "uMaterialSpecular");
	 materialShinninesUniform = glGetUniformLocation(shaderProgramObject, "uMaterialShinniness");
	 keyPressedUniform        = glGetUniformLocation(shaderProgramObject, "uKeyPressed");
	
	 textureSamplerUniform    = glGetUniformLocation(shaderProgramObject, "uTextureSampler");

	 // Get the Sphere Data 
	 generateSphereVertices(1.0f, 64.0f, 32.0f);		// params - radius, sectors, stacks


	 // vao_sphere and VBO related code
	 glGenVertexArrays(1, &vao_sphere);
	 glBindVertexArray(vao_sphere);

	 // VBO for position
	 glGenBuffers(1, &vbo_position_sphere);
	 glBindBuffer(GL_ARRAY_BUFFER, vbo_position_sphere);
	 glBufferData(GL_ARRAY_BUFFER, (unsigned int)vertices.size() * sizeof(float), (const float*)vertices.data(), GL_STATIC_DRAW);
	 glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	 glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
	 glBindBuffer(GL_ARRAY_BUFFER, 0);

	 // element
	 glGenBuffers(1, &vbo_element_sphere);
	 glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element_sphere);
	 glBufferData(GL_ELEMENT_ARRAY_BUFFER, (unsigned int)indices.size() * sizeof(unsigned int), (const unsigned int*)indices.data(), GL_STATIC_DRAW);
	 glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	 // Normal 
	 glGenBuffers(1, &vbo_normal_sphere);
	 glBindBuffer(GL_ARRAY_BUFFER, vbo_normal_sphere);
	 glBufferData(GL_ARRAY_BUFFER, (unsigned int)normals.size() * sizeof(float), (const float*)normals.data(), GL_STATIC_DRAW);
	 glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	 glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
	 glBindBuffer(GL_ARRAY_BUFFER, 0);

	 // VBO for texture
	 glGenBuffers(1, &vbo_texture_sphere);
	 glBindBuffer(GL_ARRAY_BUFFER, vbo_texture_sphere);
	 glBufferData(GL_ARRAY_BUFFER, (unsigned int)texcoords.size() * sizeof(unsigned int), (const float*)texcoords.data(), GL_STATIC_DRAW);
	 glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	 glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD);
	 glBindBuffer(GL_ARRAY_BUFFER, 0);

	 glBindVertexArray(0);

	//--------------------------------------------------------------------------------
	
	// Enabling Depth
	glClearDepth(1.0f);							// compulsary 
	glEnable(GL_DEPTH_TEST);					// compulsary
	glDepthFunc(GL_LEQUAL);						// compulsary

	//---------------------------------------------------------------------------
	// here openGl starts
	// Set the clear color of window to blue
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
	
	// load images 
	loadImages();

	//-----------------------------
	PlaySound(TEXT("soundName"), hInst, SND_RESOURCE | SND_ASYNC);

	//-----------------------------


	// tell opengl to enable the texture (step 4)
	glEnable(GL_TEXTURE_2D);

	// initialize orthograficprojection matrix
	PerspectiveProjectionMatrix = vmath::mat4::identity();

	// warm up resize call
	resize(WIN_WIDTH, WIN_HEIGHT);

	return(0);
}

int loadImages()
{
	// Function declarations 
	BOOL loadGLTexture_stb(GLuint*, char*); 

	// Variable declarations 
	BOOL bResult; // for texture

	// Code 
	// texture loading 
	bResult = loadGLTexture_stb(&texture_Sphere[0], "Textures/Sun.jpg");
	if (bResult == FALSE)
	{
		fprintf(gpFILE, "Loading of Texture1 failed.. \n\n");
		return -6;
	}

	bResult = loadGLTexture_stb(&texture_Sphere[1], "Textures/Mercury.jpg");
	if (bResult == FALSE)
	{
		fprintf(gpFILE, "Loading of Texture2 failed.. \n\n");
		return -7;
	}

	bResult = loadGLTexture_stb(&texture_Sphere[2], "Textures/Venus.jpg");
	if (bResult == FALSE)
	{
		fprintf(gpFILE, "Loading of Texture3 failed.. \n\n");
		return -8;
	}

	bResult = loadGLTexture_stb(&texture_Sphere[3], "Textures/Earth.jpg");
	if (bResult == FALSE)
	{
		fprintf(gpFILE, "Loading of Texture4 failed.. \n\n");
		return -9;
	}

	bResult = loadGLTexture_stb(&texture_Sphere[4], "Textures/Earth_Night.jpg");
	if (bResult == FALSE)
	{
		fprintf(gpFILE, "Loading of Texture5 failed.. \n\n");
		return -10;
	}

	bResult = loadGLTexture_stb(&texture_Sphere[5], "Textures/Moon.jpg");
	if (bResult == FALSE)
	{
		fprintf(gpFILE, "Loading of Texture6 failed.. \n\n");
		return -11;
	}

	bResult = loadGLTexture_stb(&texture_Sphere[6], "Textures/Mars.jpg");
	if (bResult == FALSE)
	{
		fprintf(gpFILE, "Loading of Texture7 failed.. \n\n");
		return -12;
	}

	bResult = loadGLTexture_stb(&texture_Sphere[7], "Textures/Jupitor.jpg");
	if (bResult == FALSE)
	{
		fprintf(gpFILE, "Loading of Texture8 failed.. \n\n");
		return -13;
	}

	bResult = loadGLTexture_stb(&texture_Sphere[8], "Textures/Uranus.jpg");
	if (bResult == FALSE)
	{
		fprintf(gpFILE, "Loading of Texture9 failed.. \n\n");
		return -14;
	}

	bResult = loadGLTexture_stb(&texture_Sphere[9], "Textures/Neptune.jpg");
	if (bResult == FALSE)
	{
		fprintf(gpFILE, "Loading of Texture10 failed.. \n\n");
		return -15;
	}

	return 0;

}

// STBI texture Loading (FFP)
BOOL loadGLTexture_stb(GLuint* texture, char* fileName)
{
	// Variable declarations 
	int width;
	int height;
	int components;
	BOOL bResult = FALSE;

	// fileName valya file chi width n height yat bharun deto//&components madhi kiti components yet te yat bharun yeil...3..4  // last parametre sangto ki mla kiti parameters have yet  
    //unsigned char* imageData = stbi_load(fileName, &width, &height, &components, 0); 

	unsigned char* imageData = stbi_load(fileName, &width, &height, &components, 0);

	if (imageData == NULL)
	{
		fprintf(gpFILE, "Failed to load texture %s \n", fileName);
		return bResult;
	}
	else
	{
		bResult = TRUE;

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		if (components == 3)
		{    // 1- target ,2-mipmap level , 6 - border ahe ka , 3- image format , 7 - opengl format donhi samech ast 99% , 8- datatype image cha , 9 - image cha data 
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

		}
		else if (components == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

		}

		//glGenerateMipMap(GL_TEXTURE_2D); 

		// unbind the texture 
		//glBindTexture(GL_TEXTURE_2D, 0); 

		// free stbi image
		//stbi_image_free(imageData);

	}

	return bResult;

}

void printGLInfo(void)
{
	// Local Variable declarations 
	GLint Num_Extensions;  // As a Array Use krtoy aapan ith
	GLint i = 0;

	// Code 
	fprintf(gpFILE, "OpenGL Vendor   : %s \n", glGetString(GL_VENDOR));
	fprintf(gpFILE, "OpenGL Rendorer : %s \n", glGetString(GL_RENDERER)); 
	fprintf(gpFILE, "OpenGL Version  : %s \n", glGetString(GL_VERSION)); 
	fprintf(gpFILE, "GLSL Version    : %s \n", glGetString(GL_SHADING_LANGUAGE_VERSION));   // Grafic library shading language

	// Listing of Supported Extensions 
	glGetIntegerv(GL_EXTENSIONS, &Num_Extensions); 

	for (i = 0; i < Num_Extensions; i++)
	{
		fprintf(gpFILE, "%s \n", glGetStringi(GL_EXTENSIONS, i));  
	}

}

void resize(int width, int height)
{
	// Code

	if (height <= 0) // precausion (height na division krnar y pudh )
		height = 1;

	// set Perspective projection matrix
	PerspectiveProjectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	
	glViewport(0, 0, (GLsizei)width, (GLsizei)height); // glViewport = binocular - durbin
}

void display(void)
{
	void update();
	void uninitialize();

	// Code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Blue color hya line mul set hoto
	
	// 1: use shader program 
	glUseProgram(shaderProgramObject);
	
	// Transformation 

	mat4 translationMatrix = vmath::mat4::identity(); 
	translationMatrix = vmath::translate(0.0f, 0.0f, -2.75f);

	mat4 rotationMatrix_sphere1 = vmath::rotate(-70.0f, 1.0f, 0.0f, 0.0f);
	mat4 rotationMatrix_sphere2 = vmath::rotate(30 - roate_Sphere, 0.0f, 0.0f, 1.0f);

	mat4 modelMatrix = translationMatrix * rotationMatrix_sphere1 * rotationMatrix_sphere2;
	mat4 viewMatrix = vmath::mat4::identity();


	// Push above MVP into vertex shaders mvpuniform 
	glUniformMatrix4fv(modelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(viewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, PerspectiveProjectionMatrix);

	if (bLightingEnabled == TRUE)
	{
		glUniform1i(keyPressedUniform, 1);
		glUniform3fv(lightDiffuseUniform, 1, lightDiffuse);
		glUniform3fv(lightAmbiantUniform, 1, lightAmbiant);
		glUniform3fv(lightSpecularUniform, 1, lightSpecular);
		glUniform4fv(lightPositionUniform, 1, lightPosition);

		glUniform3fv(materialDiffuseUniform, 1, materialDiffuse);
		glUniform3fv(materialAmbiantUniform, 1, materialAmbiant);
		glUniform3fv(materialSpecularUniform, 1, materialSpecular);
		glUniform1f(materialShinninesUniform, materialShinniness);
	}
	else
	{
		glUniform1i(keyPressedUniform, 0);
	}

	// for texture 

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_Sphere[0]);
	glUniform1i(textureSamplerUniform, 0);
	
	if (change_Count < 1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_Sphere[0]);
		glUniform1i(textureSamplerUniform, 0);

	}
	else if (change_Count == 1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_Sphere[1]);
		glUniform1i(textureSamplerUniform, 0);
	}
	else if (change_Count == 2)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_Sphere[2]);
		glUniform1i(textureSamplerUniform, 0);
	}
	else if (change_Count == 3)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_Sphere[3]);
		glUniform1i(textureSamplerUniform, 0);
	}
	else if (change_Count == 4)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_Sphere[4]);
		glUniform1i(textureSamplerUniform, 0);
	}
	
	else if (change_Count == 5)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_Sphere[5]);
		glUniform1i(textureSamplerUniform, 0);
	}
	else if (change_Count == 6)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_Sphere[6]);
		glUniform1i(textureSamplerUniform, 0);
	}
	
	else if (change_Count == 7)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_Sphere[7]);
		glUniform1i(textureSamplerUniform, 0);
	}
	else if (change_Count == 8)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_Sphere[8]);
		glUniform1i(textureSamplerUniform, 0);
	}
	else if (change_Count == 9)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_Sphere[9]);
		glUniform1i(textureSamplerUniform, 0);
	}

	else
	{
		uninitialize();
	}

	glBindVertexArray(vao_sphere);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_element_sphere);
		glDrawElements(GL_TRIANGLES, (unsigned int)vertices.size() * 2, GL_UNSIGNED_INT, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	// unbind with texture 
	glBindTexture(GL_TEXTURE_2D, 0);

	// 5 : unuse the shader program
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void update(void)
{
	// Code

	roate_Sphere = roate_Sphere + 0.01f;

	if (bLightingEnabled == TRUE)
	{
		lightPosition[0] = 5.0 * (GLfloat)sin(lightAngle_Y);
		lightPosition[1] = 0.0;
		lightPosition[2] = 5.0 * (GLfloat)cos(lightAngle_Y);
		lightPosition[3] = 100.0f;

		if (lightAngle_Y > -3.14)
		{
			if (change_Count == 7 || change_Count == 8 || change_Count == 9)
			{
				lightAngle_Y = lightAngle_Y - 0.004f;
			}
			else
			{
				lightAngle_Y = lightAngle_Y - 0.005f;
			}
		}
		else
		{
			lightAngle_Y = 3.14;
			change_Count++;

			roate_Sphere = 0.0f;
		}

	}
}

void uninitialize(void)
{
	// Function Declarations
	void ToggleFullscreen(void);
	
	// Code

	// Shader program uninitialize
	if (shaderProgramObject)
	{
		glUseProgram(shaderProgramObject);
		
		GLint numShaders = 0;

		glGetProgramiv(shaderProgramObject, GL_ATTACHED_SHADERS, &numShaders);
		
		if (numShaders > 0)
		{
			GLuint* pShaders = (GLuint*)malloc(numShaders * sizeof(GLuint));

			if (pShaders != NULL)
			{
				glGetAttachedShaders(shaderProgramObject, numShaders, NULL, pShaders);
				
				for (GLint i = 0; i < numShaders; i++)
				{
					glDetachShader(shaderProgramObject, pShaders[i]);
					glDeleteShader(pShaders[i]);
					pShaders[i] = 0;
				}
				// 6
				free(pShaders);
				pShaders = NULL;
			}
		}
		// 7
		glUseProgram(0);
		//8
		glDeleteProgram(shaderProgramObject);
		shaderProgramObject = 0;
	 }

	if (vbo_texture_sphere)
	{
		glDeleteBuffers(1, &vbo_texture_sphere);
		vbo_texture_sphere = 0;
	}
	if (vbo_position_sphere)
	{
		glDeleteBuffers(1, &vbo_position_sphere);
		vbo_position_sphere = 0;
	}
	// Deletion and uninitialization of vao_sphere
	if (vao_sphere)
	{
		glDeleteVertexArrays(1, &vao_sphere);
		vao_sphere = 0;
	}

	//-------------

	// if application is exiting in fullDcreen
	if (gbfullscreen == TRUE)    
	{
		ToggleFullscreen();     // jar kuni madhich esc kel tr window small kr fullscreen asl tr an mng close kr 
		gbfullscreen = FALSE; 
	}

	// Make the hdc as the current dc
	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	// destroy / delte rendering context
	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	// release the HDC 
	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	// Destroy Window
	if (ghwnd)
	{
		DestroyWindow(ghwnd);
		ghwnd = NULL;
	}
	
	// texture uninitialize
	for (int i = 0; i < 10; i++)
	{
		if (texture_Sphere[i])
		{
			glDeleteTextures(1, &texture_Sphere[i]);
			texture_Sphere[i] = 0;
		}
	}
	

	// Close the log file 
	if (gpFILE)
	{	
		fprintf(gpFILE, "\n\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
		fprintf(gpFILE, "Program Ended Successfully...\n"); 
		fprintf(gpFILE, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n\n");
		fclose(gpFILE);
		gpFILE = NULL;
	}

}

void generateSphereVertices(float radius, float noOfSectors, float noOfStacks)
{
	// variables declarations
	float sectorStep = 2 * M_PI / noOfSectors;			// sector angle is from 0 to 360
	float stackStep = M_PI / noOfStacks;				// stack angle is from 90 (top) to -90 (bottom)
	float stackAngle, sectorAngle;
	float x, y, z, xy;									// vertices				
	float nx, ny, nz;									// normals
	float lengthInv = 1.0f / radius;
	float s, t;											// texcoords

	// code
	for (int i = 0; i <= noOfStacks; ++i)
	{
		stackAngle = M_PI / 2 - i * stackStep;
		xy = radius * cosf(stackAngle);					// radius * cos(phi)
		z = radius * sinf(stackAngle);					// radius * sin(phi) 

		for (int j = 0; j <= noOfSectors; ++j)
		{
			sectorAngle = j * sectorStep;

			// vertices x, y, z
			x = xy * cosf(sectorAngle);					// radius * cos(phi) * cos(theta)
			y = xy * sinf(sectorAngle);					// radius * cos(phi) * sin(theta)
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);

			// vertex normals
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			normals.push_back(nx);
			normals.push_back(ny);
			normals.push_back(nz);

			// texcoords
			s = (float)j / noOfSectors;
			t = (float)i / noOfStacks;
			texcoords.push_back(s);
			texcoords.push_back(t);
		}
	}

	// To get the indicies as per stacks and sectors
	int k1, k2;
	for (int i = 0; i < noOfStacks; i++)
	{
		k1 = i * (noOfSectors + 1);
		k2 = k1 + noOfSectors + 1;

		for (int j = 0; j < noOfSectors; j++, k1++, k2++)
		{
			// k1 => k2 => k1+1
			if (i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if (i != (noOfStacks - 1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}

		}
	}
}

                                                                                                        