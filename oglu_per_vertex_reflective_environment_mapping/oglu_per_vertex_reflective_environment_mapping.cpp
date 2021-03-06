//------------------------------------------------------------------------------
//           Name: oglu_per_vertex_reflective_environment_mapping.cpp
//         Author: Christoph Kabek
//  Last Modified: 07/17/07
//    Description: This sample shows how to do reflective environment mapping on
//				   a per-vertex basis using Cg.
//
//   Control Keys: 1  - Draws skull
//				   2  - Draws teapot
//				   3  - Draws sphere
//				   w  - Toggles wire frame mode
//				   r  - Toggles rotation
//				   i  - Zooms in
//				   o  - Zooms out
//				   s  - Toggles skybox rendering
//
//  Note: System requirements are any accelerated 3D card which supports 
//		  vertex and pixel shaders (shader model 1.1) and the following 
//		  OpenGL 1.3 extensions:
//		  GL_ARB_vertex_program/GL_ARB_fragment_program.
//		  Textures of the nVidia SDKs were used.
//		  This sample was tested on Windows XP + SP2 on an ATi Radeon 9800 Pro,
//		  X700 mobile and on an nVidia GeForce FX 5200, a 6200 and a 7600 GT
//		  and also with nVidia Cg 1.3 to 1.5.
//		  The X700 mobile, however, displayed no objects for some reason, but I
//		  noticed the same problems with many other demos, e.g. of the nVidia 
//		  SDK, and it troubled me a lot with other projects in the past, so I 
//		  guess it's an ATi driver's bug, since other chipsets displayed 
//		  everything fine.
//		  You can use the code for any non-commercial use, but give the author
//		  full credit if you do so.
//		  It was NOT tested on performance and it was not programmed that way.
//		  If you want to comment on this one, or if you experience or see any
//		  errors, I would be glad to know them:
//		  chrisDOTgraphicsATgmxDOTnet
//------------------------------------------------------------------------------

// Trim down the footprint
#define STRICT
#define WIN32_LEAN_AND_MEAN
#define VC_LEANMEAN

// Disable deprecation
#define _CRT_SECURE_NO_DEPRECATE

// Link to necessary libraries
#pragma comment ( lib, "opengl32.lib" )
#pragma comment ( lib, "glu32.lib" )
#pragma comment ( lib, "glaux.lib" )
#pragma comment ( lib, "cg.lib" )
#pragma comment ( lib, "cggl.lib" )

// Standard
#include <windows.h>
#include <stdio.h>

// Cg
#include <Cg/cgGL.h>
#include <Cg/cg.h>

// GL
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>

#include "resource.h"

// Note: This sample ships its own extension header.
//		 For using your local file, comment out the next line and uncomment
//		 the following line.
#include "glext.h"
//#include <GL/glext.h>

// For loading 3DS models, copyright (c) 2001 by Lev Povalahev
#include "l3ds.h"

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------

// Set view and rotation parameters
GLfloat g_fEye[3] = { 0.0f,  0.0f, 5.0f };
GLfloat g_fRot[3] = { 0.0f, 45.0f, 0.0f };

// 3DS objects
L3DS skull, teapot, sphere;
L3DS *obj;
GLuint g_uiCurrentObj = 1;

// Cg parameters
CGprofile cgVertexProfile, cgFragmentProfile;
CGcontext cgContext;
CGprogram cgVertexProgram, cgFragmentProgram;

// Used for 3DS model rendering via vertex arrays
CGparameter cgNormal, cgPosition, cgTexcoords;

// Matrices
CGparameter cgModelViewProj, cgModelView, cgModelViewI, cgModelViewIT;

// Textures
CGparameter cgEnvironmentMap;

// Further parameters
CGparameter cgEyePosition;

// Flags
BOOL g_bRotate		= TRUE;
BOOL g_bDrawSkybox	= TRUE;

// Constants defining the cubemap faces
GLuint g_uiCubeMapConstants[6] = {

	GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
};

