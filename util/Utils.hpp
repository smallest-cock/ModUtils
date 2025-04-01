#pragma once
#include "pch.h"
#include <random>
#include <codecvt>

namespace Format
{
	std::string ToASCIIString(std::string str);

	std::string ToHexString(uintptr_t address);
	std::string ToHexString(int32_t decimal_val, int32_t min_hex_digits);
	template <typename T>
	std::string ToHexString(T* ptr_to_obj)
	{
		return ToHexString(reinterpret_cast<uintptr_t>(ptr_to_obj));
	}

	std::string LinearColorToHex(const LinearColor& color);

	std::string GenRandomString(int length);
	std::vector<std::string> SplitStrByNewline(const std::string& input);
	std::vector<std::string> SplitStr(const std::string& str, const std::string& delimiter);
	std::string EscapeBraces(const std::string& str);
	std::string EscapeQuotesHTML(const std::string& input);
	std::string UnescapeQuotesHTML(const std::string& input);

	bool check_string_using_filters(const std::string& input, const std::vector<std::string>& whitelist_terms, const std::vector<std::string>& blacklist_terms);


	std::string ToLower(std::string str);
	void ToLowerInline(std::string& str);
	std::string RemoveAllChars(std::string str, char character);
	void RemoveAllCharsInline(std::string& str, char character);

	bool IsStringHexadecimal(std::string str);

	std::string ToHex(void* address, bool bNotation = true);
	std::string ToHex(uint64_t decimal, size_t width, bool bNotation = true);
	uint64_t ToDecimal(const std::string& hexStr);
	std::string ToDecimal(uint64_t hex, size_t width);

	std::string ColorToHex(float colorsArray[3], bool bNotation);
	uint64_t HexToDecimal(const std::string& hexStr);
}


namespace Files 
{
	struct ImageInfo
	{
		std::string name;
		fs::path path;
	};

	void FindPngImages(const fs::path& directory, std::unordered_map<std::string, fs::path>& imageMap);
	void FindPngImages(const fs::path& directory, std::vector<ImageInfo>& image_info);
	void FindPngImages(const fs::path& directory, std::map<std::string, ImageInfo>& image_info_map);
	void OpenFolder(const fs::path& folderPath);
	void FilterLinesInFile(const fs::path& filePath, const std::string& startString);
	
	std::string get_text_content(const fs::path& file_path);
	json get_json(const fs::path& file_path);
	bool write_json(const fs::path& file_path, const json& j);
}


namespace Process
{
	struct ProcessHandles
	{
		HANDLE hProcess =	NULL;
		HANDLE hThread =	NULL;

		inline bool is_active() const
		{
			return (hProcess != NULL && hProcess != INVALID_HANDLE_VALUE) || (hThread != NULL && hThread != INVALID_HANDLE_VALUE);
		}
	};

	struct CreateProcessResult
	{
		DWORD status_code = ERROR_SUCCESS;
		ProcessHandles handles;
	};


	void close_handle(HANDLE h);
	void terminate(HANDLE h);
	void terminate_created_process(ProcessHandles& pi);

	CreateProcessResult create_process_from_command(const std::string& command);
}


class Color
{
public:
	uint8_t R, G, B, A;

public:
	Color();
	explicit Color(uint8_t rgba);
	explicit Color(int32_t rgba);
	explicit Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	explicit Color(int32_t r, int32_t g, int32_t b, int32_t a);
	explicit Color(float r, float g, float b, float a); // Auto converts linear color values to bytes.
	explicit Color(float arr[3]);	// added by mwah
	Color(const std::string& hexColor);
	Color(const struct FColor& color);
	Color(const Color& color);
	~Color();

public:
	struct FColor UnrealColor() const; 
	class CoolerLinearColor ToLinear() const;
	uint32_t ToDecimal() const;
	uint32_t ToDecimalAlpha() const; // Same as "ToDecimal" but includes the alpha channel, supported here but may not be standard elsewhere.
	std::string ToHex(bool bNotation = true) const;
	std::string ToHexAlpha(bool bNotation = true) const; // Same as "ToHex" but includes the alpha channel, supported here but may not be standard elsewhere.
	Color& FromLinear(const CoolerLinearColor& linearColor);
	Color& FromDecimal(uint32_t decimalColor); // Supports both alpha and non alpha channels.
	Color& FromHex(std::string hexColor); // Supports both alpha and non alpha channels.
	Color& Cycle(int32_t steps = 1);

public:
	Color& operator=(const Color& other);
	Color& operator=(const struct FColor& other);
	bool operator==(const Color& other) const;
	bool operator==(const struct FColor& other) const;
	bool operator!=(const Color& other) const;
	bool operator!=(const struct FColor& other) const;
	bool operator<(const Color& other) const;
	bool operator>(const Color& other) const;
};


