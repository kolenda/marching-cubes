#include <stdio.h>
#include <windows.h>
#include <gl/gl.h>
#include <string>

#include "include/VoxelField.h"
#include "include/MarchingCubes.h"


LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

//GLvoid *font_style = GLUT_BITMAP_TIMES_ROMAN_24;

GLuint axesVertexBuffer = 0;
GLuint axesIndexBuffer = 0;

GLuint trianglesVertexBuffer = 0;
GLuint trianglesIndexedVertexBuffer = 0;
GLuint trianglesIndexedIndexBuffer = 0;

VoxelField      cf;
MarchingCubes   march(cf);
//MarchingCubes   march;
//VoxelField      cf;

bool lighting = true;
bool anim = true;
bool drawEdgesBool = false;
bool print = false;
float phase = 0.0f;

float sideAngle =   -45.0f;
float upAngle =   45.0f;
float zoomOut   = 40.0f;

int currentAxis = 0;
int debugAxes[3] = {0,0,0};

int	GRID_SIZE_X = 40;
int GRID_SIZE_Y = 40;
int GRID_SIZE_Z = 40;

GLfloat ambientColor[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat lightColorRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightColorGreen[] = {0.0f, 1.0f, 0.0f, 1.0f};
GLfloat lightColorBlue[] = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightColorWhite[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightColorRedHalf[] = {0.5f, 0.0f, 0.0f, 1.0f};
GLfloat lightColorGreenHalf[] = {0.0f, 0.5f, 0.0f, 1.0f};
GLfloat lightColorBlueHalf[] = {0.0f, 0.0f, 0.5f, 1.0f};
GLfloat lightColorWhiteHalf[] = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat lightPosRed[] = {40.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightPosGreen[] = {0.0f, 40.0f, 0.0f, 1.0f};
GLfloat lightPosBlue[] = {0.0f, 0.0f, 40.0f, 1.0f};
GLfloat lightPosWhite[] = {0.0f, 0.0f, 0.0f, 1.0f};


struct AxisVert {
    GLfloat pos[3];
    GLubyte color[3];
};

void printText( float x, float y, std::string str )
{
    int len = str.length();
	glRasterPos3f (x, y, -1);

//    for( int i = 0; i < len; i++)
//        glutBitmapCharacter( font_style, str[i] );
}

void setupLight( bool on )
{
    if( on ) {
        glEnable( GL_LIGHTING );
        glEnable( GL_LIGHT0 );
        glEnable( GL_LIGHT1 );
        glEnable( GL_LIGHT2 );
        glEnable( GL_LIGHT3 );
        glEnable( GL_NORMALIZE );
        glShadeModel( GL_FLAT );

        glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambientColor );

        glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, lightColorWhite );
        glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, lightColorWhite );
        glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, 10);

        glLightfv( GL_LIGHT0, GL_DIFFUSE, lightColorRedHalf );
        glLightfv( GL_LIGHT0, GL_SPECULAR, lightColorRedHalf );
        glLightfv( GL_LIGHT0, GL_POSITION, lightPosRed );

        glLightfv( GL_LIGHT1, GL_DIFFUSE, lightColorGreenHalf );
        glLightfv( GL_LIGHT1, GL_SPECULAR, lightColorGreenHalf );
        glLightfv( GL_LIGHT1, GL_POSITION, lightPosGreen );

        glLightfv( GL_LIGHT2, GL_DIFFUSE, lightColorBlueHalf );
        glLightfv( GL_LIGHT2, GL_SPECULAR, lightColorBlueHalf );
        glLightfv( GL_LIGHT2, GL_POSITION, lightPosBlue );

        glLightfv( GL_LIGHT3, GL_DIFFUSE, lightColorWhiteHalf );
        glLightfv( GL_LIGHT3, GL_SPECULAR, lightColorWhiteHalf );
        glLightfv( GL_LIGHT3, GL_POSITION, lightPosWhite );
    }
    else {
        glDisable( GL_LIGHTING );
    }
}

