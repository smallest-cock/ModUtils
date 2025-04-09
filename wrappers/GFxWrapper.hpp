#pragma once
#include <Components/Components/Instances.hpp>
#include "../gui/GuiTools.hpp"


class GfxWrapper
{
private:
	UGFxDataRow_X* gfx_row = nullptr;
	UGFxDataStore_X* datastore = nullptr;

public:
	GfxWrapper() = delete;
	GfxWrapper(UGFxDataRow_X* ui_obj) : gfx_row(ui_obj) { update_datastore(); }

	~GfxWrapper() {}

	UGFxDataRow_X* get_gfx_row();
	UGFxDataStore_X* get_datastore();
	[[nodiscard]] bool is_null();

	// setters
	void set_string(const std::string& col_name, const std::string& value);
	void set_string(const FName& col_name, const FString& value);
	
	void set_int(const std::string& col_name, int32_t value);
	void set_int(const FName& col_name, int32_t value);
	
	void set_float(const std::string& col_name, float value);
	void set_float(const FName& col_name, float value);
	
	void set_bool(const std::string& col_name, bool value);
	void set_bool(const FName& col_name, bool value);
	
	void set_texture(const std::string& col_name, UTexture* value);
	void set_texture(const FName& col_name, UTexture* value);

	void set_color(const std::string& col_name, const GUI::Colors::Color& value);
	void set_color(const std::string& col_name, const FColor& value);
	void set_color(const std::string& col_name, const FLinearColor& value);
	void set_color(const std::string& col_name, int32_t value);

	// getters
	std::string get_string(const std::string& col_name);
	std::string get_string(const FName& col_name);

	int get_int(const std::string& col_name);
	int get_int(const FName& col_name);
	
	float get_float(const std::string& col_name);
	float get_float(const FName& col_name);
	
	bool get_bool(const std::string& col_name);
	bool get_bool(const FName& col_name);

	UTexture* get_texture(const std::string& col_name);
	UTexture* get_texture(const FName& col_name);

private:
	void update_datastore();
	bool unable_to_set_value(const std::string& val_type);
};

