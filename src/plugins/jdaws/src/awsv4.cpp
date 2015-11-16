#include "awsv4.h"

#define POCO_MAX_INT_STRING_LEN 65

const std::string ENDL = "\n";
const std::string  STRING_TO_SIGN_ALGO = "AWS4-HMAC-SHA256";
const std::string AWS4 = "AWS4";
const std::string AWS4_REQUEST = "aws4_request";

const std::string URI::ILLEGAL           = "%<>{}|\\\"^`";

class Ascii
	/// This class contains enumerations and static
	/// utility functions for dealing with ASCII characters
	/// and their properties.
	///
	/// The classification functions will also work if
	/// non-ASCII character codes are passed to them,
	/// but classification will only check for
	/// ASCII characters.
	///
	/// This allows the classification methods to be used
	/// on the single bytes of a UTF-8 string, without
	/// causing assertions or inconsistent results (depending
	/// upon the current locale) on bytes outside the ASCII range,
	/// as may be produced by Ascii::isSpace(), etc.
{
public:
	enum CharacterProperties
		/// ASCII character properties.
	{
		ACP_CONTROL  = 0x0001,
		ACP_SPACE    = 0x0002,
		ACP_PUNCT    = 0x0004,
		ACP_DIGIT    = 0x0008,
		ACP_HEXDIGIT = 0x0010,
		ACP_ALPHA    = 0x0020,
		ACP_LOWER    = 0x0040,
		ACP_UPPER    = 0x0080,
		ACP_GRAPH    = 0x0100,
		ACP_PRINT    = 0x0200
	};

	static int properties(int ch);
		/// Return the ASCII character properties for the
		/// character with the given ASCII value.
		///
		/// If the character is outside the ASCII range
		/// (0 .. 127), 0 is returned.

	static bool hasSomeProperties(int ch, int properties);
		/// Returns true if the given character is
		/// within the ASCII range and has at least one of
		/// the given properties.

	static bool hasProperties(int ch, int properties);
		/// Returns true if the given character is
		/// within the ASCII range and has all of
		/// the given properties.

	static bool isAscii(int ch);
		/// Returns true iff the given character code is within
		/// the ASCII range (0 .. 127).

	static bool isSpace(int ch);
		/// Returns true iff the given character is a whitespace.

	static bool isDigit(int ch);
		/// Returns true iff the given character is a digit.

	static bool isHexDigit(int ch);
		/// Returns true iff the given character is a hexadecimal digit.

	static bool isPunct(int ch);
		/// Returns true iff the given character is a punctuation character.

	static bool isAlpha(int ch);
		/// Returns true iff the given character is an alphabetic character.

	static bool isAlphaNumeric(int ch);
		/// Returns true iff the given character is an alphabetic character.

	static bool isLower(int ch);
		/// Returns true iff the given character is a lowercase alphabetic
		/// character.

	static bool isUpper(int ch);
		/// Returns true iff the given character is an uppercase alphabetic
		/// character.

	static int toLower(int ch);
		/// If the given character is an uppercase character,
		/// return its lowercase counterpart, otherwise return
		/// the character.

	static int toUpper(int ch);
		/// If the given character is a lowercase character,
		/// return its uppercase counterpart, otherwise return
		/// the character.

private:
	static const int CHARACTER_PROPERTIES[128];
};