int setView()
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -0.1, 0.1, -0.1, 0.1, 0.1, 100.0 );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef( 0.0f, -0.0f, -zoomOut  );
    glRotatef( upAngle, 1.0f, 0.0f, 0.0f );
    glRotatef( sideAngle, 0.0f, 1.0f, 0.0f );
    glTranslatef( -cf.getSizeX()/2, -cf.getSizeY()/2, -cf.getSizeZ()/2 );
    //glEnable	//
    glDisable
			( GL_CULL_FACE );
    glEnable( GL_DEPTH_TEST );

    return 1;
}

void drawAxes()
{
    glBegin( GL_LINES );
        glColor3f( 1.0f, 1.0f, 1.0f );
        glVertex3f( 0, 0, 0 );
        glColor3f( 1.0f, 0.0f, 0.0f );
        glVertex3f( cf.getSizeX(), 0, 0 );

        glColor3f( 1.0f, 1.0f, 1.0f );
        glVertex3f( 0, 0, 0 );
        glColor3f( 0.0f, 1.0f, 0.0f );
        glVertex3f( 0, cf.getSizeY(), 0 );

        glColor3f( 1.0f, 1.0f, 1.0f );
        glVertex3f( 0, 0, 0 );
        glColor3f( 0.0f, 0.0f, 1.0f );
        glVertex3f( 0, 0, cf.getSizeZ() );
    glEnd();
}
void drawTris( float x, float y, float z, MarchingCubes::TriangleF* tris, int triNum )
{
//	int counter = 0;
	glBegin( GL_TRIANGLES );
	for( int t = 0; t < triNum; t++ ) {
		MarchingCubes::TriangleF&     tri = tris[t];
		glColor3f( 0.9f, 0.9f, 0.9f );

		MarchingCubes::Vector3F  delta1 = tri.v[1].pos - tri.v[0].pos;
		MarchingCubes::Vector3F  delta2 = tri.v[2].pos - tri.v[0].pos;

		MarchingCubes::Vector3F  normal;
		MarchingCubes::getCrossProduct( delta1.f, delta2.f, normal.f );

		glNormal3f( normal.f[0], normal.f[1], normal.f[2] );

		glVertex3f( tri.v[0].pos.f[0]+x, tri.v[0].pos.f[1]+y, tri.v[0].pos.f[2]+z );
		glVertex3f( tri.v[1].pos.f[0]+x, tri.v[1].pos.f[1]+y, tri.v[1].pos.f[2]+z );
		glVertex3f( tri.v[2].pos.f[0]+x, tri.v[2].pos.f[1]+y, tri.v[2].pos.f[2]+z );

		// if you press 'p' key it will print current geometry to the console
		if( print ) {
			printf( "%.2f %.2f %.2f \t%.2f %.2f %.2f \t%.2f %.2f %.2f\n",
					tri.v[0].pos.f[0]+x, tri.v[0].pos.f[1]+y, tri.v[0].pos.f[2]+z,
					tri.v[1].pos.f[0]+x, tri.v[1].pos.f[1]+y, tri.v[1].pos.f[2]+z,
					tri.v[2].pos.f[0]+x, tri.v[2].pos.f[1]+y, tri.v[2].pos.f[2]+z );
		}
	}
	glEnd();
}
void drawEdges( float x, float y, float z, MarchingCubes::TriangleF* tris, int triNum )
{
	//  triangle edges
	glBegin(GL_LINES);
	for( int t = 0; t < triNum; t++ ) {
		MarchingCubes::TriangleF&     tri = tris[t];

		glColor3f( 0.0f, 0.0f, 0.0f );
		glVertex3f( tri.v[0].pos.f[0]+x, tri.v[0].pos.f[1]+y, tri.v[0].pos.f[2]+z );
		glVertex3f( tri.v[1].pos.f[0]+x, tri.v[1].pos.f[1]+y, tri.v[1].pos.f[2]+z );

		glVertex3f( tri.v[1].pos.f[0]+x, tri.v[1].pos.f[1]+y, tri.v[1].pos.f[2]+z );
		glVertex3f( tri.v[2].pos.f[0]+x, tri.v[2].pos.f[1]+y, tri.v[2].pos.f[2]+z );

		glVertex3f( tri.v[2].pos.f[0]+x, tri.v[2].pos.f[1]+y, tri.v[2].pos.f[2]+z );
		glVertex3f( tri.v[0].pos.f[0]+x, tri.v[0].pos.f[1]+y, tri.v[0].pos.f[2]+z );
	}
	glEnd();
}

