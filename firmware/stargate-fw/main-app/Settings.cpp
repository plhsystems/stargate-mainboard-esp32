#include "Settings.hpp"
#include <cstring>

void Settings::Init()
{
    const NVSJSON_ESETRET ret = NVSJSON_Init(&m_setting_handle, &m_setting_config);
    assert(NVSJSON_ESETRET_OK == ret);
}

void Settings::Load()
{
    NVSJSON_Load(&m_setting_handle);
}

void Settings::Commit()
{
    NVSJSON_Save(&m_setting_handle);
}

int32_t Settings::GetValueInt32(Settings::Entry entry)
{
    return NVSJSON_GetValueInt32(&m_setting_handle, (uint16_t)entry);
}

NVSJSON_ESETRET Settings::SetValueInt32(Settings::Entry entry, bool is_dry_run, int32_t new_value)
{
    return NVSJSON_SetValueInt32(&m_setting_handle, (uint16_t)entry, is_dry_run, new_value);
}

NVSJSON_ESETRET Settings::SetValueInt32(Settings::Entry entry, int32_t new_value)
{
    return NVSJSON_SetValueInt32(&m_setting_handle, (uint16_t)entry, false, new_value);
}

void Settings::GetValueString(Settings::Entry entry, char* out_value, size_t* length)
{
    NVSJSON_GetValueString(&m_setting_handle, (uint16_t)entry, out_value, length);
}

NVSJSON_ESETRET Settings::SetValueString(Settings::Entry entry, bool is_dry_run, const char* value)
{
    return NVSJSON_SetValueString(&m_setting_handle, (uint16_t)entry, is_dry_run, value);
}

double Settings::GetValueDouble(Settings::Entry entry)
{
    return NVSJSON_GetValueDouble(&m_setting_handle, (uint16_t)entry);
}

NVSJSON_ESETRET Settings::SetValueDouble(Settings::Entry entry, bool is_dry_run, double value)
{
    return NVSJSON_SetValueDouble(&m_setting_handle, (uint16_t)entry, is_dry_run, value);
}

NVSJSON_ESETRET Settings::SetValueDouble(Settings::Entry entry, double value)
{
    return NVSJSON_SetValueDouble(&m_setting_handle, (uint16_t)entry, false, value);
}

char* Settings::ExportJSON()
{
    return NVSJSON_ExportJSON(&m_setting_handle);
}

bool Settings::ImportJSON(const char* json)
{
    return NVSJSON_ImportJSON(&m_setting_handle, json);
}

bool Settings::ValidateWifiPassword(const NVSJSON_SSettingEntry* setting_entry, const char* value)
{
    const size_t n = strlen(value);
    return n > 8 || n == 0;
}
