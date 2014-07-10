#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
#include <stdio.h>
#include <string>
#include <windows.h>

#include "include/VoxelField.h"
#include "include/MarchingCubes.h"


LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

GLvoid *font_style = GLUT_BITMAP_TIMES_ROMAN_24;

GLuint axesVertexBuffer = 0;
GLuint axesIndexBuffer = 0;

GLuint trianglesVertexBuffer = 0;
GLuint trianglesIndexedVertexBuffer = 0;
GLuint trianglesIndexedIndexBuffer = 0;


VoxelField      cf;
MarchingCubes   march(cf);

bool	lighting = true;
bool	anim = true;
bool	drawEdgesBool = false;
bool	geomNeedsUpdate = false;
bool	wireframe = false;
bool	print = false;
float	phase = 0.0f;

int		currentTestCase = 1;

float	sideAngle =   -45.0f;
float	upAngle =   45.0f;
float	zoomOut   = 40.0f;

bool	shiftBtn = false;

int		currentAxis = 0;
int		debugAxes[3] = {0,0,0};

const	int					MAX_TRIS = 4000;
MarchingCubes::Vertex       verts[MAX_TRIS];
MarchingCubes::TriangleI    trisI[MAX_TRIS];
int		vertexNum = 0;
int		triNum = 0;
int		activeTriangle = -1;

int		GRID_SIZE_X = 20;
int		GRID_SIZE_Y = 20;
int		GRID_SIZE_Z = 20;

GLfloat ambientColor[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat lightColorRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightColorGreen[] = {0.0f, 1.0f, 0.0f, 1.0f};
GLfloat lightColorBlue[] = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightColorWhite[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightColorRedHalf[] = {0.5f, 0.0f, 0.0f, 1.0f};
GLfloat lightColorGreenHalf[] = {0.0f, 0.5f, 0.0f, 1.0f};
GLfloat lightColorBlueHalf[] = {0.0f, 0.0f, 0.5f, 1.0f};
GLfloat lightColorWhiteHalf[] = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat lightPosRed[] = {50.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightPosGreen[] = {0.0f, 50.0f, 0.0f, 1.0f};
GLfloat lightPosBlue[] = {0.0f, 0.0f, 50.0f, 1.0f};
GLfloat lightPosWhite[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat materialColorSpec[] = {0.0f, 0.0f, 0.0f, 1.0f};

void drawTrianglesIndexedWireframe( MarchingCubes::Vertex* verts, MarchingCubes::TriangleI* trisI, int triNum );
void drawTrianglesIndexedNormals( MarchingCubes::Vertex* verts, MarchingCubes::TriangleI* trisI, int triNum );


void printText( float x, float y, std::string str )
{
    int len = str.length();
	glRasterPos3f (x, y, -1);

    for( int i = 0; i < len; i++)
        glutBitmapCharacter( font_style, str[i] );
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
        glShadeModel( GL_SMOOTH );
    }
    else {
        glDisable( GL_LIGHTING );
    }
}

void setupLightParams()
{
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, ambientColor );

	glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, lightColorWhite );
	glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, materialColorSpec );
	glMaterialf( GL_FRONT_AND_BACK, GL_SHININESS, 1 );

	glLightfv( GL_LIGHT0, GL_DIFFUSE, lightColorRed );	//Half );
	glLightfv( GL_LIGHT0, GL_SPECULAR, materialColorSpec );	//lightColorRedHalf );
	glLightfv( GL_LIGHT0, GL_POSITION, lightPosRed );

	glLightfv( GL_LIGHT1, GL_DIFFUSE, lightColorGreen );	//Half );
	glLightfv( GL_LIGHT1, GL_SPECULAR, materialColorSpec );	//lightColorGreenHalf );
	glLightfv( GL_LIGHT1, GL_POSITION, lightPosGreen );

	glLightfv( GL_LIGHT2, GL_DIFFUSE, lightColorBlue );	//Half );
	glLightfv( GL_LIGHT2, GL_SPECULAR, materialColorSpec );	//lightColorBlueHalf );
	glLightfv( GL_LIGHT2, GL_POSITION, lightPosBlue );

	glLightfv( GL_LIGHT3, GL_DIFFUSE, lightColorWhite );	//Half );
	glLightfv( GL_LIGHT3, GL_SPECULAR, materialColorSpec );	//lightColorWhiteHalf );
	glLightfv( GL_LIGHT3, GL_POSITION, lightPosWhite );
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

	glEnable(
//	glDisable(
			GL_CULL_FACE );
    glEnable( GL_DEPTH_TEST );

    return 1;
}

