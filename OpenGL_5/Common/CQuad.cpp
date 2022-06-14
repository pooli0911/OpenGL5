#include "CQuad.h"
// Example 4 開始
// 面同 Example 3 朝上(Y軸)
// 每一個 Vertex 增加 Normal ，改成繼曾自 CShape，一併處理相關的設定需求


CQuad::CQuad()
{
	_iNumVtx = QUAD_NUM;
	_pPoints = NULL; _pNormals = NULL; _pTex1 = NULL;

	_pPoints  = new vec4[_iNumVtx];
	_pNormals = new vec3[_iNumVtx];
	_pColors  = new vec4[_iNumVtx]; 
	_pTex1 = new vec2[_iNumVtx];		// 產生所需的貼圖座標
// LIGHT_MAP --------------------------------
	_pTex2 = new vec2[_iNumVtx];  // 產生 light map 所需的貼圖座標
//-------------------------------------------
// NORMAL_MAP -------------------------------
	_pTex3 = new vec2[_iNumVtx];	// 產生 normal map 所需的貼圖座標
	// 產生所需要的 Tanget vector for each vertex
	_pTangentV = new vec3[_iNumVtx];
//-------------------------------------------

	_pPoints[0] = vec4( -0.5f, 0.0f,  0.5f, 1.0f);
	_pPoints[1] = vec4(  0.5f, 0.0f,  0.5f, 1.0f);
	_pPoints[2] = vec4(  0.5f, 0.0f, -0.5f, 1.0f);
	_pPoints[3] = vec4( -0.5f, 0.0f,  0.5f, 1.0f);
	_pPoints[4] = vec4(  0.5f, 0.0f, -0.5f, 1.0f);
	_pPoints[5] = vec4( -0.5f, 0.0f, -0.5f, 1.0f);

	_pNormals[0] = vec3(  0, 1.0f, 0);  // Normal Vector 的 W 為 0
	_pNormals[1] = vec3(  0, 1.0f, 0);
	_pNormals[2] = vec3(  0, 1.0f, 0);
	_pNormals[3] = vec3(  0, 1.0f, 0);
	_pNormals[4] = vec3(  0, 1.0f, 0);
	_pNormals[5] = vec3(  0, 1.0f, 0);

	_pColors[0] = vec4( 1.0f, 1.0f,  1.0f, 1.0f);  // (r, g, b, a)
	_pColors[1] = vec4( 1.0f, 1.0f,  1.0f, 1.0f);
	_pColors[2] = vec4( 1.0f, 1.0f,  1.0f, 1.0f);
	_pColors[3] = vec4( 1.0f, 1.0f,  1.0f, 1.0f);
	_pColors[4] = vec4( 1.0f, 1.0f,  1.0f, 1.0f);
	_pColors[5] = vec4( 1.0f, 1.0f,  1.0f, 1.0f);

	_pTex1[0] = vec2(0.0f, 0.0f);
	_pTex1[1] = vec2(1.0f, 0.0f);
	_pTex1[2] = vec2(1.0f, 1.0f);
	_pTex1[3] = vec2(0.0f, 0.0f);
	_pTex1[4] = vec2(1.0f, 1.0f);
	_pTex1[5] = vec2(0.0f, 1.0f);

// LIGHT_MAP --------------------------------
	_pTex2[0] = vec2(0.0f, 0.0f);
	_pTex2[1] = vec2(1.0f, 0.0f);
	_pTex2[2] = vec2(1.0f, 1.0f);
	_pTex2[3] = vec2(0.0f, 0.0f);
	_pTex2[4] = vec2(1.0f, 1.0f);
	_pTex2[5] = vec2(0.0f, 1.0f);
//-------------------------------------------
// NORMAL_MAP -------------------------------
	_pTex3[0] = vec2(0.0f, 0.0f);
	_pTex3[1] = vec2(1.0f, 0.0f);
	_pTex3[2] = vec2(1.0f, 1.0f);
	_pTex3[3] = vec2(0.0f, 0.0f);
	_pTex3[4] = vec2(1.0f, 1.0f);
	_pTex3[5] = vec2(0.0f, 1.0f);
	// 計算 tangent vector
	for (int i = 0; i < 6; i += 3) { // 三個 vertex 一組
		float dU1 = _pTex1[i + 1].x - _pTex1[i].x;
		float dV1 = _pTex1[i + 1].y - _pTex1[i].y;
		float dU2 = _pTex1[i + 2].x - _pTex1[i].x;
		float dV2 = _pTex1[i + 2].y - _pTex1[i].y;
		float f = 1.0f/(dU1 * dV2 - dU2*dV1);
		vec4 E1 = _pPoints[i + 1] - _pPoints[i];
		vec4 E2 = _pPoints[i + 2] - _pPoints[i];

		vec3 tangent;
		tangent.x = f*(dV2 * E1.x + E2.x * (-dV1));
		tangent.y = f*(dV2 * E1.y + E2.y * (-dV1));
		tangent.z = f*(dV2 * E1.z + E2.z * (-dV1));

		_pTangentV[i] += tangent;
		_pTangentV[i + 1] += tangent;
		_pTangentV[i + 2] = tangent;
	}
	for (int i = 0; i < 6; i++)
		_pTangentV[i] = normalize(_pTangentV[i]);
//-------------------------------------------

	for( int i = 0 ; i < _iNumVtx ; i++ ) _pColors[i] = vec4(-1.0f,-1.0f,-1.0f,1.0f);

	// 設定材質
	setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	setKaKdKsShini(0, 0.8f, 0.2f, 1);
}


