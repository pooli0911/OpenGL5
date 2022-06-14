#ifndef CSOLIDCUBE_H
#define CSOLIDCUBE_H
#include "../header/Angel.h"
#include "CShape.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define SOLIDCUBE_NUM 36  // 6 faces, 2 triangles/face , 3 vertices/triangle

class CSolidCube : public CShape
{
private:
	vec4 _vertices[8];
	int  _iIndex;

	void Quad( int a, int b, int c, int d );
public:
	CSolidCube();
	~CSolidCube(){};

	void update(float dt, const LightSource &lights);
	void update(float dt); // 不計算光源的照明

	void draw();
	void drawW();

	//void renderWithFlatShading(const LightSource &lights);
	//void renderWithGouraudShading(const LightSource &lights);
};

#endif