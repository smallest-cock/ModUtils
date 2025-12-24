#include "pch.h"
#include "Utils.hpp"
#include <optional>
#include <random>
#include <regex>
#include <shellapi.h>
#pragma comment(lib, "Shlwapi.lib")

namespace Memory
{
	PatternData::~PatternData()
	{
		delete[] arrayOfBytes;
		delete[] mask;
	}

	PatternData::PatternData(const std::string& sig)
	{
		if (!parseSig(sig, *this))
		{
			arrayOfBytes = nullptr;
			mask         = nullptr;
			length       = 0;
		}
	}

	PatternData::PatternData(PatternData&& other) noexcept
	{
		arrayOfBytes       = other.arrayOfBytes;
		mask               = other.mask;
		length             = other.length;
		other.arrayOfBytes = nullptr;
		other.mask         = nullptr;
		other.length       = 0;
	}

	PatternData& PatternData::operator=(PatternData&& other) noexcept
	{
		if (this != &other)
		{
			delete[] arrayOfBytes;
			delete[] mask;

			arrayOfBytes = other.arrayOfBytes;
			mask         = other.mask;
			length       = other.length;

			other.arrayOfBytes = nullptr;
			other.mask         = nullptr;
			other.length       = 0;
		}
		return *this;
	}

	// Parses a pretty pattern string into an AOB + mask
	bool parseSig(const std::string& sig, PatternData& outPattern)
	{
		size_t   maxLen   = sig.size() / 2 + 1;
		uint8_t* tmpBytes = new uint8_t[maxLen];
		char*    tmpMask  = new char[maxLen + 1];

		size_t  byteIndex   = 0;
		int     nibbleCount = 0;
		uint8_t currentByte = 0;

		auto hexToNibble = [](char c) -> int
		{
			if (c >= '0' && c <= '9')
				return c - '0';
			if (c >= 'A' && c <= 'F')
				return c - 'A' + 10;
			if (c >= 'a' && c <= 'f')
				return c - 'a' + 10;
			return -1;
		};

		for (size_t i = 0; i < sig.size(); ++i)
		{
			char c = sig[i];

			if (std::isspace(static_cast<unsigned char>(c)))
				continue;

			if (c == '?')
			{
				if (nibbleCount == 1)
				{
					tmpBytes[byteIndex]  = 0x00;
					tmpMask[byteIndex++] = '?';
					nibbleCount          = 0;
				}
				else
				{
					if (i + 1 < sig.size() && sig[i + 1] == '?')
						++i;
					tmpBytes[byteIndex]  = 0x00;
					tmpMask[byteIndex++] = '?';
				}
				continue;
			}

			int nibble = hexToNibble(c);
			if (nibble == -1)
			{
				delete[] tmpBytes;
				delete[] tmpMask;
				return false;
			}

			if (nibbleCount == 0)
			{
				currentByte = nibble << 4;
				nibbleCount = 1;
			}
			else
			{
				currentByte |= nibble;
				tmpBytes[byteIndex]  = currentByte;
				tmpMask[byteIndex++] = 'x';
				nibbleCount          = 0;
			}
		}

		if (nibbleCount == 1)
		{
			tmpBytes[byteIndex]  = 0x00;
			tmpMask[byteIndex++] = '?';
		}

		delete[] outPattern.arrayOfBytes;
		delete[] outPattern.mask;

		outPattern.arrayOfBytes = new uint8_t[byteIndex];
		outPattern.mask         = new char[byteIndex + 1];
		outPattern.length       = byteIndex;

		std::memcpy(outPattern.arrayOfBytes, tmpBytes, byteIndex);
		std::memcpy(outPattern.mask, tmpMask, byteIndex);
		outPattern.mask[byteIndex] = '\0';

		delete[] tmpBytes;
		delete[] tmpMask;
		return true;
	}

	uintptr_t findPattern(HMODULE module, const std::string& pattern)
	{
		Memory::PatternData sig{pattern};
		if (sig.length == 0)
		{
			LOGERROR("Unable to parse sig! Returning 0...");
			return 0;
		}
		return findPattern(module, sig.arrayOfBytes, sig.mask);
	}

	uintptr_t findPattern(HMODULE module, const unsigned char* pattern, const char* mask)
	{
		MODULEINFO info = {};
		GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(MODULEINFO));

		uintptr_t start  = reinterpret_cast<uintptr_t>(module);
		size_t    length = info.SizeOfImage;

		size_t pos        = 0;
		size_t maskLength = std::strlen(mask) - 1;

		for (uintptr_t retAddress = start; retAddress < start + length; retAddress++)
		{
			if (*reinterpret_cast<unsigned char*>(retAddress) == pattern[pos] || mask[pos] == '?')
			{
				if (pos == maskLength)
					return (retAddress - maskLength);
				pos++;
			}
			else
			{
				retAddress -= pos;
				pos = 0;
			}
		}
		return NULL;
	}

	uintptr_t getRipRelativeAddr(uintptr_t startAddr, int offsetToDisplacementInt32)
	{
		if (!startAddr)
			return 0;
		uintptr_t ripRelativeOffsetAddr = startAddr + offsetToDisplacementInt32;
		int32_t   displacement          = *reinterpret_cast<int32_t*>(ripRelativeOffsetAddr);
		return (ripRelativeOffsetAddr + 4) + displacement;
	}
} // namespace Memory

namespace Format
{
	void construct_label(const std::vector<int>& codes, std::string& out_str)
	{
		out_str.clear();

		for (int code : codes)
		{
			out_str += letters[code];
		}
	}

	std::string ToASCIIString(std::string str)
	{
		// Remove non-ASCII characters
		str.erase(std::remove_if(str.begin(), str.end(), [](unsigned char c) { return c > 127; }), str.end());

		return str;
	}

