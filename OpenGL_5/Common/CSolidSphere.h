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
	void update(float dt); // 不計算光源的照明

	// Sphere 的繪製方始使用多組的 GL_TRIANGLE_STRIP 來繪製, 因此沒有辦法提供 Flat Shading，
	// 只有以 vertex 為基礎的計算顏色的 Ground Shading
	void renderWithFlatShading(const LightSource &lights);
	void renderWithGouraudShading(const LightSource &lights);
	void draw();
	void drawW(); // 呼叫不再次設定 Shader 的描繪方式

};

#endif