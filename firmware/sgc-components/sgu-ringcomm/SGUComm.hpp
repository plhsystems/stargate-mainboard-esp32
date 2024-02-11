#pragma once
#include <stdint.h>
#include <stdbool.h>

namespace SGUCommNS
{
    enum class EProtocolCmd
    {
        TurnOff = 1,
        UpdateLight = 2,
        ChevronsLightning = 3,
        GotoFactory = 4,
        //RESERVED = 5,
        PingPong = 6,
    };

    struct sRGB
    {
        uint8_t u8Red;
        uint8_t u8Green;
        uint8_t u8Blue;
    };

    enum class EChevronAnimation
    {
        FadeIn = 0,
        FadeOut = 1,
        ErrorToWhite = 2,
        ErrorToOff = 3,
        AllSymbolsOn = 4,
        PoweringOff = 5,

        Count
    };

    struct SUpdateLightArg
    {
        sRGB sColor;
        const uint8_t* u8Lights;
        uint8_t u8LightCount;
    };

    struct SChevronsLightningArg
    {
        EChevronAnimation eChevronAnim;
    };

    struct SPingPongArg
    {
        uint32_t u32PingPong;
    };

    typedef void(*fnUpdateLight)(const SUpdateLightArg* psArg);
    typedef void(*fnTurnOff)(void);
    typedef void(*fnChevronsLightning)(const SChevronsLightningArg* psChevronLightningArg);

    typedef void(*fnGotoFactory)();

    typedef void(*fnPingPong)(const SPingPongArg* psArg);

    typedef struct
    {
        fnTurnOff fnTurnOffHandler;

        fnUpdateLight fnUpdateLightHandler;
        fnChevronsLightning fnChevronsLightningHandler;

        fnGotoFactory fnGotoFactoryHandler;

        fnPingPong fnPingPongHandler;
    } SConfig;

    class SGUComm
    {
        public:
        static bool Decode(const SConfig& refConfig, const uint8_t* u8Datas, uint16_t u16Length);

        // Encode keep alive command.
        static uint32_t EncTurnOff(uint8_t* u8Dst, uint16_t u16MaxLen);
        static uint32_t EncUpdateLight(uint8_t* u8Dst, uint16_t u16MaxLen, const SUpdateLightArg* psArg);
        static uint32_t EncChevronLightning(uint8_t* u8Dst, uint16_t u16MaxLen, const SChevronsLightningArg* psArg);
        static uint32_t EncGotoFactory(uint8_t* u8Dst, uint16_t u16MaxLen);
        
        static uint32_t EncPingPong(uint8_t* u8Dst, uint16_t u16MaxLen, const SPingPongArg* psArg);
    };
}
