#include "CWireCube.h"

CWireCube::CWireCube()
{

	_iNumVtx = WIRECUBE_NUM;
	_pPoints = NULL; _pNormals = NULL; _pTex1 = NULL;

	_pPoints  = new vec4[_iNumVtx];
	_pNormals = new vec3[_iNumVtx];
	_pColors  = new vec4[_iNumVtx]; 
	_pTex1     = new vec2[_iNumVtx];
#if MULTITEXTURE >= LIGHT_MAP
	_pTex2 = new vec2[_iNumVtx];  // 產生 light map 所需的貼圖座標
#endif
#if MULTITEXTURE >= NORMAL_MAP
	_pTex3 = new vec2[_iNumVtx];	// 產生 normal map 所需的貼圖座標
	_pTangentV = new vec3[_iNumVtx];
#endif

    _vertices[0] = point4( -0.5, -0.5,  0.5, 1.0 );
    _vertices[1] = point4( -0.5,  0.5,  0.5, 1.0 );
    _vertices[2] = point4(  0.5,  0.5,  0.5, 1.0 );
    _vertices[3] = point4(  0.5, -0.5,  0.5, 1.0 );
    _vertices[4] = point4( -0.5, -0.5, -0.5, 1.0 );
    _vertices[5] = point4( -0.5,  0.5, -0.5, 1.0 );
    _vertices[6] = point4(  0.5,  0.5, -0.5, 1.0 );
	_vertices[7] = point4(  0.5, -0.5, -0.5, 1.0 );

	_iIndex = 0;
	// generate 12 triangles: 36 vertices and 36 colors
    Quad( 1, 0, 3, 2 );
    Quad( 2, 3, 7, 6 );
    Quad( 3, 0, 4, 7 );
    Quad( 6, 5, 1, 2 );
    Quad( 4, 5, 6, 7 );
    Quad( 5, 4, 0, 1 );

	// 預設將所有的面都設定成灰色
	for( int i = 0 ; i < _iNumVtx ; i++ ) _pColors[i] = vec4(-1.0f,-1.0f,-1.0f,1.0f);

#ifdef PERVERTEX_LIGHTING
	setShaderName("vsPerVtxLighting.glsl", "fsPerVtxLighting.glsl");
#else
	setShaderName("vsPerPixelLighting.glsl", "fsPerPixelLighting.glsl");
#endif  

	// Create and initialize a buffer object ，將此部分的設定移入 SetShader 中
	// CreateBufferObject();

	// 設定材質
	setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	setKaKdKsShini(0, 0.8f, 0.2f, 1);
}

void CWireCube::Quad( int a, int b, int c, int d )
{
    // Initialize temporary vectors along the quad's edge to
    //   compute its face normal 
    vec4 u = _vertices[b] - _vertices[a];
    vec4 v = _vertices[c] - _vertices[b];
    vec3 normal = normalize( cross(u, v) );

    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[a]; _pTex1[_iIndex] = vec2(0,0);_iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[b]; _pTex1[_iIndex] = vec2(1,0);_iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[c]; _pTex1[_iIndex] = vec2(1,1);_iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[d]; _pTex1[_iIndex] = vec2(0,1);_iIndex++;
}


void CWireCube::renderWithFlatShading(const LightSource &lights)
{
	// 以每一個面的三個頂點計算其重心，以該重心作為顏色計算的點頂
	// 根據 Phong lighting model 計算相對應的顏色，並將顏色儲存到此三個點頂
	// 因為每一個平面的頂點的 Normal 都相同，所以此處並沒有計算此三個頂點的平均 Normal

	vec4 vCentroidP;
	for( int i = 0 ; i < _iNumVtx ; i += 4 ) {
		// 計算三角形的重心
		vCentroidP = (_pPoints[i] + _pPoints[i+1] + _pPoints[i+2] + _pPoints[i+3])/4.0f;
		_pColors[i] = _pColors[i + 1] = _pColors[i + 2] = _pColors[i + 3] = PhongReflectionModel(vCentroidP, _pNormals[i], lights);
	}
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
}


