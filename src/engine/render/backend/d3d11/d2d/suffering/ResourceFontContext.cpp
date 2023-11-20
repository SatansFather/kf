#if !_SERVER

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
//----------------------------------------------------------------------------

#include "Common.h"
#include "ResourceFontContext.h"
#include "ResourceFontCollectionLoader.h"
#include "ResourceFontFileLoader.h"
#include "../../d3d11_interface.h"
#include "../d2d_interface.h"

ResourceFontContext::ResourceFontContext() : hr_(S_FALSE)
{
}

ResourceFontContext::~ResourceFontContext()
{


}

HRESULT ResourceFontContext::Initialize()
{
	if (hr_ == S_FALSE)
	{
		hr_ = InitializeInternal();
	}
	return hr_;
}

HRESULT ResourceFontContext::InitializeInternal()
{
	HRESULT hr = S_OK;

	if (!ResourceFontFileLoader::IsLoaderInitialized()
		||  !ResourceFontCollectionLoader::IsLoaderInitialized())
	{
		return E_FAIL;
	}

	// Register our custom loaders with the factory object.
	//
	// Note: For this application we just use the shared DWrite factory object which is accessed via 
	//       a global variable. If we were using fonts embedded in *documents* then it might make sense 
	//       to create an isolated factory for each document. When unloading the document, one would
	//       also release the isolated factory, thus ensuring that all cached font data specific to
	//       that document would be promptly disposed of.
	//
	if (FAILED(hr = GetD3D11Interface()->D2D->WriteFactory->RegisterFontFileLoader(ResourceFontFileLoader::GetLoader())))
		return hr;

	hr = GetD3D11Interface()->D2D->WriteFactory->RegisterFontCollectionLoader(ResourceFontCollectionLoader::GetLoader());

	return hr;
}

HRESULT ResourceFontContext::CreateFontCollection(
	UINT const* fontCollectionKey,  // [keySize] in bytes
	UINT32 keySize,
	OUT IDWriteFontCollection** result
)
{
	*result = NULL;

	HRESULT hr = S_OK;

	K_ASSERT_HR(Initialize(), "could not initialize font context");

	K_ASSERT_HR(GetD3D11Interface()->D2D->WriteFactory->CreateCustomFontCollection(
		ResourceFontCollectionLoader::GetLoader(),
		fontCollectionKey,
		keySize,
		result
	), "write factory could not create font collection");

	return hr;
}

#endif