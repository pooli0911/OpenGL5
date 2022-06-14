// Phong reflection model
#version 150
in vec4 vPosition;	// Vertex Position
in vec3 vNormal;    // Vertex Normal
in vec4 vVtxColor;  // Vertex Color
in vec2 vDiffuseMapCoord;
in vec2 vLightMapCoord;
in vec2 vNormalMapCoord;
in vec3 vTangentV;

out vec3 v3N; // ��X Normal �b Tangent Space �y�ФU����V
out vec3 v3L; // ��X Light Direction �b Tangent Space �y�ФU����V
out vec3 v3E; // ��X View Direction �b Tangent Space �y�ФU����V
out vec2 DiffuseMapUV;  // ��X�K�Ϯy��
out vec2 LightMapUV;    // ��X�K�Ϯy��
out vec2 NormalMapUV;  // ��X�K�Ϯy��

uniform mat4  ModelView;   // Model View Matrix
uniform mat4  Projection;  // Projection Matrix
uniform vec4  LightInView; // Light's position in View Space

void main()
{
	// Vertex �ഫ�� World Space
	vec4 vPosInView = ModelView * vPosition;

	// Building the matrix Eye Space -> Tangent Space
	// �H�U���Ω�p��磌��i�� non-uniform scale �ɡA���� normal ���ץ��p��
	mat3 ITModelView = transpose(inverse(mat3(ModelView))); 
	//	vec3 vN = normalize(ITModelView * vNormal); 

    // �p�� vertex normal �P vTangent �ഫ�쥿�T�� object local space �W
	vec3 vn = normalize(ITModelView * vNormal);
	vec3 vt = normalize(ITModelView * vTangentV);
	vec3 vb = cross(vn, vt);

	// �N Light Vector �ഫ�� Tangent Space
	vec3 tmpV = vec3(LightInView.xyz - vPosInView.xyz);
	v3L.x = dot(tmpV, vt);
	v3L.y = dot(tmpV, vb);
	v3L.z = dot(tmpV, vn);

	// �N Eye Vector �ഫ�� Tangent Space
	tmpV = -vPosInView.xyz;
	v3E.x = dot(tmpV, vt);
	v3E.y = dot(tmpV, vb);
	v3E.z = dot(tmpV, vn);

	gl_Position = Projection * vPosInView;
	DiffuseMapUV = vDiffuseMapCoord;
	LightMapUV  = vLightMapCoord;
	NormalMapUV = vNormalMapCoord;
}
