#include "kwad_chunk_obj.h"

u32 KWadChunk_Obj::GetSize()
{
	return 8 + ObjSize + MtlSize;
}

void KWadChunk_Obj::WriteToBuffer(u8* buffer)
{
	memcpy(buffer, &ObjSize, 4);
	memcpy(buffer + 4, &MtlSize, 4);
	memcpy(buffer + 8, Obj.CStr(), ObjSize);
	memcpy(buffer + 8 + ObjSize, Mtl.CStr(), MtlSize);
}

void KWadChunk_Obj::ReadFromBuffer(u8* buffer, u32 size)
{
	memcpy(&ObjSize, buffer, 4);
	memcpy(&MtlSize, buffer + 4, 4);
	
	Obj.GetMut().resize(ObjSize, '\0');
	memcpy((void*)(Obj.Get().data()), buffer + 8, ObjSize);

	Mtl.GetMut().resize(MtlSize, '\0');
	memcpy((void*)(Mtl.Get().data()), buffer + 8 + ObjSize, MtlSize);
}
