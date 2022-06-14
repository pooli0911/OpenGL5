// 
// Example 3LM : Light Map �B�@��z�i��
// 
// �a�O g_pFloor�A�ϥ� ShadingMode::PHONG_SHADING
// ���d�Ҩϥ�  setTiling(10,10) �N diffuse �K�Ϯy�бq (0,0)-(1,1)�A�ܦ� (0,0)-(10,10)
//			   setLightMapTiling(2, 2)�h�O�]�w light map �K�Ϯy�бq (0,0)-(1,1)�A�ܦ� (0,0)-(2,2)
// 
// �T�ӧi�ܪO���ϥιw�]�� diffuse �K�ϡA�ҥH�L�ݥt�~���� setTextureLayer
// �C�@�i�K�ϳ��a�� alpha (RGBA) �� PNG ���ɡC�]�i�H�ϥ� ShadingMode::PHONG_SHADING
// 
// Light Map �ϥ� vsPerPixelLighting.glsl fsPerPixelLighting.glsl �o�աA
//	   �]�w�ϥ� Light Map�A�z�L setTextureLayer(TextureMode::DIFFUSE_MAP | TextureMode::LIGHT_MAP)
//

#include "header/Angel.h"
#include "Common/CQuad.h"
#include "Common/CSolidCube.h"
#include "Common/CSolidSphere.h"
#include "Common/CWireSphere.h"
#include "Common/CWireCube.h"
#include "Common/CChecker.h"
#include "Common/CCamera.h"
#include "Common/CTexturePool.h"
#include "Common/CShaderPool.h"
#include "png_loader.h"


#define SPACE_KEY 32
#define SCREEN_SIZE 800
#define HALF_SIZE SCREEN_SIZE /2 
#define VP_HALFWIDTH  20.0f
#define VP_HALFHEIGHT 20.0f
#define GRID_SIZE 20 // must be an even number

// For Model View and Projection Matrix
mat4 g_mxModelView(1.0f);
mat4 g_mxProjection;

// For Objects
CQuad		  *g_pFloor;
CSolidCube    *g_pCube;

// For Translucent Object
CQuad  *g_pAimal[3];
GLuint g_uiAimalTexID[3];

// For View Point
GLfloat g_fRadius = 8.0;
GLfloat g_fTheta = 45.0f*DegreesToRadians;
GLfloat g_fPhi = 45.0f*DegreesToRadians;

//----------------------------------------------------------------------------
// Part 2 : for single light source
bool g_bAutoRotating = false;
float g_fElapsedTime = 0;
float g_fLightRadius = 5;
float g_fLightTheta = 0;

float g_fLightR = 1.0f;
float g_fLightG = 1.0f;
float g_fLightB = 1.0f;

LightSource g_Light1 = {
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // ambient 
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // diffuse
	color4(g_fLightR, g_fLightG, g_fLightB, 1.0f), // specular
	point4(5.0f, 4.0f, 0.0f, 1.0f),   // position
	point4(0.0f, 0.0f, 0.0f, 1.0f),   // halfVector
	vec3(0.0f, 0.0f, 0.0f),			  // spotTarget
	vec3(0.0f, 0.0f, 0.0f),			  //spotDirection
	2.0f,	// spotExponent(parameter e); cos^(e)(phi) 
	45.0f,	// spotCutoff;	// (range: [0.0, 90.0], 180.0)  spot ���ө��d��
	0.707f,	// spotCosCutoff; // (range: [1.0,0.0],-1.0), �ө���V�P�Q�ө��I���������ר� cos ��, cut off ����
	1,	// constantAttenuation	(a + bd + cd^2)^-1 ���� a, d ��������Q�ө��I���Z��
	0,	// linearAttenuation	    (a + bd + cd^2)^-1 ���� b
	0,	// quadraticAttenuation (a + bd + cd^2)^-1 ���� c
	LightType::OMNI_LIGHT
};

CWireSphere *g_pLight;
//----------------------------------------------------------------------------

// Texture ���ե�
GLuint g_uiFTexID[3]; // �T�Ӫ�����O�����P���K��
int g_iTexWidth,  g_iTexHeight;

//----------------------------------------------------------------------------
// �禡���쫬�ŧi
extern void IdleProcess();
void releaseResources();

