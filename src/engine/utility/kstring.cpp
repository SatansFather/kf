#include "kstring.h"


bool KString::operator!=(const char* str)
{
	return String != std::string(str);
}

bool KString::operator!=(const KString& str)
{
	return String != str.String;
}

bool KString::operator!=(const std::string& str)
{
	return String != str;
}

bool KString::operator==(const char* str)
{
	return String == std::string(str);
}

bool KString::operator==(const KString& str)
{
	return String == str.String;
}

bool KString::operator==(const std::string& str)
{
	return String == str;
}

KString& KString::operator+=(i64 v)
{
	String.append(KString(v).String); return *this;
}

KString& KString::operator+=(u64 v)
{
	String.append(KString(v).String); return *this;
}

KString& KString::operator+=(i32 v)
{
	String.append(KString(v).String); return *this;
}

KString& KString::operator+=(u32 v)
{
	String.append(KString(v).String); return *this;
}

KString& KString::operator+=(i16 v)
{
	String.append(KString(v).String); return *this;
}

KString& KString::operator+=(u16 v)
{
	String.append(KString(v).String); return *this;
}

KString& KString::operator+=(i8 v)
{
	String.append(KString(v).String); return *this;
}

KString& KString::operator+=(u8 v)
{
	String.append(KString(v).String); return *this;
}

KString& KString::operator+=(f32 v)
{
	String.append(KString(v).String); return *this;
}

KString& KString::operator+=(char str)
{
	String += str; return *this;
}

KString& KString::operator+=(const char* str)
{
	String.append(std::string(str)); return *this;
}

KString& KString::operator+=(const std::string& str)
{
	String.append(str); return *this;
}

KString& KString::operator+=(const KString& str)
{
	String.append(KString(str).String); return *this;
}

KString KString::operator+(i64 v) const
{
	return KString(String + KString(v).String);
}

KString KString::operator+(u64 v) const
{
	return KString(String + KString(v).String);
}

KString KString::operator+(i32 v) const
{
	return KString(String + KString(v).String);
}

KString KString::operator+(u32 v) const
{
	return KString(String + KString(v).String);
}

KString KString::operator+(i16 v) const
{
	return KString(String + KString(v).String);
}

KString KString::operator+(u16 v) const
{
	return KString(String + KString(v).String);
}

KString KString::operator+(i8 v) const
{
	return KString(String + KString(v).String);
}

KString KString::operator+(u8 v) const
{
	return KString(String + KString(v).String);
}

KString KString::operator+(f32 v) const
{
	return KString(String + KString(v).String);
}

KString KString::operator+(const char* str) const
{
	return KString(String + std::string(str));
}

KString KString::operator+(const std::string& str) const
{
	return KString(String + str);
}

KString KString::operator+(const KString& str) const
{
	return KString(String + str.String);
}

void KString::operator=(const char* str)
{
	String = std::string(str);
}

void KString::operator=(const std::string& str)
{
	String = str;
}

void KString::operator=(const KString& str)
{
	String = str.String;
}

KString::KString(f64 v, u32 fixedFrac) : String(std::to_string(v))
{
	FixFloatFraction(fixedFrac);
}

KString::KString(f32 v, u32 fixedFrac) : String(std::to_string(v))
{
	FixFloatFraction(fixedFrac);
}

KString::KString(f64 v, i32 decPrecision /*= -1*/)
{

	if (decPrecision < 0)
	{
		String = std::to_string(v);
		TrimZeroesFromFloat();
	}
	else
	{
		std::stringstream st;
		st << std::fixed << std::setprecision(decPrecision) << v;
		String = st.str();
	}
}

KString::KString(f32 v, i32 decPrecision /*= -1*/)
{

	if (decPrecision < 0)
	{
		String = std::to_string(v);
		TrimZeroesFromFloat();
	}
	else
	{
		std::stringstream st;
		st << std::fixed << std::setprecision(decPrecision) << v;
		String = st.str();
	}
}

KString::KString(const wchar_t* str)
{
	std::wstring w = std::wstring(str);
	String = std::string(w.begin(), w.end());
}

KString::KString(i64 v) : String(std::to_string(v))
{

}

KString::KString(u64 v) : String(std::to_string(v))
{

}

KString::KString(i32 v) : String(std::to_string(v))
{

}

KString::KString(u32 v) : String(std::to_string(v))
{

}

KString::KString(i16 v) : String(std::to_string(v))
{

}

KString::KString(u16 v) : String(std::to_string(v))
{

}

KString::KString(i8 v) : String(std::to_string(v))
{

}

KString::KString(u8 v) : String(std::to_string(v))
{

}

KString::KString(const std::wstring& str) : String(str.begin(), str.end())
{

}

