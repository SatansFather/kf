#if 0
#include "font.h"
#include "engine/global/paths.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "interface/texture2d.h"
#include "interface/render_interface.h"

FT_Library FtLib;

KFont::~KFont() {}

void KFont::InitFreetype()
{
	FT_Init_FreeType(&FtLib);
}

void KFont::LoadFromFile(const KString& file, u32 fontSize, u32 index /*= 0*/)
{
	FT_Error error = FT_New_Face(FtLib, KString(FONT_DIR + file).CStr(), index, &FtFace);
	if (error == FT_Err_Unknown_File_Format)
	{
		LOG(file + " is not a valid font file", 1, 0, 0);
		return;
	}
	else if (error)
	{
		LOG("invalid file: " + file, 1, 0, 0);
		return;
	}
	Size = fontSize;

	FinishInit();
}

void KFont::LoadFromMemory(u8* buffer, u32 bufferSize, u32 fontSize, u32 index /*= 0*/)
{
	FT_New_Memory_Face(FtLib, buffer, bufferSize, index, &FtFace);
	Size = fontSize;

	FinishInit();
}

void KFont::FinishInit()
{

	// windows is 96 dpi by default
	FT_Set_Char_Size(FtFace, Size * 32, Size * 32, 96, 96);
	FT_Select_Charmap(FtFace, ft_encoding_unicode);

FTimePoint s = KTime::Now();
	for (u32 i = 0; i <= MAX_U8; i++)
	{
		FT_Load_Char(FtFace, 'G', FT_LOAD_RENDER);
		FT_GlyphSlot slot = FtFace->glyph;
		//Texture = GetRenderInterface()->CreateTexture2DFromFreetypeGlyph(slot);
	}
LOG(KTime::Since(s));

	/*for (u32 i = 0; i < FtFace->num_glyphs; i++)
	{
		
	}*/
}
#endif