// Location of cubemap textures
char *g_cCubemaps[6] = {

	"terrain_posx.bmp",
	"terrain_negx.bmp",
	"terrain_posy.bmp",
	"terrain_negy.bmp",
	"terrain_posz.bmp",
	"terrain_negz.bmp"
};

// Texture IDs
GLuint	g_uiCubemapTexture;

// Window related globals
GLfloat	g_fWidth   = 1024.0f;
GLfloat	g_fHeight  = 768.0f;

HDC	      g_hDC       = NULL;
HGLRC     g_hRC       = NULL;
HWND      g_hWnd      = NULL;
HINSTANCE g_hInstance = NULL;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE g_hInstance, HINSTANCE hPrevInstance, 
					LPSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WindowProc( HWND g_hWnd, UINT msg, WPARAM wParam, 
					LPARAM lParam );
void loadCubeMap( void );
void display( void );
void cgErrorCallback( void );
void init( void );
void drawSkyBox( void );
void cleanExit( int exitval );
void drawtext( void );
void draw3DSObject( void );


//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: The application's entry point
//-----------------------------------------------------------------------------
int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPSTR     lpCmdLine,
					int       nCmdShow ) {

	WNDCLASSEX winClass;
	MSG        uMsg;

    memset( &uMsg,0,sizeof( uMsg ) );

	// Specify window properties
	winClass.lpszClassName = "MY_WINDOWS_CLASS";
	winClass.cbSize        = sizeof( WNDCLASSEX );
	winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	winClass.lpfnWndProc   = WindowProc;
	winClass.hInstance     = hInstance;
    winClass.hIcon	       = LoadIcon( hInstance, ( LPCTSTR )IDI_OPENGL_ICON );
    winClass.hIconSm	   = LoadIcon( hInstance, ( LPCTSTR )IDI_OPENGL_ICON );
	winClass.hCursor       = LoadCursor( NULL, IDC_ARROW );
	winClass.hbrBackground = ( HBRUSH )GetStockObject( BLACK_BRUSH );
	winClass.lpszMenuName  = NULL;
	winClass.cbClsExtra    = 0;
	winClass.cbWndExtra    = 0;
	
	if( !RegisterClassEx( &winClass ) )
		return E_FAIL;

	// Create window
	g_hWnd = CreateWindowEx( NULL,"MY_WINDOWS_CLASS",
						    "OpenGL - Per-Vertex Reflective Environment Mapping Using Cg",
							WS_OVERLAPPEDWINDOW,
					 	    0, 0, ( GLint )g_fWidth, ( GLint )g_fHeight, NULL, NULL, 
							g_hInstance, NULL );

	if( g_hWnd == NULL )
		return E_FAIL;

	// Obligatory calls
    ShowWindow( g_hWnd, nCmdShow );
    UpdateWindow( g_hWnd );

	// Initialize all stuff
	init();

	// Process commands
	while( uMsg.message != WM_QUIT )
	{
		if( PeekMessage( &uMsg, NULL, 0, 0, PM_REMOVE ) )
		{ 
			TranslateMessage( &uMsg );
			DispatchMessage( &uMsg );
		}
        else
			// Main render function
		    display();
	}

	cleanExit( 0 );

    UnregisterClass( "MY_WINDOWS_CLASS", g_hInstance );

	return uMsg.wParam;
}


