#include "CSolidSphere.h"

CSolidSphere::CSolidSphere(const GLfloat fRadius, const int iSlices,const  int iStacks)
{
    GLfloat drho = (GLfloat)(3.141592653589) / (GLfloat) iStacks;  
    GLfloat dtheta = 2.0f * (GLfloat)(3.141592653589) / (GLfloat) iSlices;  
    GLfloat ds = 1.0f / (GLfloat) iSlices;  
    GLfloat dt = 1.0f / (GLfloat) iStacks;  
    GLfloat t = 1.0f;      
    GLfloat s = 0.0f;  
    GLint i, j;     // Looping variables  
	int idx = 0; // 儲存 vertex 順序的索引值

	_fRadius = fRadius;
	_iSlices = iSlices;
	_iStacks = iStacks;
	_iNumVtx = iStacks*(2*(iSlices+1));

	_pPoints = NULL; _pNormals = NULL; _pTex1 = NULL;

	_pPoints  = new vec4[_iNumVtx];
	_pNormals = new vec3[_iNumVtx];
	_pColors  = new vec4[_iNumVtx]; 
	_pTex1    = new vec2[_iNumVtx];

#if MULTITEXTURE >= LIGHT_MAP
	_pTex2 = new vec2[_iNumVtx];  // 產生 light map 所需的貼圖座標
#endif
#if MULTITEXTURE >= NORMAL_MAP
	_pTex3 = new vec2[_iNumVtx];	// 產生 normal map 所需的貼圖座標
	_pTangentV = new vec3[_iNumVtx];
#endif

	for (i = 0; i < iStacks; i++ ) {  
		GLfloat rho = (GLfloat)i * drho;  
		GLfloat srho = (GLfloat)(sin(rho));  
		GLfloat crho = (GLfloat)(cos(rho));  
		GLfloat srhodrho = (GLfloat)(sin(rho + drho));  
		GLfloat crhodrho = (GLfloat)(cos(rho + drho));  
		
		// Many sources of OpenGL sphere drawing code uses a triangle fan  
		// for the caps of the sphere. This however introduces texturing   
		// artifacts at the poles on some OpenGL implementations  
		s = 0.0f;  
		for ( j = 0; j <= iSlices; j++) {  
            GLfloat theta = (j == iSlices) ? 0.0f : j * dtheta;  
            GLfloat stheta = (GLfloat)(-sin(theta));  
            GLfloat ctheta = (GLfloat)(cos(theta));  
  
            GLfloat x = stheta * srho;  
            GLfloat y = ctheta * srho;  
            GLfloat z = crho;  
              
			_pPoints[idx].x = x * _fRadius;
			_pPoints[idx].y = y * _fRadius;
			_pPoints[idx].z = z * _fRadius;
			_pPoints[idx].w = 1;

			_pNormals[idx].x = x;
			_pNormals[idx].y = y;
			_pNormals[idx].z = z;

			_pTex1[idx].x = s;
			_pTex1[idx].y = t; // 設定貼圖座標
			idx++;

            x = stheta * srhodrho;  
            y = ctheta * srhodrho;  
            z = crhodrho;

			_pPoints[idx].x = x * _fRadius;
			_pPoints[idx].y = y * _fRadius;
			_pPoints[idx].z = z * _fRadius;
			_pPoints[idx].w = 1;

			_pNormals[idx].x = x;
			_pNormals[idx].y = y;
			_pNormals[idx].z = z;

			_pTex1[idx].x = s;
			_pTex1[idx].y = t - dt; // 設定貼圖座標
			idx++;
			s += ds; 
		}  
		t -= dt;  
	}  

	// 預設將所有的面都設定成灰色
	for( int i = 0 ; i < _iNumVtx ; i++ ) _pColors[i] = vec4(-1.0f,-1.0f,-1.0f,1.0f);

#ifdef PERVERTEX_LIGHTING
	setShaderName("vsPerVtxLighting.glsl", "fsPerVtxLighting.glsl");
#else
	setShaderName("vsPerPixelLighting.glsl", "fsPerPixelLighting.glsl");
#endif 

	// Create and initialize a buffer object ，將此部分的設定移入 SetShader 中
	// CreateBufferObject();

	// 設定材質
	setMaterials(vec4(0), vec4(0.5f, 0.5f, 0.5f, 1), vec4(1.0f, 1.0f, 1.0f, 1.0f));
	setKaKdKsShini(0, 0.8f, 0.2f, 1);
}

// 回家自己寫
void CSolidSphere::renderWithFlatShading(const LightSource &lights)
{


}

