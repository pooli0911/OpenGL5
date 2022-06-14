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

// GPU 的計算必須傳更多的參數進入 Shader

//#define PERVERTEX_LIGHTING
//// 設定上面為註解就是啟動 PERPIXEL_LIGHTING

// 以下為開啟多層次貼圖
//#define MULTITEXTURE  NONE_MAP
//#define MULTITEXTURE  DIFFUSE_MAP  
//#define MULTITEXTURE  (DIFFUSE_MAP|LIGHT_MAP)  // For Example3AB/3LM
//#define MULTITEXTURE  (DIFFUSE_MAP|LIGHT_MAP|NORMAL_MAP)// For Example 4 ~~

// 開啟以下的定義代表使用 Environment  Mapping
// 上面開啟 #define MULTITEXTURE  DIFFUSE_MAP  
// 使用 Environment Mapping 必須設定使用 vsCubeMapping.glsl 與 fsCubeMapping.glsl
// 目前並沒有在 Cubic Mapping 中納入 NORMAL_MAP 與 LIGHT_MAP
//#define CUBIC_MAP 1  // 這個只用在 Example 5

// 當模型有執行 non-uniform scale 的操作時，必須透過計算反矩陣來得到正確的 Normal 方向
// 開啟以下的定義即可，目前 CPU 計算的有提供
// GPU 的部分則是設定成註解

// #define GENERAL_CASE 1 


enum class ShadingMode
{
	FLAT_SHADING_CPU = 1,
	GOURAUD_SHADING_CPU = 2, // CPU Per Vertex Lighting / vsVtxColor
	GOURAUD_SHADING_GPU = 3, // GPU Per Vertex Lighting / vsPerVtxLighting
	PHONG_SHADING = 4		 // GPU Per PIXEL Lighting / vsPerPixelLighting
							 // vsPerPixelLighting 只有支援到 Diffue + Light Map
							 // 使用 Normal Map 時，會自動切換到 vsNormalMapLighting
							 // 使用 Cubic Map 時，會自動切換到 vsCubeMapping
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
	vec2 *_pTex2;	// 新增第二張貼圖 for example 3
//-------------------------------------------
// NORMAL_MAP
	vec2 *_pTex3;		// 新增第三張貼圖 for example 4
	vec3 *_pTangentV;	// 新增 tangent vector for each vertex
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

	point4  _vLightInView;	 // 光源在世界座標的位置
	GLuint  _uiLightInView;	 // 光源在 shader 的位置
	GLuint  _uiAmbient;		 // light's ambient  與 Object's ambient  與 ka 的乘積
	GLuint  _uiDiffuse;		 // light's diffuse  與 Object's diffuse  與 kd 的乘積
	GLuint  _uiSpecular;	 // light's specular 與 Object's specular 與 ks 的乘積
	GLuint  _uiShininess;
	GLuint  _uiLighting;	// 光源的個數
	GLuint  _uiTexLayer;	// 貼圖的層次，預設就是一層 diffuse

	// 將該物件目前為止出現的時間傳入，如果需要控制物件隨著時間改變，可以用這個變數
	GLuint  _uiElapsedTime;	// 該物件出現到目前為止經過的時間
	GLfloat _fElapsedTime;


// CUBIC_MAP------------------------------------------------------------------
	GLuint  _uiTRS;			// TRSMatrix 傳入 Pixel Shader 的位置
	GLuint  _uiViewPos;		// ViewPoint 傳入 Pixel Shader 的位置
	point4  _v4Eye;			// Camera 的位置

	GLuint  _uiCubeMap;     // pixel shader 中的 CubeMap Texture Name 的位置
	GLuint  _uiCubeMapTexName;  // 傳入 pixel shader 中的 Cube map 的貼圖編號， 
//----------------------------------------------------------------------------

	LightSource _Light1;

	color4 _AmbientProduct;
	color4 _DiffuseProduct;
	color4 _SpecularProduct;
	int    _iLighting;		// 設定是否要打燈

	// For Matrices
	mat4    _mxView, _mxProjection, _mxTRS;
	mat4    _mxMVFinal;
	mat3    _mxMV3X3Final;		// 使用在計算 物體旋轉後的新 Normal
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
	virtual void update(float dt) = 0; // 不計算光源的照明

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
	vec4 PhongReflectionModel(vec4 vPoint, vec3 vNormal, vec4 vLightPos, color4 vLightI); // 保留，但範例不會用到

	// 顏色的計算以三角面為基礎，並假設一個頂點、顏色、法向量與貼圖座標都是一對一對應
	void renderWithFlatShading(point4 vLightPos, color4 vLightI);    // 保留，但範例不會用到
	void renderWithGouraudShading(point4 vLightPos, color4 vLightI); // 保留，但範例不會用到
	void renderWithFlatShading(const LightSource& Lights);//  vLightI: Light Intensity
	void renderWithGouraudShading(const LightSource& Lights);//  vLightI: Light Intensity

	// For controlling texture mapping
	void setMirror(bool uAxis, bool vAxis); // U軸 與 V軸 是否要鏡射
	void setTiling(float uTiling, float vTiling);  // 對 U軸 與 V軸 進行拼貼的縮放
	void setLightMapTiling(float uTiling, float vTiling);  // 對 LightMap U軸 與 V軸 進行拼貼的縮放

	void setLightingDisable() {_iLighting = 0;}

	// For Cube Map
	void setCubeMapTexName(GLuint uiTexName) { _uiCubeMapTexName = uiTexName; }
	void setViewPosition(point4 vEye) { _v4Eye = vEye; }
};

#endif
