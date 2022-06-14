#ifndef CCHECKER_H
#define CCHECKER_H

#include "TypeDefine.h"
#include "CQuad.h"


// �H (0,0) ������ ���X�@�� nXn ���ѽL���
// n �b�ŧi�ɵ��w
class CChecker {

private:
	CQuad *_pSquares;
	int    _iGridSize;
	float  *_pfSquaresT;
	GLuint _uiShaderHandle;
	float  _fYPos;
	ShadingMode _iMode;


public:
	CChecker(int iSize = 6, float fYPos = 0.0f); // �w�]�� 6 X 6 ���, �C�@�Ӥ��������O 1
	~CChecker();

	void setShader();
	void setProjectionMatrix(mat4 &mat);
	void setViewMatrix(mat4 &mat);
	void setTRSMatrix(mat4 &mat);
	void setShadingMode(ShadingMode iMode);
	void setTextureLayer(TextureMode texlayer);
	void update(float dt, const LightSource &lights);
	void draw();

	// For setting materials 
	void setMaterials(color4 ambient, color4 diffuse, color4 specular);
	void setKaKdKsShini(float ka, float kd, float ks, float shininess); // ka kd ks shininess
};

#endif