#pragma once

#include <fstream>
#include "kfglobal.h"
#include "brush/brush.h"

class KMapEntity
{
	friend class KMapCompiler;
	friend class KMapFile;

private:


	DBoundingBox Bounds;

	f64 OriginX = 0, OriginY = 0, OriginZ = 0;
	f64 Angle = 0;

	KString Classname;

	u32 CompiledID = 0;

public:

	TMap<class KString, class KString> Properties;
	TVector<UPtr<KMapBrush>> Brushes;
	TVector<UPtr<KBrushFace>> Faces; // face that survived junk removal
	
public:

	KMapEntity();
	~KMapEntity();

	class KString GetProperty(class KString key) const;
	class KString GetName() const;

	void CreateBrushFaces(bool loadingMap = false);

	bool HasCollisionBrushes() const;
	bool HasRenderableBrushes() const;
	const TVector<UPtr<KMapBrush>>& GetBrushes() const { return Brushes; }

	const DBoundingBox& GetBounds() const { return Bounds; }
	DVec3 GetOrigin();

	void SetOrigin(const DVec3& o);

	u32 GetCollisionChannels() const;
	u32 GetCollisionPass() const;

	u32 GetCompiledID() const { return CompiledID; }
	f64 GetAngle() const { return Angle; }

	void Copy(KMapEntity& ent); // warning: moves brushes and faces out of this

#if _COMPILER

	// parses this entity starting from the cursor position in the file
	void ParseEntity(std::ifstream& file);

	// parses a property and adds it to the Properties map
	void ParseProperty(const class KString& line);

	void FinalizeParse();

	void MoveBrushesToVector(TVector<UPtr<KMapBrush>>& vector);

	void RemoveJunkFaces();

	bool ShouldBeCompiled() const;
	bool ShouldCompileBrushes() const;

private:

	void RemoveInsideFaces();
	void RemoveUnreachableFaces();
	void MergeBrushFaces();
#endif	
};
