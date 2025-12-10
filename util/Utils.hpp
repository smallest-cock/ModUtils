#pragma once
#include "pch.h"
#include <unordered_set>
#include <array>

#ifdef NO_RLSDK
namespace StringUtils
{
	inline std::string ToString(const std::wstring& str)
	{
		if (str.empty())
			return "";
		int32_t size = WideCharToMultiByte(CP_UTF8, 0, str.data(), -1, nullptr, 0, nullptr, nullptr);
		if (size <= 0)
			return "";
		std::string return_str(size - 1, 0);
		WideCharToMultiByte(CP_UTF8, 0, str.data(), -1, return_str.data(), size, nullptr, nullptr);
		return return_str;
	}

	inline std::wstring ToWideString(const std::string& str)
	{
		if (str.empty())
			return L"";
		int32_t size = MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, nullptr, 0);
		if (size <= 0)
			return L"";
		std::wstring return_str(size - 1, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.data(), -1, return_str.data(), size);
		return return_str;
	}
} // namespace StringUtils
#endif

namespace Memory
{
	struct PatternData
	{
		uint8_t* arrayOfBytes = nullptr;
		char*    mask         = nullptr;
		size_t   length       = 0;

		PatternData() = default;
		explicit PatternData(const std::string& sig);
		~PatternData();

		// disable copy
		PatternData(const PatternData&)            = delete;
		PatternData& operator=(const PatternData&) = delete;

		// enable move
		PatternData(PatternData&& other) noexcept;
		PatternData& operator=(PatternData&& other) noexcept;
	};

	bool      parseSig(const std::string& sig, PatternData& outPattern);
	uintptr_t findPattern(HMODULE module, const std::string& sig);
	uintptr_t findPattern(HMODULE module, const unsigned char* pattern, const char* mask);
	uintptr_t getRipRelativeAddr(uintptr_t startAddr, int offsetToDisplacementInt32);
} // namespace Memory

namespace Format
{
	constexpr std::array<char, 53> letters = {'a',
	    'b',
	    'c',
	    'd',
	    'e',
	    'f',
	    'g',
	    'h',
	    'i',
	    'j',
	    'k',
	    'l',
	    'm',
	    'n',
	    'o',
	    'p',
	    'q',
	    'r',
	    's',
	    't',
	    'u',
	    'v',
	    'w',
	    'x',
	    'y',
	    'z',
	    'A',
	    'B',
	    'C',
	    'D',
	    'E',
	    'F',
	    'G',
	    'H',
	    'I',
	    'J',
	    'K',
	    'L',
	    'M',
	    'N',
	    'O',
	    'P',
	    'Q',
	    'R',
	    'S',
	    'T',
	    'U',
	    'V',
	    'W',
	    'X',
	    'Y',
	    'Z',
	    ' '};

	void construct_label(const std::vector<int>& codes, std::string& out_str);

	std::string ToASCIIString(std::string str);

	std::string ToHexString(uintptr_t address);

	inline std::string ToHexString(HRESULT hr)
	{
		constexpr int hexDigitWidth = sizeof(HRESULT) * 2; // Each byte is 2 hex digits
		return std::format("0x{:0{}X}", static_cast<unsigned long>(hr), hexDigitWidth);
	}

	// Case-insensitive check if a substring exists in a string. Efficient, no allocations
	inline bool iContains(std::string_view text, std::string_view pattern)
	{
		auto it = std::search(text.begin(),
		    text.end(),
		    pattern.begin(),
		    pattern.end(),
		    [](unsigned char c1, unsigned char c2) { return std::tolower(c1) == std::tolower(c2); });
		return it != text.end();
	}

	std::string                       ToHexString(int32_t decimal_val, int32_t min_hex_digits);
	template <typename T> std::string ToHexString(T* ptr_to_obj) { return ToHexString(reinterpret_cast<uintptr_t>(ptr_to_obj)); }
	uintptr_t                         HexToIntPointer(const std::string& hexStr);

#ifndef NO_BAKKESMOD
	std::string LinearColorToHex(const LinearColor& color, bool use_alpha = true);
#endif

