#pragma once
#include "pch.h"

namespace GUI
{
struct WordColor
{
	std::string text;
	ImVec4      color;
};

namespace detail
{
void coloredTextFormatImpl(const std::string& fmt, std::span<const WordColor> args);
}

template <typename... Args> void ColoredTextFormat(const std::string& fmt, Args&&... args)
{
	std::array<WordColor, sizeof...(Args)> arr{std::forward<Args>(args)...};
	detail::coloredTextFormatImpl(fmt, arr);
}

void open_link(const char* url);
void open_link(const wchar_t* url);

void ClickableLink(const char* label, const char* url, const ImVec4& textColor = ImVec4(1, 1, 1, 1), ImVec2 size = ImVec2(0, 0));

void Spacing(int amount = 1);
void SameLineSpacing_relative(float horizontalSpacingPx);
void SameLineSpacing_absolute(float horizontalSpacingPx);
void ToolTip(const char* tip);
void centerTextX(const char* text, float offsetCorrection = 0.0f);
void centerTextColoredX(const ImVec4& col, const char* text, float offsetCorrection = 0.0f);

// scoped version of BeginChild/EndChild
// Usage: GUI::ScopedChild c{ "SomeLabel", ... };
struct ScopedChild
{
	ScopedChild(const char* str_id, const ImVec2& size = ImVec2(0, 0), bool border = false, ImGuiWindowFlags flags = 0)
	{
		ImGui::BeginChild(str_id, size, border, flags);
	}

	~ScopedChild() { ImGui::EndChild(); }
};

// scoped version of PushID/PopID
// Usage: GUI::ScopedID id{ "SomeID" };
struct ScopedID
{
	ScopedID(const char* str_id) { ImGui::PushID(str_id); }
	ScopedID(const char* str_id_begin, const char* str_id_end) { ImGui::PushID(str_id_begin, str_id_end); }
	ScopedID(const void* ptr_id) { ImGui::PushID(ptr_id); }
	ScopedID(int id) { ImGui::PushID(id); }

	~ScopedID() { ImGui::PopID(); }
};

struct ScopedItemWidth
{
	ScopedItemWidth(float width) { ImGui::PushItemWidth(width); }
	~ScopedItemWidth() { ImGui::PopItemWidth(); }
};

// scoped version of Indent/Unindent
// Usage: GUI::ScopedIndent indent{ 20.0f };
struct ScopedIndent
{
	const float m_width;

	ScopedIndent(float width = 0.0f) : m_width(width) { ImGui::Indent(width); }
	~ScopedIndent() { ImGui::Unindent(m_width); }
};

namespace Colors
{
#ifndef NO_RLSDK
struct Color
{
	float r, g, b, a;

	Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}

	// Initialize with float values
	Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}

	Color(const ImVec4& color) : r(color.w), g(color.x), b(color.y), a(color.z) {}

	// Initialize with an FLinearColor
	Color(const FLinearColor& color) : r(color.R), g(color.G), b(color.B), a(color.A) {}

	// Initialize with a int32_t color (0xAARRGGBB format)
	Color(int32_t color)
	{
		a = ((color >> 24) & 0xFF) / 255.0f; // Extract and normalize Alpha
		r = ((color >> 16) & 0xFF) / 255.0f; // Extract and normalize Red
		g = ((color >> 8) & 0xFF) / 255.0f;  // Extract and normalize Green
		b = (color & 0xFF) / 255.0f;         // Extract and normalize Blue
	}

	// Initialize with an FColor
	Color(const FColor& color)
	{
		r = color.R / 255.0f; // Normalize Red
		g = color.G / 255.0f; // Normalize Green
		b = color.B / 255.0f; // Normalize Blue
		a = color.A / 255.0f; // Normalize Alpha
	}

	ImVec4 GetImGuiColor() const { return ImVec4{r, g, b, a}; }

	FLinearColor GetLinearColor() const { return FLinearColor{r, g, b, a}; }

	int32_t GetIntColor() const
	{
		FColor col = GetFColor();

		return (static_cast<int32_t>(col.A) << 24) | (static_cast<int32_t>(col.R) << 16) | (static_cast<int32_t>(col.G) << 8) |
		       static_cast<int32_t>(col.B);
	}

	FColor GetFColor() const
	{
		FColor col;

		col.R = static_cast<uint8_t>(std::round(r * 255.0f));
		col.G = static_cast<uint8_t>(std::round(g * 255.0f));
		col.B = static_cast<uint8_t>(std::round(b * 255.0f));
		col.A = static_cast<uint8_t>(std::round(a * 255.0f));

		return col;
	}
};
#endif // NO_RLSDK

extern const ImVec4 White;
extern const ImVec4 Red;
extern const ImVec4 Green;
extern const ImVec4 Blue;

extern const ImVec4 Yellow;
extern const ImVec4 BlueGreen;
extern const ImVec4 Pinkish;

extern const ImVec4 LightBlue;
extern const ImVec4 LightRed;
extern const ImVec4 LightGreen;
extern const ImVec4 ChillGreen;

extern const ImVec4 DarkGreen;
extern const ImVec4 ForestGreen;
extern const ImVec4 RealDarkGreen;

extern const ImVec4 Gray;
extern const ImVec4 LightGray;
extern const ImVec4 LighterGray;

extern const ImVec4 Orange;
extern const ImVec4 LightOrange;
extern const ImVec4 VividOrange;
} // namespace Colors

void SettingsHeader(const char* id, const char* pluginVersion, const ImVec2& size, bool showBorder = false);
void OldSettingsFooter(const char* id, const ImVec2& size, bool showBorder = false);

void alt_settings_header(const char*    text,
    const char*                         plugin_version,
    const std::shared_ptr<GameWrapper>& gw,
    bool                                isPluginUpdater = false,
    const ImVec4&                       text_color      = Colors::Pinkish);
void alt_settings_footer(const char* text, const char* url, const ImVec4& text_color = Colors::Yellow);

void plugin_update_message(const std::shared_ptr<GameWrapper>& gw, bool isPluginUpdater);
} // namespace GUI