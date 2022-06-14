#ifndef CTEXTUREPOOL_H
#define CTEXTUREPOOL_H

#define CC_SAFE_DELETE(p)           do { delete (p); (p) = nullptr; } while(0)
#define CC_SAFE_DELETE_ARRAY(p)     do { if(p) { delete[] (p); (p) = nullptr; } } while(0)

#include "../header/Angel.h"

#define	TEXTUREPOOL_MAX 100

typedef struct structTextureElement{
	GLuint  uiTextureID;
	char   *TextureName;
	int		iTexWidth;
	int		iTexHeight;
} TextureElement;

class CTexturePool
{
	int _iNumTextures;
	TextureElement _Pool[TEXTUREPOOL_MAX];		// �w�]���x�s 100 �i�K��
public:
	static CTexturePool* create();		// �إ� CTexturePool ������
	static CTexturePool *getInstance();	// ���o  CTexturePool ������

	void destroyInstance();		// ������o�귽
	GLuint addTexture(char *texName);
	GLuint getTextureID(char* texName);

protected:
	CTexturePool();			// �קK�Q�~��
	~CTexturePool();
	void initDefault(){};
	static CTexturePool* _pTexturePool;
};

#endif

