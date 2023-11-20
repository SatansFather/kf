#pragma once

#include <string>
#include <stdexcept>
#include <algorithm>
#include <sstream>

#include "engine/global/types_numeric.h"
#include "engine/global/types_container.h"
#include "k_assert.h"
#include <ostream>
#include <iomanip>

#if _WIN32
// converting to wide
#include "engine/os/windows/windows.h"
#include <stringapiset.h>
#endif

// wrapper for std::string with convenience functions as members 
class KString
{	
private:

	std::string String;

public:

	/*************** constructors ***************/

	KString() = default;
	KString(std::string str);
	KString(std::string* str);
	KString(const char* str);
	KString(char* str);
	KString(const std::wstring& str);
	KString(u8 v);
	KString(i8 v);
	KString(u16 v);
	KString(i16 v);
	KString(u32 v);
	KString(i32 v);
	KString(u64 v);
	KString(i64 v);
	KString(const wchar_t* str);
	KString(f32 v, i32 decPrecision = -1);
	KString(f64 v, i32 decPrecision = -1);
	KString(f32 v, u32 fixedFrac);
	KString(f64 v, u32 fixedFrac);
	static KString FromChar(char c);

	/*************** standard methods ***************/

	u32 Size()		 const;
	u32 Length()	 const;
	u32 Capacity()	 const;
	u32 MaxSize()	 const;
	bool IsEmpty()	 const;														
	u32 Find(char c) const;
	void Reserve(size_t size);
	void ShrinkToFit();
	void Clear();
	void PopBack();
	void Insert(u32 pos, const KString& s);
	void Erase(u32 pos, u32 len);

	const std::string& Get() const;
	std::string& GetMut();
	const char* CStr() const;
	
	void ReplaceCharInline(char old, char replacement);;

	std::string::const_iterator begin() const;
	std::string::const_iterator end()   const;

	/*************** numeric conversions ***************/

	f32 ToFloat()	const;
	f32 ToF32()		const;
	f64 ToDouble()	const;
	f64 ToF64()		const;
	i32 ToInt()		const;
	u8  ToU8()		const;
	i8  ToI8()		const;
	u16 ToU16()		const;
	i16 ToI16()		const;
	u32 ToU32()		const;
	i32 ToI32()		const;
	u64 ToU64()		const;
	i64 ToI64()		const;
	

	template <typename T>
	T ToNum() const
	{

	}

	template <>
	f32 ToNum<f32>() const
	{
		return ToF32();
	}

	template <>
	f64 ToNum<f64>() const
	{
		return ToF64();
	}

	template <>
	i8 ToNum<i8>() const
	{
		return ToI8();
	}

	template <>
	u8 ToNum<u8>() const
	{
		return ToU8();
	}

	template <>
	i16 ToNum<i16>() const
	{
		return ToI16();
	}

	template <>
	u16 ToNum<u16>() const
	{
		return ToU16();
	}

	template <>
	i32 ToNum<i32>() const
	{
		return ToI32();
	}

	template <>
	u32 ToNum<u32>() const
	{
		return ToU32();
	}

	template <>
	i64 ToNum<i64>() const
	{
		return ToI64();
	}

	template <>
	u64 ToNum<u64>() const
	{
		return ToU64();
	}

	template <typename T>
	bool ToNumSafe(T& val) const
	{

	}

	template <>
	bool ToNumSafe<f32>(f32& val) const
	{
		return ToFloatSafe(val);
	}

	template <>
	bool ToNumSafe<f64>(f64& val) const
	{
		return ToDoubleSafe(val);
	}

	template <>
	bool ToNumSafe<i8>(i8& val) const
	{
		return ToI8Safe(val);
	}

	template <>
	bool ToNumSafe<u8>(u8& val) const
	{
		return ToU8Safe(val);
	}

	template <>
	bool ToNumSafe<i16>(i16& val) const
	{
		return ToI16Safe(val);
	}

	template <>
	bool ToNumSafe<u16>(u16& val) const
	{
		return ToU16Safe(val);
	}

	template <>
	bool ToNumSafe<i32>(i32& val) const
	{
		return ToI32Safe(val);
	}

	template <>
	bool ToNumSafe<u32>(u32& val) const
	{
		return ToU32Safe(val);
	}

	template <>
	bool ToNumSafe<i64>(i64& val) const
	{
		return ToI64Safe(val);
	}

	template <>
	bool ToNumSafe<u64>(u64& val) const
	{
		return ToU64Safe(val);
	}

private:
	
	template<typename T>
	bool stoi_safe(T& val) const
	{
		try { val = std::stoi(String); }
		catch (std::invalid_argument) { return false; }
		return true;
	}

	bool stoul_safe(u32& val) const;

	bool stoll_safe(i64& val) const;

