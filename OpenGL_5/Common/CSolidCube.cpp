#include "CSolidCube.h"

CSolidCube::CSolidCube()
{
	_iNumVtx = SOLIDCUBE_NUM;
	_pPoints = NULL; _pNormals = NULL; _pTex1 = NULL;

	_pPoints  = new vec4[_iNumVtx];
	_pNormals = new vec3[_iNumVtx];
	_pColors  = new vec4[_iNumVtx]; 
	_pTex1    = new vec2[_iNumVtx];
//#if MULTITEXTURE >= LIGHT_MAP
	_pTex2 = new vec2[_iNumVtx];  // ���� light map �һݪ��K�Ϯy��
//#endif
//#if MULTITEXTURE >= NORMAL_MAP
	_pTex3 = new vec2[_iNumVtx];	// ���� normal map �һݪ��K�Ϯy��
	_pTangentV = new vec3[_iNumVtx];
//#endif

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

	// �w�]�N�Ҧ��������]�w���Ǧ�
	for( int i = 0 ; i < _iNumVtx ; i++ ) _pColors[i] = vec4(-1.0f,-1.0f,-1.0f,1.0f);

	for( int i = 0 ; i < _iNumVtx ; i++ ) {
//#if MULTITEXTURE >= LIGHT_MAP
		_pTex2[i] = _pTex1[i];  // ���� light map �һݪ��K�Ϯy��
//#endif
//#if MULTITEXTURE >= NORMAL_MAP
		_pTex3[i] = _pTex1[i];;	// ���� normal map �һݪ��K�Ϯy��
//#endif
	}
//#if MULTITEXTURE >= NORMAL_MAP
	// �p�� tangent vector
	for (int i = 0; i < SOLIDCUBE_NUM; i += 3) { // �T�� vertex �@��
		float dU1 = _pTex3[i + 1].x - _pTex3[i].x;
		float dV1 = _pTex3[i + 1].y - _pTex3[i].y;
		float dU2 = _pTex3[i + 2].x - _pTex3[i].x;
		float dV2 = _pTex3[i + 2].y - _pTex3[i].y;
		float f = 1.0f / (dU1 * dV2 - dU2*dV1);
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
	for (int i = 0; i < SOLIDCUBE_NUM; i++)
		_pTangentV[i] = normalize(_pTangentV[i]);
//#endif

	// �]�w����
	setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	setKaKdKsShini(0, 0.8f, 0.2f, 1);
}

void CSolidCube::Quad( int a, int b, int c, int d )
{
    // Initialize temporary vectors along the quad's edge to
    //   compute its face normal 
    vec4 u = _vertices[b] - _vertices[a];
    vec4 v = _vertices[c] - _vertices[b];
    vec3 normal = normalize( cross(u, v) );

    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[a]; _pTex1[_iIndex] = vec2(0,0);_iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[b]; _pTex1[_iIndex] = vec2(1,0);_iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[c]; _pTex1[_iIndex] = vec2(1,1);_iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[a]; _pTex1[_iIndex] = vec2(0,0);_iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[c]; _pTex1[_iIndex] = vec2(1,1);_iIndex++;
    _pNormals[_iIndex] = normal; _pPoints[_iIndex] = _vertices[d]; _pTex1[_iIndex] = vec2(0,1);_iIndex++;

}

void CSolidCube::draw()
{
//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Change to wireframe mode
	drawingSetShader();
	glDrawArrays( GL_TRIANGLES, 0, SOLIDCUBE_NUM );
//	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Return to solid mode
}


void CSolidCube::drawW()
{
	drawingWithoutSetShader();
	glDrawArrays( GL_TRIANGLES, 0, SOLIDCUBE_NUM );
}


void CSolidCube::update(float dt, const LightSource& lights)
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
}


void CSolidCube::update(float dt)
{
	updateMatrix();
	//if (_bViewUpdated || _bTRSUpdated) { // Model View �������x�}���e�����
	//	_mxMVFinal = _mxView * _mxTRS;
	//	_mxITMV = InverseTransposeMatrix(_mxMVFinal); 
	//	_bViewUpdated = _bTRSUpdated = false;
	//}
}


//void CSolidCube::renderWithFlatShading(const LightSource& lights)
//{
//	// �H�C�@�ӭ����T�ӳ��I�p��䭫�ߡA�H�ӭ��ߧ@���C��p�⪺�I��
//	// �ھ� Phong lighting model �p��۹������C��A�ñN�C���x�s�즹�T���I��
//	// �]���C�@�ӥ��������I�� Normal ���ۦP�A�ҥH���B�èS���p�⦹�T�ӳ��I������ Normal
//
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
//void CSolidCube::renderWithGouraudShading(const LightSource& lights)
//{
//	vec4 vCentroidP;
//	for (int i = 0; i < _iNumVtx; i += 6) {
//		_pColors[i] = _pColors[i + 3] = PhongReflectionModel(_pPoints[i], _pNormals[i], lights);
//		_pColors[i + 2] = _pColors[i + 4] = PhongReflectionModel(_pPoints[i + 2], _pNormals[i + 2], lights);
//		_pColors[i + 1] = PhongReflectionModel(_pPoints[i + 1], _pNormals[i + 1], lights);
//		_pColors[i + 5] = PhongReflectionModel(_pPoints[i + 5], _pNormals[i + 5], lights);
//	}
//	glBindBuffer(GL_ARRAY_BUFFER, _uiBuffer);
//	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * _iNumVtx + sizeof(vec3) * _iNumVtx, sizeof(vec4) * _iNumVtx, _pColors); // vertcies' Color
//}
//void CSolidCube::update(float dt, const LightSource& lights)
//{
//#ifdef LIGHTING_WITHCPU
//	if (_bViewUpdated || _bTRSUpdated) { // Model View �������x�}���e�����
//		_mxMVFinal = _mxView * _mxTRS;
//		_mxMV3X3Final = mat3(
//			_mxMVFinal._m[0].x, _mxMVFinal._m[1].x, _mxMVFinal._m[2].x,
//			_mxMVFinal._m[0].y, _mxMVFinal._m[1].y, _mxMVFinal._m[2].y,
//			_mxMVFinal._m[0].z, _mxMVFinal._m[1].z, _mxMVFinal._m[2].z);
//
//#ifdef GENERAL_CASE
//		_mxITMV = InverseTransposeMatrix(_mxMVFinal);
//#endif
//
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
//
//}