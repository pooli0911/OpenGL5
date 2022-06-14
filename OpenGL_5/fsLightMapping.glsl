// #define CHANGE_COLOR
#version 130
in vec3 fN;
in vec3 fL;
in vec3 fE;

in vec2 DfMapUV; // ��J�K�Ϯy��
in vec2 LgMapUV; // ��J�K�Ϯy��

// �H�U���s�W������
uniform vec4  LightInView;        // Light's position in View Space
uniform vec4  AmbientProduct;  // light's ambient  �P Object's ambient  �P ka �����n
uniform vec4  DiffuseProduct;  // light's diffuse  �P Object's diffuse  �P kd �����n
uniform vec4  SpecularProduct; // light's specular �P Object's specular �P ks �����n
uniform float fShininess;
uniform int   iLighting;
uniform vec4  vObjectColor;    // �N���󪺳�@�C��

// For Texture Sampler
uniform sampler2D   diffuMap; // �K�Ϫ��ѼƳ]�w
uniform sampler2D   lightMap; // �K�Ϫ��ѼƳ]�w
uniform samplerCube cubeMap; // �K�Ϫ��ѼƳ]�w

void main()
{
	// ���ŧi diffuse �P specular
    vec4 diffuse = vec4(0.0,0.0,0.0,1.0);
	vec4 specular = vec4(0.0,0.0,0.0,1.0);
	vec4 LightingColor = vec4(0.0,0.0,0.0,1.0);
	
	if( iLighting != 1 ) {
#ifndef CHANGE_COLOR
		// �����ϥζǤJ�� LightMap
		gl_FragColor = vObjectColor * texture2D(diffuMap, DfMapUV) * texture2D(lightMap, LgMapUV);
#else
		vec4 lightMap = texture2D(lightMap, LgMapUV);	
		lightMap = lightMap * vec4(0.79,0.95, 0.96, 1.0);
		gl_FragColor = vObjectColor * texture2D(diffuMap, DfMapUV) * lightMap;
#endif
		gl_FragColor.w = 1.0f;
	}
	else {	
		// 1. �p�� Ambient color : Ia = AmbientProduct = Ka * Material.ambient * La = 
		vec4 ambient = AmbientProduct; // m_sMaterial.ka * m_sMaterial.ambient * vLightI;

		// ���ƶǤJ�� Normal Dir
		vec3 vN = normalize(fN); 

		// 2. ���ƶǤJ�� Light Dir
		vec3 vL = normalize(fL); // normalize light vector

		// 5. �p�� L dot N
		float fLdotN = vL.x*vN.x + vL.y*vN.y + vL.z*vN.z;
		if( fLdotN >= 0 ) { // ���I�Q�����Ө�~�ݭn�p��
			// Diffuse Color : Id = Kd * Material.diffuse * Ld * (L dot N)
			diffuse = fLdotN * DiffuseProduct; 

			// Specular color
			// Method 1: Phone Model
			// �p�� View Vector
			// ���ƶǤJ�� fE , View Direction
			vec3 vV = normalize(fE);

			//�p�� Light �� �Ϯg�� vRefL = 2 * fLdotN * vN - L
			// �P�ɧQ�� normalize �ন���V�q
			vec3 vRefL = normalize(2.0f * (fLdotN) * vN - vL);

			//   �p��  vReflectedL dot View
			float RdotV = vRefL.x*vV.x + vRefL.y*vV.y + vRefL.z*vV.z;

			// Specular Color : Is = Ks * Material.specular * Ls * (R dot V)^Shininess;
			if( RdotV > 0 ) specular = SpecularProduct * pow(RdotV, fShininess); 
		}

		LightingColor = ambient + diffuse + specular;  // �p���C�� ambient + diffuse + specular;
		LightingColor.w = 1.0;	// �]�w alpha �� 1.0
//		gl_FragColor = LightingColor;
		gl_FragColor = LightingColor * texture2D(diffuMap, DfMapUV);

		// gl_FragColor = vec4((ambient + diffuse + specular).xyz, 1.0);

	}
}