//-----------------------------------------------------------------------------
// Name: WindowProc()
// Desc: The window's message handler
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc( HWND   g_hWnd, 
							 UINT   msg, 
							 WPARAM wParam, 
							 LPARAM lParam ) {

    static POINT ptLastMousePosit;
	static POINT ptCurrentMousePosit;
	static bool bMousing;
    
    switch( msg )
	{
        case WM_KEYDOWN:
		{	
			switch( wParam )
			{
				case VK_ESCAPE:
					PostQuitMessage( 0 );
					cleanExit( 0 );
					break;

				case 'R':
					g_bRotate = !g_bRotate;
					break;

				case 'W':
					GLint mode[2];
					glGetIntegerv( GL_POLYGON_MODE, mode );
					if( mode[1] == GL_FILL ) {
						glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); 
					}
					else if( mode[1] == GL_LINE ) {
						glPointSize( 4 );
						glPolygonMode( GL_FRONT_AND_BACK, GL_POINT ); 
					}
					else {
						glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ); 
					}
					break;

				case 'I':
					g_fEye[2] -= 0.1f;
					break;

				case 'O':
					g_fEye[2] += 0.1f;
					break;

				case 'S':
					g_bDrawSkybox = !g_bDrawSkybox;
					break;

				case '1':
				case '2':
				case '3':
					g_uiCurrentObj = wParam - '0';
					break;
			}
		}
        break;

        case WM_LBUTTONDOWN:
			ptLastMousePosit.x = ptCurrentMousePosit.x = LOWORD( lParam );
            ptLastMousePosit.y = ptCurrentMousePosit.y = HIWORD( lParam );
			bMousing = true;
			break;

		case WM_LBUTTONUP:
			bMousing = false;
			break;

		case WM_MOUSEMOVE:
			ptCurrentMousePosit.x = LOWORD( lParam );
			ptCurrentMousePosit.y = HIWORD( lParam );
			if( bMousing ) {
				g_fRot[1] += ( ptCurrentMousePosit.x - ptLastMousePosit.x );
				g_fRot[0] += ( ptCurrentMousePosit.y - ptLastMousePosit.y );
			}
			ptLastMousePosit.x = ptCurrentMousePosit.x;
            ptLastMousePosit.y = ptCurrentMousePosit.y;
			break;
		
		case WM_SIZE:
		{
			int nWidth  = LOWORD( lParam ); 
			int nHeight = HIWORD( lParam );
			glViewport( 0, 0, nWidth, nHeight );
			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			gluPerspective( 45.0, 
							( GLfloat )nWidth / ( GLfloat )nHeight, 
							0.1, 100.0 );
		}
		break;

		case WM_CLOSE:
			cleanExit( 0 );
			PostQuitMessage( 0 );	
			break;

        case WM_DESTROY:
			cleanExit( 0 );
            PostQuitMessage( 0 );
			break;
		
		default:
			return DefWindowProc( g_hWnd, msg, wParam, lParam );
			break;
	}

	return 0;
}


//-----------------------------------------------------------------------------
// Name: loadCubeMap( void )
// Desc: Loads the skybox/environment/cubemap textures
//-----------------------------------------------------------------------------
void loadCubeMap( void ) {

	AUX_RGBImageRec *pTextureImage = NULL;

	glGenTextures( 1, &g_uiCubemapTexture );
	glBindTexture( GL_TEXTURE_CUBE_MAP_ARB, g_uiCubemapTexture );

	int j;
	for ( j=0; j < 6; j++ )
	{
		pTextureImage = auxDIBImageLoad( g_cCubemaps[j] );

		if( pTextureImage != NULL ) {

			// Build textures and mipmaps
			glTexImage2D( g_uiCubeMapConstants[j], 0, GL_RGB8, 
				pTextureImage->sizeX, pTextureImage->sizeY, 0, GL_RGB,
				GL_UNSIGNED_BYTE, pTextureImage->data );
			gluBuild2DMipmaps( g_uiCubeMapConstants[j], GL_RGB8, 
				pTextureImage->sizeX, pTextureImage->sizeY, GL_RGB,
				GL_UNSIGNED_BYTE, pTextureImage->data );
		}
		else {
			MessageBox( NULL, "Could not load cube texture", "Error",
						MB_OK|MB_ICONEXCLAMATION );
			cleanExit( 0 );
		}

		if( pTextureImage ) {

			// Release all data, since OpenGL holds its own copy of the textures
			if( pTextureImage->data )
				free( pTextureImage->data );
			free( pTextureImage );
		}
	}

	// Set texture properties
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_CUBE_MAP_ARB, GL_GENERATE_MIPMAP_SGIS, GL_TRUE );
	glTexParameterf( GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f );
}


