#ifndef CSHADERPOOL_H
#define CSHADERPOOL_H

#define CC_SAFE_DELETE(p)           do { delete (p); (p) = nullptr; } while(0)
#define CC_SAFE_DELETE_ARRAY(p)     do { if(p) { delete[] (p); (p) = nullptr; } } while(0)

#include "../header/Angel.h"

#define	SHADERPOOL_MAX 100

typedef struct structShaderElement{
	GLuint  uiShaderID;
	char   vsName[80];
	char   fsName[80];
} ShaderElement;

class CShaderPool
{
	int _iNumShaders;
	ShaderElement _Pool[SHADERPOOL_MAX];		// �w�]���x�s 100 �� shader
public:
	static CShaderPool* create();		// �إ� CShaderPool ������
	static CShaderPool *getInstance();	// ���o  CShaderPool ������

	void destroyInstance();		// ������o�귽
	GLuint addShader(char *vsName, char* fsName);
	GLuint getShaderID(char* vsName, char* fsName);

protected:
	CShaderPool();			
	~CShaderPool();
	void initDefault(){};
	static CShaderPool* _pShaderPool;
};

#endif

