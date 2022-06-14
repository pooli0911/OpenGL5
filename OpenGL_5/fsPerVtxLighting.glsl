#version 130
// iTexLayer ���]�w�P�P�_�覡�A���]���B�i�H�䴩�줻�i�K��
#define NONE_MAP    0
#define DIFFUSE_MAP 1
#define LIGHT_MAP   2

//#define LIGHTMAP_COLOR

in vec4 vColor;
in vec2 DiffuseMapUV;   // ��J Diffuse Map �K�Ϯy��
in vec2 LightMapUV;     // ��J Light Map �K�Ϯy��

uniform int iTexLayer;
uniform float fElapsedTime;

// For Texture Sampler
uniform sampler2D diffuMap; // �K�Ϫ��ѼƳ]�w
uniform sampler2D lightMap; // �K�Ϫ��ѼƳ]�w

void main()
{
#ifndef LIGHTMAP_COLOR
	if( iTexLayer == NONE_MAP ) gl_FragColor = vColor;
	else if( iTexLayer == DIFFUSE_MAP ) gl_FragColor = vColor * texture2D(diffuMap, DiffuseMapUV);
	else if( iTexLayer == (DIFFUSE_MAP|LIGHT_MAP)) gl_FragColor = (vColor * texture2D(diffuMap, DiffuseMapUV)) + (texture2D(diffuMap, DiffuseMapUV)* texture2D(lightMap, LightMapUV) );
#else
	
	if( iTexLayer == NONE_MAP ) gl_FragColor = vColor;
	else if( iTexLayer == DIFFUSE_MAP ) gl_FragColor = vColor * texture2D(diffuMap, DiffuseMapUV);
	else if( iTexLayer == (DIFFUSE_MAP|LIGHT_MAP) ) {
		gl_FragColor = (vColor * texture2D(diffuMap, DiffuseMapUV)) + (texture2D(diffuMap, DiffuseMapUV) * texture2D(lightMap, LightMapUV) * vec4(0.0f, 0.05f, 0.55f, 1.0));
	}
#endif
}
