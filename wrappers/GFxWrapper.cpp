#include "pch.h"
#include "GFxWrapper.hpp"


// ============================================================================================================
// =========================================== GFxWrapper class ===============================================
// ============================================================================================================


UGFxDataRow_X* GfxWrapper::get_gfx_row() { return gfx_row; }

UGFxDataStore_X* GfxWrapper::get_datastore()
{
	update_datastore();
	return datastore;
}

bool GfxWrapper::is_null() { return gfx_row == nullptr; }


void GfxWrapper::update_datastore()
{
	if (!gfx_row)
	{
		LOG("ok gfx_row is null");
		return;
	}

	auto shell = gfx_row->Shell;
	if (!shell)
	{
		LOG("ok gfx_row->Shell is null");
		return;
	}

	auto ds = shell->DataStore;
	if (!ds)
	{
		LOG("ok shell->DataStore is null");
		return;
	}

	datastore = ds;
}


bool GfxWrapper::unable_to_set_value(const std::string& val_type)
{
	if (!gfx_row)
	{
		LOG("[ERROR] Unable to set {} value! UGFxDataRow_X* is null", val_type);
		return true;
	}

	if (!datastore)
	{
		update_datastore();
		if (!datastore)
		{
			LOG("[ERROR] Unable to set {} value! UGFxDataStore_X* is null (datstore pointer location: {})", val_type, Format::ToHexString(reinterpret_cast<uintptr_t>(gfx_row) + 0x088));
			return true;
		}
	}

	return false;
}




// setters
void GfxWrapper::set_string(const std::string& col_name, const std::string& value)
{
	set_string(FName::find(col_name), FString::create(value));
}

void GfxWrapper::set_string(const FName& col_name, const FString& value)
{
	if (unable_to_set_value("string"))
		return;

	datastore->SetStringValue(gfx_row->TableName, gfx_row->RowIndex, col_name, value);
}


void GfxWrapper::set_int(const std::string& col_name, int32_t value)
{
	set_int(FName::find(col_name), value);
}

void GfxWrapper::set_int(const FName& col_name, int32_t value)
{
	if (unable_to_set_value("int"))
		return;

	datastore->SetIntValue(gfx_row->TableName, gfx_row->RowIndex, col_name, value);
}


void GfxWrapper::set_float(const std::string& col_name, float value)
{
	set_float(FName::find(col_name), value);
}

void GfxWrapper::set_float(const FName& col_name, float value)
{
	if (unable_to_set_value("float"))
		return;

	datastore->SetFloatValue(gfx_row->TableName, gfx_row->RowIndex, col_name, value);
}


void GfxWrapper::set_bool(const std::string& col_name, bool value)
{
	set_bool(FName::find(col_name), value);
}

void GfxWrapper::set_bool(const FName& col_name, bool value)
{
	if (unable_to_set_value("bool"))
		return;

	datastore->SetBoolValue(gfx_row->TableName, gfx_row->RowIndex, col_name, value);
}


void GfxWrapper::set_texture(const std::string& col_name, UTexture* value)
{
	set_texture(FName::find(col_name), value);
}

void GfxWrapper::set_texture(const FName& col_name, UTexture* value)
{
	if (!value || unable_to_set_value("texture"))
		return;

	datastore->SetTextureValue(gfx_row->TableName, gfx_row->RowIndex, col_name, value);
}



void GfxWrapper::set_color(const std::string& col_name, const GUI::Colors::Color& value)
{
	if (unable_to_set_value("color")) return;

	FASValue val;
	val.Type = static_cast<uint8_t>(EASType::AS_UInt);
	val.I = value.GetIntColor();

	datastore->SetASValue(gfx_row->TableName, gfx_row->RowIndex, FName::find(col_name), val);
}

void GfxWrapper::set_color(const std::string& col_name, const FColor& value)
{
	if (unable_to_set_value("color")) return;

	GUI::Colors::Color color{ value };

	FASValue val;
	val.Type = static_cast<uint8_t>(EASType::AS_UInt);
	val.I = color.GetIntColor();

	datastore->SetASValue(gfx_row->TableName, gfx_row->RowIndex, FName::find(col_name), val);
}

void GfxWrapper::set_color(const std::string& col_name, const FLinearColor& value)
{
	if (unable_to_set_value("color")) return;

	GUI::Colors::Color color{ value };

	FASValue val;
	val.Type = static_cast<uint8_t>(EASType::AS_UInt);
	val.I = color.GetIntColor();

	datastore->SetASValue(gfx_row->TableName, gfx_row->RowIndex, FName::find(col_name), val);
}

void GfxWrapper::set_color(const std::string& col_name, int32_t value)
{
	if (unable_to_set_value("color")) return;

	FASValue val;
	val.Type = static_cast<uint8_t>(EASType::AS_UInt);
	val.I = value;

	datastore->SetASValue(gfx_row->TableName, gfx_row->RowIndex, FName::find(col_name), val);
}




// getters
std::string GfxWrapper::get_string(const std::string& col_name)
{
	return get_string(FName::find(col_name));
}

std::string GfxWrapper::get_string(const FName& col_name)
{
	return datastore->GetValue(gfx_row->TableName, gfx_row->RowIndex, col_name).S.ToString();
}


int GfxWrapper::get_int(const std::string& col_name)
{
	return get_int(FName::find(col_name));
}

int GfxWrapper::get_int(const FName& col_name)
{
	return datastore->GetValue(gfx_row->TableName, gfx_row->RowIndex, col_name).I;
}


float GfxWrapper::get_float(const std::string& col_name)
{
	return get_float(FName::find(col_name));
}

float GfxWrapper::get_float(const FName& col_name)
{
	return datastore->GetValue(gfx_row->TableName, gfx_row->RowIndex, col_name).N;
}


bool GfxWrapper::get_bool(const std::string& col_name)
{
	return get_bool(FName::find(col_name));
}

bool GfxWrapper::get_bool(const FName& col_name)
{
	return datastore->GetValue(gfx_row->TableName, gfx_row->RowIndex, col_name).B;
}


UTexture* GfxWrapper::get_texture(const std::string& col_name)
{
	return get_texture(FName::find(col_name));
}

UTexture* GfxWrapper::get_texture(const FName& col_name)
{
	return datastore->GetValue(gfx_row->TableName, gfx_row->RowIndex, col_name).T;
}