	std::string ToHexString(uintptr_t address)
	{
		// Adjust width based on the platform's pointer size
		constexpr int pointerWidth = sizeof(uintptr_t) * 2; // Each byte is 2 hex digits
		return std::format("0x{:0{}X}", address, pointerWidth);
	}

	std::string ToHexString(int32_t decimal_val, int32_t min_hex_digits) { return std::format("0x{:0{}X}", decimal_val, min_hex_digits); }

	uintptr_t HexToIntPointer(const std::string& hexStr)
	{
		uintptr_t         decimal = NULL;
		std::stringstream stream;

		stream << std::hex << hexStr; // Load the hex string into the stream for conversion

		// Attempt conversion and handle errors
		if (!(stream >> decimal))
		{
			LOG("[ERROR] Invalid hexadecimal string: " + hexStr);
			return NULL;
		}

		return decimal;
	}

#ifndef NO_BAKKESMOD
	std::string LinearColorToHex(const LinearColor& color, bool use_alpha)
	{
		// Create a stringstream to format the hex string
		std::stringstream ss;
		ss << "#";

		// Convert each color component (R, G, B, A) to a 2-digit hex string
		ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(color.R);
		ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(color.G);
		ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(color.B);
		if (use_alpha)
			ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(color.A);

		// Return the formatted string
		return ss.str();
	}
#endif

	std::string GenRandomString(int length)
	{
		// Define character set
		const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

		// Initialize random number generator
		std::random_device                 rd;
		std::mt19937                       gen(rd());
		std::uniform_int_distribution<int> distr(0, charset.length() - 1);

		// Generate random string
		std::string randomString;
		randomString.reserve(length);
		for (int i = 0; i < length; ++i)
			randomString += charset[distr(gen)];

		return randomString;
	}

	std::vector<std::string> SplitStrByNewline(const std::string& input)
	{
		std::vector<std::string> lines;
		std::istringstream       iss(input);
		std::string              line;

		// Read each line using std::getline with '\n' as delimiter
		while (std::getline(iss, line))
			lines.push_back(line);

		return lines;
	}

	std::vector<std::string> SplitStr(const std::string& str, char delimiter)
	{
		std::vector<std::string> parts;
		std::stringstream        ss(str);
		std::string              item;
		while (std::getline(ss, item, delimiter))
			parts.push_back(item);
		return parts;
	}

	std::vector<std::string> SplitStr(const std::string& str, const std::string& delimiter)
	{
		std::vector<std::string> tokens;
		size_t                   start = 0;
		size_t                   end   = str.find(delimiter);

		while (end != std::string::npos)
		{
			tokens.push_back(str.substr(start, end - start));
			start = end + delimiter.length();
			end   = str.find(delimiter, start);
		}

		// Add the last token
		tokens.push_back(str.substr(start, end - start));

		return tokens;
	}

	std::vector<std::string> splitAndTrim(const std::string& str, const std::string& delimiter)
	{
		auto                     terms = Format::SplitStr(str, delimiter);
		std::vector<std::string> result;

		for (auto term : terms)
		{
			if (term.empty())
				continue;

			// Trim left
			term.erase(term.begin(), std::ranges::find_if(term, [](unsigned char ch) { return !std::isspace(ch); }));
			// Trim right
			term.erase(std::find_if(term.rbegin(), term.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), term.end());

			if (!term.empty()) // last check after trimming
				result.push_back(term);
		}
		return result;
	};

	std::pair<std::string, std::string> SplitStringInTwo(const std::string& str, const std::string& delimiter)
	{
		size_t pos = str.find(delimiter);
		if (pos == std::string::npos)
			return {str, ""}; // If delimiter is not found, return the original string and an empty string.

		std::string left  = str.substr(0, pos);
		std::string right = str.substr(pos + delimiter.length());

		return {left, right};
	}

	std::string EscapeBraces(const std::string& str)
	{
		std::string escaped;
		for (char ch : str)
		{
			if (ch == '{' || ch == '}')
				escaped += ch; // Add an extra brace to escape it
			escaped += ch;
		}
		return escaped;
	}

	std::string EscapeQuotesHTML(const std::string& input)
	{
		std::string escapedString;
		for (char ch : input)
		{
			if (ch == '"')
				escapedString += "******"; // Replace " with ******
			else if (ch == '#')
				escapedString += "$$$$$$"; // Replace # with $$$$$$
			else
				escapedString += ch;
		}
		return escapedString;
	}

	std::string UnescapeQuotesHTML(const std::string& input)
	{
		std::string unescapedString;
		size_t      pos = 0;

		// Find and replace ****** with "
		while (pos < input.length())
		{
			size_t found = input.find("******", pos);
			if (found != std::string::npos)
			{
				unescapedString += input.substr(pos, found - pos); // Add characters before &quot;
				unescapedString += '"';                            // Replace ****** with "
				pos = found + 6;                                   // Move past "******"
			}
			else
			{
				unescapedString += input.substr(pos); // Add remaining characters
				break;
			}
		}

		std::string okPal;
		pos = 0;

		// Find and replace $$$$$$ with #
		while (pos < unescapedString.length())
		{
			size_t found = unescapedString.find("$$$$$$", pos);
			if (found != std::string::npos)
			{
				okPal += unescapedString.substr(pos, found - pos); // Add characters before &quot;
				okPal += '#';                                      // Replace $$$$$$ with #
				pos = found + 6;                                   // Move past "$$$$$$"
			}
			else
			{
				okPal += unescapedString.substr(pos); // Add remaining characters
				break;
			}
		}

		return okPal;
	}

	std::string RemoveTrailingChar(std::string str, char trailingChar)
	{
		if (!str.empty() && str.back() == trailingChar)
			str.pop_back(); // Removes the last character

		return str;
	}

	std::string EscapeForHTML(const std::string& input)
	{
		std::string output;
		output.reserve(input.size()); // reserve space to reduce reallocations

		for (char ch : input)
		{
			switch (ch)
			{
			case '&':
				output += "&amp;";
				break;
			case '<':
				output += "&lt;";
				break;
			case '>':
				output += "&gt;";
				break;
			case '"':
				output += "&quot;";
				break;
			case '\'':
				output += "&apos;";
				break;
			default:
				output += ch;
				break;
			}
		}

		return output;
	}