void init( void )
{
	mat4 mxT;
	vec4 vT, vColor;
	// ���ͩһݤ� Model View �P Projection Matrix
	// ���ͩһݤ� Model View �P Projection Matrix
	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);
	auto camera = CCamera::create();
	camera->updateViewLookAt(eye, at);
	camera->updatePerspective(60.0, (GLfloat)SCREEN_SIZE / (GLfloat)SCREEN_SIZE, 1.0, 1000.0);

	auto texturepool = CTexturePool::create();
	g_uiFTexID[0] = texturepool->addTexture("texture/checker.png");
	g_uiFTexID[1] = texturepool->addTexture("texture/Masonry.Brick.png");
	g_uiFTexID[2] = texturepool->addTexture("texture/lightMap1.png"); // Light Map

	g_uiAimalTexID[0] = texturepool->addTexture("texture/wi_tata.png");
	g_uiAimalTexID[1] = texturepool->addTexture("texture/wi_sma.png");
	g_uiAimalTexID[2] = texturepool->addTexture("texture/wi_tree.png");

	// ���ͪ��󪺹���
	g_pFloor = new CQuad;
	g_pFloor->setTRSMatrix(Scale(15,1,15));
	// �]�w�K��
	g_pFloor->setMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pFloor->setKaKdKsShini(0, 0.8f, 0.5f, 1);
	g_pFloor->setTiling(10, 10);
	g_pFloor->setLightMapTiling(2, 2);
	g_pFloor->setTextureLayer(TextureMode::DIFFUSE_MAP | TextureMode::LIGHT_MAP);
	g_pFloor->setShadingMode(ShadingMode::GOURAUD_SHADING_GPU);
	g_pFloor->setShader();

	g_pCube = new CSolidCube;
	// �]�w Cube
	vT.x = 1.5f; vT.y = 0.75f; vT.z = -1.5f;
	mxT = Translate(vT);
	mxT._m[0][0] = 1.5f; mxT._m[1][1] = 1.5f; mxT._m[2][2] = 1.5f;
	g_pCube->setTRSMatrix(mxT);

	// materials
	g_pCube->setMaterials(vec4(0.35f, 0.35f, 0.35f, 1), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pCube->setKaKdKsShini(0.25f, 0.8f, 0.2f, 2);
	g_pCube->setShadingMode(ShadingMode::GOURAUD_SHADING_GPU);
	g_pCube->setShader();

	// For g_pAimal1
	g_pAimal[0] = new CQuad;
	mxT = Translate(0, 1.0f, 2.0f) * RotateX(90) *Scale(2,2,2);
	g_pAimal[0]->setTRSMatrix(mxT);
	// �]�w�K��
	g_pAimal[0]->setMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pAimal[0]->setKaKdKsShini(0, 0.8f, 0.5f, 1);
	g_pAimal[0]->setShadingMode(ShadingMode::GOURAUD_SHADING_GPU);
	g_pAimal[0]->setShader();

	// For g_pAimal2
	g_pAimal[1] = new CQuad;
	mxT = Translate(0, 1.0f, 0.0f) * RotateX(90) * Scale(2,2,2);
	g_pAimal[1]->setTRSMatrix(mxT);
	// �]�w�K��
	g_pAimal[1]->setMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pAimal[1]->setKaKdKsShini(0, 0.8f, 0.5f, 1);
	g_pAimal[1]->setShadingMode(ShadingMode::GOURAUD_SHADING_GPU);
	g_pAimal[1]->setShader();

	// For g_pAimal3
	g_pAimal[2] = new CQuad;
	mxT =Translate(0, 1.0f, -2.0f) * RotateX(90) * Scale(2,2,2);
	g_pAimal[2]->setTRSMatrix(mxT);
	// �]�w�K��
	g_pAimal[2]->setMaterials(vec4(0), vec4(0.85f, 0.85f, 0.85f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	g_pAimal[2]->setKaKdKsShini(0, 0.8f, 0.5f, 1);
	g_pAimal[2]->setShadingMode(ShadingMode::GOURAUD_SHADING_GPU);
	g_pAimal[2]->setShader();


	// �]�w �N�� Light �� WireSphere
	g_pLight = new CWireSphere(0.25f, 6, 3);
	mxT = Translate(g_Light1.position);
	g_pLight->setTRSMatrix(mxT);
	g_pLight->setColor(g_Light1.diffuse);
	g_pLight->setLightingDisable();
	g_pLight->setTextureLayer(TextureMode::NO_TEXTURE);	// �S���K��
	g_pLight->setShadingMode(ShadingMode::GOURAUD_SHADING_GPU);
	g_pLight->setShader();

	// �]�����d�Ҥ��|�ʨ� Projection Matrix �ҥH�b�o�̳]�w�@���Y�i
	// �N���g�b OnFrameMove ���C���� Check
	bool bPDirty;
	mat4 mpx = camera->getProjectionMatrix(bPDirty);
	g_pFloor->setProjectionMatrix(mpx);
	g_pCube->setProjectionMatrix(mpx);
	g_pAimal[0]->setProjectionMatrix(mpx);
	g_pAimal[1]->setProjectionMatrix(mpx);
	g_pAimal[2]->setProjectionMatrix(mpx);
	g_pLight->setProjectionMatrix(mpx);
}

//----------------------------------------------------------------------------
void GL_Display( void )
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // clear the window

	glEnable(GL_BLEND);  // �]�w2D Texure Mapping ���@��
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

// No Light Map-------------------------------------------
	//glBindTexture(GL_TEXTURE_2D, g_uiFTexID[0]);
	//g_pFloor->draw();
//-------------------------------------------
	glActiveTexture(GL_TEXTURE0); // select active texture 0
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[0]); // �P Diffuse Map ���X
	glActiveTexture(GL_TEXTURE1); // select active texture 1
	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[2]); // �P Light Map ���X
	g_pFloor->draw();
	glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, 0);

	glBindTexture(GL_TEXTURE_2D, g_uiFTexID[1]);
	g_pCube->draw();
	glBindTexture(GL_TEXTURE_2D, 0);
	g_pLight->draw();

	glDepthMask(GL_FALSE);
	glBindTexture(GL_TEXTURE_2D, g_uiAimalTexID[2]); 
	g_pAimal[2]->draw();
	glBindTexture(GL_TEXTURE_2D, g_uiAimalTexID[1]); 
	g_pAimal[1]->draw();
	glBindTexture(GL_TEXTURE_2D, g_uiAimalTexID[0]); 
	g_pAimal[0]->draw();
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_BLEND);	// ���� Blending
	glDepthMask(GL_TRUE);	// �}�ҹ� Z-Buffer ���g�J�ާ@

	glutSwapBuffers();	// �洫 Frame Buffer
}

