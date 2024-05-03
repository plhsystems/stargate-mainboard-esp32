#pragma once

#include "HW/SGHW_HAL.hpp"
#include "WebServer/WebServer.hpp"

class App
{
    public:
    struct Config
    {
        SGHW_HAL* pSGHWHal;
    };

    public:
    App() = default;

    void Init(Config* psConfig);

    void LoopTick();

    private:
    Config* m_psConfig;
};