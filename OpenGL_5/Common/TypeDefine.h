#ifndef TYPEDEFINE_H
#define TYPEDEFINE_H
#include "../Header/Angel.h"

//#define NONE_MAP    0x0
//#define DIFFUSE_MAP 0x1
//#define LIGHT_MAP   0x2
//#define NORMAL_MAP  0x4

// 
// bit 2 : Normal Map
// iTexLayer ���]�w�P�P�_�覡�A���]���B�i�H�䴩�줻�i�K��
// bit 0 : diffuce Map
// bit 1 : Light Map
// bit 2 : 
// 
//
//


typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

typedef struct MaterialParameters {
	color4 ambient;		// ambient �C��
	color4 diffuse;		
	color4 specular;
	color4 emission;
	float  ka, kd, ks;	// ²�ƭp��, ���] RGB �T���C��W���Ϯg�Y�ƬҬۦP
	float  shininess;
} Material;

enum class LightType
{
    OMNI_LIGHT = 1,
    SPOT_LIGHT = 2
};

typedef struct LightSourceParameters
{
	color4 ambient;
	color4 diffuse;
	color4 specular;
	point4 position;
	point4 halfVector;
	vec3 spotTarget;
	vec3 spotDirection;
	float spotExponent;
	float spotCutoff; // (range: [0.0,90.0], 180.0)  
	float spotCosCutoff; // (range: [1.0,0.0],-1.0)  
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;

	LightType type;

	void UpdateDirection() {
		spotDirection.x = spotTarget.x - position.x;
		spotDirection.y = spotTarget.y - position.y;
		spotDirection.z = spotTarget.z - position.z;
		spotDirection = normalize(spotDirection);
	}
} LightSource;


#endif