#include "SoundFX.hpp"
#include "esp_log.h"
#include "driver/uart.h"

static const char* TAG = "SoundFX";

SoundFX::SoundFX()
{
}

void SoundFX::Init(SGHW_HAL* p_sghw_hal)
{
    m_sghw_hal = p_sghw_hal;
}

void SoundFX::Start()
{
    // TODO: Create a task for i2s
}

bool SoundFX::PlaySound(FileID sound_file, bool repeat)
{
    const SoundFile* p_sound_file = GetFile(sound_file);
    if (nullptr == p_sound_file) {
        return false;
    }
    ESP_LOGI(TAG, "TODO: Play sound file, name: %s", p_sound_file->name);

    char buffers[64];
    // GetValueInt32(&g_sSettingHandle, SETTINGS_EENTRY_Mp3PlayerVolume)

    sprintf(buffers, "AT+VOL=%d\r\n", (int)100);
    m_sghw_hal->SendMp3PlayerCMD(buffers);
    vTaskDelay(pdMS_TO_TICKS(25));

    // Play once
    if (repeat)
        m_sghw_hal->SendMp3PlayerCMD("AT+PLAYMODE=1\r\n");
    else
        m_sghw_hal->SendMp3PlayerCMD("AT+PLAYMODE=3\r\n");
    vTaskDelay(pdMS_TO_TICKS(25));  // Not sure how long it needs to take the command but it doesn't like being spammed

    // Play number
    const uint32_t real_index = (uint32_t)sound_file + 1;
    sprintf(buffers, "AT+PLAYNUM=%d\r\n", (int)real_index);
    m_sghw_hal->SendMp3PlayerCMD(buffers);

    return true;
}

void SoundFX::StopSound()
{
    ESP_LOGI(TAG, "Stop playing sound");

    // We don't know when it is playing and there is no explicit STOP command. It just toggle
    // so the trick is to play something then immediately put it on pause.
    m_sghw_hal->SendMp3PlayerCMD("AT+PLAYMODE=3\r\n");
    vTaskDelay(pdMS_TO_TICKS(35));
    m_sghw_hal->SendMp3PlayerCMD("AT+PLAYNUM=6\r\n");
    vTaskDelay(pdMS_TO_TICKS(35));
    m_sghw_hal->SendMp3PlayerCMD("AT+PLAY=PP\r\n");
}