const int Ascii::CHARACTER_PROPERTIES[128] = 
{
	/* 00 . */ ACP_CONTROL,
	/* 01 . */ ACP_CONTROL,
	/* 02 . */ ACP_CONTROL,
	/* 03 . */ ACP_CONTROL,
	/* 04 . */ ACP_CONTROL,
	/* 05 . */ ACP_CONTROL,
	/* 06 . */ ACP_CONTROL,
	/* 07 . */ ACP_CONTROL,
	/* 08 . */ ACP_CONTROL,
	/* 09 . */ ACP_CONTROL | ACP_SPACE,
	/* 0a . */ ACP_CONTROL | ACP_SPACE,
	/* 0b . */ ACP_CONTROL | ACP_SPACE,
	/* 0c . */ ACP_CONTROL | ACP_SPACE,
	/* 0d . */ ACP_CONTROL | ACP_SPACE,
	/* 0e . */ ACP_CONTROL,
	/* 0f . */ ACP_CONTROL,
	/* 10 . */ ACP_CONTROL,
	/* 11 . */ ACP_CONTROL,
	/* 12 . */ ACP_CONTROL,
	/* 13 . */ ACP_CONTROL,
	/* 14 . */ ACP_CONTROL,
	/* 15 . */ ACP_CONTROL,
	/* 16 . */ ACP_CONTROL,
	/* 17 . */ ACP_CONTROL,
	/* 18 . */ ACP_CONTROL,
	/* 19 . */ ACP_CONTROL,
	/* 1a . */ ACP_CONTROL,
	/* 1b . */ ACP_CONTROL,
	/* 1c . */ ACP_CONTROL,
	/* 1d . */ ACP_CONTROL,
	/* 1e . */ ACP_CONTROL,
	/* 1f . */ ACP_CONTROL,
	/* 20   */ ACP_SPACE | ACP_PRINT,
	/* 21 ! */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 22 " */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 23 # */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 24 $ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 25 % */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 26 & */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 27 ' */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 28 ( */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 29 ) */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 2a * */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 2b + */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 2c , */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 2d - */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 2e . */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 2f / */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 30 0 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 31 1 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 32 2 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 33 3 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 34 4 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 35 5 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 36 6 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 37 7 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 38 8 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 39 9 */ ACP_DIGIT | ACP_HEXDIGIT | ACP_GRAPH | ACP_PRINT,
	/* 3a : */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 3b ; */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 3c < */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 3d = */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 3e > */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 3f ? */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 40 @ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 41 A */ ACP_HEXDIGIT | ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 42 B */ ACP_HEXDIGIT | ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 43 C */ ACP_HEXDIGIT | ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 44 D */ ACP_HEXDIGIT | ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 45 E */ ACP_HEXDIGIT | ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 46 F */ ACP_HEXDIGIT | ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 47 G */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 48 H */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 49 I */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 4a J */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 4b K */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 4c L */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 4d M */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 4e N */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 4f O */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 50 P */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 51 Q */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 52 R */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 53 S */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 54 T */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 55 U */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 56 V */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 57 W */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 58 X */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 59 Y */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 5a Z */ ACP_ALPHA | ACP_UPPER | ACP_GRAPH | ACP_PRINT,
	/* 5b [ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 5c \ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 5d ] */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 5e ^ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 5f _ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 60 ` */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 61 a */ ACP_HEXDIGIT | ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 62 b */ ACP_HEXDIGIT | ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 63 c */ ACP_HEXDIGIT | ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 64 d */ ACP_HEXDIGIT | ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 65 e */ ACP_HEXDIGIT | ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 66 f */ ACP_HEXDIGIT | ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 67 g */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 68 h */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 69 i */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 6a j */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 6b k */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 6c l */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 6d m */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 6e n */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 6f o */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 70 p */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 71 q */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 72 r */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 73 s */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 74 t */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 75 u */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 76 v */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 77 w */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 78 x */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 79 y */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 7a z */ ACP_ALPHA | ACP_LOWER | ACP_GRAPH | ACP_PRINT,
	/* 7b { */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 7c | */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 7d } */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 7e ~ */ ACP_PUNCT | ACP_GRAPH | ACP_PRINT,
	/* 7f . */ ACP_CONTROL
};


//
// inlines
//
inline int Ascii::properties(int ch)
{
	if (isAscii(ch))
		return CHARACTER_PROPERTIES[ch];
	else
		return 0;
}


inline bool Ascii::isAscii(int ch)
{
	return (static_cast<unsigned int>(ch) & 0xFFFFFF80) == 0;
}


inline bool Ascii::hasProperties(int ch, int props)
{
	return (properties(ch) & props) == props;
}


inline bool Ascii::hasSomeProperties(int ch, int props)
{
	return (properties(ch) & props) != 0;
}


inline bool Ascii::isSpace(int ch)
{
	return hasProperties(ch, ACP_SPACE);
}


inline bool Ascii::isDigit(int ch)
{
	return hasProperties(ch, ACP_DIGIT);
}


inline bool Ascii::isHexDigit(int ch)
{
	return hasProperties(ch, ACP_HEXDIGIT);
}


inline bool Ascii::isPunct(int ch)
{
	return hasProperties(ch, ACP_PUNCT);
}


inline bool Ascii::isAlpha(int ch)
{
	return hasProperties(ch, ACP_ALPHA);
}


inline bool Ascii::isAlphaNumeric(int ch)
{
	return hasSomeProperties(ch, ACP_ALPHA | ACP_DIGIT);
}


inline bool Ascii::isLower(int ch)
{
	return hasProperties(ch, ACP_LOWER);
}


inline bool Ascii::isUpper(int ch)
{
	return hasProperties(ch, ACP_UPPER);
}


inline int Ascii::toLower(int ch)
{
	if (isUpper(ch))
		return ch + 32;
	else
		return ch;
}


inline int Ascii::toUpper(int ch)
{
	if (isLower(ch))
		return ch - 32;
	else
		return ch;
}

//==============================================

class SyntaxException
{
	public:
	SyntaxException(std::string strErr){
		// Emit
	}
	SyntaxException(std::string strErr, std::string strArg){
		// Emit
	}
};

std::string& toLowerInPlace(std::string& str)
{
	//std::string result;
	//TextConverter converter(utf8, utf8);
	//converter.convert(str, result, Unicode::toLower);
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);

	//std::swap(str, result);
	return str;
}

URI::URI():
	_port(0)
{
}

URI::URI(const std::string& uri):
	_port(0)
{
	parse(uri);
}

	URI::URI(const URI& uri):
	_scheme(uri._scheme),
	_userInfo(uri._userInfo),
	_host(uri._host),
	_port(uri._port),
	_path(uri._path),
	_query(uri._query),
	_fragment(uri._fragment)
{
}

void URI::parse(const std::string& uri)
{
	std::string::const_iterator it  = uri.begin();
	std::string::const_iterator end = uri.end();
	if (it == end) return;
	if (*it != '/' && *it != '.' && *it != '?' && *it != '#')
	{
		std::string scheme;
		while (it != end && *it != ':' && *it != '?' && *it != '#' && *it != '/') scheme += *it++;
		if (it != end && *it == ':')
		{
			++it;
			if (it == end) throw SyntaxException("URI scheme must be followed by authority or path", uri);
			setScheme(scheme);
			if (*it == '/')
			{
				++it;
				if (it != end && *it == '/')
				{
					++it;
					parseAuthority(it, end);
				}
				else --it;
			}
			parsePathEtc(it, end);
		}
		else 
		{
			it = uri.begin();
			parsePathEtc(it, end);
		}
	}
	else parsePathEtc(it, end);
}

void URI::parsePathEtc(std::string::const_iterator& it, const std::string::const_iterator& end)
{
	if (it == end) return;
	if (*it != '?' && *it != '#')
		parsePath(it, end);
	if (it != end && *it == '?')
	{
		++it;
		parseQuery(it, end);
	}
	if (it != end && *it == '#')
	{
		++it;
		parseFragment(it, end);
	}	
}

void URI::parsePath(std::string::const_iterator& it, const std::string::const_iterator& end)
{
	std::string path;
	while (it != end && *it != '?' && *it != '#') path += *it++;
	decode(path, _path);
}

void URI::parseQuery(std::string::const_iterator& it, const std::string::const_iterator& end)
{
	_query.clear();
	while (it != end && *it != '#') _query += *it++;
}
void URI::parseFragment(std::string::const_iterator& it, const std::string::const_iterator& end)
{
	std::string fragment;
	while (it != end) fragment += *it++;
	decode(fragment, _fragment);
}


void URI::parseAuthority(std::string::const_iterator& it, const std::string::const_iterator& end)
{
	std::string userInfo;
	std::string part;
	while (it != end && *it != '/' && *it != '?' && *it != '#')
	{
		if (*it == '@')
		{
			userInfo = part;
			part.clear();
		}
		else part += *it;
		++it;
	}
	std::string::const_iterator pbeg = part.begin();
	std::string::const_iterator pend = part.end();
	parseHostAndPort(pbeg, pend);
	_userInfo = userInfo;
}


void URI::parseHostAndPort(std::string::const_iterator& it, const std::string::const_iterator& end)
{
	if (it == end) return;
	std::string host;
#if 0 //TODO
	if (*it == '[')
	{
		// IPv6 address
		++it;
		while (it != end && *it != ']') host += *it++;
		if (it == end) throw SyntaxException("unterminated IPv6 address");
		++it;
	}
	else
	{
		while (it != end && *it != ':') host += *it++;
	}
	if (it != end && *it == ':')
	{
		++it;
		std::string port;
		while (it != end) port += *it++;
		if (!port.empty())
		{
			int nport = 0;
			if (NumberParser::tryParse(port, nport) && nport > 0 && nport < 65536)
				_port = (unsigned short) nport;
			else
				throw SyntaxException("bad or invalid port number", port);
		}
		else _port = getWellKnownPort();
	}
	else _port = getWellKnownPort();
#else
	while (it != end && *it != ':') host += *it++;
#endif
	_host = host;
	toLowerInPlace(_host);
}

void URI::setScheme(const std::string& scheme)
{
	_scheme = scheme;
	toLowerInPlace(_scheme);
	//if (_port == 0)
	//	_port = getWellKnownPort();
}
void URI::decode(const std::string& str, std::string& decodedStr, bool plusAsSpace)
{
	bool inQuery = false;
	std::string::const_iterator it  = str.begin();
	std::string::const_iterator end = str.end();
	while (it != end)
	{
		char c = *it++;
		if (c == '?') inQuery = true;
		// spaces may be encoded as plus signs in the query
		if (inQuery && plusAsSpace && c == '+') c = ' ';
		else if (c == '%')
		{
			if (it == end) throw SyntaxException("URI encoding: no hex digit following percent sign", str);
			char hi = *it++;
			if (it == end) throw SyntaxException("URI encoding: two hex digits must follow percent sign", str);
			char lo = *it++;
			if (hi >= '0' && hi <= '9')
				c = hi - '0';
			else if (hi >= 'A' && hi <= 'F')
				c = hi - 'A' + 10;
			else if (hi >= 'a' && hi <= 'f')
				c = hi - 'a' + 10;
			else throw SyntaxException("URI encoding: not a hex digit");
			c *= 16;
			if (lo >= '0' && lo <= '9')
				c += lo - '0';
			else if (lo >= 'A' && lo <= 'F')
				c += lo - 'A' + 10;
			else if (lo >= 'a' && lo <= 'f')
				c += lo - 'a' + 10;
			else throw SyntaxException("URI encoding: not a hex digit");
		}
		decodedStr += c;
	}
}


class RangeException
{
public:
	RangeException()
	{
		// TODO
	}
};

class Ptr
		/// Utility char pointer wrapper class.
		/// Class ensures increment/decrement remain within boundaries.
	{
	public:
		Ptr(char* ptr, std::size_t offset): _beg(ptr), _cur(ptr), _end(ptr + offset)
		{
		}
	
		char*& operator ++ () // prefix
		{
			check(_cur + 1);
			return ++_cur;
		}

		char* operator ++ (int) // postfix
		{
			check(_cur + 1);
			char* tmp = _cur++;
			return tmp;
		}
	
		char*& operator -- () // prefix
		{
			check(_cur - 1);
			return --_cur;
		}

		char* operator -- (int) // postfix
		{
			check(_cur - 1);
			char* tmp = _cur--;
			return tmp;
		}

		char*& operator += (int incr)
		{
			check(_cur + incr);
			return _cur += incr;
		}

		char*& operator -= (int decr)
		{
			check(_cur - decr);
			return _cur -= decr;
		}

		operator char* () const
		{
			return _cur;
		}

		std::size_t span() const
		{
			return _end - _beg;
		}

	private:
		void check(char* ptr)
		{
			if (ptr > _end) throw RangeException();
		}

		const char* _beg;
		char*       _cur;
		const char* _end;
};	

template <typename T>
bool uIntToStr(T value,
	unsigned short base,
	char* result,
	std::size_t& size,
	bool prefix = false,
	int width = -1,
	char fill = ' ',
	char thSep = 0)
	/// Converts unsigned integer to string. Numeric bases from binary to hexadecimal are supported.
	/// If width is non-zero, it pads the return value with fill character to the specified width.
	/// When padding is zero character ('0'), it is prepended to the number itself; all other
	/// paddings are prepended to the formatted result with minus sign or base prefix included
	/// If prefix is true and base is octal or hexadecimal, respective prefix ('0' for octal,
	/// "0x" for hexadecimal) is prepended. For all other bases, prefix argument is ignored.
	/// Formatted string has at least [width] total length.
{
	if (base < 2 || base > 0x10)
	{
		*result = '\0';
		return false;
	}
	
	Ptr ptr(result, size);
	int thCount = 0;
	T tmpVal;
	do
	{
		tmpVal = value;
		value /= base;
		*ptr++ = "FEDCBA9876543210123456789ABCDEF"[15 + (tmpVal - value * base)];
		if (thSep && (base == 10) && (++thCount == 3))
		{
			*ptr++ = thSep;
			thCount = 0;
		}
	} while (value);
	
	if ('0' == fill)
	{
		if (prefix && base == 010) --width;
		if (prefix && base == 0x10) width -= 2;
		while ((ptr - result) < width) *ptr++ = fill;
	}
	
	if (prefix && base == 010) *ptr++ = '0';
	else if (prefix && base == 0x10)
	{
		*ptr++ = 'x';
		*ptr++ = '0';
	}
	
	if ('0' != fill)
	{
		while ((ptr - result) < width) *ptr++ = fill;
	}
	
	size = ptr - result;
	//poco_assert_dbg (size <= ptr.span());
	//poco_assert_dbg ((-1 == width) || (size >= size_t(width)));
	*ptr-- = '\0';
	
	char* ptrr = result;
	char tmp;
	while(ptrr < ptr)
	{
		tmp    = *ptr;
		*ptr--  = *ptrr;
		*ptrr++ = tmp;
	}
	
	return true;
}

template <typename T>
bool intToStr (T number, unsigned short base, std::string& result, bool prefix = false, int width = -1, char fill = ' ', char thSep = 0)
	/// Converts integer to string; This is a wrapper function, for details see see the
	/// bool intToStr(T, unsigned short, char*, int, int, char, char) implementation.
{
	char res[POCO_MAX_INT_STRING_LEN] = {0};
	std::size_t size = POCO_MAX_INT_STRING_LEN;
	bool ret = intToStr(number, base, res, size, prefix, width, fill, thSep);
	result.assign(res, size);
	return ret;
}

template <typename T>
bool uIntToStr (T number, unsigned short base, std::string& result, bool prefix = false, int width = -1, char fill = ' ', char thSep = 0)
	/// Converts unsigned integer to string; This is a wrapper function, for details see see the
	/// bool uIntToStr(T, unsigned short, char*, int, int, char, char) implementation.
{
	char res[POCO_MAX_INT_STRING_LEN] = {0};
	std::size_t size = POCO_MAX_INT_STRING_LEN;
	bool ret = uIntToStr(number, base, res, size, prefix, width, fill, thSep);
	result.assign(res, size);
	return ret;
}

class  NumberFormatter
{
public:
	static std::string formatHex(unsigned value, bool prefix = false);
	static std::string formatHex(int value, bool prefix = false);

};

inline std::string NumberFormatter::formatHex(unsigned value, bool prefix)
{
	std::string result;
	uIntToStr(value, 0x10, result, prefix);
	return result;
}

inline std::string NumberFormatter::formatHex(int value, bool prefix)
{
	std::string result;
	uIntToStr(static_cast<unsigned int>(value), 0x10, result, prefix);
	return result;
}

void URI::encode(const std::string& str, const std::string& reserved, std::string& encodedStr)
{
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		char c = *it;
		if ((c >= 'a' && c <= 'z') || 
		    (c >= 'A' && c <= 'Z') || 
		    (c >= '0' && c <= '9') ||
		    c == '-' || c == '_' || 
		    c == '.' || c == '~')
		{
			encodedStr += c;
		}
		else if (c <= 0x20 || c >= 0x7F || ILLEGAL.find(c) != std::string::npos || reserved.find(c) != std::string::npos)
		{
			encodedStr += '%';
			encodedStr += NumberFormatter::formatHex((unsigned) (unsigned char) c, 2);
		}
		else encodedStr += c;
	}
}

std::string URI::getQuery() const
{
	std::string query;
	decode(_query, query);
	return query;
}
//==========================

class StringTokenizer
{
public:
	enum Options
	{
		TOK_IGNORE_EMPTY = 1, /// ignore empty tokens
		TOK_TRIM	 = 2  /// remove leading and trailing whitespace from tokens
	};

	typedef std::vector<std::string> TokenVec;
	typedef TokenVec::const_iterator Iterator;

	StringTokenizer(const std::string& str, const std::string& separators, int options = 0);
		/// Splits the given string into tokens. The tokens are expected to be
		/// separated by one of the separator characters given in separators.
		/// Additionally, options can be specified:
		///   * TOK_IGNORE_EMPTY: empty tokens are ignored
		///   * TOK_TRIM: trailing and leading whitespace is removed from tokens.

	~StringTokenizer();
		/// Destroys the tokenizer.

	Iterator begin() const;
	Iterator end() const;

	const std::string& operator [] (std::size_t index) const;
		/// Returns const reference the index'th token.
		/// Throws a RangeException if the index is out of range.

	std::string& operator [] (std::size_t index);
		/// Returns reference to the index'th token.
		/// Throws a RangeException if the index is out of range.

	bool has(const std::string& token) const;
		/// Returns true if token exists, false otherwise.

	std::size_t find(const std::string& token, std::size_t pos = 0) const;
		/// Returns the index of the first occurence of the token
		/// starting at position pos.
		/// Throws a NotFoundException if the token is not found.

	std::size_t replace(const std::string& oldToken, const std::string& newToken, std::size_t pos = 0);
		/// Starting at position pos, replaces all subsequent tokens having value
		/// equal to oldToken with newToken.
		/// Returns the number of modified tokens.

	std::size_t count() const;
		/// Returns the total number of tokens.

	std::size_t count(const std::string& token) const;
		/// Returns the number of tokens equal to the specified token.

private:
	StringTokenizer(const StringTokenizer&);
	StringTokenizer& operator = (const StringTokenizer&);

	void trim (std::string& token);

	TokenVec _tokens;
};


//
// inlines
//


inline StringTokenizer::Iterator StringTokenizer::begin() const
{
	return _tokens.begin();
}


inline StringTokenizer::Iterator StringTokenizer::end() const
{
	return _tokens.end();
}


inline std::string& StringTokenizer::operator [] (std::size_t index)
{
	return _tokens[index];
}


inline const std::string& StringTokenizer::operator [] (std::size_t index) const
{
	return _tokens[index];
}


inline std::size_t StringTokenizer::count() const
{
	return _tokens.size();
}

	StringTokenizer::StringTokenizer(const std::string& str, const std::string& separators, int options)
	{
		std::string::const_iterator it = str.begin();
		std::string::const_iterator end = str.end();
		std::string token;
		bool doTrim = ((options & TOK_TRIM) != 0);
		bool ignoreEmpty = ((options & TOK_IGNORE_EMPTY) != 0);
		bool lastToken = false;

		for (;it != end; ++it)
		{
			if (separators.find(*it) != std::string::npos)
			{
				if (doTrim) trim(token);
				if (!token.empty() || !ignoreEmpty)_tokens.push_back(token);
				if (!ignoreEmpty) lastToken = true;
				token = "";
			}
			else
			{
				token += *it;
				lastToken = false;
			}
		}

		if (!token.empty())
		{
			if (doTrim) trim(token);
			if (!token.empty()) _tokens.push_back(token);
		}
		else if (lastToken) _tokens.push_back("");
	}


	StringTokenizer::~StringTokenizer()
	{
	}


	void StringTokenizer::trim (std::string& token)
	{
		std::size_t front = 0, back = 0, length = token.length();
		std::string::const_iterator tIt = token.begin();
		std::string::const_iterator tEnd = token.end();
		for (; tIt != tEnd; ++tIt, ++front)
		{
			if (!Ascii::isSpace(*tIt)) break;
		}
		if (tIt != tEnd)
		{
			std::string::const_reverse_iterator tRit = token.rbegin();
			std::string::const_reverse_iterator tRend = token.rend();
			for (; tRit != tRend; ++tRit, ++back)
			{
				if (!Ascii::isSpace(*tRit)) break;
			}
		}
		token = token.substr(front, length - back - front);
	}


	std::size_t StringTokenizer::count(const std::string& token) const
	{
		std::size_t result = 0;
		TokenVec::const_iterator it = std::find(_tokens.begin(), _tokens.end(), token);
		while(it != _tokens.end())
		{
			result++;
			it = std::find(++it, _tokens.end(), token);
		}
		return result;
	}


	std::size_t StringTokenizer::find(const std::string& token, std::size_t pos) const
	{
		TokenVec::const_iterator it = std::find(_tokens.begin() + pos, _tokens.end(), token);
		if ( it != _tokens.end() )
		{
			return it - _tokens.begin();
		}
	}

	bool StringTokenizer::has(const std::string& token) const
	{
		TokenVec::const_iterator it = std::find(_tokens.begin(), _tokens.end(), token);
		return it != _tokens.end();
	}

	std::size_t StringTokenizer::replace(const std::string& oldToken, const std::string& newToken, std::size_t pos)
	{
		std::size_t result = 0;
		TokenVec::iterator it = std::find(_tokens.begin() + pos, _tokens.end(), oldToken);
		while(it != _tokens.end())
		{
			result++;
			*it = newToken;
			it = std::find(++it, _tokens.end(), oldToken);
		}
		return result;
	}

const std::string join(const std::vector<std::string>& ss,const std::string delim) {
        std::stringstream sstream;
        int l = ss.size() - 1;
        std::vector<int>::size_type i;
        for (i = 0; i < l; i++) {
            sstream << ss.at(i) << delim;
        }
        sstream << ss.back();
        return sstream.str();
    }

    // http://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c
    void sha256(const std::string str, unsigned char outputBuffer[SHA256_DIGEST_LENGTH]){
        char *c_string = new char [str.length()+1];
        strcpy(c_string, str.c_str());
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, c_string, strlen(c_string));
        SHA256_Final(hash, &sha256);
        for (int i=0;i<SHA256_DIGEST_LENGTH;i++) {
            outputBuffer[i] = hash[i];
        }
    }
    
    const std::string sha256_base16(const std::string str) {
        unsigned char hashOut[SHA256_DIGEST_LENGTH];
        sha256(str,hashOut);
        char outputBuffer[65];
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            sprintf(outputBuffer + (i * 2), "%02x", hashOut[i]);
        }
        outputBuffer[64] = 0;
        return std::string(outputBuffer);
    }

    // -----------------------------------------------------------------------------------
    // TASK 1 - create a canonical request
    // http://docs.aws.amazon.com/general/latest/gr/sigv4-create-canonical-request.html

    // uri should be normalize()'d before calling here, as this takes a const ref param and we don't 
    // want to normalize repeatedly. the return value is not a uri specifically, but a uri fragment,
    // as such the return value should not be used to initialize a uri object
    const std::string canonicalize_uri(const URI& uri){
        const std::string p = uri.getPath();
        if (p.empty()) return "/";
        std::string encoded_path;
        URI::encode(uri.getPath(),"",encoded_path);
        return encoded_path;
    }

    const std::string canonicalize_query(const URI& uri)   {
        const std::string query_delim = "&";
        const std::string q = uri.getQuery();
        if (q.empty()) return "";
        const StringTokenizer tok(q,query_delim,0);
        std::vector<std::string> parts; 
        for (std::vector<std::string>::const_iterator it = tok.begin(); it != tok.end(); it++) {
            std::string encoded_arg;
            URI::encode(*it,"",encoded_arg);
            parts.push_back(encoded_arg);
       }
        std::sort(parts.begin(),parts.end());
        return join(parts,query_delim);
    }

    // create a map of the "canonicalized" headers
    // will return empty map on malformed input.
    const std::map<std::string,std::string> canonicalize_headers(const std::vector<std::string>& headers)   {
        const std::string header_delim = ":";
        std::map<std::string,std::string> header_key2val;

        for (std::vector<std::string>::const_iterator it = headers.begin(); it != headers.end();it++) {
        	std::string h = *it;
        	const StringTokenizer pair(h,header_delim,2); // 2 -> TOK_TRIM, trim whitespace
            if (pair.count() != 2) { 
                std::cerr << "malformed header: " << h << std::endl;
                header_key2val.clear();
                return header_key2val;
            }
            std::string key = pair[0];
            const std::string val = pair[1];
            if (key.empty() || val.empty()) {
                std::cerr << "malformed header: " << h << std::endl;
                header_key2val.clear();
                return header_key2val;
            }
            std::transform(key.begin(), key.end(), key.begin(),::tolower);
            header_key2val[key] = val;
        }
        return header_key2val;
    }

    // get a string representation of header:value lines
    const std::string map_headers_string(const std::map<std::string,std::string>& header_key2val)   {
        const std::string pair_delim = ":";
        std::string h;
        for (std::map<std::string,std::string>::const_iterator it = header_key2val.begin(); it != header_key2val.end(); it++) {
            h.append(it->first + pair_delim + it->second + ENDL);
        }
        return h;
    }

    // get a string representation of the header names
    const std::string map_signed_headers(const std::map<std::string,std::string>& header_key2val)   {
        const std::string signed_headers_delim = ";";
        std::vector<std::string> ks;
        for (std::map<std::string,std::string>::const_iterator it=header_key2val.begin(); it!=header_key2val.end(); it++) {
            ks.push_back(it->first);
        }
        return join(ks,signed_headers_delim);
    }

    const std::string canonicalize_request(const std::string& http_request_method,
                                           const std::string& canonical_uri,
                                           const std::string& canonical_query_string,
                                           const std::string& canonical_headers,
                                           const std::string& signed_headers,
                                           const std::string& payload)   {
        return http_request_method + ENDL + 
            canonical_uri + ENDL +
            canonical_query_string + ENDL + 
            canonical_headers + ENDL + 
            signed_headers + ENDL +
            sha256_base16(payload);
    }

    // -----------------------------------------------------------------------------------
    // TASK 2 - create a string-to-sign
    // http://docs.aws.amazon.com/general/latest/gr/sigv4-create-string-to-sign.html

    const std::string string_to_sign(const std::string& algorithm,
                                     const std::time_t& request_date,
                                     const std::string& credential_scope,
                                     const std::string& hashed_canonical_request)   {
        return algorithm + ENDL + 
            ISO8601_date(request_date) + ENDL +
            credential_scope + ENDL + 
            hashed_canonical_request;
    }

    // time_t -> 20131222
    const std::string utc_yyyymmdd(const std::time_t& t)   {
        char buf[sizeof "20111008"];
        std::strftime(buf, sizeof buf, "%Y%m%d", std::gmtime(&t));
        return std::string(buf);
    }


    const std::string credential_scope(const std::time_t& request_date, 
                                       const std::string region,
                                       const std::string service)   {
        const std::string s = "/";
        std::string cred = utc_yyyymmdd(request_date);
        return cred + s + region + s + service + s + AWS4_REQUEST;
    }

    // time_t -> 20131222T043039Z
    const std::string ISO8601_date(const std::time_t& t)   {
        char buf[sizeof "20111008T070709Z"];
        std::strftime(buf, sizeof buf, "%Y%m%dT%H%M%SZ", std::gmtime(&t));
        return std::string(buf);
    }

    // -----------------------------------------------------------------------------------
    // TASK 3
    // http://docs.aws.amazon.com/general/latest/gr/sigv4-calculate-signature.html

    const std::string calculate_signature(const std::time_t& request_date,
                                          const std::string secret,
                                          const std::string region,
                                          const std::string service,
                                          const std::string string_to_sign)   {

        const std::string k1 = AWS4 + secret;
        char *c_k1 = new char [k1.length()+1];
        strcpy(c_k1, k1.c_str());

        std::string yyyymmdd = utc_yyyymmdd(request_date);
        char *c_yyyymmdd = new char [yyyymmdd.length()+1];
        strcpy(c_yyyymmdd, yyyymmdd.c_str());

        unsigned char* kDate;
        kDate = HMAC(EVP_sha256(), c_k1, strlen(c_k1), 
                     (unsigned char*)c_yyyymmdd, strlen(c_yyyymmdd), NULL, NULL); 

        char *c_region = new char [region.length()+1];
        strcpy(c_region, region.c_str());
        unsigned char *kRegion;
        kRegion = HMAC(EVP_sha256(), kDate, strlen((char *)kDate), 
                     (unsigned char*)c_region, strlen(c_region), NULL, NULL); 

        char *c_service = new char [service.length()+1];
        strcpy(c_service, service.c_str());
        unsigned char *kService;
        kService = HMAC(EVP_sha256(), kRegion, strlen((char *)kRegion), 
                     (unsigned char*)c_service, strlen(c_service), NULL, NULL); 

        char *c_aws4_request = new char [AWS4_REQUEST.length()+1];
        strcpy(c_aws4_request, AWS4_REQUEST.c_str());
        unsigned char *kSigning;
        kSigning = HMAC(EVP_sha256(), kService, strlen((char *)kService), 
                     (unsigned char*)c_aws4_request, strlen(c_aws4_request), NULL, NULL); 

        char *c_string_to_sign = new char [string_to_sign.length()+1];
        strcpy(c_string_to_sign, string_to_sign.c_str());
        unsigned char *kSig;
        kSig = HMAC(EVP_sha256(), kSigning, strlen((char *)kSigning), 
                     (unsigned char*)c_string_to_sign, strlen(c_string_to_sign), NULL, NULL); 

        char outputBuffer[65];
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            sprintf(outputBuffer + (i * 2), "%02x", kSig[i]);
        }
        outputBuffer[64] = 0;
        return std::string(outputBuffer);
    }


