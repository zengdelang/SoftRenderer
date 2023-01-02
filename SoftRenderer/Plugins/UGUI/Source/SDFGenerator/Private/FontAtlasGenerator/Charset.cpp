#include "FontAtlasGenerator/Charset.h"

namespace FontAtlasGenerator
{
	bool FCharsetCollector::ContainsCodePoint(unicode_t CodePoint, EFontStyle FontStyle)
	{
		for (const auto& Charset : Charsets)
		{
			if (Charset.FontStyle == FontStyle)
			{
				if (Charset.CodePoints.Contains(CodePoint))
				{
					return true;
				}
			}
		}
		return false;
	}
	
	static FCharset CreateAsciiCharset()
	{
		FCharset ASCII;
		for (unicode_t CodePoint = 0x00; CodePoint < 0x101; ++CodePoint)
			ASCII.Add(CodePoint);
		return ASCII;
	}

	const FCharset FCharset::ASCII = CreateAsciiCharset();

	void FCharset::Add(unicode_t CodePoint)
	{
		if (CharsetCollector)
		{
			if (CharsetCollector->ContainsCodePoint(CodePoint, FontStyle))
			{
				return;
			}
		}
		
		if (!CodePoints.Contains(CodePoint))
		{
			CodePoints.Add(CodePoint);
			Charset.AppendChar(CodePoint);
		}
	}

	static TCHAR ReadWord(FString& Buffer, const FString& CharsetString, int32& TextIndex)
	{
		for (const int32 Len = CharsetString.Len(); TextIndex < Len; ++TextIndex)
		{
			const TCHAR Char = CharsetString[TextIndex];
			if ((Char >= 'A' && Char <= 'Z') || (Char >= 'a' && Char <= 'z') || (Char>= '0' && Char <= '9') || Char == '_')
			{
				Buffer.AppendChar(Char);
			}
			else
			{
				return Char;
			}
		}
		return 0;
	}

	static bool ParseInt(int32& Integer, const FString& BufferStr)
	{
		Integer = 0;
		if (BufferStr.Len() > 1 && BufferStr[0] == '0' && (BufferStr[1] == 'x' || BufferStr[1] == 'X'))
		{
			// hex
			for (int32 TextIndex = 2, Len = BufferStr.Len(); TextIndex < Len; ++TextIndex)
			{
				const TCHAR Char = BufferStr[TextIndex];
				if (Char >= '0' && Char <= '9')
				{
					Integer <<= 4;
					Integer += Char - '0';
				}
				else if (Char >= 'A' && Char <= 'F')
				{
					Integer <<= 4;
					Integer += Char - 'A' + 10;
				}
				else if (Char >= 'a' && Char <= 'f')
				{
					Integer <<= 4;
					Integer += Char - 'a' + 10;
				}
				else
				{
					return false;
				}
			}
		}
		else
		{
			// dec
			for (int32 TextIndex = 0, Len = BufferStr.Len(); TextIndex < Len; ++TextIndex)
			{
				const TCHAR Char = BufferStr[TextIndex];
				if (Char >= '0' && Char <= '9')
				{
					Integer *= 10;
					Integer += Char - '0';
				}
				else
				{
					return false;
				}
			}
		}
		return true;
	}

	static TCHAR EscapedChar(TCHAR Char)
	{
		switch (Char)
		{
		case '0':
			return '\0';
		case 'n':
		case 'N':
			return '\n';
		case 'r':
		case 'R':
			return '\r';
		case 's':
		case 'S':
			return ' ';
		case 't':
		case 'T':
			return '\t';
		case '\\':
		case '"':
		case '\'':
		default:
			return Char;
		}
	}
	
	static bool ReadString(FString& Buffer, TCHAR Terminator, const FString& CharsetString, int32& TextIndex)
	{
		TextIndex += 1;
		bool bEscape = false;
		for (const int32 Len = CharsetString.Len(); TextIndex < Len; ++TextIndex)
		{
			const TCHAR Char = CharsetString[TextIndex];
			if (bEscape)
			{
				Buffer.AppendChar(EscapedChar(Char));
				bEscape = false;
			}
			else
			{
				if (Char == Terminator)
					return true;

				if (Char == '\\')
				{
					bEscape = true;
				}
				else
				{
					Buffer.AppendChar(Char);
				}
			}
		}
		
		return false;
	}
	
