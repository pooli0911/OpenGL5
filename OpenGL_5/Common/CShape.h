#ifndef CSHAPE_H
#define CSHAPE_H

#define  _CRT_SECURE_NO_WARNINGS 1
#include "../Header/Angel.h"
#include "../SOIL/SOIL.h"
#include "TypeDefine.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

//#define FLAT_SHADING    0
//#define GOURAUD_SHADING 1

// GPU ���p�⥲���ǧ�h���Ѽƶi�J Shader

//#define PERVERTEX_LIGHTING
//// �]�w�W�������ѴN�O�Ұ� PERPIXEL_LIGHTING

// �H�U���}�Ҧh�h���K��
//#define MULTITEXTURE  NONE_MAP
//#define MULTITEXTURE  DIFFUSE_MAP  
//#define MULTITEXTURE  (DIFFUSE_MAP|LIGHT_MAP)  // For Example3AB/3LM
//#define MULTITEXTURE  (DIFFUSE_MAP|LIGHT_MAP|NORMAL_MAP)// For Example 4 ~~

// �}�ҥH�U���w�q�N��ϥ� Environment  Mapping
// �W���}�� #define MULTITEXTURE  DIFFUSE_MAP  
// �ϥ� Environment Mapping �����]�w�ϥ� vsCubeMapping.glsl �P fsCubeMapping.glsl
// �ثe�èS���b Cubic Mapping ���ǤJ NORMAL_MAP �P LIGHT_MAP
//#define CUBIC_MAP 1  // �o�ӥu�Φb Example 5

// ��ҫ������� non-uniform scale ���ާ@�ɡA�����z�L�p��ϯx�}�ӱo�쥿�T�� Normal ��V
// �}�ҥH�U���w�q�Y�i�A�ثe CPU �p�⪺������
// GPU �������h�O�]�w������

// #define GENERAL_CASE 1 


enum class ShadingMode
{
	FLAT_SHADING_CPU = 1,
	GOURAUD_SHADING_CPU = 2, // CPU Per Vertex Lighting / vsVtxColor
	GOURAUD_SHADING_GPU = 3, // GPU Per Vertex Lighting / vsPerVtxLighting
	PHONG_SHADING = 4		 // GPU Per PIXEL Lighting / vsPerPixelLighting
							 // vsPerPixelLighting �u���䴩�� Diffue + Light Map
							 // �ϥ� Normal Map �ɡA�|�۰ʤ����� vsNormalMapLighting
							 // �ϥ� Cubic Map �ɡA�|�۰ʤ����� vsCubeMapping
};

enum class TextureMode
{
	NO_TEXTURE = 0x0,
	DIFFUSE_MAP = 0x1,
	LIGHT_MAP = 0x2,
	NORMAL_MAP = 0x4,
	CUBIC_MAP = 0x8
};

constexpr TextureMode operator | (TextureMode m1, TextureMode m2)
{
	return static_cast<TextureMode>(static_cast<int>(m1) | static_cast<int>(m2));
}
constexpr TextureMode operator & (TextureMode m1, TextureMode m2)
{
	return static_cast<TextureMode>(static_cast<int>(m1) & static_cast<int>(m2));
}


class CShape 
{
protected:
	vec4 *_pPoints;
	vec3 *_pNormals;
	vec4 *_pColors;
	vec2 *_pTex1;

// LIGHT_MAP
	vec2 *_pTex2;	// �s�W�ĤG�i�K�� for example 3
//-------------------------------------------
// NORMAL_MAP
	vec2 *_pTex3;		// �s�W�ĤT�i�K�� for example 4
	vec3 *_pTangentV;	// �s�W tangent vector for each vertex
//-------------------------------------------

	int  _iNumVtx;

	GLfloat _fColor[4]; // Object's color
	// For shaders' name
	char *_pVXshader, *_pFSshader;

	// For VAO
	GLuint _uiVao;

	// For Shader
	GLuint  _uiModelView, _uiProjection, _uiColor;
	GLuint  _uiProgram;
	GLuint  _uiBuffer;

	point4  _vLightInView;	 // �����b�@�ɮy�Ъ���m
	GLuint  _uiLightInView;	 // �����b shader ����m
	GLuint  _uiAmbient;		 // light's ambient  �P Object's ambient  �P ka �����n
	GLuint  _uiDiffuse;		 // light's diffuse  �P Object's diffuse  �P kd �����n
	GLuint  _uiSpecular;	 // light's specular �P Object's specular �P ks �����n
	GLuint  _uiShininess;
	GLuint  _uiLighting;	// �������Ӽ�
	GLuint  _uiTexLayer;	// �K�Ϫ��h���A�w�]�N�O�@�h diffuse