class CoolerLinearColor
{
public:
	float R, G, B, A;

public:
	CoolerLinearColor();
	explicit CoolerLinearColor(float rgba);
	explicit CoolerLinearColor(float r, float g, float b, float a);
	explicit CoolerLinearColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a); // Auto converts byte color values to floats.
	CoolerLinearColor(const std::string& hexColor);
	CoolerLinearColor(const struct FLinearColor& linearColor);
	CoolerLinearColor(const CoolerLinearColor& linearColor);
	~CoolerLinearColor();

public:
	struct FLinearColor UnrealColor() const;
	Color ToColor() const;
	uint32_t ToDecimal() const;
	uint32_t ToDecimalAlpha() const; // Same as "ToDecimal" but includes the alpha channel, supported here but may not be standard elsewhere.
	std::string ToHex(bool bNotation = true) const;
	std::string ToHexAlpha(bool bNotation = true) const; // Same as "ToHex" but includes the alpha channel, supported here but may not be standard elsewhere.
	CoolerLinearColor& FromColor(const Color& color);
	CoolerLinearColor& FromDecimal(uint32_t decimalColor); // Supports both alpha and non alpha channels.
	CoolerLinearColor& FromHex(std::string hexColor); // Supports both alpha and non alpha channels.
	CoolerLinearColor& Cycle(int32_t steps = 1);

public:
	CoolerLinearColor& operator=(const CoolerLinearColor& other);
	CoolerLinearColor& operator=(const struct FLinearColor& other);
	bool operator==(const CoolerLinearColor& other) const;
	bool operator==(const struct FLinearColor& other) const;
	bool operator!=(const CoolerLinearColor& other) const;
	bool operator!=(const struct FLinearColor& other) const;
	bool operator<(const CoolerLinearColor& other) const;
	bool operator>(const CoolerLinearColor& other) const;
};


namespace std
{
	template<>
	struct hash<Color>
	{
		size_t operator()(const Color& other) const
		{
			return hash<string>()(other.ToHexAlpha(false));
		}
	};

	template<>
	struct hash<CoolerLinearColor>
	{
		size_t operator()(const CoolerLinearColor& other) const
		{
			return hash<string>()(other.ToHexAlpha(false));
		}
	};
}


// This is a global rainbow color class, hook your own function to the "Tick" function for it to update.
// This means you can sync up multiple objects to cycle through RGB at the same rate.
class GRainbowColor
{
private:
	static inline Color ByteRainbow = Color(0, 0, 255, 255);
	static inline CoolerLinearColor LinearRainbow = CoolerLinearColor(0.0f, 0.0f, 1.0f, 1.0f);
	static inline uint32_t tickCounter = 0;				// custom shit

public:
	static Color GetByte();
	static CoolerLinearColor GetLinear();
	static int32_t GetDecimal();
	static void Reset();
	static void OnTick();
	static void IncrementTick();						// custom shit
	static bool TickCountIsMultipleOf(int num);			// custom shit
	static void TickRGB(int speed, int defaultSpeed);	// custom shit
};


// Inline helper functions for different color type conversions.
namespace Colors
{
	FLinearColor CvarColorToFLinearColor(const LinearColor& cvarColor);
	FColor FLinearColorToFColor(const FLinearColor& color);
	int32_t FLinearColorToInt(const FLinearColor& color);

	// Color to Decimal/Base10
	inline uint32_t HexToDecimal(std::string hexStr) { return Color(hexStr).ToDecimal(); }
	inline uint32_t ColorToDecimal(const Color& color) { return color.ToDecimal(); }
	inline uint32_t LinearToDecimal(const CoolerLinearColor& linearColor) { return linearColor.ToDecimal(); }

	// Decimal/Base10 to Color
	inline Color DecimalToColor(uint32_t decimal) { return Color().FromDecimal(decimal); }
	inline CoolerLinearColor DecimalToLinear(uint32_t decimal) { return CoolerLinearColor().FromDecimal(decimal); }

	// Color to Hexidecimal/Base16
	inline std::string DecimalToHex(uint32_t decimal, bool bNotation = true) { return Color().FromDecimal(decimal).ToHex(bNotation); }
	inline std::string ColorToHex(const Color& color, bool bNotation = true) { return color.ToHex(bNotation); }
	inline std::string LinearToHex(const CoolerLinearColor& linearColor, bool bNotation = true) { return linearColor.ToHex(bNotation); }

	// Hexidecimal/Base16 to Color
	inline Color HexToColor(std::string hexStr) { return Color(hexStr); }
	inline CoolerLinearColor HexToLinear(std::string hexStr) { return CoolerLinearColor(hexStr); }

	// Direct Color Conversions
	inline Color LinearToColor(const CoolerLinearColor& linearColor) { return linearColor.ToColor(); }
	inline CoolerLinearColor ColorToLinear(const Color& color) { return color.ToLinear(); }
}