int updateVoxelField( float phase )
{
	cf.setAllValues( -0.5f );
	float x = 0.4 * cf.getSizeX() * sin(phase*0.1) + 0.5 * cf.getSizeX();
	float y = 0.4 * cf.getSizeY() * cos(phase*0.2) + 0.5 * cf.getSizeY();
	float z = 0.4 * cf.getSizeZ() * sin(1+phase*0.15) + 0.5 * cf.getSizeZ();
	float rad = cf.getSizeX() / 4;
	cf.addSphere( x, y, z, rad );
	cf.addSphere( y, z, x, rad );
	cf.addSphere( z, x, y, rad );

	cf.addSphere( z, y, x, rad );
	cf.addSphere( y, x, z, rad );
	cf.addSphere( x, z, y, rad );

	return 1;
}


void iterate()
{
	for( int x = 0; x < cf.getSizeX()-1; x++ )
	for( int y = 0; y < cf.getSizeY()-1; y++ )
	for( int z = 0; z < cf.getSizeZ()-1; z++ )
	{
		Cube2 cube = cf.getCube( x, y, z );
		cube.setGridSize( GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z );

		if( cube.notEmpty() )
		{
			MarchingCubes::TriangleF     tris[8];

			march.setValues( cube );	//.vec );
			int triNum = march.fillInTriangles( tris );

			if( triNum )
			{
				drawTris( x, y, z, tris, triNum );

/*				if( drawEdgesBool ) {
					drawEdges( x, y, z, tris, triNum );
				}*/
			} //if triNum
		}
	}
}

void drawFrame()
{
	setupLight( lighting );
	setView();

	iterate();
}


void printUsageStats()
{
    printf( "Usage stats:" );
    int rowSize = 16;
    int casesOk = 0;
    int casesEmpty = 0;
    for( int i = 0; i < 256; i++ ) {
        if( !(i%rowSize) )
            printf( "\n%d: ", i );

        int usageI = march.getUsageStats(i);
        printf( "%d ", usageI );

        if( i != 0 && i != 255 ) {
            MarchingCubes::MarchingCubesCase& cubeCase = march.getCase(i);
            if( cubeCase.numTri > 0 )
                casesOk += usageI;
            else
                casesEmpty += usageI;
        }
    }
    printf( "\ncasesOk:%d\tcasesEmpty:%d\n", casesOk, casesEmpty );
    printf( "casesOk:%e\tcasesEmpty:%e\n", (float)casesOk, (float)casesEmpty );
}



int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
//    float theta = 0.0f;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;

    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "Marching Cubes",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          512,
                          512,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

		march.init();
		cf.setSize(	//				5, 5, 5 );
				GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z );
//				40, 40, 40 );
//				10, 10, 10 );



    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
				if( anim )
					phase += 0.05;

				updateVoxelField( phase );

            /* OpenGL animation code goes here */


            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


				drawFrame();

//            glPopMatrix();
            print = false;
            SwapBuffers(hDC);

//            Sleep (1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);
    printUsageStats();

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;

                case VK_LEFT:
                    sideAngle += 5.0f;
                break;
                case VK_RIGHT:
                    sideAngle -= 5.0f;
                break;
                case VK_UP:
                    upAngle += 5.0f;
                break;
                case VK_DOWN:
                    upAngle -= 5.0f;
                break;
                case VK_PRIOR:
                    zoomOut *= 0.9f;
                break;
                case VK_NEXT:
                    zoomOut *= 1.1f;
                break;
                case 'P':
                    print = true;
                break;
                case 'W':
                    phase += 0.5f;
                break;
                case 'Q':
                    phase -= 0.5f;
                break;
                case 'L':
                    lighting = !lighting;
                break;
                case 'A':
                    anim = !anim;
                break;
                case 'E':
                    drawEdgesBool = !drawEdgesBool;
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

