#include "CShape.h"
#include "CShaderPool.h"

CShape::CShape()
{
	_bTRSUpdated = _bViewUpdated = _bProjUpdated = false;

	// ambient �w�]�� 0, diffuse, specular ���C�ⳣ�O�Ǧ� 0.5
	// Ka = 0 �Y�� , kd = 0.8 , ks = 0.2
	_Material.ambient = vec4(vec3(0));
	_Material.diffuse = vec4(vec3(0.5f));
	_Material.specular = vec4(vec3(0.5f));
	_Material.ka = 0; _Material.kd = 0.8f; _Material.ks = 0.2f;
	_Material.shininess = 2.0f;

	_iMode = ShadingMode::FLAT_SHADING_CPU;
	// �w�]�� RGBA �� (0.5,0.5,0.5,1.0) , �ѳo�ӦǶ��C��ӥN�������C��
	_fColor[0] = 0.5f; _fColor[1] = 0.5f; _fColor[2] = 0.5f; _fColor[3] = 1.0f;

	_iLighting = 1;	// �w�]�����O�����ө�
	_iTexLayer = TextureMode::DIFFUSE_MAP;	// �w�]���@�i Diffuse �K��
	_pVXshader = nullptr;
	_pFSshader = nullptr;

	_fElapsedTime = 0;		// �w�]�ɶ��q 0 �}�l

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
	else if (_iTexLayer <= (TextureMode::DIFFUSE_MAP | TextureMode::LIGHT_MAP)) { // �]�t��i�K��
		glBufferData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2) * 2) * _iNumVtx, NULL, GL_STATIC_DRAW);
	}
	else { // diffuse/light map �P tangent map �����}��
		glBufferData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2) * 3 + sizeof(vec3)) * _iNumVtx, NULL, GL_STATIC_DRAW);
	}

	// vertices
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * _iNumVtx, _pPoints);
	// vertices' normal
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx, sizeof(vec3) * _iNumVtx, _pNormals);
	// vertcies' Color
	glBufferSubData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3)) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors);

	if ( _iTexLayer >= TextureMode::DIFFUSE_MAP  ) { // Cubic Map �|�Ψ� diffuse map
		// Diffuse Map's Texture Coordinates
		glBufferSubData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4)) * _iNumVtx, sizeof(vec2) * _iNumVtx, _pTex1);  //�Ĥ@�i�K��
	}
	if (_iTexLayer >= TextureMode::LIGHT_MAP ) { 
		// Light Map's Texture Coordinates, cubic map �� shader �S���䴩 LIGHT MAP
		glBufferSubData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2)) * _iNumVtx, sizeof(vec2) * _iNumVtx, _pTex2); // �ĤG�i�K��
	}
	if (_iTexLayer >= TextureMode::NORMAL_MAP ) {
		// Normal+ Light Map Texture Coordinates, cubic map �� shader �S���䴩 LIGHT MAP
		glBufferSubData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2) * 2) * _iNumVtx, sizeof(vec2) * _iNumVtx, _pTex3); // �ĤT�i�K��
		glBufferSubData(GL_ARRAY_BUFFER, (sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2) * 3) * _iNumVtx, sizeof(vec3) * _iNumVtx, _pTangentV); // �ĤT�i�K��	
	}
}