//-----------------------------------------------------------------------------
// Name: display( void ) 
// Desc: Render function
//-----------------------------------------------------------------------------
void display( void ) {

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

    // Translate view
    glTranslatef ( -g_fEye[0], -g_fEye[1], -g_fEye[2] );

    // Rotates the screen by the angles
    glRotatef( g_fRot[0], 1.0f, 0.0f, 0.0f );
    glRotatef( g_fRot[1], 0.0f, 1.0f, 0.0f );
    glRotatef( g_fRot[2], 0.0f, 0.0f, 1.0f );

	// Draw the environment
	if( g_bDrawSkybox )
		drawSkyBox();

	// Bind and enable shaders
    cgGLBindProgram( cgVertexProgram );
	cgGLBindProgram( cgFragmentProgram );
	cgGLEnableProfile( cgVertexProfile );
    cgGLEnableProfile( cgFragmentProfile );

	// Enable textures
    cgGLEnableTextureParameter( cgEnvironmentMap );

	// Set all matrices 
	cgGLSetStateMatrixParameter( cgModelViewProj,	CG_GL_MODELVIEW_PROJECTION_MATRIX,	CG_GL_MATRIX_IDENTITY );
	cgGLSetStateMatrixParameter( cgModelView,		CG_GL_MODELVIEW_MATRIX,				CG_GL_MATRIX_IDENTITY );
	cgGLSetStateMatrixParameter( cgModelViewI,		CG_GL_MODELVIEW_MATRIX,				CG_GL_MATRIX_INVERSE );
	cgGLSetStateMatrixParameter( cgModelViewIT,		CG_GL_MODELVIEW_MATRIX,				CG_GL_MATRIX_INVERSE_TRANSPOSE );

	// Sets 3DS object corresponding to key input
	switch( g_uiCurrentObj ) {

		case 1:
			obj = &skull;
			break;

		case 2:
			obj = &teapot;
			break;

		case 3:
			obj = &sphere;
			break;

		default:
			break;
	}

	// Draws 3DS object
	draw3DSObject();

	// Disable textures
	cgGLDisableTextureParameter( cgEnvironmentMap );

	// Disable profiles
	cgGLDisableProfile( cgVertexProfile );
    cgGLDisableProfile( cgFragmentProfile );

	// Draw FPS, etc.
	drawtext();

	SwapBuffers( g_hDC );

	// Rotate around the y-axis, higher values increase the speed
	if( g_bRotate )
		g_fRot[1] += 0.01f;
}


//-----------------------------------------------------------------------------
// Name: cgErrorCallback( void )
// Desc: Cg error callback function
//-----------------------------------------------------------------------------
void cgErrorCallback( void ) {

    CGerror LastError = cgGetError();

    if( LastError ) {

		char outbuf[1024] = "";
		int i;

		// Retrieve time stamp
		SYSTEMTIME time;
		GetSystemTime( &time );

		// Get last Cg error
        char *Listing = ( char* )cgGetLastListing( cgContext );
		
		// Copy into buffer
        i = sprintf( outbuf, 
					 "\n---------------------------------------------------\n");
		i += sprintf( outbuf + i, "%d.%d.%d, %d:%d:%d\n", time.wDay, 
					  time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond );
        i += sprintf( outbuf + i, "%s\n\n", cgGetErrorString( LastError ) );
        i += sprintf( outbuf + i, "%s\n", Listing );
        i += sprintf( outbuf + i, 
					  "---------------------------------------------------\n");
        
		// Write into CGError.txt
		FILE *fp;
		fp = fopen( "CGError.txt", "a" );
		if( NULL != fp )
			fprintf( fp, outbuf );

		MessageBox( NULL, "Cg error, check CGError.txt", "Error", 
					MB_OK|MB_ICONEXCLAMATION );

		cleanExit( 0 );
    }
}


