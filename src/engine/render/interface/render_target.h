#pragma once

#if !_SERVER

#include "engine/global/types_numeric.h"
#include "engine/render/color.h"
#include <memory>
#include "../shader_stage.h"
#include "uav.h"

enum class ERenderTargetFlags : u16
{
	BackBuffer			= 1, // this is the back buffer for our window
	Staging				= 2, // allows data to be read back on the CPU
	Dynamic				= 4, 
	Color32Bit			= 8, // uses 32 bit colors instead of 8 bit
	UnorderedAccess		= 16,
	SingleFloat			= 32,
	DoubleFloat			= 64,
	SignedNorm          = 128
};

struct FRenderTargetCreationFlags
{
private:

	u8 Flags = 0;

public:

	void EnableBackBuffer()			{ Flags |= (u8)ERenderTargetFlags::BackBuffer; }
	void EnableStaging()			{ Flags |= (u8)ERenderTargetFlags::Staging; }
	void EnableDynamic()			{ Flags |= (u8)ERenderTargetFlags::Dynamic; }
	void Enable32Bit()				{ Flags |= (u8)ERenderTargetFlags::Color32Bit; }
	void EnableUnorderedAccess()	{ Flags |= (u8)ERenderTargetFlags::UnorderedAccess; }
	void EnableSingleFloat()		{ Flags |= (u8)ERenderTargetFlags::SingleFloat;	}
	void EnableDoubleFloat()		{ Flags |= (u8)ERenderTargetFlags::DoubleFloat;	}
	void EnableSignedNorm()		    { Flags |= (u8)ERenderTargetFlags::SignedNorm;	}

	bool IsBackBuffer() const		{ return Flags & (u8)ERenderTargetFlags::BackBuffer; }
	bool IsStaging() const			{ return Flags & (u8)ERenderTargetFlags::Staging; }
	bool IsDynamic()	const		{ return Flags & (u8)ERenderTargetFlags::Dynamic; }
	bool Is32Bit() const			{ return Flags & (u8)ERenderTargetFlags::Color32Bit; }
	bool IsUnorderedAccess() const	{ return Flags & (u8)ERenderTargetFlags::UnorderedAccess; }
	bool IsSingleFloat() const		{ return Flags & (u8)ERenderTargetFlags::SingleFloat; }
	bool IsDoubleFloat() const		{ return Flags & (u8)ERenderTargetFlags::DoubleFloat; }
	bool IsSignedNorm() const		{ return Flags & (u8)ERenderTargetFlags::SignedNorm; }
};

class KRenderTarget : public KUnorderedAccessView
{
protected:

	FRenderTargetCreationFlags CreationFlags;
	u32 Width = 0;
	u32 Height = 0;

	FColor32 PreferredClearColor;

public:

	virtual ~KRenderTarget() = default;

	/* virtual interface */
	virtual void Create(u32 w, u32 h, FRenderTargetCreationFlags flags) = 0;

	// clears this render target to a color
	virtual void Clear(f32 r = .1, f32 g = .1, f32 b = .1, f32 a = 1) = 0;

	// returns the KMappedRender class below for CPU read
	virtual UPtr<class KMappedRender> GetMapped() = 0;

	virtual void BindTexture2D(u8 slot, EShaderStage stage = EShaderStage::Pixel) = 0;

	inline u32 GetWidth() { return Width; }
	inline u32 GetHeight() { return Height; }
	void Screenshot();
	void ClearToPreferred();
	void SetPreferredClearColor(FColor32 color) { PreferredClearColor = color; }
};

// utility class to make it easy to read back a render target on CPU
class KMappedRender
{
	u32 Width = 0;
	u32 Height = 0;

protected:

	UPtr<u8[]> Data;
	u32 RowPitch, DepthPitch;

public:

	KMappedRender(UPtr<u8[]> data, u32 row, u32 depth, u32 w, u32 h) : 
		Data(std::move(data)), 
		RowPitch(row), 
		DepthPitch(depth), 
		Width(w),
		Height(h)
	{}

	virtual ~KMappedRender();

	/* virtual interface */
	virtual FColor8 GetAt(u32 x, u32 y) const = 0;

	UPtr<class KSurface2D> ToSurface();
	u8* GetRaw() { return Data.get(); }
	inline u32 GetHeight() const { return Height; }
	inline u32 GetWidth() const { return Width; }

};

#endif