	// �N�Ӫ���ثe����X�{���ɶ��ǤJ�A�p�G�ݭn������H�ۮɶ����ܡA�i�H�γo���ܼ�
	GLuint  _uiElapsedTime;	// �Ӫ���X�{��ثe����g�L���ɶ�
	GLfloat _fElapsedTime;


// CUBIC_MAP------------------------------------------------------------------
	GLuint  _uiTRS;			// TRSMatrix �ǤJ Pixel Shader ����m
	GLuint  _uiViewPos;		// ViewPoint �ǤJ Pixel Shader ����m
	point4  _v4Eye;			// Camera ����m

	GLuint  _uiCubeMap;     // pixel shader ���� CubeMap Texture Name ����m
	GLuint  _uiCubeMapTexName;  // �ǤJ pixel shader ���� Cube map ���K�Ͻs���A 
//----------------------------------------------------------------------------

	LightSource _Light1;

	color4 _AmbientProduct;
	color4 _DiffuseProduct;
	color4 _SpecularProduct;
	int    _iLighting;		// �]�w�O�_�n���O

	// For Matrices
	mat4    _mxView, _mxProjection, _mxTRS;
	mat4    _mxMVFinal;
	mat3    _mxMV3X3Final;		// �ϥΦb�p�� �������᪺�s Normal
	mat3		_mxITMV;
	bool    _bProjUpdated, _bViewUpdated, _bTRSUpdated;

	// For materials
	Material _Material;

	// For Shading Mode
	ShadingMode	_iMode; 

	// For Texture Mode
	TextureMode _iTexLayer;

	void	createBufferObject();
	void	drawingSetShader();
	void	drawingWithoutSetShader();

public:
	CShape();
	virtual ~CShape();
	virtual void draw() = 0;
	virtual void drawW() = 0; // Drawing without setting shaders
	virtual void update(float dt, const LightSource &lights) = 0;
	virtual void update(float dt) = 0; // ���p��������ө�

	void updateMatrix();
	void setShaderName(const char vxShader[], const char fsShader[]);
	void setShader(GLuint uiShaderHandle = MAX_UNSIGNED_INT);
	void setColor(vec4 vColor);
	void setViewMatrix(mat4 &mat);
	void setProjectionMatrix(mat4 &mat);
	void setTRSMatrix(mat4 &mat);
	void setTextureLayer(TextureMode texlayer);

	// For setting materials 
	void setMaterials(color4 ambient, color4 diffuse, color4 specular);
	void setKaKdKsShini(float ka, float kd, float ks, float shininess); // ka kd ks shininess

	// For Lighting Calculation
	void setShadingMode(ShadingMode iMode) {_iMode = iMode;}
	vec4 PhongReflectionModel(vec4 vPoint, vec3 vNormal, const LightSource &lights);
	vec4 PhongReflectionModel(vec4 vPoint, vec3 vNormal, vec4 vLightPos, color4 vLightI); // �O�d�A���d�Ҥ��|�Ψ�

	// �C�⪺�p��H�T��������¦�A�ð��]�@�ӳ��I�B�C��B�k�V�q�P�K�Ϯy�г��O�@��@����
	void renderWithFlatShading(point4 vLightPos, color4 vLightI);    // �O�d�A���d�Ҥ��|�Ψ�
	void renderWithGouraudShading(point4 vLightPos, color4 vLightI); // �O�d�A���d�Ҥ��|�Ψ�
	void renderWithFlatShading(const LightSource& Lights);//  vLightI: Light Intensity
	void renderWithGouraudShading(const LightSource& Lights);//  vLightI: Light Intensity

	// For controlling texture mapping
	void setMirror(bool uAxis, bool vAxis); // U�b �P V�b �O�_�n��g
	void setTiling(float uTiling, float vTiling);  // �� U�b �P V�b �i����K���Y��
	void setLightMapTiling(float uTiling, float vTiling);  // �� LightMap U�b �P V�b �i����K���Y��

	void setLightingDisable() {_iLighting = 0;}

	// For Cube Map
	void setCubeMapTexName(GLuint uiTexName) { _uiCubeMapTexName = uiTexName; }
	void setViewPosition(point4 vEye) { _v4Eye = vEye; }
};

#endif