//-----------------------------------------------------------------------------
// Name: init( void ) 
// Desc: Initializes all stuff
//-----------------------------------------------------------------------------
void init( void ) {

	GLuint PixelFormat;

	// Specify WGL context
	PIXELFORMATDESCRIPTOR pfd;
	memset( &pfd, 0, sizeof( PIXELFORMATDESCRIPTOR ) );

    pfd.nSize      = sizeof( PIXELFORMATDESCRIPTOR );
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 16;
    pfd.cDepthBits = 16;
	
	g_hDC = GetDC( g_hWnd );
	PixelFormat = ChoosePixelFormat( g_hDC, &pfd );
	SetPixelFormat( g_hDC, PixelFormat, &pfd);
	g_hRC = wglCreateContext( g_hDC );
	wglMakeCurrent( g_hDC, g_hRC );

	// Set up OpenGL
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, g_fWidth / g_fHeight, 0.1f, 100.0f );

	// Set up Cg
	cgVertexProfile		= cgGLGetLatestProfile( CG_GL_VERTEX );
	cgFragmentProfile	= cgGLGetLatestProfile( CG_GL_FRAGMENT );
	cgGLSetOptimalOptions( cgVertexProfile );
	cgGLSetOptimalOptions( cgFragmentProfile );

	// Load 3DS models
	skull.LoadFile( "skull.3ds" );
	teapot.LoadFile( "teapot.3ds" );
	sphere.LoadFile( "sphere.3ds" );

	// Register error callback
    cgSetErrorCallback( cgErrorCallback );

	// Context creation
    cgContext = cgCreateContext();

	// Catch all vertex shader parameters and load vertex shader
    cgVertexProgram = cgCreateProgramFromFile( cgContext,
											CG_SOURCE,
											"oglu_per_vertex_reflective_environment_mapping_vs.cg",
											cgVertexProfile,
											NULL, 
											NULL );
    cgGLLoadProgram( cgVertexProgram );

    cgModelViewProj = cgGetNamedParameter( cgVertexProgram, "ModelViewProj" );
	cgModelView		= cgGetNamedParameter( cgVertexProgram, "ModelView" );
	cgModelViewI	= cgGetNamedParameter( cgVertexProgram, "ModelViewI" );
	cgModelViewIT	= cgGetNamedParameter( cgVertexProgram, "ModelViewIT" );
	cgPosition		= cgGetNamedParameter( cgVertexProgram, "position" );
	cgNormal		= cgGetNamedParameter( cgVertexProgram, "N" );
	cgTexcoords		= cgGetNamedParameter( cgVertexProgram, "texCoord" );
	cgEyePosition	= cgGetNamedParameter( cgVertexProgram, "eyePosition" );

    if ( !cgModelViewProj || !cgModelView || !cgModelViewI || !cgModelViewIT
		|| !cgPosition || !cgNormal	|| !cgTexcoords || !cgEyePosition ) {

		MessageBox( NULL, "Unable to retrieve vertex program parameters, exiting...",
					"Error", MB_OK|MB_ICONEXCLAMATION );
        cleanExit( 0 );
    }

	// Set eye to origin
	cgGLSetParameter3f( cgEyePosition, 0.0f, 0.0f, 0.0f );

	// Catch all pixel shader parameters and load pixel shader
	cgFragmentProgram = cgCreateProgramFromFile( cgContext,
											CG_SOURCE,
											"oglu_per_vertex_reflective_environment_mapping_ps.cg",
											cgFragmentProfile,
											NULL, 
											NULL );
    cgGLLoadProgram( cgFragmentProgram );

    cgEnvironmentMap = cgGetNamedParameter( cgFragmentProgram, "environmentMap" );

    if ( !cgEnvironmentMap ) {

		MessageBox( NULL, "Unable to retrieve fragment program parameters, exiting...",
					"Error", MB_OK|MB_ICONEXCLAMATION );
        cleanExit( 0 );
    }

	// Load the cubemap textures
	loadCubeMap( );

	// Set texture parameter
	cgGLSetTextureParameter( cgEnvironmentMap,	g_uiCubemapTexture );
}


