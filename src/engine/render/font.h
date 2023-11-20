#if 0
#pragma once

#if !_SERVER

#include "kfglobal.h"

class KFont
{
	KString FontName;
	u32 Size = 0;
	struct FT_FaceRec_* FtFace = nullptr;


public:

	UPtr<class KTexture2D> Texture;
	~KFont();

	static void InitFreetype();
	
	void LoadFromFile(const KString& file, u32 fontSize, u32 index = 0);
	void LoadFromMemory(u8* buffer, u32 bufferSize, u32 fontSize, u32 index = 0);

private:
	void FinishInit();



};

#endif
#endif