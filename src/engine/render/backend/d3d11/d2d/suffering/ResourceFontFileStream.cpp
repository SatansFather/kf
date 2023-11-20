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
#include "ResourceFontFileStream.h"
#include "../../d3d11_interface.h"
#include "../d2d_interface.h"
#include "engine/kwad/kwad_chunk_font.h"

HMODULE const ResourceFontFileStream::moduleHandle_(GetCurrentModule());

// GetCurrentModule
//
//      Helper to get the module handle for the application.
//
HMODULE ResourceFontFileStream::GetCurrentModule()
{
	HMODULE handle = NULL;

	// Determine the module handle from the address of this function.
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, 
		reinterpret_cast<LPCTSTR>(&GetCurrentModule), 
		&handle
	);

	return handle;
}

/*#include <fstream>
TVector<char> FileBuffer;*/
ResourceFontFileStream::ResourceFontFileStream(UINT resourceID) :
	refCount_(0),
	resourcePtr_(NULL),
	resourceSize_(0)
{
	/*std::ifstream file("RedOctober.ttf", std::ios::in | std::ios::binary | std::ios::ate);

	if (!file.is_open())
	{
		MessageBox(NULL, L"file not found", L"", MB_OK);
		exit(0);
	}

	// get size and reset cursor
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	// copy file to buffer
	FileBuffer.clear();
	FileBuffer.resize((size_t)size);
	file.read(FileBuffer.data(), size);

	file.close();

	resourcePtr_ = FileBuffer.data();
	resourceSize_ = FileBuffer.size();*/

	KWadChunk_Font* chunk = GetD3D11Interface()->D2D->FontChunks[resourceID];
	resourcePtr_ = chunk->Data.data();
	resourceSize_ = chunk->Data.size();
}

// IUnknown methods
HRESULT STDMETHODCALLTYPE ResourceFontFileStream::QueryInterface(REFIID iid, void** ppvObject)
{
	if (iid == IID_IUnknown || iid == __uuidof(IDWriteFontFileStream))
	{
		*ppvObject = this;
		AddRef();
		return S_OK;
	}
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}
}

ULONG STDMETHODCALLTYPE ResourceFontFileStream::AddRef()
{
	return InterlockedIncrement(&refCount_);
}

ULONG STDMETHODCALLTYPE ResourceFontFileStream::Release()
{
	ULONG newCount = InterlockedDecrement(&refCount_);
	if (newCount == 0)
		delete this;

	return newCount;
}

// IDWriteFontFileStream methods
HRESULT STDMETHODCALLTYPE ResourceFontFileStream::ReadFileFragment(
	void const** fragmentStart, // [fragmentSize] in bytes
	UINT64 fileOffset,
	UINT64 fragmentSize,
	OUT void** fragmentContext
)
{
	// The loader is responsible for doing a bounds check.
	if (fileOffset <= resourceSize_ && 
		fragmentSize <= resourceSize_ - fileOffset)
	{
		*fragmentStart = static_cast<BYTE const*>(resourcePtr_) + static_cast<size_t>(fileOffset);
		*fragmentContext = NULL;
		return S_OK;
	}
	else
	{
		*fragmentStart = NULL;
		*fragmentContext = NULL;
		return E_FAIL;
	}
}

void STDMETHODCALLTYPE ResourceFontFileStream::ReleaseFileFragment(
	void* fragmentContext
)
{
}

HRESULT STDMETHODCALLTYPE ResourceFontFileStream::GetFileSize(
	OUT UINT64* fileSize
)
{
	*fileSize = resourceSize_;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE ResourceFontFileStream::GetLastWriteTime(
	OUT UINT64* lastWriteTime
)
{
	// The concept of last write time does not apply to this loader.
	*lastWriteTime = 0;
	return E_NOTIMPL;
}

#endif 