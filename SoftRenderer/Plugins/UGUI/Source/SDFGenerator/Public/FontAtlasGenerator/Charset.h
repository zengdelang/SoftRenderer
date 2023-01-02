#pragma once

#include "FontAtlasGenerator.h"

namespace FontAtlasGenerator
{
    typedef uint32 unicode_t;

	class FCharset;

	class FCharsetCollector
	{
	public:
		TArray<FCharset> Charsets;

		bool ContainsCodePoint(unicode_t CodePoint, EFontStyle FontStyle);
	};
    
    /**
     * Represents a set of Unicode codepoints (characters)
     */
    class FCharset
    {
    	friend class FCharsetCollector;
    	
    public:
        /// The set of the 95 printable ASCII characters
        static const FCharset ASCII;

        /**
         * Adds a codepoint
         */
        void Add(unicode_t CodePoint);

		const FString& GetCharsetString() const { return Charset; }

    	int32 Num() const { return CodePoints.Num(); }
    	
        /**
         * Load character set from a string with the correct syntax
         */
        bool Load(const FString& CharsetString, bool bDisableCharLiterals = false);

    private:
        TSet<unicode_t> CodePoints;
        FString Charset;

    public:
    	EFontStyle FontStyle = EFontStyle::Normal;
    	FCharsetCollector* CharsetCollector = nullptr;
    };
}