KString::KString(char* str) : String(std::string(str))
{

}

KString::KString(const char* str) : String(std::string(str))
{

}

KString::KString(std::string* str) : String(*str)
{

}

KString::KString(std::string str) : String(str)
{

}

KString KString::FromChar(char c)
{
	KString s;
	s.String = std::string(1, c);
	return s;
}

u32 KString::Size() const
{
	return (u32)String.size();
}

u32 KString::Length() const
{
	return (u32)String.size();
}

u32 KString::Capacity() const
{
	return (u32)String.capacity();
}

u32 KString::MaxSize() const
{
	return (u32)String.max_size();
}

bool KString::IsEmpty() const
{
	return String.empty();
}

u32 KString::Find(char c) const
{
	return String.find(c);
}

void KString::Reserve(size_t size)
{
	String.reserve(size);
}

void KString::ShrinkToFit()
{
	String.shrink_to_fit();
}

void KString::Clear()
{
	String.clear();
}

void KString::PopBack()
{
	String.pop_back();
}

void KString::Insert(u32 pos, const KString& s)
{
	String.insert(pos, s.Get());
}

void KString::Erase(u32 pos, u32 len)
{
	String.erase(pos, len);
}

const std::string& KString::Get() const
{
	return String;
}

std::string& KString::GetMut()
{
	return String;
}

const char* KString::CStr() const
{
	return String.c_str();
}

void KString::ReplaceCharInline(char old, char replacement)
{
	std::replace(String.begin(), String.end(), old, replacement);
}

std::string::const_iterator KString::begin() const
{
	return String.begin();
}

std::string::const_iterator KString::end() const
{
	return String.end();
}

f32 KString::ToFloat() const
{
	return std::stof(String);
}

f32 KString::ToF32() const
{
	return std::stof(String);
}

f64 KString::ToDouble() const
{
	return std::stod(String);
}

f64 KString::ToF64() const
{
	return std::stod(String);
}

i32 KString::ToInt() const
{
	return std::stoi(String);
}

u8 KString::ToU8() const
{
	return (u8)std::stoi(String);
}

i8 KString::ToI8() const
{
	return (i8)std::stoi(String);
}

u16 KString::ToU16() const
{
	return (u16)std::stoi(String);
}

i16 KString::ToI16() const
{
	return (i16)std::stoi(String);
}

u32 KString::ToU32() const
{
	return std::stoul(String);
}

i32 KString::ToI32() const
{
	return std::stoi(String);
}

u64 KString::ToU64() const
{
	return std::stoull(String);
}

i64 KString::ToI64() const
{
	return std::stoll(String);
}

bool KString::stoul_safe(u32& val) const
{
	try { val = std::stoul(String); }
	catch (std::invalid_argument) { return false; }
	return true;
}

bool KString::stoll_safe(i64& val) const
{
	try { val = std::stoll(String); }
	catch (std::invalid_argument) { return false; }
	return true;
}

bool KString::stoull_safe(u64& val) const
{
	try { val = std::stoull(String); }
	catch (std::invalid_argument) { return false; }
	return true;
}

bool KString::stof_safe(f32& val) const
{
	try { val = std::stof(String); }
	catch (std::invalid_argument) { return false; }
	return true;
}

bool KString::stod_safe(f64& val) const
{
	try { val = std::stod(String); }
	catch (std::invalid_argument) { return false; }
	return true;
}

bool KString::ToFloatSafe(f32& val) const
{
	return stof_safe(val);
}

bool KString::ToDoubleSafe(f64& val) const
{
	return stod_safe(val);
}

bool KString::ToIntSafe(i32& val) const
{
	return stoi_safe<i32>(val);
}

bool KString::ToU8Safe(u8& val) const
{
	return stoi_safe<u8>(val);
}

bool KString::ToI8Safe(i8& val) const
{
	return stoi_safe<i8>(val);
}

bool KString::ToU16Safe(u16& val) const
{
	return stoi_safe<u16>(val);
}

bool KString::ToI16Safe(i16& val) const
{
	return stoi_safe<i16>(val);
}

bool KString::ToU32Safe(u32& val) const
{
	return stoul_safe(val);
}

bool KString::ToI32Safe(i32& val) const
{
	return stoi_safe<i32>(val);
}

bool KString::ToU64Safe(u64& val) const
{
	return stoull_safe(val);
}

bool KString::ToI64Safe(i64& val) const
{
	return stoll_safe(val);
}

KString KString::GetSubstring(u32 start, u32 num) const
{
	return String.substr(start, num);
}

i32 KString::FindSubstring(const KString& sub) const
{
	size_t pos = String.find(sub.String);
	if (pos == std::string::npos) return -1;
	return i32(pos);
}

