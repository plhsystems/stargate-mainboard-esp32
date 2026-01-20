#pragma once

#include "HW/SGHW_HAL.hpp"
#include "WebServer/WebServer.hpp"

class App
{
    public:
    struct Config
    {
        SGHW_HAL* m_sghw_hal;
    };

    public:
    App() = default;

    void Init(Config* config);

    void LoopTick();

    private:
    Config* m_config;
};