	std::string EscapeForHTMLIncludingSpaces(const std::string& input)
	{
		std::string output;
		output.reserve(input.size()); // reserve space to reduce reallocations

		for (char ch : input)
		{
			switch (ch)
			{
			case '&':
				output += "&amp;";
				break;
			case ' ':
				output += "+";
				break;
			case '<':
				output += "&lt;";
				break;
			case '>':
				output += "&gt;";
				break;
			case '"':
				output += "&quot;";
				break;
			case '\'':
				output += "&apos;";
				break;
			default:
				output += ch;
				break;
			}
		}

		return output;
	}

	std::string EscapeCharForHTML(char ch)
	{
		switch (ch)
		{
		case '&':
			return "&amp;";
		case '<':
			return "&lt;";
		case '>':
			return "&gt;";
		case '"':
			return "&quot;";
		case '\'':
			return "&apos;";
		default:
			return std::string(1, ch); // Wrap single char into string
		}
	}

	bool check_string_using_filters(
	    const std::string& input, const std::vector<std::string>& whitelist_terms, const std::vector<std::string>& blacklist_terms)
	{
		if (!whitelist_terms.empty())
		{
			// Search for a whitelisted term
			for (const auto& term : whitelist_terms)
			{
				auto foundPos = input.find(term);
				if (foundPos != std::string::npos)
				{
					// If a whitelisted term is found, check if any blacklisted term appears
					for (const auto& bTerm : blacklist_terms)
					{
						if (input.find(bTerm) != std::string::npos)
							return false; // Found a blacklisted term, return false
					}
					return true; // No blacklisted term found, return true
				}
			}
			return false; // No whitelisted term found
		}
		else
		{
			for (const auto& bTerm : blacklist_terms)
			{
				if (input.find(bTerm) != std::string::npos)
					return false; // Found a blacklisted term, return false
			}
			return true;
		}
	}

	std::string toCamelCase(const std::string& str)
	{
		std::string result;
		bool        capitalizeNext = false;

		for (char ch : str)
		{
			if (ch == ' ' || ch == '_')
				capitalizeNext = true;
			else
			{
				if (result.empty())
					result += std::tolower(ch); // Always lowercase the first character
				else if (capitalizeNext)
				{
					result += std::toupper(ch);
					capitalizeNext = false;
				}
				else
					result += std::tolower(ch);
			}
		}

		return result;
	}

	std::string ToLower(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), tolower);
		return str;
	}

	void ToLowerInline(std::string& str) { std::transform(str.begin(), str.end(), str.begin(), tolower); }

	std::string RemoveAllChars(std::string str, char character)
	{
		str.erase(std::remove(str.begin(), str.end(), character), str.end());
		return str;
	}

	void RemoveAllCharsInline(std::string& str, char character) { str.erase(std::remove(str.begin(), str.end(), character), str.end()); }

	bool IsStringHexadecimal(std::string str)
	{
		if (str.empty())
			return false;

		ToLowerInline(str);

		bool first    = true;
		bool negative = false;
		bool found    = false;

		for (char c : str)
		{
			if (first)
			{
				first    = false;
				negative = (c == '-');
			}

			if (std::isxdigit(c))
				found = true;
			else if (!negative)
				return false;
		}

		return found;
	}

	std::string ToHex(void* address, bool bNotation) { return ToHex(reinterpret_cast<uint64_t>(address), sizeof(uint64_t), bNotation); }

	std::string ToHex(uint64_t decimal, size_t width, bool bNotation)
	{
		std::ostringstream stream;
		if (bNotation)
			stream << "0x";
		stream << std::setfill('0') << std::setw(width) << std::right << std::uppercase << std::hex << decimal;
		return stream.str();
	}

	uint64_t ToDecimal(const std::string& hexStr)
	{
		uint64_t          decimal = 0;
		std::stringstream stream;
		stream << std::right << std::uppercase << std::hex << RemoveAllChars(hexStr, '#');
		stream >> decimal;
		return decimal;
	}

	std::string ToDecimal(uint64_t hex, size_t width)
	{
		std::ostringstream stream;
		stream << std::setfill('0') << std::setw(width) << std::right << std::uppercase << std::dec << hex;
		return stream.str();
	}

	std::string ColorToHex(float colorArray[3], bool bNotation)
	{
		std::string hexStr = (bNotation ? "#" : "");
		hexStr += Format::ToHex(static_cast<uint64_t>(colorArray[0]), 2, false);
		hexStr += Format::ToHex(static_cast<uint64_t>(colorArray[1]), 2, false);
		hexStr += Format::ToHex(static_cast<uint64_t>(colorArray[2]), 2, false);
		return hexStr;
	}

	uint64_t HexToDecimal(const std::string& hexStr)
	{
		uint64_t          decimal = 0;
		std::stringstream stream;
		stream << std::right << std::uppercase << std::hex << RemoveAllChars(hexStr, '#');
		stream >> decimal;
		return decimal;
	}
} // namespace Format

namespace Math
{
#ifndef NO_RLSDK
	float distanceSquared(const FVector& a, const FVector& b)
	{
		const float dx = a.X - b.X;
		const float dy = a.Y - b.Y;
		const float dz = a.Z - b.Z;
		return dx * dx + dy * dy + dz * dz;
	}
#endif // NO_RLSDK
} // namespace Math

namespace Helper
{
#ifndef NO_JSON
	std::optional<json> getJsonFromStr(const std::string& str)
	{
		try
		{
			return json::parse(str);
		}
		catch (...)
		{
			LOGERROR("Unable to parse JSON!");
			return std::nullopt;
		}
	}
#endif
} // namespace Helper

