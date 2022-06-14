#include "CQuad.h"
// Example 4 �}�l
// ���P Example 3 �¤W(Y�b)
// �C�@�� Vertex �W�[ Normal �A�令�~���� CShape�A�@�ֳB�z�������]�w�ݨD


CQuad::CQuad()
{
	_iNumVtx = QUAD_NUM;
	_pPoints = NULL; _pNormals = NULL; _pTex1 = NULL;

	_pPoints  = new vec4[_iNumVtx];
	_pNormals = new vec3[_iNumVtx];
	_pColors  = new vec4[_iNumVtx]; 
	_pTex1 = new vec2[_iNumVtx];		// ���ͩһݪ��K�Ϯy��
// LIGHT_MAP --------------------------------
	_pTex2 = new vec2[_iNumVtx];  // ���� light map �һݪ��K�Ϯy��
//-------------------------------------------
// NORMAL_MAP -------------------------------
	_pTex3 = new vec2[_iNumVtx];	// ���� normal map �һݪ��K�Ϯy��
	// ���ͩһݭn�� Tanget vector for each vertex
	_pTangentV = new vec3[_iNumVtx];
//-------------------------------------------

	_pPoints[0] = vec4( -0.5f, 0.0f,  0.5f, 1.0f);
	_pPoints[1] = vec4(  0.5f, 0.0f,  0.5f, 1.0f);
	_pPoints[2] = vec4(  0.5f, 0.0f, -0.5f, 1.0f);
	_pPoints[3] = vec4( -0.5f, 0.0f,  0.5f, 1.0f);
	_pPoints[4] = vec4(  0.5f, 0.0f, -0.5f, 1.0f);
	_pPoints[5] = vec4( -0.5f, 0.0f, -0.5f, 1.0f);

	_pNormals[0] = vec3(  0, 1.0f, 0);  // Normal Vector �� W �� 0
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
	// �p�� tangent vector
	for (int i = 0; i < 6; i += 3) { // �T�� vertex �@��
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

	// �]�w����
	setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	setKaKdKsShini(0, 0.8f, 0.2f, 1);
}


void CQuad::update(float dt, const LightSource& lights)
{
	updateMatrix(); // �o��@�w�n���A�i��x�}����s�A�A�i������C��p��

	if (_iMode == ShadingMode::FLAT_SHADING_CPU) renderWithFlatShading(lights);
	else if (_iMode == ShadingMode::GOURAUD_SHADING_CPU) renderWithGouraudShading(lights);
	else {
		_vLightInView = _mxView * lights.position;		// �N Light �ഫ�����Y�y�ЦA�ǤJ
		// ��X AmbientProduct DiffuseProduct �P SpecularProduct �����e
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
	//if (_bViewUpdated || _bTRSUpdated) { // Model View �������x�}���e�����
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

	// �N�Ҧ� vertices �C���s�� VBO ��
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
}

void CQuad::setVtxColors(vec4 vFColor, vec4 vSColor) // �]�w��ӤT���Ϊ��C��
{
	_pColors[0] = _pColors[1] = _pColors[2] = vFColor;
	_pColors[3] = _pColors[4] = _pColors[5] = vSColor;

	// �N�Ҧ� vertices �C���s�� VBO ��
	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
}

void CQuad::draw()
{
	drawingSetShader();
	glDrawArrays( GL_TRIANGLES, 0, QUAD_NUM );
//	glBindTexture(GL_TEXTURE_2D, 0);  // ����{�����K�ϳ]�w
}

void CQuad::drawW()
{
	drawingWithoutSetShader();
	glDrawArrays( GL_TRIANGLES, 0, QUAD_NUM );
//	glBindTexture(GL_TEXTURE_2D, 0);  // ����{�����K�ϳ]�w
}


//void CQuad::renderWithFlatShading(const LightSource& lights)
//{
//	// �H�C�@�ӭ����T�ӳ��I�p��䭫�ߡA�H�ӭ��ߧ@���C��p�⪺�I��
//	// �ھ� Phong lighting model �p��۹������C��A�ñN�C���x�s�즹�T���I��
//	// �]���C�@�ӥ��������I�� Normal ���ۦP�A�ҥH���B�èS���p�⦹�T�ӳ��I������ Normal
//	vec4 vCentroidP;
//	for (int i = 0; i < _iNumVtx; i += 3) {
//		// �p��T���Ϊ�����
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
//	// �H vertex �����A�ھڸ��I����m�P��k�V�q�A�Q�� Phong lighting model �p��۹������C��
//	// �N���C���x�s�^�ӳ��I
//
//	// �Ĥ����I���c������ӤT����
//	// �p�� 0 1 2 5 �|�ӳ��I���C��Y�i�A0 �P 3�B2 �P 4 ���C��ۦP
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
//	if (_bViewUpdated || _bTRSUpdated) { // Model View �������x�}���e�����
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
//	_vLightInView = _mxView * lights.position;		// �N Light �ഫ�����Y�y�ЦA�ǤJ
//	// ��X AmbientProduct DiffuseProduct �P SpecularProduct �����e
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