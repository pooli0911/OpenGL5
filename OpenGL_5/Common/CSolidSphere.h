#ifndef CSOLIDSPHERE_H
#define CSOLIDSPHERE_H
#include "../header/Angel.h"
#include "CShape.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

class CSolidSphere : public CShape
{
private:
	GLfloat _fRadius;
	GLint _iSlices, _iStacks;

public:
	CSolidSphere(const GLfloat fRadius=1.0f, const int iSlices=12,const  int iStacks = 6);
	~CSolidSphere();

	void update(float dt, const LightSource &lights);
	void update(float dt); // ���p��������ө�

	// Sphere ��ø�s��l�ϥΦh�ժ� GL_TRIANGLE_STRIP ��ø�s, �]���S����k���� Flat Shading�A
	// �u���H vertex ����¦���p���C�⪺ Ground Shading
	void renderWithFlatShading(const LightSource &lights);
	void renderWithGouraudShading(const LightSource &lights);
	void draw();
	void drawW(); // �I�s���A���]�w Shader ���yø�覡

};

#endif