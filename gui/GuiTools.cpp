#include "pch.h"
#include "GuiTools.hpp"
#include "../util/Utils.hpp"

namespace GUI
{
namespace detail
{
void coloredTextFormatImpl(const std::string& fmt, std::span<const WordColor> args)
{
	size_t argIndex = 0;
	size_t pos      = 0;

	while (pos < fmt.size())
	{
		if (fmt[pos] == '{' && pos + 1 < fmt.size() && fmt[pos + 1] == '}')
		{
			// Insert a colored word
			if (argIndex < args.size())
			{
				const auto& wc = args[argIndex++];
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertFloat4ToU32(wc.color));
				ImGui::TextUnformatted(wc.text.c_str());
				ImGui::PopStyleColor();
				ImGui::SameLine(0.0f, 0.0f);
			}
			pos += 2;
		}
		else
		{
			// Collect plain text until next {}
			size_t      next  = fmt.find("{}", pos);
			std::string plain = fmt.substr(pos, next - pos);
			if (!plain.empty())
			{
				ImGui::TextUnformatted(plain.c_str());
				ImGui::SameLine(0.0f, 0.0f);
			}
			pos = (next == std::string::npos) ? fmt.size() : next;
		}
	}

	// Kill trailing SameLine so ImGui doesn’t glue the next widget
	ImGui::NewLine();
}
} // namespace detail

namespace Colors
{
const ImVec4 White         = {1, 1, 1, 1};
const ImVec4 Red           = {1, 0, 0, 1};
const ImVec4 Green         = {0, 1, 0, 1};
const ImVec4 Blue          = {0, 0, 1, 1};
const ImVec4 Yellow        = {1, 1, 0, 1};
const ImVec4 BlueGreen     = {0, 1, 1, 1};
const ImVec4 Pinkish       = {1, 0, 1, 1};
const ImVec4 LightBlue     = {0.5f, 0.5f, 1.0f, 1.0f};
const ImVec4 LightRed      = {1.0f, 0.4f, 0.4f, 1.0f};
const ImVec4 LightGreen    = {0.5f, 1.0f, 0.5f, 1.0f};
const ImVec4 ChillGreen    = {0.2f, 0.7f, 0.2f, 1.0f};
const ImVec4 DarkGreen     = {0.0f, 0.5f, 0.0f, 1.0f};
const ImVec4 ForestGreen   = {0.3f, 0.5f, 0.3f, 1.0f};
const ImVec4 RealDarkGreen = {0.1f, 0.3f, 0.1f, 1.0f};
const ImVec4 Gray          = {0.4f, 0.4f, 0.4f, 1.0f};
const ImVec4 LightGray     = {0.7f, 0.7f, 0.7f, 1.0f};
const ImVec4 LighterGray   = {0.8f, 0.8f, 0.8f, 1.0f};
const ImVec4 LightOrange   = {1.0f, 0.6f, 0.2f, 1.0f};
const ImVec4 VividOrange   = {1.0f, 0.4f, 0.0f, 1.0f};
const ImVec4 Orange        = {1.0f, 0.6f, 0.0f, 1.0f};
} // namespace Colors

void open_link(const char* url)
{
	std::wstring wide_url = StringUtils::ToWideString(url);
	ShellExecute(NULL, L"open", wide_url.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void open_link(const wchar_t* url) { ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL); }

// old bummy shit
void ClickableLink(const char* label, const char* url, const ImVec4& textColor, ImVec2 size)
{
	// default size of selectable is just size of label text
	if (size.x == 0 && size.y == 0)
		size = ImGui::CalcTextSize(label);

	ImGui::PushStyleColor(ImGuiCol_Text, textColor);

	if (ImGui::Selectable(label, false, ImGuiSelectableFlags_None, size))
		open_link(url);

	ImGui::PopStyleColor();

	if (ImGui::IsItemHovered())
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
		ImGui::SetTooltip("%s", url);
	}
}

void Spacing(int amount)
{
	for (int i = 0; i < amount; ++i)
		ImGui::Spacing();
}

void SameLineSpacing_relative(float horizontalSpacingPx)
{
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + horizontalSpacingPx);
}

void SameLineSpacing_absolute(float horizontalSpacingPx)
{
	ImGui::SameLine();
	ImGui::SetCursorPosX(horizontalSpacingPx);
}

void ToolTip(const char* tip)
{
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", tip);
}

void centerTextX(const char* text, float offsetCorrection)
{
	ImVec2 text_size        = ImGui::CalcTextSize(text);
	float  horizontalOffset = (ImGui::GetContentRegionAvail().x - text_size.x) * 0.5f;
	ImGui::SetCursorPosX(horizontalOffset + offsetCorrection);
	ImGui::TextUnformatted(text);
}