void CShape::setShader(GLuint uiShaderHandle)
{
	// ���m�o��, ��K�C�@�Ӫ��󪺳]�w
	createBufferObject();

	if (_iMode == ShadingMode::FLAT_SHADING_CPU || _iMode == ShadingMode::GOURAUD_SHADING_CPU )  // CPU �p�� Vertex �C��
		setShaderName("vsLighting_CPU.glsl", "fsLighting_CPU.glsl");
	else if (_iMode == ShadingMode::GOURAUD_SHADING_GPU)
		setShaderName("vsPerVtxLighting.glsl", "fsPerVtxLighting.glsl"); // GPU �b Vertex Shader �ݭp�⳻�I�C��
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
		GLuint vLightMapCoord = glGetAttribLocation(_uiProgram, "vLightMapCoord");  // Light maps' texture coordinates�A �����s�W�� Shader ��
		glEnableVertexAttribArray(vLightMapCoord);
		glVertexAttribPointer(vLightMapCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET((sizeof(vec4) + sizeof(vec3) + sizeof(vec4) + sizeof(vec2)) * _iNumVtx));
		glUniform1i(glGetUniformLocation(_uiProgram, "lightMap"), 1);
		//#endif
	}
	if (_iTexLayer >= TextureMode::NORMAL_MAP ) {
		// For Normal Map
		GLuint vNormalMapCoord = glGetAttribLocation(_uiProgram, "vNormalMapCoord");  // Light maps' texture coordinates�A �����s�W�� Shader ��
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
	glUniform1i(_uiLighting, _iLighting); // �������Ӽ�

	_uiTexLayer = glGetUniformLocation(_uiProgram, "iTexLayer");
	glUniform1i(_uiTexLayer, static_cast<int>(_iTexLayer));	// �K�Ϫ��ӼơA�w�]�� 1�A�����ǤJ pixel shader

	_uiElapsedTime = glGetUniformLocation(_uiProgram, "fElapsedTime");
	glUniform1f(_uiElapsedTime, _fElapsedTime);	// �K�Ϫ��ӼơA�w�]�� 1�A�����ǤJ pixel shader

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
	// �N��T��s�� Shader ��
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

// ���B�w�]�ϥΫe�@�Ӵyø����ҨϥΪ� Shader
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
	if (_bViewUpdated || _bTRSUpdated) { // Model View �������x�}���e�����
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


// ���]�C�@�Ӽҫ��������O�T�����A�ӥB���I�B�C��P�k�V�q���x�s�]�O�T�ӤT�Ӥ@��
void CShape::renderWithFlatShading(point4 vLightPos, color4 vLightI) //  vLightI: Light Intensity
{
	// �H�C�@�ӭ����T�ӳ��I�p��䭫�ߡA�H�ӭ��ߧ@���C��p�⪺�I��
	// �ھ� Phong lighting model �p��۹������C��A�ñN�C���x�s�즹�T���I��
	// �p��T�ӳ��I�� Normal �������Ȭ��ӭ����k�V�q

	vec4 vCentroidP;
	vec3 vNormal;
	for (int i = 0; i < _iNumVtx; i += 3) {
		// �p��T���Ϊ�����
		vCentroidP = (_pPoints[i] + _pPoints[i + 1] + _pPoints[i + 2]) / 3.0f;
		vNormal = (_pNormals[i] + _pNormals[i + 1] + _pNormals[i + 2]) / 3.0f;
		_pColors[i] = _pColors[i + 1] = _pColors[i + 2] = PhongReflectionModel(vCentroidP, vNormal, vLightPos, vLightI);
	}
	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
}

void CShape::renderWithFlatShading(const LightSource& Lights) //  vLightI: Light Intensity
{
	// �H�C�@�ӭ����T�ӳ��I�p��䭫�ߡA�H�ӭ��ߧ@���C��p�⪺�I��
	// �ھ� Phong lighting model �p��۹������C��A�ñN�C���x�s�즹�T���I��
	// �p��T�ӳ��I�� Normal �������Ȭ��ӭ����k�V�q
	vec4 vCentroidP;
	vec3 vNormal;
	for (int i = 0; i < _iNumVtx; i += 3) {
		// �p��T���Ϊ�����
		vCentroidP = (_pPoints[i] + _pPoints[i + 1] + _pPoints[i + 2]) / 3.0f;
		vNormal = (_pNormals[i] + _pNormals[i + 1] + _pNormals[i + 2]) / 3.0f;
		_pColors[i] = _pColors[i + 1] = _pColors[i + 2] = PhongReflectionModel(vCentroidP, vNormal, Lights.position, Lights.diffuse);
	}
	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
}

// ���]�C�@�Ӽҫ��������O�T�����A�ӥB���I�B�C��P�k�V�q���x�s�]�O�T�ӤT�Ӥ@��
void CShape::renderWithGouraudShading(point4 vLightPos, color4 vLightI)//  vLightI: Light Intensity
{
	// �H vertex �����A�ھڸ��I����m�P��k�V�q�A�Q�� Phong lighting model �p��۹������C��
	// �N���C���x�s�^�ӳ��I

	for (int i = 0; i < _iNumVtx; i++) {
		_pColors[i] = PhongReflectionModel(_pPoints[i], _pNormals[i], vLightPos, vLightI);
	}

	// �p�G�n�]�w Spot Light�A�b���B�p������P�Q�ө��I�������A���� vLightI �����e�A�W���A�I�s�ǤJ�Y�i

	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
}

void  CShape::renderWithGouraudShading(const LightSource& Lights) //  vLightI: Light Intensity
{
	// �H vertex �����A�ھڸ��I����m�P��k�V�q�A�Q�� Phong lighting model �p��۹������C��
	// �N���C���x�s�^�ӳ��I
	for (int i = 0; i < _iNumVtx; i++) {
		_pColors[i] = PhongReflectionModel(_pPoints[i], _pNormals[i], Lights);
	}
	// �p�G�n�]�w Spot Light�A�b���B�p������P�Q�ө��I�������A���� vLightI �����e�A�W���A�I�s�ǤJ�Y�i

	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
}


// Phong Reflection Model
vec4 CShape::PhongReflectionModel(vec4 vPoint, vec3 vNormal, const LightSource &lights)
{
	vec3 vN;		// �� vec3 �ӫŧi�O���F�`�٭p��, �p�G�n���{���g�_�ӧ��K�A�i��� vec4 �ӫŧi
	vec4 vColor;	// �ӳ��I���C��
	vec4 vPosInView, vLightInView;

	// 1. �N�I��������Y�y�Шt�ΡA�]�����I�|�ھ� TRS Matrix ���ʡA�ҥH�����H _mxMVFinal �p��b�@�ɮy�Ъ������m
	vPosInView = _mxMVFinal * vPoint;

	// 2. �N���� Normal ������Y�y�Шt�ΡA���ন���V�q
	//    �u������ rotation �|���� Normal ����V�A���W���� Final ModelView Matrix�A�P���ഫ�����Y�y��
	//    _mxMV3X3Final �b Update �B����  _mxMVFinal �����W�� 3X3 ����, ���ݭn�첾�q
#ifdef GENERAL_CASE
	//	  _mxITMV �w�g�b�I�s setModelViewMatrix(mat4 &mat) �P setTRSMatrix(mat4 &mat) �N�P�ɭp��
	//    ���B�N���ΦA���s�p��
	vN = _mxITMV * vNormal;
#else
	vN = _mxMV3X3Final * vNormal;
#endif

	vN = normalize(vN);

	// 3. �p�� Ambient color :  Ia = Ka * Material.ambient * La
	vColor = _Material.ka * _Material.ambient * lights.diffuse;

	// 4. �p�� Light �b���Y�y�Шt�Ϊ���m
	//    ���B���] Light ����m�w�g�b�@�ɮy�Ъ������m
	vLightInView = _mxView * lights.position;

	vec3 vLight; // �� vec3 �ӫŧi�O���F�`�٭p��, �p�G�n���{���g�_�ӧ��K�A�i��� vec4 �ӫŧi
	vLight.x = vLightInView.x - vPosInView.x;
	vLight.y = vLightInView.y - vPosInView.y;
	vLight.z = vLightInView.z - vPosInView.z;
	vLight = normalize(vLight); // normalize light vector

	// 5. �p�� L dot N
	GLfloat fLdotN = vLight.x*vN.x + vLight.y*vN.y + vLight.z*vN.z;
	if (fLdotN > 0) { // ���I�Q�����Ө�~�ݭn�p��
		// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
		vColor += _Material.kd * _Material.diffuse * lights.diffuse * fLdotN;

		// Specular color
		// Method 1: Phone Model
		//   �p�� View Vector
		vec3 vView;
		vView.x = 0 - vPosInView.x;  // �ثe�w�g�H���Y�y�Ь���¦, �ҥH View ����m�N�O (0,0,0)
		vView.y = 0 - vPosInView.y;
		vView.z = 0 - vPosInView.z;
		vView = normalize(vView);

		//	 �p�� Light �� �Ϯg�� vRefL
		vec3 vRefL = 2.0f * (fLdotN)* vN - vLight;

		vRefL = normalize(vRefL);
		//   �p��  vReflectedL dot View
		GLfloat RdotV = vRefL.x*vView.x + vRefL.y*vView.y + vRefL.z*vView.z;

		// Specular Color : Is = Ks * Ls * (R dot V)^Shininess;
		if (RdotV > 0) vColor += _Material.ks * _Material.specular * lights.diffuse * powf(RdotV, _Material.shininess);
	}

	vColor.w = 1; // Alpha ��]�w�� 1�A�w�]���O���z������
	// Method 2: Modified Phone Model 

	// �N�C���x�s��  _Colors ���A�]�� Quad �O��Ӧ@�������T�����Һc��, �ҥH�]�w��ӤT���������ۦP���C��
	// �]�N�O�]�w�Ҧ������I���O�o���C��
	return vColor;
}

// Phong Reflection Model
vec4 CShape::PhongReflectionModel(vec4 vPoint, vec3 vNormal, vec4 vLightPos, color4 vLightI)
{
	vec3 vN;		// �� vec3 �ӫŧi�O���F�`�٭p��, �p�G�n���{���g�_�ӧ��K�A�i��� vec4 �ӫŧi
	vec4 vColor;	// �ӳ��I���C��
	vec4 vPosInView, vLightInView;

	// 1. �N�I��������Y�y�Шt�ΡA�]�����I�|�ھ� TRS Matrix ���ʡA�ҥH�����H _mxMVFinal �p��b�@�ɮy�Ъ������m
	vPosInView = _mxMVFinal * vPoint;

	// 2. �N���� Normal ������Y�y�Шt�ΡA���ন���V�q
	//    �u������ rotation �|���� Normal ����V�A���W���� Final ModelView Matrix�A�P���ഫ�����Y�y��
	//    _mxMV3X3Final �b Update �B���� _mxMVFinal �����W�� 3X3 ����, ���ݭn�첾�q
#ifdef GENERAL_CASE
	//	  _mxITMV �w�g�b�I�s setModelViewMatrix(mat4 &mat) �P setTRSMatrix(mat4 &mat) �N�P�ɭp��
	//    ���B�N���ΦA���s�p��
	vN = _mxITMV * vNormal;
#else
	vN = _mxMV3X3Final * vNormal;
#endif

	vN = normalize(vN);

	// 3. �p�� Ambient color :  Ia = Ka * Material.ambient * La
	vColor = _Material.ka * _Material.ambient * vLightI;

	// 4. �p�� Light �b���Y�y�Шt�Ϊ���m
	//    ���B���] Light ����m�w�g�b�@�ɮy�Ъ������m
	vLightInView = _mxView * vLightPos;

	vec3 vLight; // �� vec3 �ӫŧi�O���F�`�٭p��, �p�G�n���{���g�_�ӧ��K�A�i��� vec4 �ӫŧi
	vLight.x = vLightInView.x - vPosInView.x;
	vLight.y = vLightInView.y - vPosInView.y;
	vLight.z = vLightInView.z - vPosInView.z;
	vLight = normalize(vLight); // normalize light vector

	// 5. �p�� L dot N
	GLfloat fLdotN = vLight.x * vN.x + vLight.y * vN.y + vLight.z * vN.z;
	if (fLdotN > 0) { // ���I�Q�����Ө�~�ݭn�p��
		// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
		vColor += _Material.kd * _Material.diffuse * vLightI * fLdotN;

		// Specular color
		// Method 1: Phone Model
		//   �p�� View Vector
		vec3 vView;
		vView.x = 0 - vPosInView.x;  // �ثe�w�g�H���Y�y�Ь���¦, �ҥH View ����m�N�O (0,0,0)
		vView.y = 0 - vPosInView.y;
		vView.z = 0 - vPosInView.z;
		vView = normalize(vView);

		//	 �p�� Light �� �Ϯg�� vRefL
		vec3 vRefL = 2.0f * (fLdotN)*vN - vLight;

		vRefL = normalize(vRefL);
		//   �p��  vReflectedL dot View
		GLfloat RdotV = vRefL.x * vView.x + vRefL.y * vView.y + vRefL.z * vView.z;

		// Specular Color : Is = Ks * Ls * (R dot V)^Shininess;
		if (RdotV > 0) vColor += _Material.ks * _Material.specular * vLightI * powf(RdotV, _Material.shininess);
	}

	vColor.w = 1; // Alpha ��]�w�� 1�A�w�]���O���z������
	// Method 2: Modified Phone Model 

	// �N�C���x�s��  _Colors ���A�]�� Quad �O��Ӧ@�������T�����Һc��, �ҥH�]�w��ӤT���������ۦP���C��
	// �]�N�O�]�w�Ҧ������I���O�o���C��
	return vColor;
}


// For controlling texture mapping ----------------
void CShape::setMirror(bool uAxis, bool vAxis) // U�b �P V�b �O�_�n��g
{
	if( uAxis )  {
		if( vAxis ) { // U V �b�P����g
			for(int i = 0 ; i < _iNumVtx ; i++ ) { // �N�C�@�� vertex ���K�Ϯy�Х� 1 �h��
				_pTex1[i].x -= 1.0f; _pTex1[i].y -= 1.0f;
			}
		}
		else { // �u�� U �b��g
			for(int i = 0 ; i < _iNumVtx ; i++ ) { // �N�C�@�� vertex ���K�Ϯy�Х� 1 �h��
				_pTex1[i].x -= 1.0f; // x �N�O U �b
			}
		}
	}
	else if( vAxis ) { // �u�� V �b��g
		for(int i = 0 ; i < _iNumVtx ; i++ ) { // �N�C�@�� vertex ���K�Ϯy�Х� 1 �h��
			_pTex1[i].y -= 1.0f; // y �� V �b
		}
	}
	else ;
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, (sizeof(vec4)+sizeof(vec3)+sizeof(vec4))*_iNumVtx, sizeof(vec2)*_iNumVtx, _pTex1 ); // vertcies' Color

}

void CShape::setTiling(float uTiling, float vTiling)  // �� U�b �P V�b �i����K���Y��
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