void drawDebugAxes()
{
	if( wireframe )
	{
        float x = debugAxes[0];
        float y = debugAxes[1];
        float z = debugAxes[2];

		float axisLen = 2.0f;

        setupLight( false );
        glBegin(GL_LINES);
            glColor3f( 1.0f, 1.0f, 1.0f );
            glVertex3f( x, y, z );
            glColor3f( 1.0f, 0.0f, 0.0f );
            glVertex3f( x+axisLen, y, z );

            glColor3f( 1.0f, 1.0f, 1.0f );
            glVertex3f( x, y, z );
            glColor3f( 0.0f, 1.0f, 0.0f );
            glVertex3f( x, y+axisLen, z );

            glColor3f( 1.0f, 1.0f, 1.0f );
            glVertex3f( x, y, z );
            glColor3f( 0.0f, 0.0f, 1.0f );
            glVertex3f( x, y, z+axisLen );
        glEnd();

		float debugPlaneLen = 1.0f;
		glDisable( GL_CULL_FACE );
        glBegin(GL_QUADS);
            glColor3f( 1.0f, 0.0f, 0.0f );
            glVertex3f( x, y, z );
            glVertex3f( x+debugPlaneLen, y, z );
            glVertex3f( x+debugPlaneLen, y+debugPlaneLen, z );
            glVertex3f( x, y+debugPlaneLen, z );

            glColor3f( 0.0f, 1.0f, 0.0f );
            glVertex3f( x, y, z );
            glVertex3f( x, y+debugPlaneLen, z );
            glVertex3f( x, y+debugPlaneLen, z+debugPlaneLen );
            glVertex3f( x, y, z+debugPlaneLen );

            glColor3f( 0.0f, 0.0f, 1.0f );
            glVertex3f( x, y, z );
            glVertex3f( x+debugPlaneLen, y, z );
            glVertex3f( x+debugPlaneLen, y, z+debugPlaneLen );
            glVertex3f( x, y, z+debugPlaneLen );
		glEnd();

//        setupLight( false );	//lighting );
        glColor3f( 1.0f, 1.0f, 1.0f );

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1,-1);

		drawTrianglesIndexedWireframe( verts, trisI, triNum );

		glColor3f( 1.0f, 1.0f, 1.0f );
		glLineWidth( 2.0f );
		drawTrianglesIndexedNormals( verts, trisI, triNum );

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1,1);
	}
}


void updateGeometry()
{
	for( int i = 0; i < MAX_TRIS; i++ ) {
		verts[i].norm.setValue( 0.0f, 0.0f, 0.0f );
		verts[i].used = 0;
	}

	int triangleNum = march.fillInTrianglesIndexed( verts, MAX_TRIS, trisI, MAX_TRIS, vertexNum, triNum );
}

void drawTrianglesIndexed( MarchingCubes::Vertex* verts, MarchingCubes::TriangleI* trisI, int triNum )
{
	glBegin( GL_TRIANGLES );
	for( int i = 0; i < triNum; i++ )	//	triangle
	{
			if( activeTriangle >= 0 && activeTriangle != i )	// Draw all triangles or only the active one
				continue;										// just for debugging, TODO: Remove
		MarchingCubes::TriangleI& tri = trisI[i];

		for( int j = 0; j < 3; j++ ) {	//	corner
			int vertIndex = tri[j];
			MarchingCubes::Vector3F& pos = verts[vertIndex].pos;
			MarchingCubes::Vector3F& norm = verts[vertIndex].norm;

			glNormal3f( norm.f[0], norm.f[1], norm.f[2] );
			glVertex3f( pos.f[0], pos.f[1], pos.f[2] );
		}
	}
	glEnd();

	drawDebugAxes();
}