void centerTextColoredX(const ImVec4& col, const char* text, float offsetCorrection)
{
	ImVec2 text_size        = ImGui::CalcTextSize(text);
	float  horizontalOffset = (ImGui::GetContentRegionAvail().x - text_size.x) * 0.5f;
	ImGui::SetCursorPosX(horizontalOffset + offsetCorrection);
	ImGui::TextColored(col, "%s", text);
}

void SettingsHeader(const char* id, const char* pluginVersion, const ImVec2& size, bool showBorder)
{
	if (ImGui::BeginChild(id, size, showBorder))
	{
		Spacing(4);

		ImGui::TextColored(Colors::Pinkish, "Plugin made by SSLow");

		Spacing(3);

		ImGui::Text("%s", pluginVersion);
		ImGui::Separator();
	}
	ImGui::EndChild();
}

void OldSettingsFooter(const char* id, const ImVec2& size, bool showBorder)
{
	if (ImGui::BeginChild(id, size, showBorder))
	{
		constexpr const char* linkText = "Need help? Join the Discord";

		// center the the cursor position horizontally
		ImVec2 text_size         = ImGui::CalcTextSize(linkText);
		float  horizontal_offset = (ImGui::GetContentRegionAvail().x - text_size.x) / 2;
		horizontal_offset -= 50; // seems slightly shifted to the right, so subtract 50 to compensate
		ImGui::SetCursorPosX(horizontal_offset);

		// make link a lil more visible
		ImGui::SetWindowFontScale(1.3);

		ClickableLink(linkText, "https://discord.gg/tHZFsMsvDU", Colors::Yellow, text_size);
	}
	ImGui::EndChild();
}

void alt_settings_header(const char*    text,
    const char*                         currentPluginVersion,
    const std::shared_ptr<GameWrapper>& gw,
    bool                                isPluginUpdater,
    const ImVec4&                       text_color)
{
	Spacing(4);

	ImGui::TextColored(text_color, "%s", text);

	Spacing(3);

	ImGui::Text("%s", currentPluginVersion);

	plugin_update_message(gw, isPluginUpdater);

	ImGui::Separator();
}

void alt_settings_footer(const char* text, const char* url, const ImVec4& text_color)
{
	ImGui::SetWindowFontScale(1.3); // make link a lil more visible

	// center the the cursor position horizontally
	ImVec2 text_size         = ImGui::CalcTextSize(text);
	float  horizontal_offset = (ImGui::GetContentRegionAvail().x - text_size.x) / 2;
	horizontal_offset -= 50; // seems slightly shifted to the right, so subtract 50 to compensate
	ImGui::SetCursorPosX(horizontal_offset);

	ClickableLink(text, url, text_color, text_size);

	ImGui::SetWindowFontScale(1); // undo font scale modification, so it doesnt affect the rest of the UI
}

void plugin_update_message(const std::shared_ptr<GameWrapper>& gw, bool isPluginUpdater)
{
	PluginUpdates::PluginUpdateInfo updateStatus;
	{
		std::lock_guard<std::mutex> lock(PluginUpdates::updateMutex);
		updateStatus = PluginUpdates::updateInfo;
	}

	if (updateStatus.outOfDate)
	{
		GUI::SameLineSpacing_relative(20);

		ImGui::TextColored(GUI::Colors::Red, "<------ PLUGIN IS OUT OF DATE!");
		GUI::ToolTip(isPluginUpdater
		                 ? "Click \"Latest version\" and follow the install steps on the GitHub release page\t(it's very easy)"
		                 : "Using an outdated version can cause features to work incorrectly or even crash the game!\n\n"
		                   "Click \"Update\" (and wait a few seconds) to update the plugin.\nIf that doesn't work, click \"Latest "
		                   "version\" and follow the steps on the "
		                   "GitHub release page\t(it's very easy)");

		GUI::SameLineSpacing_relative(20);
		ImGui::Text("The latest version is v%s:", updateStatus.latestVersion.c_str());

		GUI::SameLineSpacing_relative(10);

		if (!isPluginUpdater)
		{
			if (ImGui::Button("Update"))
				PluginUpdates::installUpdate(gw);
			GUI::ToolTip(
			    "Click this and wait a few seconds to allow the plugin to update.\n\nIf you spam this button and it messes stuff up, "
			    "that's your fault.");

			GUI::SameLineSpacing_relative(10);
		}

		GUI::ClickableLink("Latest version", updateStatus.releaseUrl.c_str(), GUI::Colors::Green);
	}
}
} // namespace GUI