void CQuad::update(float dt, const LightSource& lights)
{
	updateMatrix(); // 這行一定要有，進行矩陣的更新，再進行後續的顏色計算

	if (_iMode == ShadingMode::FLAT_SHADING_CPU) renderWithFlatShading(lights);
	else if (_iMode == ShadingMode::GOURAUD_SHADING_CPU) renderWithGouraudShading(lights);
	else {
		_vLightInView = _mxView * lights.position;		// 將 Light 轉換到鏡頭座標再傳入
		// 算出 AmbientProduct DiffuseProduct 與 SpecularProduct 的內容
		_AmbientProduct = _Material.ka * _Material.ambient * lights.ambient;
		_AmbientProduct.w = _Material.ambient.w;
		_DiffuseProduct = _Material.kd * _Material.diffuse * lights.diffuse;
		_DiffuseProduct.w = _Material.diffuse.w;
		_SpecularProduct = _Material.ks * _Material.specular * lights.specular;
		_SpecularProduct.w = _Material.specular.w;
	}
	_fElapsedTime += dt;
}


void CQuad::update(float dt)
{
	updateMatrix();
	//if (_bViewUpdated || _bTRSUpdated) { // Model View 的相關矩陣內容有更動
	//	_mxMVFinal = _mxView * _mxTRS;
	//	_mxITMV = InverseTransposeMatrix(_mxMVFinal); 
	//	_bViewUpdated = _bTRSUpdated = false;
	//}
}

void CQuad::setVtxColors(vec4 vLFColor, vec4 vLRColor, vec4 vTRColor, vec4 vTLColor)
{
	_pColors[3] = _pColors[0] = vLFColor;
	_pColors[1] = vLRColor;
	_pColors[4] = _pColors[2] = vTRColor;
	_pColors[5] = vTLColor;

	// 將所有 vertices 顏色更新到 VBO 中
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
}

void CQuad::setVtxColors(vec4 vFColor, vec4 vSColor) // 設定兩個三角形的顏色
{
	_pColors[0] = _pColors[1] = _pColors[2] = vFColor;
	_pColors[3] = _pColors[4] = _pColors[5] = vSColor;

	// 將所有 vertices 顏色更新到 VBO 中
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
}

void CQuad::draw()
{
	drawingSetShader();
	glDrawArrays( GL_TRIANGLES, 0, QUAD_NUM );
//	glBindTexture(GL_TEXTURE_2D, 0);  // 釋放現有的貼圖設定
}