void drawTrianglesIndexedWireframe( MarchingCubes::Vertex* verts, MarchingCubes::TriangleI* trisI, int triNum )
{
	glBegin( GL_LINES );
	for( int i = 0; i < triNum; i++ )	//	triangle
	{
		MarchingCubes::TriangleI& tri = trisI[i];

		for( int j = 0; j < 3; j++ ) {	//	corner
			int vertIndex = tri[j];
			MarchingCubes::Vector3F& pos = verts[vertIndex].pos;
			glVertex3f( pos.f[0], pos.f[1], pos.f[2] );

			int vertIndex2 = tri[(j+1)%3];	// next vertex in line loop
			MarchingCubes::Vector3F& pos2 = verts[vertIndex2].pos;
			glVertex3f( pos2.f[0], pos2.f[1], pos2.f[2] );
		}
	}
	glEnd();
}

void drawTrianglesIndexedNormals( MarchingCubes::Vertex* verts, MarchingCubes::TriangleI* trisI, int triNum )
{
	float NORM_LEN = 0.4f;
	glBegin( GL_LINES );
	for( int i = 0; i < triNum; i++ ) {	//	triangle
		MarchingCubes::TriangleI& tri = trisI[i];
		for( int j = 0; j < 3; j++ ) {	//	corner
			int vertIndex = tri[j];
			MarchingCubes::Vector3F& pos = verts[vertIndex].pos;
			MarchingCubes::Vector3F& norm = verts[vertIndex].norm;

			glVertex3f( pos.f[0], pos.f[1], pos.f[2] );
			glVertex3f( pos.f[0] + norm.f[0]*NORM_LEN,
						pos.f[1] + norm.f[1]*NORM_LEN,
						pos.f[2] + norm.f[2]*NORM_LEN );
		}
	}
	glEnd();
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
	glColor3f( 1.0f, 1.0f, 1.0f );
}

