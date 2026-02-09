#pragma once
#include "pch.h"
#include <span>

static float CalcMaxPopupHeightFromItemCount(int items_count) {
	ImGuiContext &g = *GImGui;
	if (items_count <= 0)
		return FLT_MAX;
	return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}

namespace ImGui {

	/* Custom version of SearchableCombo that works with unordered_map<K, V>
	 * to include a input field to be able to filter the combo values. */
	template <typename K, typename V>
	IMGUI_API bool SearchableComboFromMap(const char *label,
	    K                                            *current_key,
	    const std::unordered_map<K, V>               &items_map,
	    const std::function<const char *(const V &)> &get_display_text,
	    const char                                   *default_preview_text,
	    const char                                   *input_preview_value,
	    int                                           popup_max_height_in_items = -1) {
		ImGuiContext &g = *GImGui;

		// Find the current item in the map
		auto        current_it   = items_map.find(*current_key);
		const char *preview_text = default_preview_text;

		if (current_it != items_map.end())
			preview_text = get_display_text(current_it->second);

		// The old Combo() API exposed "popup_max_height_in_items". The new more general BeginCombo() API doesn't have/need it, but we
		// emulate it here.
		if (popup_max_height_in_items != -1 && !(g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint))
			SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));

		const int input_size               = 64;
		char      input_buffer[input_size] = "";
		if (!BeginSearchableCombo(label, preview_text, input_buffer, input_size, input_preview_value, ImGuiComboFlags_None))
			return false;

		// Display items
		// FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call to SetItemDefaultFocus() is
		// processed)
		int  matched_items = 0;
		bool value_changed = false;

		std::string search_input = input_buffer;
		std::transform(
		    search_input.begin(), search_input.end(), search_input.begin(), [](unsigned char c) { return (unsigned char)std::tolower(c); });

		// Store keys in a vector for consistent ordering
		std::vector<K> keys;
		keys.reserve(items_map.size());
		for (const auto &pair : items_map)
			keys.push_back(pair.first);

		// Sort by display text for consistent UX (same as original SearchableCombo maintains order)
		std::sort(keys.begin(), keys.end(), [&items_map, &get_display_text](const K &a, const K &b) {
			return strcmp(get_display_text(items_map.at(a)), get_display_text(items_map.at(b))) < 0;
		});

		for (const K &key : keys) {
			const V    &item      = items_map.at(key);
			const char *item_text = get_display_text(item);

			// Filtering
			std::string item_text_lower = item_text;
			std::transform(item_text_lower.begin(), item_text_lower.end(), item_text_lower.begin(), [](unsigned char c) {
				return (unsigned char)std::tolower(c);
			});

			if (!search_input.empty() && item_text_lower.find(search_input) == std::string::npos)
				continue;

			matched_items++;
			PushID((void *)(intptr_t)key);
			const bool item_selected = (key == *current_key);
			if (Selectable(item_text, item_selected)) {
				value_changed = true;
				*current_key  = key;
			}
			if (item_selected)
				SetItemDefaultFocus();
			PopID();
		}

		if (matched_items == 0)
			Selectable("No items found", false, ImGuiSelectableFlags_Disabled);

		EndSearchableCombo();

		return value_changed;
	}
}

namespace GUI {
	struct WordColor {
		std::string text;
		ImVec4      color = {1.0f, 1.0f, 1.0f, 1.0f};

		void display() const {
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertFloat4ToU32(color));
			ImGui::TextUnformatted(text.c_str());
			ImGui::PopStyleColor();
			ImGui::SameLine(0.0f, 0.0f);
		}
	};

	namespace detail {
		void coloredTextFormatImpl(const std::string &fmt, std::span<const WordColor> args);
	}

	template <typename... Args>
	void ColoredTextFormat(const std::string &fmt, Args &&...args) {
		std::array<WordColor, sizeof...(Args)> arr{std::forward<Args>(args)...};
		detail::coloredTextFormatImpl(fmt, arr);
	}

	void open_link(const char *url);
	void open_link(const wchar_t *url);

	void ClickableLink(const char *label, const char *url, const ImVec4 &textColor = ImVec4(1, 1, 1, 1), ImVec2 size = ImVec2(0, 0));

	void AddHoverHand();
	void Spacing(int amount = 1);
	void SameLineSpacing_relative(float horizontalSpacingPx);
	void SameLineSpacing_absolute(float horizontalSpacingPx);
	void centerTextX(const char *text, float offsetCorrection = 0.0f);
	void centerTextColoredX(const ImVec4 &col, const char *text, float offsetCorrection = 0.0f);
	void CopyButton(const char *label, const char *copyText, float sameLineSpacing = 20.0f);

	template <typename... Args>
	inline void ToolTipFmt(const char *fmt, Args &&...args) {
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(fmt, std::forward<Args>(args)...);
	}

	inline void ToolTip(std::string_view str) {
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", str.data());
	}

	// }

#define GUI_ToolTip(body)                                                                                                                  \
	if (ImGui::IsItemHovered()) {                                                                                                          \
		ImGui::BeginTooltip();                                                                                                             \
		body;                                                                                                                              \
		ImGui::EndTooltip();                                                                                                               \
	}

// scoped version of BeginChild/EndChild
// Usage: GUI::ScopedChild c{ "SomeLabel", ... };
#if IMGUI_VERSION_NUM >= 19000
	// NEW IMGUI API (>= 1.90): BeginChild(size, child_flags, window_flags)
	struct ScopedChild {
		ScopedChild(
		    const char *idStr, const ImVec2 &size = ImVec2(0, 0), ImGuiChildFlags childFlags = 0, ImGuiWindowFlags windowFlags = 0) {
			ImGui::BeginChild(idStr, size, childFlags, windowFlags);
		}

		~ScopedChild() { ImGui::EndChild(); }
	};
