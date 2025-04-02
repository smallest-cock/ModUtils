#include "pch.h"
#include "GuiTools.hpp"


namespace GUI
{
	namespace Colors
	{
		const ImVec4 White =				{ 1, 1, 1, 1 };
		const ImVec4 Red =					{ 1, 0, 0, 1 };
		const ImVec4 Green =				{ 0, 1, 0, 1 };
		const ImVec4 Blue =					{ 0, 0, 1, 1 };

		const ImVec4 Yellow =				{ 1, 1, 0, 1 };
		const ImVec4 BlueGreen =			{ 0, 1, 1, 1 };
		const ImVec4 Pinkish =				{ 1, 0, 1, 1 };

		const ImVec4 LightBlue =			{ 0.5f, 0.5f, 1.0f, 1.0f };
		const ImVec4 LightRed =				{ 1.0f, 0.4f, 0.4f, 1.0f };
		const ImVec4 LightGreen =			{ 0.5f, 1.0f, 0.5f, 1.0f };

		const ImVec4 DarkGreen =			{ 0.0f, 0.5f, 0.0f, 1.0f };
		const ImVec4 ForestGreen =			{ 0.3f, 0.5f, 0.3f, 1.0f };
		const ImVec4 RealDarkGreen =		{ 0.1f, 0.3f, 0.1f, 1.0f };

		const ImVec4 Gray =					{ 0.4f, 0.4f, 0.4f, 1.0f };
	}


	void open_link(const char* url)
	{
		std::wstring wide_url = StringUtils::ToWideString(url);
		ShellExecute(NULL, L"open", wide_url.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}

	void open_link(const wchar_t* url)
	{
		ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
	}


	// old bummy shit
	void ClickableLink(const char* label, const char* url, const ImVec4& textColor, ImVec2 size)
	{
		// default size of selectable is just size of label text
		if (size.x == 0 && size.y == 0)
		{
			size = ImGui::CalcTextSize(label);
		}

		ImGui::PushStyleColor(ImGuiCol_Text, textColor);

		if (ImGui::Selectable(label, false, ImGuiSelectableFlags_None, size))
		{
			open_link(url);
		}

		ImGui::PopStyleColor();
		
		if (ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
			ImGui::SetTooltip(url);
		}
	}


	void Spacing(int amount)
	{
		for (int i = 0; i < amount; i++) {
			ImGui::Spacing();
		}
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


	void SettingsHeader(const char* id, const char* pluginVersion, const ImVec2& size, bool showBorder)
	{
		if (ImGui::BeginChild(id, size, showBorder))
		{
			Spacing(4);

			ImGui::TextColored(Colors::Pinkish, "Plugin made by SSLow");

			Spacing(3);

			ImGui::Text(pluginVersion);
			ImGui::Separator();
		}
		ImGui::EndChild();
	}

	void SettingsFooter(const char* id, const ImVec2& size, std::shared_ptr<FooterLinks> footer_links, bool showBorder)
	{
		if (ImGui::BeginChild(id, size, showBorder))
		{
			footer_links->display();
		}
		ImGui::EndChild();
	}

	void OldSettingsFooter(const char* id, const ImVec2& size, bool showBorder)
	{
		if (ImGui::BeginChild(id, size, showBorder))
		{
			constexpr const char* linkText = "Need help? Join the Discord";

			// center the the cursor position horizontally
			ImVec2 text_size = ImGui::CalcTextSize(linkText);
			float horizontal_offset = (ImGui::GetContentRegionAvail().x - text_size.x) / 2;
			horizontal_offset -= 50;	// seems slightly shifted to the right, so subtract 50 to compensate
			ImGui::SetCursorPosX(horizontal_offset);

			// make link a lil more visible
			ImGui::SetWindowFontScale(1.3);

			ClickableLink(linkText, "https://discord.gg/tHZFsMsvDU", Colors::Yellow, text_size);
		}
		ImGui::EndChild();
	}


	void alt_settings_header(const char* text, const char* plugin_version, const ImVec4& text_color)
	{
		Spacing(4);

		ImGui::TextColored(text_color, text);

		Spacing(3);

		ImGui::Text(plugin_version);

		plugin_update_message();

		ImGui::Separator();
	}

	void alt_settings_footer(const char* text, const char* url, const ImVec4& text_color)
	{
		ImGui::SetWindowFontScale(1.3);	// make link a lil more visible

		// center the the cursor position horizontally
		ImVec2 text_size = ImGui::CalcTextSize(text);
		float horizontal_offset = (ImGui::GetContentRegionAvail().x - text_size.x) / 2;
		horizontal_offset -= 50;	// seems slightly shifted to the right, so subtract 50 to compensate
		ImGui::SetCursorPosX(horizontal_offset);

		ClickableLink(text, url, text_color, text_size);

		ImGui::SetWindowFontScale(1);	// undo font scale modification, so it doesnt affect the rest of the UI
	}

	void plugin_update_message()
	{
		PluginUpdates::PluginUpdateResponse update_status;

		{
			std::lock_guard<std::mutex> lock(PluginUpdates::update_mutex);
			update_status = PluginUpdates::update_response;
		}

		if (update_status.out_of_date)
		{
			GUI::SameLineSpacing_relative(20);

			ImGui::TextColored(GUI::Colors::Red, "<------ PLUGIN IS OUT OF DATE!");
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip(
					"Using an outdated version can cause features to work incorrectly or even crash the game!\n\n"
					"Unfortunately there's no built in auto-update system for plugins that aren't on the BakkesPlugins website :(\n\n"
					"So just follow the install steps on the latest GitHub release to update manually\t(it's very easy)"
				);
			}
			GUI::SameLineSpacing_relative(20);
			ImGui::Text("The latest version is v%s:", update_status.latest_version.c_str());

			GUI::SameLineSpacing_relative(10);

			GUI::ClickableLink("Update to latest version", update_status.release_url.c_str(), GUI::Colors::Green);
		}
	}
}
