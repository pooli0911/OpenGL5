#ifndef CWIRESPHERE_H
#define CWIRESPHERE_H
#include "../header/Angel.h"
#include "CShape.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define WIRESPHERE_NUM 36  // 6 faces, 2 triangles/face , 3 vertices/triangle

class CWireSphere : public CShape
{
private:
	GLfloat _fRadius;
	GLint _iSlices, _iStacks;

	vec4 _Points[WIRESPHERE_NUM];
	vec3 _Normals[WIRESPHERE_NUM];
	vec4 _vertices[8];
	int  _iIndex;

public:
	CWireSphere(const GLfloat fRadius=1.0f, const int iSlices=12,const  int iStacks = 6);
	~CWireSphere();

	void update(float dt, const LightSource &lights);
	void update(float dt); // ���p��������ө�

	void renderWithFlatShading(const LightSource &lights);
	void renderWithGouraudShading(const LightSource &lights);
	void draw();
	void drawW(); // �I�s���A���]�w Shader ���yø�覡
};

#endif