namespace Files
{
	void FindPngImages(const fs::path& directory, std::unordered_map<std::string, fs::path>& imageMap)
	{
		for (const auto& entry : fs::recursive_directory_iterator(directory))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".png")
			{
				// Get the filename without extension
				std::string filename = entry.path().stem().string();
				// Get the full path
				fs::path filepath = entry.path();
				// Add to map
				imageMap[filename] = filepath;
			}
		}
	}

	void FindPngImages(const fs::path& directory, std::vector<ImageInfo>& image_info)
	{
		for (const auto& entry : fs::recursive_directory_iterator(directory))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".png")
			{
				fs::path    filepath = entry.path();
				std::string filename = filepath.stem().string();
				image_info.emplace_back(filename, filepath);
			}
		}
	}

	void FindPngImages(const fs::path& directory, std::map<std::string, ImageInfo>& image_info_map)
	{
		for (const auto& entry : fs::recursive_directory_iterator(directory))
		{
			if (entry.is_regular_file() && entry.path().extension() == ".png")
			{
				fs::path    filepath     = entry.path();
				std::string filename     = filepath.stem().string();
				image_info_map[filename] = {filename, filepath};
			}
		}
	}

	void OpenFolder(const fs::path& folderPath)
	{
		if (fs::exists(folderPath))
			ShellExecute(NULL, L"open", folderPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		else
			LOGERROR("Folder path does not exist: {}", folderPath.string());
	}

	void FilterLinesInFile(const fs::path& filePath, const std::string& startString)
	{
		std::fstream file(filePath, std::ios::in | std::ios::out);

		if (!file.is_open())
		{
			LOGERROR("Unable to open file {}", filePath.string());
			return;
		}

		std::string   line;
		std::ofstream tempFile("temp.txt"); // temp file to store filtered lines

		if (!tempFile.is_open())
		{
			LOGERROR("Unable to create temporary file");
			return;
		}

		while (std::getline(file, line))
		{
			if (line.substr(0, startString.length()) == startString)
			{
				tempFile << line << '\n'; // write the line to temp file if it starts with the given string
			}
		}

		file.close();
		tempFile.close();

		// replace original file with the temp file
		fs::remove(filePath);             // remove original file
		fs::rename("temp.txt", filePath); // rename temp file to original file

		LOG("Filtered lines saved to {}", filePath.string());
	}

	std::string get_text_content(const fs::path& file_path)
	{
		if (!fs::exists(file_path))
		{
			LOG("[ERROR] File doesn't exist: '{}'", file_path.string());
			return std::string();
		}

		std::ifstream     Temp(file_path);
		std::stringstream Buffer;
		Buffer << Temp.rdbuf();
		return Buffer.str();
	}

#ifndef NO_JSON

	json get_json(const fs::path& file_path)
	{
		json j;

		if (!fs::exists(file_path))
		{
			LOG("[ERROR] File doesn't exist: '{}'", file_path.string());
			return j;
		}

		try
		{
			std::ifstream file(file_path);
			file >> j;
		}
		catch (const std::exception& e)
		{
			LOG("[ERROR] Unable to read '{}': {}", file_path.filename().string(), e.what());
		}

		return j;
	}

	bool write_json(const fs::path& file_path, const json& j)
	{
		try
		{
			std::ofstream file(file_path);

			if (file.is_open())
			{
				file << j.dump(4); // pretty-print with 4 spaces indentation
				file.close();
			}
			else
			{
				LOG("[ERROR] Couldn't open file for writing: {}", file_path.string());
				return false;
			}

			return true;
		}
		catch (const std::exception& e)
		{
			LOG("[ERROR] Unable to write to '{}': {}", file_path.filename().string(), e.what());
			return false;
		}
	}
#endif

	void appendLineIfNotExist(const fs::path& file, const std::string& line)
	{
		const std::string        fileName = file.filename().string();
		std::vector<std::string> lines;

		// Read file if it exists
		if (fs::exists(file))
		{
			std::ifstream in(file);
			std::string   existingLine;
			while (std::getline(in, existingLine))
			{
				// normalize whitespace and case if you want
				if (existingLine == line)
				{
					LOG("{} already contains line: \"{}\"", fileName, line);
					return; // bail out if found
				}
				lines.push_back(existingLine);
			}
			in.close();
		}
		else
			LOG("WARNING: File doesn't exist: \"{}\"", file.string());

		// Add line
		lines.push_back(line);

		// Write back the file
		{
			std::ofstream out(file, std::ios::trunc);
			for (auto& line : lines)
				out << line << "\n";
		}

		LOG("Added line to {}: \"{}\"", fileName, line);
	}
} // namespace Files

#if !defined(NO_JSON) && !defined(NO_BAKKESMOD)
namespace PluginUpdates
{
	// global state for all the PluginUpdates:: functions. Can turn all this into a class maybe ong frfr
	PluginUpdaterInfo pluginUpdaterInfo;
	PluginUpdateInfo  updateInfo;
	std::mutex        updateMutex;

	/*
	    - Will populate updateInfo with the necessary data
	    - This update check only works if the github release name contains the version number. Like: "v1.2.3" or "Custom Thing v2.1.4" etc.
	*/
	void checkForUpdates(const std::string& modName, const std::string& currentVersion, const std::string& assetName)
	{
		CurlRequest       req;
		const std::string repoName = getRepoName(modName);
		req.url                    = std::format("https://api.github.com/repos/smallest-cock/{}/releases/latest", repoName);
		LOG("Checking for updates using this URL: {}", req.url);
		HttpWrapper::SendCurlRequest(req,
		    [modName, currentVersion, assetName](int code, std::string result)
		    {
			    if (code != 200)
			    {
				    LOGERROR("Invalid HTTP code: {}. Update check failed!", code);
				    return;
			    }

			    auto responseJsonOpt = Helper::getJsonFromStr(result);
			    if (!responseJsonOpt)
				    return;

			    std::string assetFileName = (assetName.empty() ? modName : assetName) + ".zip";
			    auto        updateInfoOpt = getUpdateInfo(*responseJsonOpt, assetFileName);
			    if (!updateInfoOpt)
				    return;

			    PluginUpdateInfo& info = *updateInfoOpt;
			    info.pluginName        = modName;
			    info.outOfDate         = currentVersion != info.latestVersion;

			    {
				    std::lock_guard<std::mutex> lock(updateMutex);
				    updateInfo = info;
			    }

			    if (info.outOfDate)
				    LOG("New version available: {}", info.latestVersion);
			    else
				    LOG("Plugin is up to date: {}", currentVersion);
		    });
	}