	std::string                         GenRandomString(int length);
	std::vector<std::string>            SplitStrByNewline(const std::string& input);
	std::vector<std::string>            SplitStr(const std::string& str, char delimiter);
	std::vector<std::string>            SplitStr(const std::string& str, const std::string& delimiter);
	std::vector<std::string>            splitAndTrim(const std::string& str, const std::string& delimiter);
	std::pair<std::string, std::string> SplitStringInTwo(const std::string& str, const std::string& delimiter);
	std::string                         EscapeBraces(const std::string& str);
	std::string                         EscapeQuotesHTML(const std::string& input);
	std::string                         UnescapeQuotesHTML(const std::string& input);
	std::string                         RemoveTrailingChar(std::string str, char trailingChar);
	std::string                         EscapeForHTML(const std::string& input);
	std::string                         EscapeForHTMLIncludingSpaces(const std::string& input);
	std::string                         EscapeCharForHTML(char ch);

	bool check_string_using_filters(
	    const std::string& input, const std::vector<std::string>& whitelist_terms, const std::vector<std::string>& blacklist_terms);

	std::string toCamelCase(const std::string& str);
	std::string ToLower(std::string str);
	void        ToLowerInline(std::string& str);
	std::string RemoveAllChars(std::string str, char character);
	void        RemoveAllCharsInline(std::string& str, char character);

	bool IsStringHexadecimal(std::string str);

	std::string ToHex(void* address, bool bNotation = true);
	std::string ToHex(uint64_t decimal, size_t width, bool bNotation = true);
	uint64_t    ToDecimal(const std::string& hexStr);
	std::string ToDecimal(uint64_t hex, size_t width);

	std::string ColorToHex(float colorsArray[3], bool bNotation);
	uint64_t    HexToDecimal(const std::string& hexStr);
} // namespace Format

namespace Math
{
#ifndef NO_RLSDK
	float distanceSquared(const FVector& a, const FVector& b);
#endif
} // namespace Math

namespace Helper
{
	class ScopedFlag
	{
		bool& m_flag;

	public:
		ScopedFlag(bool& f) : m_flag(f) { m_flag = true; }
		~ScopedFlag() { m_flag = false; }
	};

	class ScopedBannerLog
	{
		char     m_fill;
		uint32_t m_totalLength = 0;

	public:
		ScopedBannerLog(const std::string& label, char fill = '=', size_t totalWidth = 50) : m_fill(fill)
		{
			if (totalWidth < label.size() + 2)
				totalWidth = label.size() + 2;

			size_t coreLength   = label.size() + 2;
			size_t sideLen      = (totalWidth - coreLength) / 2;
			size_t sideLenRight = totalWidth - coreLength - sideLen;

			std::string output(sideLen, m_fill);
			output += " " + label + " ";
			output.append(sideLenRight, m_fill);

			m_totalLength = static_cast<uint32_t>(output.size());
			LOG(output);
		}

		~ScopedBannerLog() { LOG(std::string(m_totalLength, m_fill)); }
	};

	// Simple RAII scope guard
	struct ScopedCallback
	{
		std::function<void()> m_func;
		explicit ScopedCallback(std::function<void()> func) : m_func(std::move(func)) {}
		~ScopedCallback()
		{
			if (m_func)
				m_func();
		}
		// optional: disable copy
		ScopedCallback(const ScopedCallback&)            = delete;
		ScopedCallback& operator=(const ScopedCallback&) = delete;
		// allow move
		ScopedCallback(ScopedCallback&& other) noexcept : m_func(std::move(other.m_func)) { other.m_func = nullptr; }
		ScopedCallback& operator=(ScopedCallback&& other) noexcept
		{
			if (this != &other)
			{
				m_func       = std::move(other.m_func);
				other.m_func = nullptr;
			}
			return *this;
		}
	};

#ifndef NO_JSON
	std::optional<json> getJsonFromStr(const std::string& str);
#endif
} // namespace Helper

namespace Files
{
	struct ImageInfo
	{
		std::string name;
		fs::path    path;
	};

	constexpr std::array<std::string_view, 4> acceptableFormats = {".png", ".jpg", ".jpeg", ".bmp"};

