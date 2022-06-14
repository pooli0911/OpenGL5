// Phong reflection model
#version 130
in vec4 vPosition;	// Vertex Position
in vec3 vNormal;    // Vertex Normal
in vec4 vVtxColor;  // Vertex Color
in vec2 vDiffuseMapCoord;
in vec2 vLightMapCoord;

out vec3 v3N; // ��X Normal �b���Y�y�ФU����V
out vec3 v3L; // ��X Light Direction �b���Y�y�ФU����V
out vec3 v3E; // ��X View Direction �b���Y�y�ФU����V
out vec2 DiffuseMapUV;  // ��X�K�Ϯy��
out vec2 LightMapUV;    // ��X�K�Ϯy��

uniform mat4  ModelView;   // Model View Matrix
uniform mat4  Projection;  // Projection Matrix
uniform vec4  LightInView; // Light's position in View Space

void main()
{
	vec4 vPosInView = ModelView * vPosition;
	// �ثe�w�g�H���Y�y�Ь���¦, �ҥH View ����m�N�O (0,0,0), �ҥH��m���I���N�O View Dir
	v3E = -vPosInView.xyz;

	// �H�U���Ω�p��磌��i�� non-uniform scale �ɡA���� normal ���ץ��p��
	//		mat3 ITModelView = transpose(inverse(mat3(ModelView)); 
	//		vec3 v3N = normalize(ITModelView * vNormal); 
	v3N = (ModelView * vec4(vNormal, 0.0)).xyz;

	v3L = vec3(LightInView.xyz - vPosInView.xyz);
	gl_Position = Projection * vPosInView;
	DiffuseMapUV = vDiffuseMapCoord;
	LightMapUV  = vLightMapCoord;
}