	std::string getRepoName(const std::string& modName)
	{
		// handle edge case bc CBO repo name isnt PascalCase like the other plugin repos
		// ... and I dont know what side effects changing the repo name would have, smh bitch
		if (modName == "CustomBallOnline")
			return "Custom-Ball-Online";

		return modName;
	}

	// the high level plugin-facing function
	void installUpdate(const std::shared_ptr<GameWrapper>& gw)
	{
		const std::string updateCmd = pluginUpdaterInfo.makeUpdateCmd(updateInfo);

		// check if PluginUpdater's cvar exists (aka if the plugin is loaded)
		if (_globalCvarManager->getCvar(pluginUpdaterInfo.existenceCheckCvar))
			_globalCvarManager->executeCommand(updateCmd);
		else
		{
			LOG("{} doesn't seem to be loaded", pluginUpdaterInfo.prettyName);
			LOG("Attempting to download and install latest version of {}...", pluginUpdaterInfo.prettyName);
			downloadAndInstallUpdaterPlugin(gw, updateCmd);
		}
	}

	void downloadAndInstallUpdaterPlugin(std::shared_ptr<GameWrapper> gw, const std::string& updateCmd)
	{
		// set paths
		const fs::path pluginsFolder = gw->GetBakkesModPath() / "plugins";
		const fs::path configFile    = gw->GetBakkesModPath() / "cfg" / "plugins.cfg";
		const fs::path tempDir       = fs::temp_directory_path() / pluginUpdaterInfo.name;
		fs::create_directories(tempDir);
		const fs::path downloadFile = tempDir / pluginUpdaterInfo.assetName;

		// get latest release info for PluginUpdater using github API
		CurlRequest req;
		req.url = pluginUpdaterInfo.latestReleaseApiUrl;
		HttpWrapper::SendCurlRequest(req,
		    [pluginsFolder, configFile, downloadFile, updateCmd](int code, std::string result)
		    {
			    try
			    {
				    if (code != 200)
				    {
					    LOGERROR("Invalid HTTP code: {}", code);
					    LOGERROR("Response body: {}", result);
					    return;
				    }

				    auto responseJsonOpt = Helper::getJsonFromStr(result);
				    if (!responseJsonOpt)
					    return;

				    auto urlOpt = getAssetDownloadUrl(*responseJsonOpt, pluginUpdaterInfo.assetName);
				    if (!urlOpt)
					    return;

				    // download/install DLL from latest release
				    CurlRequest req;
				    req.verb = "GET";
				    req.url  = *urlOpt;
				    HttpWrapper::SendCurlRequest(req,
				        downloadFile.wstring(),
				        [pluginsFolder, configFile, updateCmd](int httpCode, std::wstring filePath)
				        {
					        if (httpCode != 200)
					        {
						        LOGERROR("Invalid HTTP code: {}", httpCode);
						        return;
					        }

					        // copy file to plugins folder
					        LOG("Copying file to plugins folder...");
					        fs::path downloadedPath{filePath};
					        fs::copy(downloadedPath, pluginsFolder, fs::copy_options::overwrite_existing);

					        // delete file in temp directory
					        LOG("Deleting file in temp directory...");
					        fs::remove_all(downloadedPath.parent_path());

					        // Add line to plugins.cfg
					        LOG("Adding line to plugins.cfg...");
					        const std::string pluginLoadCmd = pluginUpdaterInfo.makePluginLoadCmd();
					        Files::appendLineIfNotExist(configFile, pluginLoadCmd);

					        // this works bc "plugin load X" will wait until the plugin has fully loaded before executing the next command
					        // ... otherwise, we would've had to do cvar probing in a new thread or something
					        const std::string finalCmd = std::format("{}; {}", pluginLoadCmd, updateCmd);
					        _globalCvarManager->executeCommand(finalCmd);
				        });
			    }
			    catch (...)
			    {
				    LOGERROR("Wow something is fricked in the on_complete callback");
			    }
		    });
	}

	std::optional<PluginUpdateInfo> getUpdateInfo(const json& releaseJson, const std::string& assetName)
	{
		PluginUpdateInfo info{};

		auto versionStr = getVersionStr(releaseJson);
		if (!versionStr)
			return std::nullopt;
		info.latestVersion = *versionStr;

		auto assetDownloadUrl = getAssetDownloadUrl(releaseJson, assetName);
		if (!assetDownloadUrl)
			return std::nullopt;
		info.assetDownloadUrl = *assetDownloadUrl;

		info.releaseUrl = releaseJson["html_url"].get<std::string>();
		return info;
	}

	std::optional<std::string> getAssetDownloadUrl(const json& releaseJson, const std::string& assetName)
	{
		for (const auto& asset : releaseJson["assets"])
		{
			if (asset["name"] == assetName)
				return asset["browser_download_url"].get<std::string>();
		}
		return std::nullopt;
	}