void CWireCube::renderWithGouraudShading(const LightSource &lights)
{
	for( int i = 0 ; i < _iNumVtx ; i += 4 ) {
		_pColors[i] = PhongReflectionModel(_pPoints[i], _pNormals[i], lights);
		_pColors[i + 1] = PhongReflectionModel(_pPoints[i + 1], _pNormals[i + 1], lights);
		_pColors[i + 2] = PhongReflectionModel(_pPoints[i + 2], _pNormals[i + 2], lights);
		_pColors[i + 3] = PhongReflectionModel(_pPoints[i + 3], _pNormals[i + 3], lights);
	}
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
}


// 此處所給的 vLightPos 必須是世界座標的確定絕對位置
void CWireCube::update(float dt, const LightSource &lights)
{
#ifdef LIGHTING_WITHCPU
	if( _bViewUpdated || _bTRSUpdated  ) { // Model View 的相關矩陣內容有更動
		_mxMVFinal = _mxView * _mxTRS;
		_mxMV3X3Final = mat3(
			_mxMVFinal._m[0].x,  _mxMVFinal._m[1].x, _mxMVFinal._m[2].x,
			_mxMVFinal._m[0].y,  _mxMVFinal._m[1].y, _mxMVFinal._m[2].y,
			_mxMVFinal._m[0].z,  _mxMVFinal._m[1].z, _mxMVFinal._m[2].z);
#ifdef GENERAL_CASE
		_mxITMV = InverseTransposeMatrix(_mxMVFinal); 
#endif
		_bViewUpdated = _bTRSUpdated = false;
	}
	if (_iMode == FLAT_SHADING) RenderWithFlatShading(lights);
	else RenderWithGouraudShading(lights);

#else // Lighting With GPU
	if (_bViewUpdated || _bTRSUpdated) {
		_mxMVFinal = _mxView * _mxTRS;
		_bViewUpdated = _bTRSUpdated = false;
	}
	_vLightInView = _mxView * lights.position;		// 將 Light 轉換到鏡頭座標再傳入
	// 算出 AmbientProduct DiffuseProduct 與 SpecularProduct 的內容
	_AmbientProduct = _Material.ka * _Material.ambient  * lights.ambient;
	_AmbientProduct.w = _Material.ambient.w;
	_DiffuseProduct = _Material.kd * _Material.diffuse  * lights.diffuse;
	_DiffuseProduct.w = _Material.diffuse.w;
	_SpecularProduct = _Material.ks * _Material.specular * lights.specular;
	_SpecularProduct.w = _Material.specular.w;
#endif

}


void CWireCube::update(float dt)
{
	if (_bViewUpdated || _bTRSUpdated) { // Model View 的相關矩陣內容有更動
		_mxMVFinal = _mxView * _mxTRS;
		_mxITMV = InverseTransposeMatrix(_mxMVFinal); 
		_bViewUpdated = _bTRSUpdated = false;
	}
}


void CWireCube::draw()
{
	drawingSetShader();
	glDrawArrays( GL_LINE_LOOP,  0, 4 );
	glDrawArrays( GL_LINE_LOOP,  4, 4 );
	glDrawArrays( GL_LINE_LOOP,  8, 4 );
	glDrawArrays( GL_LINE_LOOP, 12, 4 );
	glDrawArrays( GL_LINE_LOOP, 16, 4 );
	glDrawArrays( GL_LINE_LOOP, 20, 4 );
}


void CWireCube::drawW()
{
	drawingWithoutSetShader();
	glDrawArrays( GL_LINE_LOOP,  0, 4 );
	glDrawArrays( GL_LINE_LOOP,  4, 4 );
	glDrawArrays( GL_LINE_LOOP,  8, 4 );
	glDrawArrays( GL_LINE_LOOP, 12, 4 );
	glDrawArrays( GL_LINE_LOOP, 16, 4 );
	glDrawArrays( GL_LINE_LOOP, 20, 4 );
}