//----------------------------------------------------------------------------
// Part 2 : for single light source
void updateLightPosition(float dt)
{
	mat4 mxT;
	// �C��¶ Y �b�� 90 ��
	g_fElapsedTime += dt;
	g_fLightTheta = g_fElapsedTime*(float)M_PI_2;
	if( g_fLightTheta >= (float)M_PI*2.0f ) {
		g_fLightTheta -= (float)M_PI*2.0f;
		g_fElapsedTime -= 4.0f;
	}
	g_Light1.position.x = g_fLightRadius * cosf(g_fLightTheta);
	g_Light1.position.z = g_fLightRadius * sinf(g_fLightTheta);
	mxT = Translate(g_Light1.position);
	g_pLight->setTRSMatrix(mxT);
}
//----------------------------------------------------------------------------

void onFrameMove(float delta)
{

	if( g_bAutoRotating ) { // Part 2 : ���s�p�� Light ����m
		updateLightPosition(delta);
	}

	mat4 mvx;	// view matrix & projection matrix
	bool bVDirty;	// view �P projection matrix �O�_�ݭn��s������
	auto camera = CCamera::getInstance();
	mvx = camera->getViewMatrix(bVDirty);
	if (bVDirty) {
		g_pFloor->setViewMatrix(mvx);
		g_pCube->setViewMatrix(mvx);
		g_pAimal[0]->setViewMatrix(mvx);
		g_pAimal[1]->setViewMatrix(mvx);
		g_pAimal[2]->setViewMatrix(mvx);
		g_pLight->setViewMatrix(mvx);
	}

	// �p�G�ݭn���s�p��ɡA�b�o��p��C�@�Ӫ����C��
	g_pFloor->update(delta, g_Light1);
	g_pCube->update(delta, g_Light1);
	g_pAimal[0]->update(delta, g_Light1);
	g_pAimal[1]->update(delta, g_Light1);
	g_pAimal[2]->update(delta, g_Light1);

	g_pLight->update(delta);

	GL_Display();
}

//----------------------------------------------------------------------------

void Win_Keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
	case  SPACE_KEY:

		break;
//----------------------------------------------------------------------------
// Part 2 : for single light source
	case 65: // A key
	case 97: // a key
		g_bAutoRotating = !g_bAutoRotating;
		break;
	case 82: // R key
		if( g_fLightR <= 0.95f ) g_fLightR += 0.05f;
		g_Light1.diffuse.x = g_fLightR;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 114: // r key
		if( g_fLightR >= 0.05f ) g_fLightR -= 0.05f;
		g_Light1.diffuse.x = g_fLightR;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 71: // G key
		if( g_fLightG <= 0.95f ) g_fLightG += 0.05f;
		g_Light1.diffuse.y = g_fLightG;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 103: // g key
		if( g_fLightG >= 0.05f ) g_fLightG -= 0.05f;
		g_Light1.diffuse.y = g_fLightG;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 66: // B key
		if( g_fLightB <= 0.95f ) g_fLightB += 0.05f;
		g_Light1.diffuse.z = g_fLightB;
		g_pLight->setColor(g_Light1.diffuse);
		break;
	case 98: // b key
		if( g_fLightB >= 0.05f ) g_fLightB -= 0.05f;
		g_Light1.diffuse.z = g_fLightB;
		g_pLight->setColor(g_Light1.diffuse);
		break;
