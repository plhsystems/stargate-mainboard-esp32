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

bool SoundFX::PlaySound(FileID soundFile, bool bRepeat)
{
    const SoundFile* pSoundFile = GetFile(soundFile);
    if (nullptr == pSoundFile) {
        return false;
    }
    ESP_LOGI(TAG, "TODO: Play sound file, name: %s", pSoundFile->szName);
    // TODO: Fill this
    return true;
}

void SoundFX::StopSound()
{
    // TODO: Fill this
    ESP_LOGI(TAG, "TODO: Stop playing sound");
}