	std::optional<std::string> getVersionStr(const json& releaseJson)
	{
		constexpr auto          versionPattern = R"(v?(\d+\.\d+\.\d+(?:[-\w\.]+)?))";
		static const std::regex re{versionPattern};
		std::smatch             match;

		auto releaseName = releaseJson["name"].get<std::string>();
		if (std::regex_search(releaseName, match, re))
			return match[1].str();
		else
		{
			LOGERROR("Release name \"{}\" doesn't match regex pattern: \"{}\"", releaseName, versionPattern);
			return std::nullopt;
		}
	}
} // namespace PluginUpdates
#endif
namespace Process
{
	void close_handle(HANDLE h)
	{
		if (h != NULL && h != INVALID_HANDLE_VALUE)
		{
			if (!CloseHandle(h))
			{
				LOG("CloseHandle failed with error: {}", GetLastError());
			}
		}
		else
		{
			LOG("Unable to close handle. The handle is NULL or an invalid value");
		}
	}

	void terminate(HANDLE h)
	{
		if (h != NULL && h != INVALID_HANDLE_VALUE)
		{
			if (!TerminateProcess(h, 1))
			{
				LOG("TerminateProcess failed with error: {}", GetLastError());
			}
		}
		else
		{
			LOG("Unable to terminate process. The handle is NULL or an invalid value");
		}
	}

	void terminate_created_process(ProcessHandles& pi)
	{
		terminate(pi.hProcess);
		close_handle(pi.hProcess);
		close_handle(pi.hThread);
	}