	template <typename MapType> void FindImages(const fs::path& directory, MapType& imageMap, bool useStemFilename = false)
	{
		static const std::unordered_set<std::string_view> formatSet{acceptableFormats.begin(), acceptableFormats.end()};

		for (const auto& entry : fs::recursive_directory_iterator(directory))
		{
			if (!entry.is_regular_file())
				continue;

			auto extensionLower = Format::ToLower(entry.path().extension().string());
			if (formatSet.contains(extensionLower))
			{
				std::string filename = useStemFilename ? entry.path().stem().string() : entry.path().filename().string();
				imageMap[filename]   = entry.path();
			}
		}
	}

	template <typename MapType> void FindPngImages(const fs::path& directory, MapType& imageMap)
	{
		for (const auto& entry : fs::recursive_directory_iterator(directory))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".png")
			{
				std::string filename = entry.path().stem().string();
				imageMap[filename]   = entry.path();
			}
		}
	}

	void FindPngImages(const fs::path& directory, std::unordered_map<std::string, fs::path>& imageMap);
	void FindPngImages(const fs::path& directory, std::vector<ImageInfo>& image_info);
	void FindPngImages(const fs::path& directory, std::map<std::string, ImageInfo>& image_info_map);
	void OpenFolder(const fs::path& folderPath);
	void FilterLinesInFile(const fs::path& filePath, const std::string& startString);

	std::string get_text_content(const fs::path& file_path);

#ifndef NO_JSON
	json get_json(const fs::path& file_path);
	bool write_json(const fs::path& file_path, const json& j);
#endif

	void appendLineIfNotExist(const fs::path& file, const std::string& line);
} // namespace Files

#if !defined(NO_JSON) && !defined(NO_BAKKESMOD)
namespace PluginUpdates
{
	struct PluginUpdateInfo
	{
		std::string pluginName;
		std::string latestVersion;
		std::string releaseUrl;
		std::string assetDownloadUrl;
		bool        outOfDate = false;
	};

	struct PluginUpdaterInfo
	{
		std::string name                = "PluginUpdater";
		std::string prettyName          = "Plugin Updater";
		std::string existenceCheckCvar  = "pluginupdater_unload_delay";
		std::string updateCommandName   = "pluginupdater_update";
		std::string assetName           = name + ".dll";
		std::string latestReleaseApiUrl = "https://api.github.com/repos/smallest-cock/" + name + "/releases/latest";

		// derived helpers
		std::string makeUpdateCmd(const PluginUpdateInfo& updateInfo) const
		{
			return std::format("{} {} \"{}\"", updateCommandName, updateInfo.pluginName, updateInfo.assetDownloadUrl);
		}

		std::string makePluginLoadCmd() const { return std::format("plugin load {}", Format::ToLower(name)); }
	};

	extern PluginUpdaterInfo pluginUpdaterInfo;
	extern PluginUpdateInfo  updateInfo;
	extern std::mutex        updateMutex;

	void        checkForUpdates(const std::string& modName, const std::string& currentVersion, const std::string& assetName = "");
	std::string getRepoName(const std::string& modName);
	void        installUpdate(const std::shared_ptr<GameWrapper>& gw);
	void        downloadAndInstallUpdaterPlugin(std::shared_ptr<GameWrapper> gw, const std::string& updateCmd);

	std::optional<PluginUpdateInfo> getUpdateInfo(const json& releaseJson, const std::string& assetName);
	std::optional<std::string>      getAssetDownloadUrl(const json& releaseJson, const std::string& assetName);
	std::optional<std::string>      getVersionStr(const json& releaseJson);
} // namespace PluginUpdates
#endif

namespace Process
{
	struct ProcessHandles
	{
		HANDLE hProcess = NULL;
		HANDLE hThread  = NULL;

		inline bool is_active() const
		{
			return (hProcess != NULL && hProcess != INVALID_HANDLE_VALUE) || (hThread != NULL && hThread != INVALID_HANDLE_VALUE);
		}
	};

	struct CreateProcessResult
	{
		DWORD          status_code = ERROR_SUCCESS;
		ProcessHandles handles;
	};

	void close_handle(HANDLE h);
	void terminate(HANDLE h);
	void terminate_created_process(ProcessHandles& pi);

	CreateProcessResult create_process_from_command(const std::string& command);
} // namespace Process

