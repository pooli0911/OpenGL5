#include "CShape.h"
#include "CShaderPool.h"

CShape::CShape()
{
	_bTRSUpdated = _bViewUpdated = _bProjUpdated = false;

	// ambient 預設為 0, diffuse, specular 的顏色都是灰色 0.5
	// Ka = 0 係數 , kd = 0.8 , ks = 0.2
	_Material.ambient = vec4(vec3(0));
	_Material.diffuse = vec4(vec3(0.5f));
	_Material.specular = vec4(vec3(0.5f));
	_Material.ka = 0; _Material.kd = 0.8f; _Material.ks = 0.2f;
	_Material.shininess = 2.0f;

	_iMode = ShadingMode::FLAT_SHADING_CPU;
	// 預設為 RGBA 為 (0.5,0.5,0.5,1.0) , 由這個灰階顏色來代表的物件顏色
	_fColor[0] = 0.5f; _fColor[1] = 0.5f; _fColor[2] = 0.5f; _fColor[3] = 1.0f;

	_iLighting = 1;	// 預設接受燈光的照明
	_iTexLayer = TextureMode::DIFFUSE_MAP;	// 預設有一張 Diffuse 貼圖
	_pVXshader = nullptr;
	_pFSshader = nullptr;

	_fElapsedTime = 0;		// 預設時間從 0 開始

	_pPoints = nullptr; 	_pNormals = nullptr; 	_pColors = nullptr; 	_pTex1 = nullptr;
// LIGHT_MAP ----------------------------------
	_pTex2 = nullptr;	
//---------------------------------------------
// NORMAL_MAP ---------------------------------
	_pTex3 = nullptr;
	_pTangentV = nullptr;
//---------------------------------------------
	 
}

CShape::~CShape()
{
	if( _pPoints  != nullptr ) delete [] _pPoints;  
	if( _pNormals != nullptr ) delete	[] _pNormals;
	if( _pColors  != nullptr ) delete	[] _pColors;
	if (_pTex1 != nullptr) delete _pTex1;
// LIGHT_MAP ----------------------------------
	if (_pTex2 != nullptr) delete _pTex2;		// Example 3
//---------------------------------------------
// NORMAL_MAP ---------------------------------
	if (_pTex3 != nullptr) delete _pTex3;		// Example 4
	if (_pTangentV != nullptr) delete[] _pTangentV;
//---------------------------------------------

	if( _pVXshader != nullptr) delete [] _pVXshader;
	if( _pFSshader != nullptr) delete [] _pFSshader;
}

void CShape::createBufferObject()
{
	glGenVertexArrays(1, &_uiVao);
	glBindVertexArray(_uiVao);

	// Create and initialize a buffer object
	glGenBuffers(1, &_uiBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);

	if (_iTexLayer == TextureMode::NO_TEXTURE) {
		glBufferData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4)) * _iNumVtx, NULL, GL_STATIC_DRAW);
	}
	else if (_iTexLayer <= TextureMode::DIFFUSE_MAP ) {
		glBufferData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2)) * _iNumVtx, NULL, GL_STATIC_DRAW);
		// (sizeof(vec4)*_iNumVtx + sizeof(vec3)*_iNumVtx + sizeof(vec4)*_iNumVtx <- vertices, normal and color
		// (sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2))*_iNumVtx <- vertices, normal, color and texture coordinates
	}
	else if (_iTexLayer <= (TextureMode::DIFFUSE_MAP | TextureMode::LIGHT_MAP)) { // 包含兩張貼圖
		glBufferData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2) * 2) * _iNumVtx, NULL, GL_STATIC_DRAW);
	}
	else { // diffuse/light map 與 tangent map 全部開啟
		glBufferData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2) * 3 + sizeof(vec3)) * _iNumVtx, NULL, GL_STATIC_DRAW);
	}

	// vertices
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * _iNumVtx, _pPoints);
	// vertices' normal
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx, sizeof(vec3) * _iNumVtx, _pNormals);
	// vertcies' Color
	glBufferSubData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3)) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors);

	if ( _iTexLayer >= TextureMode::DIFFUSE_MAP  ) { // Cubic Map 會用到 diffuse map
		// Diffuse Map's Texture Coordinates
		glBufferSubData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4)) * _iNumVtx, sizeof(vec2) * _iNumVtx, _pTex1);  //第一張貼圖
	}
	if (_iTexLayer >= TextureMode::LIGHT_MAP ) { 
		// Light Map's Texture Coordinates, cubic map 的 shader 沒有支援 LIGHT MAP
		glBufferSubData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2)) * _iNumVtx, sizeof(vec2) * _iNumVtx, _pTex2); // 第二張貼圖
	}
	if (_iTexLayer >= TextureMode::NORMAL_MAP ) {
		// Normal+ Light Map Texture Coordinates, cubic map 的 shader 沒有支援 LIGHT MAP
		glBufferSubData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2) * 2) * _iNumVtx, sizeof(vec2) * _iNumVtx, _pTex3); // 第三張貼圖
		glBufferSubData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2) * 3) * _iNumVtx, sizeof(vec3) * _iNumVtx, _pTangentV); // 第三張貼圖	
	}
}