bool KString::Contains(const KString& str) const
{
	return String.find(str.String) != std::string::npos;
}

void KString::TrimInPlace()
{
	size_t first = String.find_first_not_of(' ');
	if (std::string::npos != first)
	{
		size_t last = String.find_last_not_of(' ');
		String = String.substr(first, (last - first + 1));
	}
}

KString KString::Trim() const
{
	KString str = *this;
	str.TrimInPlace();
	return str;
}

bool KString::StartsWith(const KString& phrase)
{
	return String.rfind(phrase.String, 0) == 0;
}

bool KString::EndsWith(const KString& end)
{
	if (String.length() >= end.String.length())
	{
		return (0 == String.compare(String.length() - end.String.length(), end.String.length(), end.String));
	}
	return false;
}

void KString::FixFloatFraction(u8 fixedFrac)
{
	/*u32 dec = Find('.');
	if (dec == std::string::npos)
	{
		String += ".";
		while (fixedFrac > 0)
		{
			String += "0";
			fixedFrac--;
		}
		return;
	}

	u32 decPlaces = Size() - dec;
	if (decPlaces > fixedFrac)
	{
		String = String.substr(0, Size() - (decPlaces - fixedFrac));
	}
	else if (decPlaces < fixedFrac)
	{
		while (decPlaces <= fixedFrac)
		{
			String += "0";
			decPlaces++;
		}
	}*/
}

void KString::TrimZeroesFromFloat()
{
	bool decimal = false;
	bool allZero = true;
	i32 zero = -1;
	i32 d = -1;
	for (i32 i = 0; i < i32(Length()); i++)
	{
		if (String[i] == '.')
		{
			decimal = true;
			d = i;
			continue;
		}
		if (decimal)
		{
			if (String[i] == '0')
			{
				if (zero == -1)
					zero = i;
			}
			else
			{
				zero = -1;
				allZero = false;
			}
		}
	}

	if (zero > 0)
	{
		if (allZero)
			String.erase(begin() + d, end());
		else
			String.erase(begin() + zero, end());
	}
}

void KString::ToLowerInPlace()
{
	std::transform(String.begin(), String.end(), String.begin(), ::tolower);
}

void KString::ToUpperInPlace()
{
	std::transform(String.begin(), String.end(), String.begin(), ::toupper);
}

KString KString::ToLower() const
{
	KString out(*this); out.ToLowerInPlace(); return out;
}

KString KString::ToUpper() const
{
	KString out(*this); out.ToUpperInPlace(); return out;
}

void KString::SplitByChar(TVector<KString>& out, char delimiter, u32 limit /*= 0*/) const
{
	KString buff;
	u32 count = 0;
	for (u32 i = 0; i < String.size(); i++)
	{
		char c = String[i];
		if (c == delimiter)
		{
			out.push_back(buff);
			buff = "";
			count++;
			if (limit != 0 && count >= limit)
			{
				buff = GetSubstring(i, String.size() - i);
				break;
			}
		}
		else
		{
			buff += c;	
		}
	}

	if (buff.Size() > 0)
		out.push_back(buff);

	/*std::stringstream ss(String);
	std::string s;
	u32 count = 0;
	while (std::getline(ss, s, delimiter))
		out.push_back(KString(s));*/
}

std::wstring KString::GetWide() const
{
	return std::wstring(String.begin(), String.end()).c_str();
}

#if _WIN32
std::wstring KString::ToWideStr() const
{
	i32 len;
	i32 slength = (i32)String.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, String.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, String.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
#endif

TVector<f32> KString::ToFloatArray()
{
	return ToNumArray<f32>();
}

KString KString::GetTimeDateString()
{
	// weird
#ifdef __linux__
#define _STD_
#else
#define _STD_ std
#endif
	_STD_::time_t t = _STD_::time(NULL);
	char mbstr[100];
	_STD_::strftime(mbstr, 100, "%Y-%m-%d-%T", _STD_::localtime(&t));
	std::string timestr = std::string(mbstr);
	std::replace(timestr.begin(), timestr.end(), ':', '-');
	return KString(timestr);
#undef _STD_
}

void KString::FixDirectoryInline()
{
	ReplaceCharInline('\\', '/');
}

KString KString::FixDirectory() const
{
	KString s(String);
	s.FixDirectoryInline();
	return s;
}

KString::operator const char* () const
{
	return CStr();
}

const char& KString::operator[](u32 pos) const
{
	return String[pos];
}

char& KString::operator[](u32 pos)
{
	return String[pos];
}

KString::operator std::string& ()
{
	return String;
}

KString::operator std::string* ()
{
	return &String;
}

KString::operator std::string() const
{
	return String;
}

KString::operator const std::string& () const
{
	return String;
}

