#if !_SERVER

#if _WIN32
#include "engine/os/windows/gdi.h"
#else

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#endif

#include "surface2d.h"
#include "engine/utility/kstring.h"
#include "engine/utility/k_assert.h"
#include "engine/global/paths.h"
#include "engine/utility/thread_pool.h"

// DELETE
#include "../system/terminal/terminal.h"

#if !_DEV
#include "../kwad/kwad_chunk_image.h"
#include "../game_instance.h"
#include "../kwad/kwad_file.h"
#include "../kwad/kwad_reader.h"
#endif

void KSurface2D::LoadPNG(const KString& name, ESurfacePadMethod pad)
{
	K_ASSERT(!Data.get(), "cannot load an image into existing texture data");

#if !_DEV && !_COMPILER
	LoadFromWad(name);
	return;
#endif

	PadMethod = pad;
	Name = name;
	
#if _WIN32
	KGdiBitmap bitmap = CreateGdiBitmap(name);
	
	Width = bitmap->GetWidth();
	Height = bitmap->GetHeight();
	Data = std::make_unique<FColor8[]>(Width * Height);
	
	for (u32 y = 0; y < Height; y++)
	{
		for (u32 x = 0; x < Width; x++)
		{
			Gdiplus::Color c;
			bitmap->GetPixel(x, y, &c);
			Data[y * Width + x] = c.GetValue();
			if (c.GetAlpha() == 0) bHasTransparency = true;
		}
	}
#else
	i32 x, y, c; // stbi_load insists on signed ints
	Data = UPtr<u8>(stbi_load(TEX_DIR + name + ".png", &x, &y, &c, STBI_rgb_alpha));

	Width = x;
	Height = y;
#endif

	// i'd like to set a flag for transparency as soon as stb loads it
	// but the code is impossible to read so for now this works
	/*if (c == 4)
	{
		const u32 pixels = x * y;
		std::atomic<bool> found = { false };
		const auto CheckTransparency = [&](u32 start, u32 end) -> void
		{
			if (found) return;
			for (u32 i = start; i < end; i++)
			{
				FColor8* p = Data.get() + i;
				if (p->GetA() < 255)
				{
					found = true;
					break;
				}
			}
		};

		KThreadPool::Iterate(CheckTransparency, 0, pixels);
		bHasTransparency = found;
	}*/

	if (PadMethod != ESurfacePadMethod::NoPadding)
	{
		//PadMethod = ESurfacePadMethod::Repeat;

		// create another surface to temporarily store the data
		UPtr<KSurface2D> temp = CopySurface();
		temp->PadMethod = ESurfacePadMethod::NoPadding;

		ClearAndResizeData(Width, Height, PadMethod);

		//for (u32 i = 1; i < Width - 1; i++)
		//  for (u32 j = 1; j < Height - 1; j++)
		//	SetPixel(i, j, temp->GetPixel(i-1, j-1));

		for (u32 i = 0; i < Width - 2; i++)
		  for (u32 j = 0; j < Height - 2; j++)
			SetPixel(i, j, temp->GetPixel(i, j));
	}
}

#if !_DEV
void KSurface2D::LoadFromWad(const KString& name)
{
	KWadReader<KWadChunk_Image> reader("kfdata", name);
	KWadChunk_Image* image = reader.GetChunk();

	Name = name;
	Width = image->Width;
	Height = image->Height;
	DisplayWidth = image->DisplayWidth;
	DisplayHeight = image->DisplayHeight;
	bHasTransparency = image->bHasTransparency;
	PadMethod = (ESurfacePadMethod)image->PadMethod;

	Data = std::make_unique<FColor8[]>(Width * Height);
	memcpy(Data.get(), image->Data.get(), Width * Height * 4);
}
#endif

/*
void KSurface2D::CreateFromColorArray(u8* colors, u32 w, u32 h, ESurfacePadMethod pad)
{
	Data = UPtr<u8>(colors);
	Width = w;
	Height = h;
	ChannelCount = 4;
}

void KSurface2D::CreateFromColorArray(UPtr<u8>& colors, u32 w, u32 h, ESurfacePadMethod pad)
{
	Data = std::move(colors);
	Width = w;
	Height = h;
	ChannelCount = 4;
}*/

UPtr<KSurface2D> KSurface2D::CopySurface() const
{
	UPtr<KSurface2D> copy = std::make_unique<KSurface2D>();

	copy->Width = Width;
	copy->Height = Height;
	copy->bHasTransparency = bHasTransparency;
	copy->PadMethod = PadMethod;

	copy->Data = std::make_unique<FColor8[]>(Width * Height);
	memcpy(copy->Data.get(), Data.get(), Width * Height * 4);

	return copy;
}

FColor8* KSurface2D::GetPixelPointer(u32 x, u32 y) const
{
	K_ASSERT(Data.get(), "cannot get pixel offset from null data");
	K_ASSERT(Width > x && Height > y, "cannot get pixel out of bounds");

	return Data.get() + (y * Width + x);
}

KSurface2D::KSurface2D(u32 w, u32 h, ESurfacePadMethod pad)
{
	Width = w;
	Height = h;

	if (pad != ESurfacePadMethod::NoPadding)
	{
		w += h;
		h += 2;
	}

	Data = std::make_unique<FColor8[]>(w * h);
}

