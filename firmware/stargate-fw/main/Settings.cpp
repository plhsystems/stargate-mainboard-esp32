#include "Settings.hpp"
#include <cstring>

Settings::Settings()
{

}

void Settings::Init()
{
    const NVSJSON_ESETRET ret = NVSJSON_Init(&m_sSettingHandle, &m_sSettingConfig);
    assert(NVSJSON_ESETRET_OK == ret);
}

void Settings::Load()
{
    NVSJSON_Load(&m_sSettingHandle);
}

void Settings::Commit()
{
    NVSJSON_Save(&m_sSettingHandle);
}

int32_t Settings::GetValueInt32(Settings::Entry eEntry)
{
    return NVSJSON_GetValueInt32(&m_sSettingHandle, (uint16_t)eEntry);
}

NVSJSON_ESETRET Settings::SetValueInt32(Settings::Entry eEntry, bool bIsDryRun, int32_t s32NewValue)
{
    return NVSJSON_SetValueInt32(&m_sSettingHandle, (uint16_t)eEntry, bIsDryRun, s32NewValue);
}

NVSJSON_ESETRET Settings::SetValueInt32(Settings::Entry eEntry, int32_t s32NewValue)
{
    return NVSJSON_SetValueInt32(&m_sSettingHandle, (uint16_t)eEntry, false, s32NewValue);
}

void Settings::GetValueString(Settings::Entry eEntry, char* out_value, size_t* length)
{
    NVSJSON_GetValueString(&m_sSettingHandle, (uint16_t)eEntry, out_value, length);
}

NVSJSON_ESETRET Settings::SetValueString(Settings::Entry eEntry, bool bIsDryRun, const char* szValue)
{
    return NVSJSON_SetValueString(&m_sSettingHandle, (uint16_t)eEntry, bIsDryRun, szValue);
}

char* Settings::ExportJSON()
{
    return NVSJSON_ExportJSON(&m_sSettingHandle);
}

bool Settings::ImportJSON(const char* szJSON)
{
    return NVSJSON_ImportJSON(&m_sSettingHandle, szJSON);
}

bool Settings::ValidateWifiPassword(const NVSJSON_SSettingEntry* pSettingEntry, const char* szValue)
{
    const size_t n = strlen(szValue);
    return n > 8 || n == 0;
}