//-----------------------------------------------------------------------------
// Name: drawSkyBox( void )
// Desc: Draws the skybox/environment
//-----------------------------------------------------------------------------
void drawSkyBox( void ) {

	// Define planes for our cube
	GLfloat xPlane[] = { 1.0f, 0.0f, 0.0f, 0.0f };
	GLfloat yPlane[] = { 0.0f, 1.0f, 0.0f, 0.0f };
	GLfloat zPlane[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	
	// Enable texture coordinates generation
	glEnable( GL_TEXTURE_GEN_S );
	glEnable( GL_TEXTURE_GEN_T );
	glEnable( GL_TEXTURE_GEN_R );

	// Set texture environment parameter
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

	// Enable the cubemap extension
	glEnable( GL_TEXTURE_CUBE_MAP_ARB );
    glBindTexture( GL_TEXTURE_CUBE_MAP_ARB, g_uiCubemapTexture );

	// Generate texture coordinates
	glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
	glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
	glTexGeni( GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );

	glTexGenfv( GL_S, GL_OBJECT_PLANE, xPlane );
	glTexGenfv( GL_T, GL_OBJECT_PLANE, yPlane );
	glTexGenfv( GL_R, GL_OBJECT_PLANE, zPlane );

	// Skybox rendering needs disabling of z buffer
	glDisable( GL_DEPTH_TEST );

	glMatrixMode( GL_MODELVIEW );
	
	// Draw the cube
	glBegin( GL_QUADS );

		// Front
        glTexCoord2f( 0, 0 );
        glVertex3f( -30, -30, -30 );

        glTexCoord2f( 1, 0 );
        glVertex3f( 30, -30, -30 );

        glTexCoord2f( 1, 1 );
        glVertex3f( 30, 30, -30 );
    
        glTexCoord2f( 0, 1 );
        glVertex3f( -30, 30, -30 );

		// Right
        glTexCoord2f( 0, 0 );
        glVertex3f( 30, -30, -30 );

        glTexCoord2f( 1, 0 );
        glVertex3f( 30, -30, 30 );

        glTexCoord2f( 1, 1 );
        glVertex3f( 30, 30, 30 );
    
        glTexCoord2f( 0, 1 );
        glVertex3f( 30, 30, -30 );

		// Back
		glTexCoord2f( 0, 0 );
        glVertex3f( 30, -30, 30 );

        glTexCoord2f( 1, 0 );
        glVertex3f( -30, -30, 30 );

        glTexCoord2f( 1, 1 );
        glVertex3f( -30, 30, 30 );
    
        glTexCoord2f( 0, 1 );
        glVertex3f( 30, 30, 30 );

		// Left
		glTexCoord2f( 0, 0 );
        glVertex3f( -30, -30, 30 );

        glTexCoord2f( 1, 0 );
        glVertex3f( -30, -30, -30 );

        glTexCoord2f( 1, 1 );
        glVertex3f( -30, 30, -30 );
    
        glTexCoord2f( 0, 1 );
        glVertex3f( -30, 30, 30 );

		// Top
		glTexCoord2f( 0, 0 );
        glVertex3f( -30, 30, -30 );

        glTexCoord2f( 1, 0 );
        glVertex3f( 30, 30, -30 );

        glTexCoord2f( 1, 1 );
        glVertex3f( 30, 30, 30 );
    
        glTexCoord2f( 0, 1 );
        glVertex3f( -30, 30, 30 );

		// Bottom
		glTexCoord2f( 0, 0 );
        glVertex3f( -30, -30, 30 );

        glTexCoord2f( 1, 0 );
        glVertex3f( 30, -30, 30 );

        glTexCoord2f( 1, 1 );
        glVertex3f( 30, -30, -30 );
    
        glTexCoord2f( 0, 1 );
        glVertex3f( -30, -30, -30 );

    glEnd();

	glEnable( GL_DEPTH_TEST );
	
	glDisable( GL_TEXTURE_CUBE_MAP_ARB );
	
	glDisable( GL_TEXTURE_GEN_S );
	glDisable( GL_TEXTURE_GEN_T );
	glDisable( GL_TEXTURE_GEN_R );
}


//-----------------------------------------------------------------------------
// Name: cleanExit( int exitval )
// Desc: Releases all stuff properly
//-----------------------------------------------------------------------------
void cleanExit( int exitval ) {

    if( cgVertexProgram )	cgDestroyProgram( cgVertexProgram );
    if( cgFragmentProgram ) cgDestroyProgram( cgFragmentProgram );
    if( cgContext ) cgDestroyContext( cgContext );

	if( g_hRC != NULL ) {
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( g_hRC );
		g_hRC = NULL;							
	}

	if( g_hRC != NULL )	{
		ReleaseDC( g_hWnd, g_hDC );
		g_hDC = NULL;
	}

    exit( exitval );
}


//-----------------------------------------------------------------------------
// Name: drawtext( void )
// Desc: Draws frames per second (FPS), model vertex and triangle count
//-----------------------------------------------------------------------------
void drawtext( void ) {

	GLint x=0, y=0;
	char string[80] = { 0 };
	RECT rect;

	// For FPS measuring
	static GLfloat framesPerSecond = 0.0f;
	static long lastTime = 0;
	static char strFrameRate[50] = "";
	DWORD currentTime = 0;

	// Fetch screen values
	GetClientRect( g_hWnd, &rect );
	GLint ww = rect.right;
	GLint wh = rect.bottom;

    glDisable( GL_DEPTH_TEST );

	// Set up OpenGL matrix stacks
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D( 0, ww-1, 0, wh-1 );
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

	// Create bitmaps for the device context font's first 256 glyphs 
	wglUseFontBitmaps( g_hDC, 0, 256, 1000 );

	// Set up for a string-drawing display list call 
	glListBase( 1000 );

	// FPS
	currentTime = GetTickCount() / 1000;
	x = ww-200; y = wh-15;

	// Increase the frame counter
	++framesPerSecond;
	if ( currentTime - lastTime >= 1 ) {

		lastTime = currentTime;
		
		// Copy the frames per second into a string to display in the window
		sprintf( strFrameRate, "FPS:                      %d  ", GLint( framesPerSecond ) );
		
		// Reset the frames per second
		framesPerSecond = 0;
	}

	// Draw FPS on screen
    glRasterPos2i( x, y );
	glCallLists( 29, GL_UNSIGNED_BYTE, strFrameRate );

	// Get mesh
	LMesh &mesh = obj->GetMesh( 0 );
	
	// Draw vertex count
	x = ww-200; y = wh-30;
	glRasterPos2i( x, y );
	ZeroMemory( string, 80 );
	sprintf( string, "Vertex count:         %d  ", mesh.GetVertexCount() );
	glCallLists( 26, GL_UNSIGNED_BYTE, string );
	
	// Draw triangle count
	x = ww-200; y = wh-45;
	glRasterPos2i( x, y );
	ZeroMemory( string, 80 );
	sprintf( string, "Triangle count:      %d  ", mesh.GetTriangleCount() );
	glCallLists( 25, GL_UNSIGNED_BYTE, string );

	// Reconfigure OpenGL matrix stacks
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    glEnable( GL_DEPTH_TEST );
}


//-----------------------------------------------------------------------------
// Name: draw3DSObject( void )
// Desc: 3DS model rendering function
//-----------------------------------------------------------------------------
void draw3DSObject( void ) {

	for ( GLuint z=0; z < obj->GetMeshCount(); z++ ) {

		LMesh &mesh = obj->GetMesh( z );

		// Set vertex arrays as vertex shader inputs
		cgGLEnableClientState( cgNormal );
		cgGLEnableClientState( cgPosition );
		cgGLEnableClientState( cgTexcoords );

		// Point to corresponding vertex arrays
		cgGLSetParameterPointer( cgNormal,		3, GL_FLOAT, 0, &mesh.GetNormal( 0 ) );
		cgGLSetParameterPointer( cgPosition,	4, GL_FLOAT, 0, &mesh.GetVertex( 0 ) );
		cgGLSetParameterPointer( cgTexcoords,	2, GL_FLOAT, 0, &mesh.GetUV( 0 ) );

		// Draw primitives
		glDrawElements( GL_TRIANGLES, mesh.GetTriangleCount()*3, GL_UNSIGNED_SHORT, &mesh.GetTriangle( 0 ) );

		cgGLDisableClientState( cgNormal );
		cgGLDisableClientState( cgPosition );
		cgGLDisableClientState( cgTexcoords );
    }
}