void drawTris( float x, float y, float z, MarchingCubes::TriangleF* tris, int triNum )
{
	glBegin( GL_TRIANGLES );
	for( int t = 0; t < triNum; t++ ) {
		MarchingCubes::TriangleF&     tri = tris[t];
		glColor3f( 0.9f, 0.9f, 0.9f );

//		MarchingCubes::Vector3F  delta1 = tri.v[1].pos - tri.v[0].pos;
//		MarchingCubes::Vector3F  delta2 = tri.v[2].pos - tri.v[0].pos;
//		MarchingCubes::Vector3F  normal;
//		MarchingCubes::getCrossProduct( delta1.f, delta2.f, normal.f );
		MarchingCubes::Vector3F  normal = MarchingCubes::getTriangleNormal( tri.v[0].pos, tri.v[1].pos, tri.v[2].pos );

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
	glBegin( GL_LINES );
	for( int t = 0; t < triNum; t++ ) {
		MarchingCubes::TriangleF&     tri = tris[t];

        glColor3f( 1.0f, 1.0f, 1.0f );
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
//	cf.setPerlinNoise( 0 );
	cf.setSpheres( phase );
	return 1;
}


void printTime()
{
    static double lastTime = glutGet(GLUT_ELAPSED_TIME);

    static int nbFrames = 0;
    static float fps = 0;

     // Measure speed
     double currentTime = glutGet(GLUT_ELAPSED_TIME);
     nbFrames++;
     if ( currentTime - lastTime >= 100.0 ){
         fps = double(nbFrames)*1000.0/
         (currentTime - lastTime);
         nbFrames = 0;
         lastTime = currentTime;
     }

     char buff[64];
     sprintf( buff, "FPS: %.2f", fps );
     printText( -1, -1, buff );
     sprintf( buff, "phase: %.2f", phase );
     printText( -1, -0.9, buff );
     sprintf( buff, "x:%d, y:%d, z:%d", debugAxes[0], debugAxes[1], debugAxes[2] );
     printText( -1, -0.8, buff );

			int debugCubeIdx = -1;
			{
				Cube2 cube = cf.getCube( debugAxes[0], debugAxes[1], debugAxes[2] );
				cube.setGridSize( cf.getSizeX(), cf.getSizeY(), cf.getSizeZ() );

				march.setValues( cube );
				MarchingCubes::MarchingCubesCase &cubeCase = march.getCaseFromValues();
				debugCubeIdx = cubeCase.index;
			}
     sprintf( buff, "actTri:%d, i0:%d, i1:%d, i2:%d,  case:%d",
					activeTriangle, trisI[activeTriangle].i[0],
					trisI[activeTriangle].i[1], trisI[activeTriangle].i[2], debugCubeIdx );
     printText( -1, -0.7, buff );
}


void drawFrame()
{
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	setView();

	setupLightParams();
	setupLight( lighting );

	drawTrianglesIndexed( verts, trisI, triNum );

	setupLight( false );
	drawAxes();

	glLoadIdentity ();
	glColor3f( 1, 1, 1 );

	printTime();
	print = false;
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

    char *myargv [1];
    int myargc=1;
    myargv [0]=strdup ("Myappname");

    glutInit( &myargc, myargv );

	cout<<"Renderer = "<<glGetString(GL_RENDERER)<<endl<<endl;
	cout<<"Version = "<<glGetString(GL_VERSION)<<endl<<endl;
	cout<<"Extensions =\n"<<glGetString(GL_EXTENSIONS)<<endl<<endl;

	GLenum err = glewInit();
	if (err!=GLEW_OK)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		cout << "glewInit failed, aborting." << endl;
		return 0;
	}

	printf("OpenGL version supported by this platform (%s): \n", glGetString(GL_VERSION));

//    generateAxesVBO();
//    generateTrianglesVBO();
//    generateTrianglesIndexedVBO();


	march.init();
	cf.setSize( GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z );

	while (!bQuit)
	{
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
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
			if( anim ) {
;//				phase += 0.05;
			}

							phase = //59.35;	//
									23.85f;	//20,20,20

			if( currentTestCase >= 0 && currentTestCase < 6 )
				cf.setAmbiguousCase( currentTestCase );
			else if( currentTestCase == 6 ) {
				cf.setSize( GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z );
				cf.setPerlinNoise( 0 );
			}
			else if( currentTestCase == 7 )
				updateVoxelField( phase );
//	cf.setZeroSlice();

            if( anim || geomNeedsUpdate ) {
				updateGeometry() ;
				geomNeedsUpdate = false;
            }

			drawFrame();
			SwapBuffers(hDC);
        }
    }
//    deleteAxesVBO();
//    deleteTrianglesVBO();
//    deleteTrianglesIndexedVBO();

    DisableOpenGL( hwnd, hDC, hRC );
    DestroyWindow( hwnd );

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

        case WM_KEYUP:
		{
            switch (wParam)
            {
				case VK_SHIFT:
					shiftBtn = false;
				break;
            }
            break;
		}

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_SHIFT:
                    shiftBtn = true;
                break;

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

                case 'F':
                    wireframe = !wireframe;
                break;

                case 'X':
                    currentAxis = 0;
                break;
                case 'Y':
                    currentAxis = 1;
                break;
                case 'Z':
                    currentAxis = 2;
                break;
                case 'E':
                    drawEdgesBool = !drawEdgesBool;
                break;

                case VK_OEM_4:
                    if( debugAxes[currentAxis] > 0 )
                        debugAxes[currentAxis]--;
                break;
                case VK_OEM_6:
                    debugAxes[currentAxis]++;
                break;

                case VK_OEM_PLUS:
				{
                	if( shiftBtn )
						activeTriangle += 100;
                	else
						activeTriangle++;
					break;
				}
                case VK_OEM_MINUS:
				{
					if( shiftBtn && activeTriangle >= 100 )
						activeTriangle -= 100;
                	else if( activeTriangle >= 0 )
						activeTriangle--;
					break;
				}

                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
				{
					currentTestCase = wParam - '1';
					break;
				}
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
