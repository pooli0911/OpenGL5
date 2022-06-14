#ifndef CQUAD_H
#define CQUAD_H
#include "../header/Angel.h"
#include "CShape.h"

#define QUAD_NUM 6		// 2 faces, 2 triangles/face 

class CQuad : public CShape
{
private:

	//vec4 m_Points[QUAD_NUM];
	//vec3 m_Normal[QUAD_NUM];	// �� vec3 �ӫŧi�O���F�`�٭p��, �p�G�n���{���g�_�ӧ��K�A�i��� vec4 �ӫŧi
	//vec4 m_Colors[QUAD_NUM];

public:
	CQuad();

	void update(float dt, const LightSource &lights);
	void update(float dt); // ���p��������ө�

	GLuint getShaderHandle() { return _uiProgram;} 
	void setVtxColors(vec4 vLFColor, vec4 vLRColor, vec4 vTRColor, vec4 vTLColor); // four Vertices' Color
	void setVtxColors(vec4 vFColor, vec4 vSColor);	// three Vertices' Color with idx as the first 

	void draw();
	void drawW();

	//void renderWithFlatShading(const LightSource& lights);//  vLightI: Light Intensity
	//void renderWithGouraudShading(const LightSource& lights);//  vLightI: Light Intensity
};




#endif