KSurface2D::KSurface2D(u32 w, u32 h, UPtr<FColor8[]> buffer)
{
	Width = w;
	Height = h;
	Data = std::move(buffer);
}

void KSurface2D::ClearAndResizeData(u32 w, u32 h, ESurfacePadMethod pad)
{
	if (PadMethod != ESurfacePadMethod::NoPadding)
	{
		w += 2;
		h += 2;
	}

	Data.reset();
	Width = w;
	Height = h;
	Data = std::make_unique<FColor8[]>(Width * Height);
}

u32 KSurface2D::GetContentWidth() const
{
	return Width - (PadMethod == ESurfacePadMethod::NoPadding ? 0 : 2);
}

u32 KSurface2D::GetContentHeight() const
{
	return Height - (PadMethod == ESurfacePadMethod::NoPadding ? 0 : 2);
}

FColor8 KSurface2D::GetPixel(u32 x, u32 y) const
{
	if (PadMethod != ESurfacePadMethod::NoPadding)
	{
		x++;
		y++;
	}

	return *GetPixelPointer(x, y);
}

FColor8 KSurface2D::GetPaddedPixel(u32 x, u32 y) const
{
	return *GetPixelPointer(x, y);
}

void KSurface2D::SetPixel(u32 x, u32 y, u8 r, u8 g, u8 b, u8 a)
{
	SetPixel(x, y, FColor8(r, g, b, a));
}

void KSurface2D::SetPixel(u32 x, u32 y, FColor8 color)
{
	if (PadMethod != ESurfacePadMethod::NoPadding)
	{
		x++;
		y++;
	}

	FColor8* pixel = GetPixelPointer(x, y);

	if (color.GetA() < 255) bHasTransparency = true;

	*pixel = color;

	if (x == 1 || x == Width - 2 || y == 1 || y == Height - 2)
	{
		TVector<FColor8*> pixels;
		switch (PadMethod)
		{
			case ESurfacePadMethod::Repeat:
			{
				if (x == 1)			pixels.push_back(GetPixelPointer(Width - 1, y));
				if (x == Width - 2) pixels.push_back(GetPixelPointer(0, y));

				if (y == 1)			 pixels.push_back(GetPixelPointer(x, Height - 1));
				if (y == Height - 2) pixels.push_back(GetPixelPointer(x, 0));

				if (x == 1 && y == 1)				   pixels.push_back(GetPixelPointer(Width - 1, Height - 1));
				if (x == Width - 2 && y == Height - 2) pixels.push_back(GetPixelPointer(0, 0));
				if (x == Width - 2 && y == 1)		   pixels.push_back(GetPixelPointer(0, Height - 1));
				if (x == 1 && y == Height - 2)		   pixels.push_back(GetPixelPointer(Width - 1, 0));

				break;
			}
			case ESurfacePadMethod::Clamp:
			{
				if (x == 1)			pixels.push_back(GetPixelPointer(0, y));
				if (x == Width - 2) pixels.push_back(GetPixelPointer(Width - 1, y));

				if (y == 1)			 pixels.push_back(GetPixelPointer(x, 0));
				if (y == Height - 2) pixels.push_back(GetPixelPointer(x, Height - 1));

				if (x == 1 && y == 1) pixels.push_back(GetPixelPointer(0, 0));
				if (x == Width - 2 && y == Height - 2) pixels.push_back(GetPixelPointer(Width - 1, Height - 1));
				if (x == Width - 2 && y == 1)  pixels.push_back(GetPixelPointer(Width - 1, 0));
				if (x == 1 && y == Height - 2) pixels.push_back(GetPixelPointer(0, Height - 1));

				break;
			}
		}
		for (FColor8* p : pixels)
		{
			*p = color;
		}
	}
}

void KSurface2D::SavePNG() const
{
	K_ASSERT(Data.get(), "cannot save png from null image data");
	
#ifdef WIN32
	CreateDirectory(KString(SCREENSHOT_DIR).ToWideStr().c_str(), NULL);
#endif

	KString file = SCREENSHOT_DIR + "screenshot-" + KString::GetTimeDateString() + ".png";

	auto GetEncoderClsid = [&file](const WCHAR* format, CLSID* pClsid) -> void
	{
		UINT  num = 0;          // number of image encoders
		UINT  size = 0;         // size of the image encoder array in bytes

		Gdiplus::ImageCodecInfo* pImageCodecInfo = nullptr;

		Gdiplus::GetImageEncodersSize(&num, &size);
		pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
		GetImageEncoders(num, size, pImageCodecInfo);
		for (UINT j = 0; j < num; ++j)
		{
			if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
			{
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return;
			}
		}

		free(pImageCodecInfo);
	};

	CLSID bmpID;
	GetEncoderClsid(L"image/png", &bmpID);

	// convert filename to wide string (for Gdiplus)
	wchar_t wideName[512];
	mbstowcs_s(nullptr, wideName, file.CStr(), _TRUNCATE);

	Gdiplus::Bitmap bitmap(Width, Height, Width * sizeof(FColor8), PixelFormat32bppARGB, (BYTE*)Data.get());
	Gdiplus::Status s = bitmap.Save(wideName, &bmpID, nullptr);

	/*if (s != Gdiplus::Status::Ok)
	{
		std::stringstream ss;
		ss << "Saving surface to [" << filename << "]: failed to save: " << STR(s);
		LOG_ERROR(ss.str());
	}*/
}

#endif