#ifndef NO_RLSDK
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
	explicit Color(float arr[3]);                       // added by mwah
	Color(const std::string& hexColor);
	Color(const struct FColor& color);
	Color(const Color& color);
	~Color();

public:
	struct FColor           UnrealColor() const;
	class CoolerLinearColor ToLinear() const;
	uint32_t                ToDecimal() const;
	uint32_t
	ToDecimalAlpha() const; // Same as "ToDecimal" but includes the alpha channel, supported here but may not be standard elsewhere.
	std::string ToHex(bool bNotation = true) const;
	std::string ToHexAlpha(
	    bool bNotation = true) const; // Same as "ToHex" but includes the alpha channel, supported here but may not be standard elsewhere.
	Color& FromLinear(const CoolerLinearColor& linearColor);
	Color& FromDecimal(uint32_t decimalColor); // Supports both alpha and non alpha channels.
	Color& FromHex(std::string hexColor);      // Supports both alpha and non alpha channels.
	Color& Cycle(int32_t steps = 1);

public:
	Color& operator=(const Color& other);
	Color& operator=(const struct FColor& other);
	bool   operator==(const Color& other) const;
	bool   operator==(const struct FColor& other) const;
	bool   operator!=(const Color& other) const;
	bool   operator!=(const struct FColor& other) const;
	bool   operator<(const Color& other) const;
	bool   operator>(const Color& other) const;
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
	Color               ToColor() const;
	uint32_t            ToDecimal() const;
	uint32_t
	ToDecimalAlpha() const; // Same as "ToDecimal" but includes the alpha channel, supported here but may not be standard elsewhere.
	std::string ToHex(bool bNotation = true) const;
	std::string ToHexAlpha(
	    bool bNotation = true) const; // Same as "ToHex" but includes the alpha channel, supported here but may not be standard elsewhere.
	CoolerLinearColor& FromColor(const Color& color);
	CoolerLinearColor& FromDecimal(uint32_t decimalColor); // Supports both alpha and non alpha channels.
	CoolerLinearColor& FromHex(std::string hexColor);      // Supports both alpha and non alpha channels.
	CoolerLinearColor& Cycle(int32_t steps = 1);

public:
	CoolerLinearColor& operator=(const CoolerLinearColor& other);
	CoolerLinearColor& operator=(const struct FLinearColor& other);
	bool               operator==(const CoolerLinearColor& other) const;
	bool               operator==(const struct FLinearColor& other) const;
	bool               operator!=(const CoolerLinearColor& other) const;
	bool               operator!=(const struct FLinearColor& other) const;
	bool               operator<(const CoolerLinearColor& other) const;
	bool               operator>(const CoolerLinearColor& other) const;
};

namespace std
{
	template <> struct hash<Color>
	{
		size_t operator()(const Color& other) const { return hash<string>()(other.ToHexAlpha(false)); }
	};

	template <> struct hash<CoolerLinearColor>
	{
		size_t operator()(const CoolerLinearColor& other) const { return hash<string>()(other.ToHexAlpha(false)); }
	};
} // namespace std

// This is a global rainbow color class, hook your own function to the "Tick" function for it to update.
// This means you can sync up multiple objects to cycle through RGB at the same rate.
class GRainbowColor
{
private:
	static inline Color             ByteRainbow   = Color(0, 0, 255, 255);
	static inline CoolerLinearColor LinearRainbow = CoolerLinearColor(0.0f, 0.0f, 1.0f, 1.0f);
	static inline uint32_t          tickCounter   = 0; // custom shit

public:
	static Color             GetByte();
	static CoolerLinearColor GetLinear();
	static FLinearColor      GetFLinear();
	static int32_t           GetDecimal();
	static void              Reset();
	static void              OnTick();
	static void              IncrementTick();                      // custom shit
	static bool              TickCountIsMultipleOf(int num);       // custom shit
	static void              TickRGB(int speed, int defaultSpeed); // custom shit
};

// Inline helper functions for different color type conversions.
namespace Colors
{
	uint32_t    packColor(const FColor& col);
	FColor      unpackColor(uint32_t packed);
	std::string fcolorToHex(const FColor& col);
	FColor      hexToFColor(const std::string& hex);

