#include "SoundFX.hpp"
#include "esp_log.h"

static const char* TAG = "SoundFX";

SoundFX::SoundFX()
{
}

void SoundFX::Init()
{
    // TODO: Initialize the i2s driver
}

void SoundFX::Start()
{
    // TODO: Create a task for i2s
}

bool SoundFX::PlaySound(FileID soundFile, bool repeat)
{
    const SoundFile* sound_file = GetFile(soundFile);
    if (nullptr == sound_file) {
        return false;
    }
    ESP_LOGI(TAG, "TODO: Play sound file, name: %s", sound_file->name);
    // TODO: Fill this
    return true;
}

void SoundFX::StopSound()
{
    // TODO: Fill this
    ESP_LOGI(TAG, "TODO: Stop playing sound");
}