	CreateProcessResult create_process_from_command(const std::string& command)
	{
		// CreateProcess variables
		STARTUPINFO         si;
		PROCESS_INFORMATION pi;

		// Initialize STARTUPINFO & PROCESS_INFORMATION
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		std::wstring wide_command = StringUtils::ToWideString(command);

		// initialize result
		CreateProcessResult result;

		// Create the process to start python script
		if (CreateProcessW(NULL,     // Application name (set NULL to use command)
		        wide_command.data(), // Command
		        NULL,                // Process security attributes
		        NULL,                // Thread security attributes
		        FALSE,               // Inherit handles from the calling process
		        CREATE_NEW_CONSOLE,  // Creation flags (use CREATE_NEW_CONSOLE for async execution)
		        NULL,                // Use parent's environment block
		        NULL,                // Use parent's starting directory
		        &si,                 // Pointer to STARTUPINFO
		        &pi                  // Pointer to PROCESS_INFORMATION
		        ))
		{
			// Duplicate process handle so it remains valid even after original PROCESS_INFORMATION goes out of scope
			HANDLE duplicatedProcessHandle = NULL;
			HANDLE duplicatedThreadHandle  = NULL;

			if (DuplicateHandle(
			        GetCurrentProcess(), pi.hProcess, GetCurrentProcess(), &duplicatedProcessHandle, 0, FALSE, DUPLICATE_SAME_ACCESS) &&
			    DuplicateHandle(
			        GetCurrentProcess(), pi.hThread, GetCurrentProcess(), &duplicatedThreadHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
			{
				result.handles.hProcess = duplicatedProcessHandle;
				result.handles.hThread  = duplicatedThreadHandle;
			}
			else
			{
				result.status_code = GetLastError();
			}

			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		else
		{
			// If CreateProcess failed, return the error code
			result.status_code = GetLastError();
		}

		return result;
	}
} // namespace Process

#ifndef NO_RLSDK
namespace Colors
{
	// returns a AARRGGBB packed 32-bit int
	uint32_t packColor(const FColor& col)
	{
		return (static_cast<uint32_t>(col.A) << 24) | (static_cast<uint32_t>(col.R) << 16) | (static_cast<uint32_t>(col.G) << 8) |
		       static_cast<uint32_t>(col.B);
	}

	// expects a AARRGGBB packed 32-bit int
	FColor unpackColor(uint32_t packed)
	{
		return {
		    static_cast<uint8_t>(packed & 0xFF),         // B
		    static_cast<uint8_t>((packed >> 8) & 0xFF),  // G
		    static_cast<uint8_t>((packed >> 16) & 0xFF), // R
		    static_cast<uint8_t>((packed >> 24) & 0xFF)  // A
		};
	}

	// FColor --> AARRGGBB hex string
	std::string fcolorToHex(const FColor& col)
	{
		uint32_t           packedInt = packColor(col);
		std::ostringstream ss;
		ss << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << packedInt;
		return ss.str();
	}

	// AARRGGBB hex string --> FColor
	FColor hexToFColor(const std::string& hex)
	{
		if (hex.size() != 8)
		{
			LOG("ERROR: Color hex string \"{}\" isn't 8 characters. Falling back to white...");
			return {255, 255, 255, 255}; // fallback to white
		}

		uint32_t packed = static_cast<uint32_t>(std::stoul(hex, nullptr, 16));
		return unpackColor(packed);
	}

#ifndef NO_BAKKESMOD
	FLinearColor CvarColorToFLinearColor(const LinearColor& cvarColor)
	{
		LinearColor fixedCol = cvarColor / 255;

		return FLinearColor{fixedCol.R, fixedCol.G, fixedCol.B, fixedCol.A};
	}

	uint32_t CvarColorToInt(const LinearColor& col) { return FLinearColorToInt(CvarColorToFLinearColor(col)); }
#endif

	int32_t FLinearColorToInt(const FLinearColor& color)
	{
		FColor fColor = fLinearColorToFColor(color);
		return Color(fColor).ToDecimal();
	}

	std::string fcolorToHexRGBA(const FColor& col)
	{
		std::stringstream ss;
		ss << "0x" << std::uppercase << std::setfill('0') << std::hex << std::setw(2) << static_cast<int>(col.R) << std::setw(2)
		   << static_cast<int>(col.G) << std::setw(2) << static_cast<int>(col.B) << std::setw(2) << static_cast<int>(col.A);
		return ss.str();
	}

	FColor hexRGBAtoFColor(const std::string& hex)
	{
		if (hex.size() != 10 || hex.substr(0, 2) != "0x")
			throw std::invalid_argument("Invalid color hex string format. Expected format: 0xRRGGBBAA");

		// Convert hex string (skip '0x')
		uint32_t value = std::stoul(hex.substr(2), nullptr, 16);

		FColor col;
		col.R = (value >> 24) & 0xFF;
		col.G = (value >> 16) & 0xFF;
		col.B = (value >> 8) & 0xFF;
		col.A = value & 0xFF;

		return col;
	}
} // namespace Colors

// Color class
Color::Color() : R(255), G(255), B(255), A(255) {}
Color::Color(uint8_t rgba) : R(rgba), G(rgba), B(rgba), A(rgba) {}
Color::Color(int32_t rgba) : R(rgba), G(rgba), B(rgba), A(rgba) {}
Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r), G(g), B(b), A(a) {}
Color::Color(int32_t r, int32_t g, int32_t b, int32_t a) : R(r), G(g), B(b), A(a) {}
Color::Color(float r, float g, float b, float a) : R(255), G(255), B(255), A(255) { FromLinear(CoolerLinearColor(r, g, b, a)); }
Color::Color(const std::string& hexColor) : R(255), G(255), B(255), A(255) { FromHex(hexColor); }
Color::Color(const struct FColor& color) : R(color.R), G(color.G), B(color.B), A(color.A) {}
Color::Color(const Color& color) : R(color.R), G(color.G), B(color.B), A(color.A) {}
Color::Color(float arr[3]) : R(255), G(255), B(255), A(255) { FromLinear(CoolerLinearColor(arr[0], arr[1], arr[2], 1.0f)); }
Color::~Color() {}

struct FColor Color::UnrealColor() const
{
	return FColor{B, G, R, A}; // Your game might be in a different format (RGBA), so be aware of that.
}

class CoolerLinearColor Color::ToLinear() const { return CoolerLinearColor().FromColor(*this); }

uint32_t Color::ToDecimal() const
{
	return Format::ToDecimal(
	    ToHex(false)); // Standard method would be "uint32_t decimalValue = (R & 0xFF) << 16) | ((G & 0xFF) << 8) | (B & 0xFF);"
}

uint32_t Color::ToDecimalAlpha() const { return Format::ToDecimal(ToHexAlpha(false)); }

std::string Color::ToHex(bool bNotation) const
{
	std::string hexStr = (bNotation ? "#" : "");
	hexStr += Format::ToHex(static_cast<uint64_t>(R), 2, false);
	hexStr += Format::ToHex(static_cast<uint64_t>(G), 2, false);
	hexStr += Format::ToHex(static_cast<uint64_t>(B), 2, false);
	return hexStr;
}

std::string Color::ToHexAlpha(bool bNotation) const
{
	std::string hexStr = ToHex(bNotation);
	hexStr += Format::ToHex(static_cast<uint64_t>(A), 2, false);
	return hexStr;
}

Color& Color::FromLinear(const CoolerLinearColor& linearColor)
{
	R = static_cast<uint8_t>(linearColor.R * 255.0f);
	G = static_cast<uint8_t>(linearColor.G * 255.0f);
	B = static_cast<uint8_t>(linearColor.B * 255.0f);
	A = static_cast<uint8_t>(linearColor.A * 255.0f);
	return *this;
}

Color& Color::FromDecimal(uint32_t decimalColor)
{
	return FromHex(Format::ToHex(decimalColor, ((decimalColor > 0xFFFFFF) ? 8 : 6), false));
}

Color& Color::FromHex(std::string hexColor)
{
	Format::RemoveAllCharsInline(hexColor, '#');

	if (Format::IsStringHexadecimal(hexColor))
	{
		if (hexColor.length() > 8)
		{
			hexColor = hexColor.substr(0, 8);
		}

		uint32_t alpha = 255;

		if (hexColor.length() == 8)
		{
			alpha = Format::ToDecimal(hexColor.substr(6, 2)); // Optional, if an alpha value is provided.
		}

		A = static_cast<uint8_t>(alpha);

		if (hexColor.length() > 6)
		{
			hexColor = hexColor.substr(0, 6); // Removes the alpha value, or invalid trailing characters.
		}

		if (hexColor.length() == 6)
		{
			uint32_t red   = Format::ToDecimal(hexColor.substr(0, 2));
			uint32_t green = Format::ToDecimal(hexColor.substr(2, 2));
			uint32_t blue  = Format::ToDecimal(hexColor.substr(4, 2));
			R              = static_cast<uint8_t>(red);
			G              = static_cast<uint8_t>(green);
			B              = static_cast<uint8_t>(blue);
		}
	}

	return *this;
}

Color& Color::Cycle(int32_t steps)
{
	uint8_t constant = R;
	if (G > constant)
	{
		constant = G;
	}
	if (B > constant)
	{
		constant = B;
	}

	if ((R == constant) && (G < constant) && (B == 0))
	{
		G += steps;
	} // Green goes up.
	else if ((R > 0) && (G == constant) && (B == 0))
	{
		R -= steps;
	} // Red goes down.
	else if ((R == 0) && (G == constant) && (B < constant))
	{
		B += steps;
	} // Blue goes up.
	else if ((R == 0) && (G > 0) && (B == constant))
	{
		G -= steps;
	} // Green goes down.
	else if ((R < constant) && (G == 0) && (B == constant))
	{
		R += steps;
	} // Red goes up.
	else if ((R == constant) && (G == 0) && (B > 0))
	{
		B -= steps;
	} // Blue goes down.
	return *this;
}

Color& Color::operator=(const Color& other)
{
	R = other.R;
	G = other.G;
	B = other.B;
	A = other.A;
	return *this;
}

Color& Color::operator=(const struct FColor& other)
{
	R = other.R;
	G = other.G;
	B = other.B;
	A = other.A;
	return *this;
}

bool Color::operator==(const Color& other) const { return (R == other.R && G == other.G && B == other.B && A == other.A); }
bool Color::operator==(const struct FColor& other) const { return (R == other.R && G == other.G && B == other.B && A == other.A); }
bool Color::operator!=(const Color& other) const { return !(*this == other); }
bool Color::operator!=(const struct FColor& other) const { return !(*this == other); }
bool Color::operator<(const Color& other) const { return (ToHexAlpha(false) < other.ToHexAlpha(false)); }
bool Color::operator>(const Color& other) const { return (ToHexAlpha(false) > other.ToHexAlpha(false)); }

// CoolerLinearColor class
CoolerLinearColor::CoolerLinearColor() : R(1.0f), G(1.0f), B(1.0f), A(1.0f) {}
CoolerLinearColor::CoolerLinearColor(float rgba) : R(rgba), G(rgba), B(rgba), A(rgba) {}
CoolerLinearColor::CoolerLinearColor(float r, float g, float b, float a) : R(r), G(g), B(b), A(a) {}
CoolerLinearColor::CoolerLinearColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(1.0f), G(1.0f), B(1.0f), A(1.0f)
{
	FromColor(Color(a, g, b, a));
}

CoolerLinearColor::CoolerLinearColor(const std::string& hexColor) : R(1.0f), G(1.0f), B(1.0f), A(1.0f) { FromHex(hexColor); }
CoolerLinearColor::CoolerLinearColor(const struct FLinearColor& linearColor)
    : R(linearColor.R), G(linearColor.G), B(linearColor.B), A(linearColor.A)
{
}
CoolerLinearColor::CoolerLinearColor(const CoolerLinearColor& linearColor)
    : R(linearColor.R), G(linearColor.G), B(linearColor.B), A(linearColor.A)
{
}
CoolerLinearColor::~CoolerLinearColor() {}

struct FLinearColor CoolerLinearColor::UnrealColor() const { return FLinearColor{R, G, B, A}; }
Color               CoolerLinearColor::ToColor() const { return Color(R, G, B, A); }
uint32_t            CoolerLinearColor::ToDecimal() const { return ToColor().ToDecimal(); }
uint32_t            CoolerLinearColor::ToDecimalAlpha() const { return ToColor().ToDecimalAlpha(); }
std::string         CoolerLinearColor::ToHex(bool bNotation) const { return ToColor().ToHex(bNotation); }
std::string         CoolerLinearColor::ToHexAlpha(bool bNotation) const { return ToColor().ToHexAlpha(bNotation); }

CoolerLinearColor& CoolerLinearColor::FromColor(const Color& color)
{
	R = (color.R > 0 ? (static_cast<float>(color.R) / 255.0f) : 0);
	G = (color.G > 0 ? (static_cast<float>(color.G) / 255.0f) : 0);
	B = (color.B > 0 ? (static_cast<float>(color.B) / 255.0f) : 0);
	A = (color.A > 0 ? (static_cast<float>(color.A) / 255.0f) : 0);
	return *this;
}

CoolerLinearColor& CoolerLinearColor::FromDecimal(uint32_t decimalColor) { return FromColor(Color().FromDecimal(decimalColor)); }
CoolerLinearColor& CoolerLinearColor::FromHex(std::string hexColor) { return FromColor(Color(hexColor)); }
CoolerLinearColor& CoolerLinearColor::Cycle(int32_t steps) { return FromColor(ToColor().Cycle(steps)); }

CoolerLinearColor& CoolerLinearColor::operator=(const CoolerLinearColor& other)
{
	R = other.R;
	G = other.G;
	B = other.B;
	A = other.A;
	return *this;
}

CoolerLinearColor& CoolerLinearColor::operator=(const struct FLinearColor& other)
{
	R = other.R;
	G = other.G;
	B = other.B;
	A = other.A;
	return *this;
}

bool CoolerLinearColor::operator==(const CoolerLinearColor& other) const
{
	return (R == other.R && G == other.G && B == other.B && A == other.A);
}

bool CoolerLinearColor::operator==(const struct FLinearColor& other) const
{
	return (R == other.R && G == other.G && B == other.B && A == other.A);
}

bool CoolerLinearColor::operator!=(const CoolerLinearColor& other) const { return !(*this == other); }
bool CoolerLinearColor::operator!=(const struct FLinearColor& other) const { return !(*this == other); }
bool CoolerLinearColor::operator<(const CoolerLinearColor& other) const { return (ToHexAlpha(false) < other.ToHexAlpha(false)); }
bool CoolerLinearColor::operator>(const CoolerLinearColor& other) const { return (ToHexAlpha(false) > other.ToHexAlpha(false)); }

// GRainbowColor class
Color             GRainbowColor::GetByte() { return ByteRainbow; }
CoolerLinearColor GRainbowColor::GetLinear() { return LinearRainbow; }
FLinearColor      GRainbowColor::GetFLinear() { return {LinearRainbow.R, LinearRainbow.G, LinearRainbow.B, LinearRainbow.A}; }
int32_t           GRainbowColor::GetDecimal() { return (GetByte().R << 16) + (GetByte().G << 8) + GetByte().B; }

void GRainbowColor::Reset()
{
	ByteRainbow   = Color(0, 0, 255, 255);
	LinearRainbow = CoolerLinearColor(0.0f, 0.0f, 1.0f, 1.0f);
}

void GRainbowColor::OnTick()
{
	ByteRainbow.Cycle();
	LinearRainbow = ByteRainbow.ToLinear();
}

void GRainbowColor::IncrementTick()
{
	tickCounter++;

	if (tickCounter == UINT32_MAX)
		tickCounter = 0;
}

bool GRainbowColor::TickCountIsMultipleOf(int num) { return tickCounter % num == 0; }

void GRainbowColor::TickRGB(int speed, int defaultSpeed)
{
	IncrementTick();

	// if less than default speed
	if (speed < defaultSpeed)
	{
		if (TickCountIsMultipleOf(defaultSpeed - speed))
			OnTick();
	}
	// if greater than (or equal to) default speed
	else if (speed >= defaultSpeed)
	{
		for (int i = 0; i < (speed - defaultSpeed + 1); ++i)
			OnTick();
	}
}
#endif // NO_RLSDK