#else
	// OLD IMGUI API (< 1.90): BeginChild(size, border, window_flags)
	struct ScopedChild {
		ScopedChild(const char *str_id, const ImVec2 &size = ImVec2(0, 0), bool border = false, ImGuiWindowFlags flags = 0) {
			ImGui::BeginChild(str_id, size, border, flags);
		}

		~ScopedChild() { ImGui::EndChild(); }
	};
#endif

	/**
	USAGE:
	    if (ImGui::IsItemHovered())
	    {
	        GUI::ScopedTooltip tip{};
	        ImGui::Text("Hello from tooltip!");
	        ImGui::Separator();
	        ImGui::Text("More info here...");
	    }
	*/
	struct ScopedTooltip {
		ScopedTooltip() { ImGui::BeginTooltip(); }
		~ScopedTooltip() { ImGui::EndTooltip(); }

		// Non-copyable, non-movable (prevents double EndTooltip calls)
		ScopedTooltip(const ScopedTooltip &)            = delete;
		ScopedTooltip &operator=(const ScopedTooltip &) = delete;
	};

	// scoped version of PushID/PopID
	// Usage: GUI::ScopedID id{ "SomeID" };
	struct ScopedID {
		ScopedID(const char *str_id) { ImGui::PushID(str_id); }
		ScopedID(const char *str_id_begin, const char *str_id_end) { ImGui::PushID(str_id_begin, str_id_end); }
		ScopedID(const void *ptr_id) { ImGui::PushID(ptr_id); }
		ScopedID(int id) { ImGui::PushID(id); }

		~ScopedID() { ImGui::PopID(); }
	};

	struct ScopedItemWidth {
		ScopedItemWidth(float width) { ImGui::PushItemWidth(width); }
		~ScopedItemWidth() { ImGui::PopItemWidth(); }
	};

	// scoped version of Indent/Unindent
	// Usage: GUI::ScopedIndent indent{ 20.0f };
	struct ScopedIndent {
		const float m_width;

		ScopedIndent(float width = 0.0f) : m_width(width) { ImGui::Indent(width); }
		~ScopedIndent() { ImGui::Unindent(m_width); }
	};

	struct ScopedDisabled {
		std::shared_ptr<bool> state;

		explicit ScopedDisabled(const std::shared_ptr<bool> &flag) : state(flag) {
			if (!state || *state)
				return;

			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		~ScopedDisabled() {
			if (!state || *state)
				return;

			ImGui::PopStyleVar();
			ImGui::PopItemFlag();
		}

		// non-copyable, but movable
		ScopedDisabled(const ScopedDisabled &)            = delete;
		ScopedDisabled &operator=(const ScopedDisabled &) = delete;
		ScopedDisabled(ScopedDisabled &&)                 = default;
		ScopedDisabled &operator=(ScopedDisabled &&)      = default;
	};

	namespace Colors {
#ifndef NO_RLSDK
		struct Color {
			float r, g, b, a;

			Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
			Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
			Color(const ImVec4 &color) : r(color.w), g(color.x), b(color.y), a(color.z) {}
			Color(const FLinearColor &color) : r(color.R), g(color.G), b(color.B), a(color.A) {}

			// Initialize with a int32_t color (0xAARRGGBB format)
			Color(int32_t color) {
				a = ((color >> 24) & 0xFF) / 255.0f; // Extract and normalize Alpha
				r = ((color >> 16) & 0xFF) / 255.0f; // Extract and normalize Red
				g = ((color >> 8) & 0xFF) / 255.0f;  // Extract and normalize Green
				b = (color & 0xFF) / 255.0f;         // Extract and normalize Blue
			}

			// Initialize with an FColor
			Color(const FColor &color) {
				r = color.R / 255.0f; // Normalize Red
				g = color.G / 255.0f; // Normalize Green
				b = color.B / 255.0f; // Normalize Blue
				a = color.A / 255.0f; // Normalize Alpha
			}

			ImVec4 GetImGuiColor() const { return ImVec4{r, g, b, a}; }

			FLinearColor GetLinearColor() const { return FLinearColor{r, g, b, a}; }

			int32_t GetIntColor() const {
				FColor col = GetFColor();

				return (static_cast<int32_t>(col.A) << 24) | (static_cast<int32_t>(col.R) << 16) | (static_cast<int32_t>(col.G) << 8) |
				       static_cast<int32_t>(col.B);
			}

			FColor GetFColor() const {
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

		extern const ImVec4 VividGreen;
		extern const ImVec4 DarkGreen;
		extern const ImVec4 ForestGreen;
		extern const ImVec4 RealDarkGreen;

		extern const ImVec4 Gray;
		extern const ImVec4 LightGray;
		extern const ImVec4 LighterGray;
		extern const ImVec4 SuperLightGray;

		extern const ImVec4 Orange;
		extern const ImVec4 LightOrange;
		extern const ImVec4 VividOrange;
	} // namespace Colors

	void SettingsHeader(const char *id, const char *pluginVersion, const ImVec2 &size, bool showBorder = false);
	void OldSettingsFooter(const char *id, const ImVec2 &size, bool showBorder = false);

#if !defined(NO_JSON) && !defined(NO_BAKKESMOD)
	void alt_settings_header(const char    *text,
	    const char                         *plugin_version,
	    const std::shared_ptr<GameWrapper> &gw,
	    bool                                isPluginUpdater = false,
	    const ImVec4                       &text_color      = Colors::Pinkish);
	void alt_settings_footer(const char *text, const char *url, const ImVec4 &text_color = Colors::Yellow);

	void plugin_update_message(const std::shared_ptr<GameWrapper> &gw, bool isPluginUpdater);
#endif
} // namespace GUI