void CShape::setShader(GLuint uiShaderHandle)
{
	// 改放置這裡, 方便每一個物件的設定
	createBufferObject();

	if (_iMode == ShadingMode::FLAT_SHADING_CPU || _iMode == ShadingMode::GOURAUD_SHADING_CPU )  // CPU 計算 Vertex 顏色
		setShaderName("vsLighting_CPU.glsl", "fsLighting_CPU.glsl");
	else if (_iMode == ShadingMode::GOURAUD_SHADING_GPU)
		setShaderName("vsPerVtxLighting.glsl", "fsPerVtxLighting.glsl"); // GPU 在 Vertex Shader 端計算頂點顏色
	else {
		if (_iTexLayer < TextureMode::NORMAL_MAP) 
			setShaderName("vsPerPixelLighting.glsl", "fsPerPixelLighting.glsl");
		else if ((_iTexLayer & TextureMode::NORMAL_MAP) == TextureMode::NORMAL_MAP)
			setShaderName("vsNormalMapLighting.glsl", "fsNormalMapLighting.glsl");
		else setShaderName("vsCubeMapping.glsl", "fsCubeMapping.glsl");
	}

    // Load shaders and use the resulting shader program
	if (uiShaderHandle == MAX_UNSIGNED_INT) {
		_uiProgram = CShaderPool::getInstance()->getShaderID(_pVXshader, _pFSshader);
		//_uiProgram = InitShader(_pVXshader, _pFSshader);
	}
	else _uiProgram = uiShaderHandle;

	glUseProgram(_uiProgram);
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( _uiProgram, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

    GLuint vNormal = glGetAttribLocation( _uiProgram, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(vec4)*_iNumVtx) );

    GLuint vColorVtx = glGetAttribLocation( _uiProgram, "vVtxColor" );  // vertices' color 
    glEnableVertexAttribArray( vColorVtx );
    glVertexAttribPointer( vColorVtx, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET((sizeof(vec4)+sizeof(vec3))*_iNumVtx));

	GLuint vDifMapCoord = glGetAttribLocation( _uiProgram, "vDiffuseMapCoord" );  // vertices' texture coordinates
	glEnableVertexAttribArray( vDifMapCoord );
	glVertexAttribPointer( vDifMapCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET((sizeof(vec4)+sizeof(vec3)+sizeof(vec4))*_iNumVtx));
	glUniform1i(glGetUniformLocation(_uiProgram, "diffuMap"), 0);

	if ( _iTexLayer >= TextureMode::LIGHT_MAP ) {
		// For Light Map
		GLuint vLightMapCoord = glGetAttribLocation(_uiProgram, "vLightMapCoord");  // Light maps' texture coordinates， 必須新增到 Shader 中
		glEnableVertexAttribArray(vLightMapCoord);
		glVertexAttribPointer(vLightMapCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET((sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2)) * _iNumVtx));
		glUniform1i(glGetUniformLocation(_uiProgram, "lightMap"), 1);
		//#endif
	}
	if (_iTexLayer >= TextureMode::NORMAL_MAP ) {
		// For Normal Map
		GLuint vNormalMapCoord = glGetAttribLocation(_uiProgram, "vNormalMapCoord");  // Light maps' texture coordinates， 必須新增到 Shader 中
		glEnableVertexAttribArray(vNormalMapCoord);
		glVertexAttribPointer(vNormalMapCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET((sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2) * 2) * _iNumVtx));
		glUniform1i(glGetUniformLocation(_uiProgram, "normalMap"), 2);

		GLuint vTangentVec = glGetAttribLocation(_uiProgram, "vTangentV");  // vertices' color 
		glEnableVertexAttribArray(vTangentVec);
		glVertexAttribPointer(vTangentVec, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET((sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2) * 3) * _iNumVtx));
		//#endif
	}

	_uiModelView = glGetUniformLocation( _uiProgram, "ModelView" );
	glUniformMatrix4fv( _uiModelView, 1, GL_TRUE, _mxMVFinal );

    _uiProjection = glGetUniformLocation( _uiProgram, "Projection" );
	glUniformMatrix4fv( _uiProjection, 1, GL_TRUE, _mxProjection );

    _uiColor = glGetUniformLocation( _uiProgram, "vObjectColor" );
	glUniform4fv(_uiColor, 1, _fColor ); 

	_uiLightInView = glGetUniformLocation( _uiProgram, "LightInView" );
	glUniform4fv(_uiLightInView, 1, _vLightInView); 

	_uiAmbient = glGetUniformLocation( _uiProgram, "AmbientProduct" );
	glUniform4fv(_uiAmbient, 1, _AmbientProduct); 

	_uiDiffuse = glGetUniformLocation( _uiProgram, "DiffuseProduct" );
	glUniform4fv(_uiDiffuse, 1, _DiffuseProduct); 

	_uiSpecular = glGetUniformLocation( _uiProgram, "SpecularProduct" );
	glUniform4fv(_uiSpecular, 1, _SpecularProduct); 

	_uiShininess = glGetUniformLocation( _uiProgram, "fShininess" );
	glUniform1f(_uiShininess, _Material.shininess); 

	_uiLighting = glGetUniformLocation( _uiProgram, "iLighting" );
	glUniform1i(_uiLighting, _iLighting); // 光源的個數

	_uiTexLayer = glGetUniformLocation(_uiProgram, "iTexLayer");
	glUniform1i(_uiTexLayer, static_cast<int>(_iTexLayer));	// 貼圖的個數，預設為 1，直接傳入 pixel shader

	_uiElapsedTime = glGetUniformLocation(_uiProgram, "fElapsedTime");
	glUniform1f(_uiElapsedTime, _fElapsedTime);	// 貼圖的個數，預設為 1，直接傳入 pixel shader

	if ((_iTexLayer & TextureMode::CUBIC_MAP ) == TextureMode::CUBIC_MAP) {
		// For Cube Map
		_uiCubeMap = glGetUniformLocation(_uiProgram, "cubeMap");
		glUniform1i(_uiCubeMap, _uiCubeMapTexName);

		_uiViewPos = glGetUniformLocation(_uiProgram, "ViewPos");
		glUniform4fv(_uiViewPos, 1, _v4Eye);

		_uiTRS = glGetUniformLocation(_uiProgram, "TRSMatrix");
		glUniformMatrix4fv(_uiTRS, 1, GL_TRUE, _mxTRS);
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void CShape::drawingSetShader()
{
	glUseProgram(_uiProgram);
	glBindVertexArray(_uiVao);
	glUniformMatrix4fv(_uiModelView, 1, GL_TRUE, _mxMVFinal);

	if (_bProjUpdated) {
		glUniformMatrix4fv(_uiProjection, 1, GL_TRUE, _mxProjection);
		_bProjUpdated = false;
	}
	// 將資訊更新到 Shader 中
	glUniform4fv(_uiColor, 1, _fColor);

	glUniform4fv(_uiLightInView, 1, _vLightInView);
	glUniform4fv(_uiAmbient, 1, _AmbientProduct);
	glUniform4fv(_uiDiffuse, 1, _DiffuseProduct);
	glUniform4fv(_uiSpecular, 1, _SpecularProduct);
	glUniform1f(_uiShininess, _Material.shininess);
	glUniform1i(_uiLighting, _iLighting);
	glUniform1i(_uiTexLayer, static_cast<int>(_iTexLayer));

	glUniform1f(_uiElapsedTime, _fElapsedTime);

	if ((_iTexLayer & TextureMode::CUBIC_MAP) == TextureMode::CUBIC_MAP) {
		glUniform4fv(_uiViewPos, 1, _v4Eye);
		glUniformMatrix4fv(_uiTRS, 1, GL_TRUE, _mxTRS);
	}
}

// 此處預設使用前一個描繪物件所使用的 Shader
void CShape::drawingWithoutSetShader()
{
	glBindVertexArray( _uiVao );
	glUniformMatrix4fv( _uiModelView, 1, GL_TRUE, _mxMVFinal );

	if( _bProjUpdated ) {
		glUniformMatrix4fv( _uiProjection, 1, GL_TRUE, _mxProjection );
		_bProjUpdated = false;
	}
	glUniform4fv(_uiColor, 1, _fColor ); 

	glUniform4fv(_uiLightInView, 1, _vLightInView);
	glUniform4fv(_uiAmbient, 1, _AmbientProduct);
	glUniform4fv(_uiDiffuse, 1, _DiffuseProduct);
	glUniform4fv(_uiSpecular, 1, _SpecularProduct);
	glUniform1f(_uiShininess, _Material.shininess);
	glUniform1i(_uiLighting, _iLighting);
	glUniform1i(_uiTexLayer, static_cast<int>(_iTexLayer));

	if (static_cast<int>(_iTexLayer & TextureMode::CUBIC_MAP) == 0x8) {
		// CUBIC_MAP
		glUniform4fv(_uiViewPos, 1, _v4Eye);
		glUniformMatrix4fv(_uiTRS, 1, GL_TRUE, _mxTRS);
	}
}

void CShape::setShaderName(const char vxShader[], const char fsShader[])
{
	int len;
	len = strlen(vxShader);
	_pVXshader = new char[len+1];
	memcpy(_pVXshader, vxShader, len+1);

	len = strlen(fsShader);
	_pFSshader = new char[len+1];
	memcpy(_pFSshader, fsShader, len+1);
}

void CShape::setViewMatrix(mat4 &mat)
{
	_mxView = mat;
	_bViewUpdated = true;
}

void CShape::setTRSMatrix(mat4 &mat)
{
	_mxTRS = mat;
	_bTRSUpdated = true;
}

void CShape::setProjectionMatrix(mat4 &mat)
{
	_mxProjection = mat;
	_bProjUpdated = true;
}

void CShape::setColor(vec4 vColor)
{
	_fColor[0] = vColor.x;
	_fColor[1] = vColor.y;
	_fColor[2] = vColor.z;
	_fColor[3] = vColor.w;
//	glUniform4fv(_uiColor, 1, _fColor); 
}

void CShape::setTextureLayer(TextureMode texlayer)
{
	_iTexLayer = texlayer;
}

void CShape::setMaterials(color4 ambient, color4 diffuse, color4 specular)
{
	_Material.ambient =  ambient;
	_Material.diffuse =  diffuse;
	_Material.specular = specular;
}

void CShape::setKaKdKsShini(float ka, float kd, float ks, float shininess) // ka kd ks shininess
{
	_Material.ka = ka;
	_Material.kd = kd;
	_Material.ks = ks;
	_Material.shininess = shininess;
}

void CShape::updateMatrix()
{
	if (_bViewUpdated || _bTRSUpdated) { // Model View 的相關矩陣內容有更動
		_mxMVFinal = _mxView * _mxTRS;
		_bViewUpdated = _bTRSUpdated = false;
		if (_iMode == ShadingMode::FLAT_SHADING_CPU || _iMode == ShadingMode::GOURAUD_SHADING_CPU) {
			_mxMV3X3Final = mat3(
				_mxMVFinal._m[0].x, _mxMVFinal._m[1].x, _mxMVFinal._m[2].x,
				_mxMVFinal._m[0].y, _mxMVFinal._m[1].y, _mxMVFinal._m[2].y,
				_mxMVFinal._m[0].z, _mxMVFinal._m[1].z, _mxMVFinal._m[2].z);
#ifdef GENERAL_CASE
			_mxITMV = InverseTransposeMatrix(_mxMVFinal);
#endif		
		}
	}
}


// 假設每一個模型的面都是三角面，而且頂點、顏色與法向量的儲存也是三個三個一組
void CShape::renderWithFlatShading(point4 vLightPos, color4 vLightI) //  vLightI: Light Intensity
{
	// 以每一個面的三個頂點計算其重心，以該重心作為顏色計算的點頂
	// 根據 Phong lighting model 計算相對應的顏色，並將顏色儲存到此三個點頂
	// 計算三個頂點的 Normal 的平均值為該面的法向量

	vec4 vCentroidP;
	vec3 vNormal;
	for (int i = 0; i < _iNumVtx; i += 3) {
		// 計算三角形的重心
		vCentroidP = (_pPoints[i] + _pPoints[i + 1] + _pPoints[i + 2]) / 3.0f;
		vNormal = (_pNormals[i] + _pNormals[i + 1] + _pNormals[i + 2]) / 3.0f;
		_pColors[i] = _pColors[i + 1] = _pColors[i + 2] = PhongReflectionModel(vCentroidP, vNormal, vLightPos, vLightI);
	}
	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
}

void CShape::renderWithFlatShading(const LightSource& Lights) //  vLightI: Light Intensity
{
	// 以每一個面的三個頂點計算其重心，以該重心作為顏色計算的點頂
	// 根據 Phong lighting model 計算相對應的顏色，並將顏色儲存到此三個點頂
	// 計算三個頂點的 Normal 的平均值為該面的法向量
	vec4 vCentroidP;
	vec3 vNormal;
	for (int i = 0; i < _iNumVtx; i += 3) {
		// 計算三角形的重心
		vCentroidP = (_pPoints[i] + _pPoints[i + 1] + _pPoints[i + 2]) / 3.0f;
		vNormal = (_pNormals[i] + _pNormals[i + 1] + _pNormals[i + 2]) / 3.0f;
		_pColors[i] = _pColors[i + 1] = _pColors[i + 2] = PhongReflectionModel(vCentroidP, vNormal, Lights.position, Lights.diffuse);
	}
	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
}

// 假設每一個模型的面都是三角面，而且頂點、顏色與法向量的儲存也是三個三個一組
void CShape::renderWithGouraudShading(point4 vLightPos, color4 vLightI)//  vLightI: Light Intensity
{
	// 以 vertex 為單位，根據該點的位置與其法向量，利用 Phong lighting model 計算相對應的顏色
	// 將該顏色儲存回該頂點

	for (int i = 0; i < _iNumVtx; i++) {
		_pColors[i] = PhongReflectionModel(_pPoints[i], _pNormals[i], vLightPos, vLightI);
	}

	// 如果要設定 Spot Light，在此處計算光源與被照明點的夾角，改變 vLightI 的內容，上面再呼叫傳入即可

	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
}

void  CShape::renderWithGouraudShading(const LightSource& Lights) //  vLightI: Light Intensity
{
	// 以 vertex 為單位，根據該點的位置與其法向量，利用 Phong lighting model 計算相對應的顏色
	// 將該顏色儲存回該頂點
	for (int i = 0; i < _iNumVtx; i++) {
		_pColors[i] = PhongReflectionModel(_pPoints[i], _pNormals[i], Lights);
	}
	// 如果要設定 Spot Light，在此處計算光源與被照明點的夾角，改變 vLightI 的內容，上面再呼叫傳入即可

	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
}


// Phong Reflection Model
vec4 CShape::PhongReflectionModel(vec4 vPoint, vec3 vNormal, const LightSource &lights)
{
	vec3 vN;		// 用 vec3 來宣告是為了節省計算, 如果要讓程式寫起來更方便，可改用 vec4 來宣告
	vec4 vColor;	// 該頂點的顏色
	vec4 vPosInView, vLightInView;

	// 1. 將點頂轉到鏡頭座標系統，因為頂點會根據 TRS Matrix 移動，所以必須以 _mxMVFinal 計算在世界座標的絕對位置
	vPosInView = _mxMVFinal * vPoint;

	// 2. 將面的 Normal 轉到鏡頭座標系統，並轉成單位向量
	//    只有物件的 rotation 會改變 Normal 的方向，乘上物件的 Final ModelView Matrix，同時轉換到鏡頭座標
	//    _mxMV3X3Final 在 Update 處取自  _mxMVFinal 的左上的 3X3 部分, 不需要位移量
#ifdef GENERAL_CASE
	//	  _mxITMV 已經在呼叫 setModelViewMatrix(mat4 &mat) 與 setTRSMatrix(mat4 &mat) 就同時計算
	//    此處就不用再重新計算
	vN = _mxITMV * vNormal;
#else
	vN = _mxMV3X3Final * vNormal;
#endif

	vN = normalize(vN);

	// 3. 計算 Ambient color :  Ia = Ka * Material.ambient * La
	vColor = _Material.ka * _Material.ambient * lights.diffuse;

	// 4. 計算 Light 在鏡頭座標系統的位置
	//    此處假設 Light 的位置已經在世界座標的絕對位置
	vLightInView = _mxView * lights.position;

	vec3 vLight; // 用 vec3 來宣告是為了節省計算, 如果要讓程式寫起來更方便，可改用 vec4 來宣告
	vLight.x = vLightInView.x - vPosInView.x;
	vLight.y = vLightInView.y - vPosInView.y;
	vLight.z = vLightInView.z - vPosInView.z;
	vLight = normalize(vLight); // normalize light vector

	// 5. 計算 L dot N
	GLfloat fLdotN = vLight.x*vN.x + vLight.y*vN.y + vLight.z*vN.z;
	if (fLdotN > 0) { // 該點被光源照到才需要計算
		// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
		vColor += _Material.kd * _Material.diffuse * lights.diffuse * fLdotN;

		// Specular color
		// Method 1: Phone Model
		//   計算 View Vector
		vec3 vView;
		vView.x = 0 - vPosInView.x;  // 目前已經以鏡頭座標為基礎, 所以 View 的位置就是 (0,0,0)
		vView.y = 0 - vPosInView.y;
		vView.z = 0 - vPosInView.z;
		vView = normalize(vView);

		//	 計算 Light 的 反射角 vRefL
		vec3 vRefL = 2.0f * (fLdotN)* vN - vLight;

		vRefL = normalize(vRefL);
		//   計算  vReflectedL dot View
		GLfloat RdotV = vRefL.x*vView.x + vRefL.y*vView.y + vRefL.z*vView.z;

		// Specular Color : Is = Ks * Ls * (R dot V)^Shininess;
		if (RdotV > 0) vColor += _Material.ks * _Material.specular * lights.diffuse * powf(RdotV, _Material.shininess);
	}

	vColor.w = 1; // Alpha 改設定成 1，預設都是不透明物體
	// Method 2: Modified Phone Model 

	// 將顏色儲存到  _Colors 中，因為 Quad 是兩個共平面的三角面所構成, 所以設定兩個三角面都有相同的顏色
	// 也就是設定所有的頂點都是這個顏色
	return vColor;
}

// Phong Reflection Model
vec4 CShape::PhongReflectionModel(vec4 vPoint, vec3 vNormal, vec4 vLightPos, color4 vLightI)
{
	vec3 vN;		// 用 vec3 來宣告是為了節省計算, 如果要讓程式寫起來更方便，可改用 vec4 來宣告
	vec4 vColor;	// 該頂點的顏色
	vec4 vPosInView, vLightInView;

	// 1. 將點頂轉到鏡頭座標系統，因為頂點會根據 TRS Matrix 移動，所以必須以 _mxMVFinal 計算在世界座標的絕對位置
	vPosInView = _mxMVFinal * vPoint;

	// 2. 將面的 Normal 轉到鏡頭座標系統，並轉成單位向量
	//    只有物件的 rotation 會改變 Normal 的方向，乘上物件的 Final ModelView Matrix，同時轉換到鏡頭座標
	//    _mxMV3X3Final 在 Update 處取自 _mxMVFinal 的左上的 3X3 部分, 不需要位移量
#ifdef GENERAL_CASE
	//	  _mxITMV 已經在呼叫 setModelViewMatrix(mat4 &mat) 與 setTRSMatrix(mat4 &mat) 就同時計算
	//    此處就不用再重新計算
	vN = _mxITMV * vNormal;
#else
	vN = _mxMV3X3Final * vNormal;
#endif

	vN = normalize(vN);

	// 3. 計算 Ambient color :  Ia = Ka * Material.ambient * La
	vColor = _Material.ka * _Material.ambient * vLightI;

	// 4. 計算 Light 在鏡頭座標系統的位置
	//    此處假設 Light 的位置已經在世界座標的絕對位置
	vLightInView = _mxView * vLightPos;

	vec3 vLight; // 用 vec3 來宣告是為了節省計算, 如果要讓程式寫起來更方便，可改用 vec4 來宣告
	vLight.x = vLightInView.x - vPosInView.x;
	vLight.y = vLightInView.y - vPosInView.y;
	vLight.z = vLightInView.z - vPosInView.z;
	vLight = normalize(vLight); // normalize light vector

	// 5. 計算 L dot N
	GLfloat fLdotN = vLight.x * vN.x + vLight.y * vN.y + vLight.z * vN.z;
	if (fLdotN > 0) { // 該點被光源照到才需要計算
		// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
		vColor += _Material.kd * _Material.diffuse * vLightI * fLdotN;

		// Specular color
		// Method 1: Phone Model
		//   計算 View Vector
		vec3 vView;
		vView.x = 0 - vPosInView.x;  // 目前已經以鏡頭座標為基礎, 所以 View 的位置就是 (0,0,0)
		vView.y = 0 - vPosInView.y;
		vView.z = 0 - vPosInView.z;
		vView = normalize(vView);

		//	 計算 Light 的 反射角 vRefL
		vec3 vRefL = 2.0f * (fLdotN)*vN - vLight;

		vRefL = normalize(vRefL);
		//   計算  vReflectedL dot View
		GLfloat RdotV = vRefL.x * vView.x + vRefL.y * vView.y + vRefL.z * vView.z;

		// Specular Color : Is = Ks * Ls * (R dot V)^Shininess;
		if (RdotV > 0) vColor += _Material.ks * _Material.specular * vLightI * powf(RdotV, _Material.shininess);
	}

	vColor.w = 1; // Alpha 改設定成 1，預設都是不透明物體
	// Method 2: Modified Phone Model 

	// 將顏色儲存到  _Colors 中，因為 Quad 是兩個共平面的三角面所構成, 所以設定兩個三角面都有相同的顏色
	// 也就是設定所有的頂點都是這個顏色
	return vColor;
}


// For controlling texture mapping ----------------
void CShape::setMirror(bool uAxis, bool vAxis) // U軸 與 V軸 是否要鏡射
{
	if( uAxis )  {
		if( vAxis ) { // U V 軸同時鏡射
			for(int i = 0 ; i < _iNumVtx ; i++ ) { // 將每一個 vertex 的貼圖座標用 1 去減
				_pTex1[i].x -= 1.0f; _pTex1[i].y -= 1.0f;
			}
		}
		else { // 只有 U 軸鏡射
			for(int i = 0 ; i < _iNumVtx ; i++ ) { // 將每一個 vertex 的貼圖座標用 1 去減
				_pTex1[i].x -= 1.0f; // x 就是 U 軸
			}
		}
	}
	else if( vAxis ) { // 只有 V 軸鏡射
		for(int i = 0 ; i < _iNumVtx ; i++ ) { // 將每一個 vertex 的貼圖座標用 1 去減
			_pTex1[i].y -= 1.0f; // y 為 V 軸
		}
	}
	else ;
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, (sizeof(vec4)+sizeof(vec3)+sizeof(vec4))*_iNumVtx, sizeof(vec2)*_iNumVtx, _pTex1 ); // vertcies' Color

}

void CShape::setTiling(float uTiling, float vTiling)  // 對 U軸 與 V軸 進行拼貼的縮放
{
	for(int i = 0 ; i < _iNumVtx ; i++ ) {
		_pTex1[i].x *= uTiling; _pTex1[i].y *= vTiling;
	}
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, (sizeof(vec4)+sizeof(vec3)+sizeof(vec4))*_iNumVtx, sizeof(vec2)*_iNumVtx, _pTex1 ); // vertcies' Color
}

void CShape::setLightMapTiling(float uTiling, float vTiling)
{
	for (int i = 0; i < _iNumVtx; i++) {
		_pTex2[i].x *= uTiling; _pTex2[i].y *= vTiling;
	}
	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2))*_iNumVtx, sizeof(vec2)*_iNumVtx, _pTex2);
}
//-----------------------------------------------------------------------------------------------------------------------------