	inline std::string logColor(const FLinearColor& col) { return std::format("R:{}-G:{}-B:{}-A:{}", col.R, col.G, col.B, col.A); }
	inline std::string logColor(const FColor& col) { return std::format("R:{}-G:{}-B:{}-A:{}", col.R, col.G, col.B, col.A); }

#ifndef NO_BAKKESMOD
	FLinearColor CvarColorToFLinearColor(const LinearColor& cvarColor);
	uint32_t     CvarColorToInt(const LinearColor& cvarColor);
#endif
	int32_t     FLinearColorToInt(const FLinearColor& color);
	std::string fcolorToHexRGBA(const FColor& col);
	FColor      hexRGBAtoFColor(const std::string& hex);

	inline FColor fLinearColorToFColor(const FLinearColor& color)
	{
		return FColor{static_cast<uint8_t>(color.B * 255),
		    static_cast<uint8_t>(color.G * 255),
		    static_cast<uint8_t>(color.R * 255),
		    static_cast<uint8_t>(color.A * 255)}; // BGRA
	}

	inline FLinearColor fColorToFLinearColor(const FColor& color)
	{
		return FLinearColor{static_cast<float>(color.R) / 255.0f,
		    static_cast<float>(color.G) / 255.0f,
		    static_cast<float>(color.B) / 255.0f,
		    static_cast<float>(color.A) / 255.0f}; // RGBA
	}

	inline uint8_t toByte(float val) { return static_cast<uint8_t>(std::clamp(std::round(val * 255.0f), 0.0f, 255.0f)); }

	// assumes an RGBA float array
	inline FColor toFColor(const float (&col)[4])
	{
		FColor fCol{};
		fCol.R = toByte(col[0]);
		fCol.G = toByte(col[1]);
		fCol.B = toByte(col[2]);
		fCol.A = toByte(col[3]);
		return fCol;
	}

	// Swizzle pixel data (i.e. RGBA -> BGRA) <numChannels, channelA, channelB> ... requires 8-bit channels
	template <uint8_t numChannels, uint8_t channelA, uint8_t channelB> void swizzleChannels(uint8_t* pixelData, size_t numPixels)
	{
		for (size_t i = 0; i < numPixels; ++i)
		{
			uint8_t* pixel = &pixelData[i * numChannels];
			std::swap(pixel[channelA], pixel[channelB]);
		}
	}

	inline void rgbaToBGRASwizzle(uint8_t* pixelData, size_t numPixels) { swizzleChannels<4, 0, 2>(pixelData, numPixels); }

	// Color to Decimal/Base10
	inline uint32_t HexToDecimal(std::string hexStr) { return Color(hexStr).ToDecimal(); }
	inline uint32_t ColorToDecimal(const Color& color) { return color.ToDecimal(); }
	inline uint32_t LinearToDecimal(const CoolerLinearColor& linearColor) { return linearColor.ToDecimal(); }

	// Decimal/Base10 to Color
	inline Color             DecimalToColor(uint32_t decimal) { return Color().FromDecimal(decimal); }
	inline CoolerLinearColor DecimalToLinear(uint32_t decimal) { return CoolerLinearColor().FromDecimal(decimal); }

	// Color to Hexidecimal/Base16
	inline std::string DecimalToHex(uint32_t decimal, bool bNotation = true) { return Color().FromDecimal(decimal).ToHex(bNotation); }
	inline std::string ColorToHex(const Color& color, bool bNotation = true) { return color.ToHex(bNotation); }
	inline std::string LinearToHex(const CoolerLinearColor& linearColor, bool bNotation = true) { return linearColor.ToHex(bNotation); }

	// Hexidecimal/Base16 to Color
	inline Color             HexToColor(std::string hexStr) { return Color(hexStr); }
	inline CoolerLinearColor HexToLinear(std::string hexStr) { return CoolerLinearColor(hexStr); }

	// Direct Color Conversions
	inline Color             LinearToColor(const CoolerLinearColor& linearColor) { return linearColor.ToColor(); }
	inline CoolerLinearColor ColorToLinear(const Color& color) { return color.ToLinear(); }
} // namespace Colors

#endif // NO_RLSDK