void CSolidSphere::renderWithGouraudShading(const LightSource &lights)
{
	// Method 1 : 對每一個 Vertex 都計算顏色
	 for (int i = 0; i < _iStacks; i++ ) {  
		 for( int j = 0 ; j < 2*(_iSlices+1) ; j++ ) {
			 _pColors[i * 2 * (_iSlices + 1) + j] = PhongReflectionModel(_pPoints[i * 2 * (_iSlices + 1) + j], _pNormals[i * 2 * (_iSlices + 1) + j], lights);
		 }
	 } 

	// Method 2 : 重疊的 Vertex 使用前一次計算的顏色
	 //先計算第一個 Stack 的顏色
	//for( int j = 0 ; j < 2*(_iSlices+1) ; j++ ) {
	//	_pColors[j] = PhongReflectionModel(_pPoints[j], _pNormals[j], vLightPos, vLightI);
	//}
	//// 後續 Stack 的 vertex 顏色，編號偶數(含 0) 使用前一個 stack 編號+1的 顏色
	//// 編號奇數就必須計算顏色
	//// 每一個 Slices 最後兩個 vertex 於開頭前兩個 vertex 重疊，所以使用該兩個 vertex 的顏色
	//for (int i = 1; i < _iStacks; i++ ) {  
	//	for( int j = 0 ; j < 2*(_iSlices+1) - 2 ; j++ ) {
	//		if( j%2 ) _pColors[i*2*(_iSlices+1)+j] = PhongReflectionModel(_pPoints[i*2*(_iSlices+1)+j], _pNormals[i*2*(_iSlices+1)+j], vLightPos, vLightI);
	//		else _pColors[i*2*(_iSlices+1)+j] =  _pColors[(i-1)*2*(_iSlices+1)+j+1];		
	//	}
	//	_pColors[(i+1)*2*(_iSlices+1)-2] = _pColors[i*2*(_iSlices+1)];
	//	_pColors[(i+1)*2*(_iSlices+1)-1] = _pColors[i*2*(_iSlices+1)+1];
	//}

	glBindBuffer( GL_ARRAY_BUFFER, _uiBuffer );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(vec4)*_iNumVtx+sizeof(vec3)*_iNumVtx, sizeof(vec4)*_iNumVtx, _pColors ); // vertcies' Color
}


void CSolidSphere::update(float dt, const LightSource &lights)
{
#ifdef LIGHTING_WITHCPU
	if( _bViewUpdated || _bTRSUpdated  ) { // Model View 的相關矩陣內容有更動
		_mxMVFinal = _mxView * _mxTRS;
		_mxMV3X3Final = mat3(
			_mxMVFinal._m[0].x,  _mxMVFinal._m[1].x, _mxMVFinal._m[2].x,
			_mxMVFinal._m[0].y,  _mxMVFinal._m[1].y, _mxMVFinal._m[2].y,
			_mxMVFinal._m[0].z,  _mxMVFinal._m[1].z, _mxMVFinal._m[2].z);
#ifdef GENERAL_CASE
		_mxITMV = InverseTransposeMatrix(_mxMVFinal); 
#endif
		_bViewUpdated = _bTRSUpdated = false;
	}
	if (_iMode == FLAT_SHADING)  RenderWithGouraudShading(lights);
	else RenderWithGouraudShading(lights);

#else // Lighting With GPU
	if (_bViewUpdated || _bTRSUpdated ) {
		_mxMVFinal = _mxView * _mxTRS;
		_bViewUpdated = _bTRSUpdated = false;
	}
	_vLightInView = _mxView * lights.position;		// 將 Light 轉換到鏡頭座標再傳入
	// 算出 AmbientProduct DiffuseProduct 與 SpecularProduct 的內容
	_AmbientProduct = _Material.ka * _Material.ambient  * lights.ambient;
	_AmbientProduct.w = _Material.ambient.w;
	_DiffuseProduct = _Material.kd * _Material.diffuse  * lights.diffuse;
	_DiffuseProduct.w = _Material.diffuse.w;
	_SpecularProduct = _Material.ks * _Material.specular * lights.specular;
	_SpecularProduct.w = _Material.specular.w;
#endif
}

void CSolidSphere::update(float dt)
{
	if (_bViewUpdated || _bTRSUpdated) { // Model View 的相關矩陣內容有更動
		_mxMVFinal = _mxView * _mxTRS;
		_mxITMV = InverseTransposeMatrix(_mxMVFinal); 
		_bViewUpdated = _bTRSUpdated = false;
	}
}

void CSolidSphere::draw()
{
	drawingSetShader();
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Change to wireframe mode
	for (int i = 0; i < _iStacks; i++ ) {  
		glDrawArrays( GL_TRIANGLE_STRIP, i*(2*(_iSlices+1)), 2*(_iSlices+1) );
	}
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Return to solid mode
}

void CSolidSphere::drawW()
{
	drawingWithoutSetShader();
	for (int i = 0; i < _iStacks; i++ ) {  
		glDrawArrays( GL_TRIANGLE_STRIP, i*(2*(_iSlices+1)), 2*(_iSlices+1) );
	}
}

CSolidSphere::~CSolidSphere()
{

}