void CQuad::drawW()
{
	drawingWithoutSetShader();
	glDrawArrays( GL_TRIANGLES, 0, QUAD_NUM );
//	glBindTexture(GL_TEXTURE_2D, 0);  // 釋放現有的貼圖設定
}


//void CQuad::renderWithFlatShading(const LightSource& lights)
//{
//	// 以每一個面的三個頂點計算其重心，以該重心作為顏色計算的點頂
//	// 根據 Phong lighting model 計算相對應的顏色，並將顏色儲存到此三個點頂
//	// 因為每一個平面的頂點的 Normal 都相同，所以此處並沒有計算此三個頂點的平均 Normal
//	vec4 vCentroidP;
//	for (int i = 0; i < _iNumVtx; i += 3) {
//		// 計算三角形的重心
//		vCentroidP = (_pPoints[i] + _pPoints[i + 1] + _pPoints[i + 2]) / 3.0f;
//		_pColors[i] = _pColors[i + 1] = _pColors[i + 2] = PhongReflectionModel(vCentroidP, _pNormals[i], lights);
//	}
//	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
//	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
//}
//
//
//void CQuad::renderWithGouraudShading(const LightSource& lights)
//{
//	// 以 vertex 為單位，根據該點的位置與其法向量，利用 Phong lighting model 計算相對應的顏色
//	// 將該顏色儲存回該頂點
//
//	// 第六個點頂構成的兩個三角形
//	// 計算 0 1 2 5 四個頂點的顏色即可，0 與 3、2 與 4 的顏色相同
//
//	_pColors[0] = _pColors[3] = PhongReflectionModel(_pPoints[0], _pNormals[0], lights);
//	_pColors[2] = _pColors[4] = PhongReflectionModel(_pPoints[2], _pNormals[2], lights);
//	_pColors[1] = PhongReflectionModel(_pPoints[1], _pNormals[1], lights);
//	_pColors[5] = PhongReflectionModel(_pPoints[5], _pNormals[5], lights);
//
//	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
//	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
//}


//
//void CQuad::update(float dt, const LightSource& lights)
//{
//
//#ifdef LIGHTING_WITHCPU
//	if (_bViewUpdated || _bTRSUpdated) { // Model View 的相關矩陣內容有更動
//		_mxMVFinal = _mxView * _mxTRS;
//		_mxMV3X3Final = mat3(
//			_mxMVFinal._m[0].x, _mxMVFinal._m[1].x, _mxMVFinal._m[2].x,
//			_mxMVFinal._m[0].y, _mxMVFinal._m[1].y, _mxMVFinal._m[2].y,
//			_mxMVFinal._m[0].z, _mxMVFinal._m[1].z, _mxMVFinal._m[2].z);
//#ifdef GENERAL_CASE
//		_mxITMV = InverseTransposeMatrix(_mxMVFinal);
//#endif
//		_bViewUpdated = _bTRSUpdated = false;
//	}
//	if (_iMode == FLAT_SHADING) RenderWithFlatShading(lights);
//	else RenderWithGouraudShading(lights);
//
//#else // Lighting With GPU
//	if (_bViewUpdated || _bTRSUpdated) {
//		_mxMVFinal = _mxView * _mxTRS;
//		_bViewUpdated = _bTRSUpdated = false;
//	}
//	_vLightInView = _mxView * lights.position;		// 將 Light 轉換到鏡頭座標再傳入
//	// 算出 AmbientProduct DiffuseProduct 與 SpecularProduct 的內容
//	_AmbientProduct = _Material.ka * _Material.ambient * lights.ambient;
//	_AmbientProduct.w = _Material.ambient.w;
//	_DiffuseProduct = _Material.kd * _Material.diffuse * lights.diffuse;
//	_DiffuseProduct.w = _Material.diffuse.w;
//	_SpecularProduct = _Material.ks * _Material.specular * lights.specular;
//	_SpecularProduct.w = _Material.specular.w;
//#endif
//	_fElapsedTime += dt;
//
//}