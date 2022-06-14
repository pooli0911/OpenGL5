#include "CChecker.h"



CChecker::~CChecker()
{
	delete [] _pSquares;
	delete [] _pfSquaresT;
}

CChecker::CChecker(int iSize, float fYPos)
{
	// �إ߹���
	_iGridSize = iSize;
	_pSquares = new CQuad[_iGridSize*_iGridSize];
	_pfSquaresT = new float[_iGridSize*_iGridSize*3];
	_fYPos = fYPos;
	_iMode = ShadingMode::FLAT_SHADING_CPU;
}

void CChecker::setShader()
{
	//// ���ͨC�@�� Square ����m�P�y��
	mat4 mxT;
	int idx = 0; // 
	vec4 vT, vColor;
	for( int i = 0 ; i < _iGridSize ; i++ ) {
		for( int j = 0 ; j < _iGridSize ; j++ ) {
			_pfSquaresT[idx*3+2] = -_iGridSize/2 + i + 0.5f;
			_pfSquaresT[idx*3+0] = -_iGridSize/2 + j + 0.5f;
			_pfSquaresT[idx*3+1] = _fYPos;
			if( i + j == 0 ) { // �Ĥ@�ӡA �ݭn���w Shader
				_pSquares[idx].setShader();
				_uiShaderHandle = _pSquares[idx].getShaderHandle();
				vColor.x = 0.3f; vColor.y = 0.6f; vColor.z = 0.6f; vColor.w = 1.0f;
				_pSquares[idx].setColor(vColor);
			}
			else { // ��L�� 
				_pSquares[idx].setShader(_uiShaderHandle);
				if( (i+j)%2 ) {
					vColor.x = 0.2f; vColor.y = 0.2f; vColor.z = 0.2f; vColor.w = 1.0f;
					_pSquares[idx].setColor(vColor);
				}
				else {
					vColor.x = 0.3f; vColor.y = 0.6f; vColor.z = 0.6f; vColor.w = 1.0f;
					_pSquares[idx].setColor(vColor);
				}
			}
			mxT = Translate(_pfSquaresT[idx*3+0],_pfSquaresT[idx*3+1],_pfSquaresT[idx*3+2]);	
			_pSquares[idx].setTRSMatrix(mxT);
			//_pSquares[idx].setShadingMode(_iMode);
			idx++;
		}
	}
}

void CChecker::setShadingMode(ShadingMode iMode) {
	int idx = 0;
	_iMode = iMode;
	for (int i = 0; i < _iGridSize; i++)
		for (int j = 0; j < _iGridSize; j++) {
			_pSquares[idx].setShadingMode(iMode);
			idx++;
		}
}

void CChecker::setProjectionMatrix(mat4 &mat)
{
	for( int i = 0 ; i < _iGridSize*_iGridSize ; i++ )  {
		_pSquares[i].setProjectionMatrix(mat);
	}
}

void CChecker::setTRSMatrix(mat4 &mat)
{
	mat4 mxT;
	// �����[�W�C�@�Ӥ�楻�����첾
	for( int i = 0 ; i < _iGridSize*_iGridSize ; i++ )  {
		mxT = Translate(_pfSquaresT[i*3+0],_pfSquaresT[i*3+1],_pfSquaresT[i*3+2]);	
		_pSquares[i].setTRSMatrix(mat*mxT);
	}
}

void CChecker::setTextureLayer(TextureMode texlayer)
{
	for (int i = 0; i < _iGridSize*_iGridSize; i++)  {
		_pSquares[i].setTextureLayer(texlayer);
	}
}

void CChecker::draw()
{
	_pSquares[0].draw();
	for( int i = 1 ; i < _iGridSize*_iGridSize ; i++ )  {
		_pSquares[i].drawW();
	}
}

void CChecker::setViewMatrix(mat4 &mat)
{
	for( int i = 0 ; i < _iGridSize*_iGridSize ; i++ )  {
		_pSquares[i].setViewMatrix(mat);
	}
}

void CChecker::update(float dt, const LightSource &lights)
{
	for( int i = 0 ; i < _iGridSize*_iGridSize ; i++ )  {
		_pSquares[i].update(dt, lights);
	}
}

void CChecker::setMaterials(color4 ambient, color4 diffuse, color4 specular)
{
	for( int i = 0 ; i < _iGridSize*_iGridSize ; i++ )  {
		_pSquares[i].setMaterials(ambient, diffuse, specular);
	}
}
void CChecker::setKaKdKsShini(float ka, float kd, float ks, float shininess) // ka kd ks shininess
{
	for( int i = 0 ; i < _iGridSize*_iGridSize ; i++ )  {
		_pSquares[i].setKaKdKsShini(ka, kd, ks, shininess);
	}
}

//void CChecker::setShaderName(const char *vxShader, const char *fsShader)
//{
//	int idx = 0; 
//	for (int i = 0; i < _iGridSize; i++) {
//		for (int j = 0; j < _iGridSize; j++) {
//			_pSquares[idx].setShaderName(vxShader, fsShader);
//			idx++;
//		}
//	}
//}