	bool stoull_safe(u64& val) const;

	bool stof_safe(f32& val) const;

	bool stod_safe(f64& val) const;

public:

	bool ToFloatSafe(f32& val)	const;
	bool ToDoubleSafe(f64& val)	const;
	bool ToIntSafe(i32& val)		const;
	bool ToU8Safe(u8& val)		const;
	bool ToI8Safe(i8& val)		const;
	bool ToU16Safe(u16& val)		const;
	bool ToI16Safe(i16& val)		const;
	bool ToU32Safe(u32& val)		const;
	bool ToI32Safe(i32& val)		const;
	bool ToU64Safe(u64& val)		const;
	bool ToI64Safe(i64& val)		const;

	/*************** convenience ***************/

	KString GetSubstring(u32 start, u32 num) const;

	i32 FindSubstring(const KString& sub) const;

	bool Contains(const KString& str) const;

	void TrimInPlace();

	KString Trim() const;

	bool StartsWith(const KString& phrase);

	bool EndsWith(const KString& end);

	void FixFloatFraction(u8 fixedFrac);

	void TrimZeroesFromFloat();

	void ToLowerInPlace();
	void ToUpperInPlace();
	KString ToLower() const;
	KString ToUpper() const;

	void SplitByChar(TVector<KString>& out, char delimiter, u32 limit = 0) const;

	std::wstring GetWide() const;

	//const wchar_t* ToWide() const { return std::wstring(String.begin(), String.end()).c_str(); }


#if _WIN32
	std::wstring ToWideStr() const;
#endif

	template <typename T>
	TVector<T> ToNumArray() const
	{
		TVector<T> out;
		KString buffer;
		for (i32 i = 0; i < i32(Length()); i++)
		{
			char c = String[i];
			if (c == ' ' || i == Length() - 1)
			{
				if (i == Length() - 1) buffer += c;

				if (buffer.Length() > 0)
				{
					T f;
					if (buffer.ToNumSafe<T>(f))
					{
						out.push_back(f);
						buffer = "";
					}
					else
					{
						return out;
					}
				}
				continue;
			}
			buffer += c;
		}
		return out;
	}

	TVector<f32> ToFloatArray();

	static KString GetTimeDateString();

	void FixDirectoryInline();

	KString FixDirectory() const;

	/*************** operators ***************/

	operator const char*		() const;
	operator const std::string&	() const;
	operator std::string		() const;
	operator std::string*		();
	operator std::string&		();
														
	void operator = (const KString& str);
	void operator = (const std::string& str);
	void operator = (const char* str);

	KString operator + (const KString& str)	const;
	KString operator + (const std::string& str) const;
	KString operator + (const char* str) const;

	KString operator + (f32 v) const;
	KString operator + (u8 v) const;
	KString operator + (i8 v) const;
	KString operator + (u16 v) const;
	KString operator + (i16 v) const;
	KString operator + (u32 v) const;
	KString operator + (i32 v) const;
	KString operator + (u64 v) const;
	KString operator + (i64 v) const;

	KString& operator += (const KString& str);
	KString& operator += (const std::string& str);
	KString& operator += (const char* str);
	KString& operator += (char str);
	KString& operator += (f32 v);
	KString& operator += (u8 v);
	KString& operator += (i8 v);
	KString& operator += (u16 v);
	KString& operator += (i16 v);
	KString& operator += (u32 v);
	KString& operator += (i32 v);
	KString& operator += (u64 v);
	KString& operator += (i64 v);

	bool operator == (const std::string& str);;
	bool operator == (const KString& str);;
	bool operator == (const char* str);

	bool operator != (const std::string& str);;
	bool operator != (const KString& str);;
	bool operator != (const char* str);


	char& operator[](u32 pos);
	const char& operator[](u32 pos) const;
};

/*************** free function operators ***************/

static std::ostream& operator << (std::ostream& os, const KString& s)
{
	os << s.Get();
	return os;
}

static KString operator + (const std::string& lhs, const KString& rhs)
{
	return lhs + rhs.Get(); 
}

static KString operator + (const char* lhs, const KString& rhs)
{
	return KString(lhs) + rhs; 
}

static KString operator + (char lhs, const KString& rhs)
{
	return KString(lhs) + rhs;
}

static bool operator<(const KString& lhs, const KString& rhs) 
{
	return lhs.Get() < rhs.Get();
}

static bool operator>(const KString& lhs, const KString& rhs) 
{
	return lhs.Get() > rhs.Get();
}

static bool operator<=(const KString& lhs, const KString& rhs)
{
	return lhs.Get() <= rhs.Get();
}

static bool operator>=(const KString& lhs, const KString& rhs)
{
	return lhs.Get() >= rhs.Get();
}