//----------------------------------------------------------------------------
    case 033:
		glutIdleFunc( NULL );
		releaseResources();
        exit( EXIT_SUCCESS );
        break;
    }
}

//----------------------------------------------------------------------------
void Win_Mouse(int button, int state, int x, int y) {
	switch(button) {
		case GLUT_LEFT_BUTTON:   // �ثe���U���O�ƹ�����
			//if ( state == GLUT_DOWN ) ; 
			break;
		case GLUT_MIDDLE_BUTTON:  // �ثe���U���O�ƹ����� �A���� Y �b
			//if ( state == GLUT_DOWN ) ; 
			break;
		case GLUT_RIGHT_BUTTON:   // �ثe���U���O�ƹ��k��
			//if ( state == GLUT_DOWN ) ;
			break;
		default:
			break;
	} 
}
//----------------------------------------------------------------------------
void Win_SpecialKeyboard(int key, int x, int y) {

	switch(key) {
		case GLUT_KEY_LEFT:		// �ثe���U���O�V����V��

			break;
		case GLUT_KEY_RIGHT:	// �ثe���U���O�V�k��V��

			break;
		default:
			break;
	}
}

//----------------------------------------------------------------------------
// The passive motion callback for a window is called when the mouse moves within the window while no mouse buttons are pressed.
void Win_PassiveMotion(int x, int y) {

	g_fPhi = (float)-M_PI*(x - HALF_SIZE)/(HALF_SIZE); // �ഫ�� g_fPhi ���� -PI �� PI ���� (-180 ~ 180 ����)
	g_fTheta = (float)M_PI*(float)y/SCREEN_SIZE;
	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);
	CCamera::getInstance()->updateViewLookAt(eye, at);
}

// The motion callback for a window is called when the mouse moves within the window while one or more mouse buttons are pressed.
void Win_MouseMotion(int x, int y) {
	g_fPhi = (float)-M_PI*(x - HALF_SIZE)/(HALF_SIZE);  // �ഫ�� g_fPhi ���� -PI �� PI ���� (-180 ~ 180 ����)
	g_fTheta = (float)M_PI*(float)y/SCREEN_SIZE;;
	point4  eye(g_fRadius*sin(g_fTheta)*sin(g_fPhi), g_fRadius*cos(g_fTheta), g_fRadius*sin(g_fTheta)*cos(g_fPhi), 1.0f);
	point4  at(0.0f, 0.0f, 0.0f, 1.0f);
	CCamera::getInstance()->updateViewLookAt(eye, at);
}
//----------------------------------------------------------------------------
void GL_Reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	glClearColor( 0.0, 0.0, 0.0, 1.0 ); // black background
	glEnable(GL_DEPTH_TEST);
}

//----------------------------------------------------------------------------
void releaseResources()
{
	delete g_pCube;
	delete g_pFloor;
	delete g_pLight;
	delete g_pAimal[0];
	delete g_pAimal[1];
	delete g_pAimal[2];
	CCamera::getInstance()->destroyInstance();
	CTexturePool::getInstance()->destroyInstance();
	CTexturePool::getInstance()->destroyInstance();
}
//----------------------------------------------------------------------------

int main( int argc, char **argv )
{
    
	glutInit(&argc, argv);
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( SCREEN_SIZE, SCREEN_SIZE );

	// If you use freeglut the two lines of code can be added to your application 
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );

    glutCreateWindow("Compositing and Blending");

	// The glewExperimental global switch can be turned on by setting it to GL_TRUE before calling glewInit(), 
	// which ensures that all extensions with valid entry points will be exposed.
	glewExperimental = GL_TRUE; 
    glewInit();  

    init();

	glutMouseFunc(Win_Mouse);
	glutMotionFunc(Win_MouseMotion);
	glutPassiveMotionFunc(Win_PassiveMotion);  
    glutKeyboardFunc( Win_Keyboard );	// �B�z ASCI ����p A�Ba�BESC ��...����
	glutSpecialFunc( Win_SpecialKeyboard);	// �B�z NON-ASCI ����p F1�BHome�B��V��...����
    glutDisplayFunc( GL_Display );
	glutReshapeFunc( GL_Reshape );
	glutIdleFunc( IdleProcess );
	
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	// �o�@��|�������Q�j�������ɡA�{��������|�^�� glutMainLoop(); ���U�@��
	glutMainLoop();
	releaseResources();
    return 0;
}