	bool FCharset::Load(const FString& CharsetString, bool bDisableCharLiterals)
	{
		enum
		{
			CLEAR,
			TIGHT,
			RANGE_BRACKET,
			RANGE_START,
			RANGE_SEPARATOR,
			RANGE_END
		} State = CLEAR;

		FString Buffer;
		unicode_t RangeStart = 0;

		TCHAR Char =  CharsetString.Len() > 0 ? CharsetString[0] : 0;
		for (int32 TextIndex = 0, Len = CharsetString.Len(); TextIndex < Len;)
		{
			switch (Char)
			{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': // number
				if (!(State == CLEAR || State == RANGE_BRACKET || State == RANGE_SEPARATOR))
					return false;

				Char = ReadWord(Buffer, CharsetString, TextIndex);
				{
					int32 CodePoint;
					if (!ParseInt(CodePoint, Buffer))
						return false;

					switch (State)
					{
					case CLEAR:
						if (CodePoint >= 0)
						{
							Add(static_cast<unicode_t>(CodePoint));
						}
						State = TIGHT;
						break;
					case RANGE_BRACKET:
						RangeStart = static_cast<unicode_t>(CodePoint);
						State = RANGE_START;
						break;
					case RANGE_SEPARATOR:
						for (unicode_t S = RangeStart, E = CodePoint; S <= E; ++S)
						{
							Add(S);
						}
						State = RANGE_END;
						break;
					default:
						;
					}
				}
				
				Buffer.Empty();
				continue; // next character already read
			case '\'': // single UTF-8 character
				if (!(State == CLEAR || State == RANGE_BRACKET || State == RANGE_SEPARATOR) || bDisableCharLiterals)
					return false;
				
				if (!ReadString(Buffer, '\'', CharsetString, TextIndex))
					return false;
				
				if (Buffer.Len() == 1)
				{
					switch (State)
					{
					case CLEAR:
						if (Buffer[0] > 0)
							Add(Buffer[0]);
						State = TIGHT;
						break;
					case RANGE_BRACKET:
						RangeStart = Buffer[0];
						State = RANGE_START;
						break;
					case RANGE_SEPARATOR:
						for (unicode_t S = RangeStart, E = Buffer[0]; S <= E; ++S)
						{
							Add(S);
						}
						State = RANGE_END;
						break;
					default:
						;
					}
				}
				else
				{
					return false;
				}
				
				Buffer.Empty();
				break;
			case '"': // string of UTF-8 characters
				if (State != CLEAR || bDisableCharLiterals)
					return false;
				
				if (!ReadString(Buffer, '"', CharsetString, TextIndex))
					return false;

				for (int32 Index = 0, BufferLen = Buffer.Len(); Index < BufferLen; ++Index)
				{
					Add(Buffer[Index]);
				}
				
				Buffer.Empty();
				State = TIGHT;
				break;
			case '[': // character range start
				if (State != CLEAR)
					return false;
				State = RANGE_BRACKET;
				break;
			case ']': // character range end
				if (State == RANGE_END)
					State = TIGHT;
				else
					return false;
				break;
			case ',':
			case ';': // separator
				if (!(State == CLEAR || State == TIGHT))
				{
					if (State == RANGE_START)
						State = RANGE_SEPARATOR;
					else
						return false;
				} // else treat as whitespace
			case ' ':
			case '\n':
			case '\r':
			case '\t': // whitespace
				if (State == TIGHT)
					State = CLEAR;
				break;
			default: // unexpected character
				return false;
			};
			
			++TextIndex;
			if (TextIndex < Len)
			{
				Char = CharsetString[TextIndex];
			}
		}
		
		return State == CLEAR || State == TIGHT;
	}
}
