#if !_SERVER

#include "d3d11_bitmap.h"
#include "d3d11_interface.h"

f32 KHudBitmap_D3D11::GetWidth()
{
	return Bitmap->GetSize().width;
}

f32 KHudBitmap_D3D11::GetHeight()
{
	return Bitmap->GetSize().height;
}

void KHudBitmap_D3D11::UpdateScale(f32 w, f32 h)
{
	//if (LastDrawH != h)
	if (LastDrawH != GetViewportY())
	{
		if (h > 1)
		{
			ComPtr<IWICBitmapScaler> scaler;
			GetD3D11Interface()->CreateBitmapScaler(scaler);
			scaler->Initialize(Source.Get(), w, h, WICBitmapInterpolationModeFant);
			GetD3D11Interface()->CreateBitmapFromWicBitmap(scaler.Get(), this);
		}
		LastDrawW = w;
		//LastDrawH = h;
		LastDrawH = GetViewportY();
	}
}

#endif