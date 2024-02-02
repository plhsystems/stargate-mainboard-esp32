#include "SGUComm.hpp"
#include <assert.h>
#include <string.h>

#define MAGIC0 0xBE
#define MAGIC1 0xEF
#define MAGIC_LENGTH 2

using namespace SGUCommNS;

bool SGUComm::Decode(const SConfig& refConfig, const uint8_t* u8Datas, uint16_t u16Length)
{
    if (u16Length < MAGIC_LENGTH + 1)
        return false;

    if (u8Datas[0] != MAGIC0 || u8Datas[1] != MAGIC1)
        return false;

    u8Datas += MAGIC_LENGTH; // Skip header
    u16Length -= MAGIC_LENGTH;
    const EProtocolCmd eCmd = (EProtocolCmd)u8Datas[0];
    switch(eCmd)
    {
        case EProtocolCmd::KeepAlive:
        {
            if (u16Length < 5)
                return false;
            const SKeepAliveArg sKeepAlive =
            {
                .u32MaximumTimeMS = (uint32_t)((u8Datas[1] << 24U) | (u8Datas[2] << 16U) | (u8Datas[3] << 8U) | u8Datas[4])
            };
            if (refConfig.fnKeepAliveHandler != NULL)
                refConfig.fnKeepAliveHandler(&sKeepAlive);
            return true;
        }
        case EProtocolCmd::UpdateLight:
        {
            // CMD | R | G | B | LIGHT0 | LIGHT1 | ...
            if (u16Length < 5)
                return false;

            // Lights
            uint8_t u8LightCount = u16Length - 4;

            const SUpdateLightArg sUpdateLightArg =
            {
                .sColor = { .u8Red = u8Datas[1], .u8Green = u8Datas[2], .u8Blue = u8Datas[3] },
                .u8Lights = u8Datas+4,
                .u8LightCount = u8LightCount
            };

            if (refConfig.fnUpdateLightHandler != NULL)
                refConfig.fnUpdateLightHandler(&sUpdateLightArg);

            return true;
        }
        case EProtocolCmd::TurnOff:
        {
            if (refConfig.fnTurnOffHandler != NULL)
                refConfig.fnTurnOffHandler();
            return true;
        }
        case EProtocolCmd::GotoFactory:
        {
            if (refConfig.fnGotoFactoryHandler != NULL)
                refConfig.fnGotoFactoryHandler();
            return true;
        }
        case EProtocolCmd::GotoOTAMode:
        {
            if (refConfig.fnGotoOTAModeHandler != NULL)
                refConfig.fnGotoOTAModeHandler();
            return true;
        }
        case EProtocolCmd::ChevronsLightning:
        {
            // CMD | ANIM ...
            if (u16Length < 2)
                return false;

            const EChevronAnimation eChevronAnim = (EChevronAnimation)u8Datas[1];
            if (eChevronAnim >= EChevronAnimation::Count)
                return false;

            const SChevronsLightningArg sChevronsLightningArg =
            {
                .eChevronAnim = eChevronAnim
            };

            if (refConfig.fnChevronsLightningHandler != NULL)
                refConfig.fnChevronsLightningHandler(&sChevronsLightningArg);
            return true;
        }
    }
    return false;
}

uint32_t SGUComm::EncKeepAlive(uint8_t* u8Dst, uint16_t u16MaxLen, const SKeepAliveArg* psArg)
{
    const uint16_t u16ReqLength = MAGIC_LENGTH+5;
    if (u16MaxLen < u16ReqLength)
        return 0;

    u8Dst[0] = MAGIC0;
    u8Dst[1] = MAGIC1;
    u8Dst[2] = (uint8_t)EProtocolCmd::KeepAlive;
    u8Dst[3] = (uint8_t)(psArg->u32MaximumTimeMS >> 24U);
    u8Dst[4] = (uint8_t)(psArg->u32MaximumTimeMS >> 16U);
    u8Dst[5] = (uint8_t)(psArg->u32MaximumTimeMS >> 8U);
    u8Dst[6] = (uint8_t)(psArg->u32MaximumTimeMS);
    return u16ReqLength;
}

uint32_t SGUComm::EncTurnOff(uint8_t* u8Dst, uint16_t u16MaxLen)
{
    const uint16_t u16ReqLength = MAGIC_LENGTH+1;
    if (u16MaxLen < u16ReqLength)
        return 0;

    u8Dst[0] = MAGIC0;
    u8Dst[1] = MAGIC1;
    u8Dst[2] = (uint8_t)EProtocolCmd::TurnOff;
    return u16ReqLength;
}

uint32_t SGUComm::EncGotoFactory(uint8_t* u8Dst, uint16_t u16MaxLen)
{
    const uint16_t u16ReqLength = MAGIC_LENGTH+1;
    if (u16MaxLen < u16ReqLength)
        return 0;

    u8Dst[0] = MAGIC0;
    u8Dst[1] = MAGIC1;
    u8Dst[2] = (uint8_t)EProtocolCmd::GotoFactory;
    return u16ReqLength;
}

uint32_t SGUComm::EncGotoOTAMode(uint8_t* u8Dst, uint16_t u16MaxLen)
{
    const uint16_t u16ReqLength = MAGIC_LENGTH+1;
    if (u16MaxLen < u16ReqLength)
        return 0;

    u8Dst[0] = MAGIC0;
    u8Dst[1] = MAGIC1;
    u8Dst[2] = (uint8_t)EProtocolCmd::GotoOTAMode;
    return u16ReqLength;
}

uint32_t SGUComm::EncUpdateLight(uint8_t* u8Dst, uint16_t u16MaxLen, const SUpdateLightArg* psArg)
{
    const uint16_t u16ReqLength = MAGIC_LENGTH + /*Cmd*/1 + /*R|G|B*/3 + /*Light indexes...*/psArg->u8LightCount;
    if (u16MaxLen < u16ReqLength)
        return 0;

    u8Dst[0] = MAGIC0;
    u8Dst[1] = MAGIC1;
    u8Dst[2] = (uint8_t)EProtocolCmd::UpdateLight;
    u8Dst[3] = psArg->sColor.u8Red;
    u8Dst[4] = psArg->sColor.u8Green;
    u8Dst[5] = psArg->sColor.u8Blue;
    memcpy(u8Dst+6, psArg->u8Lights, psArg->u8LightCount);
    return u16ReqLength;
}

uint32_t SGUComm::EncChevronLightning(uint8_t* u8Dst, uint16_t u16MaxLen, const SChevronsLightningArg* psArg)
{
    const uint16_t u16ReqLength = MAGIC_LENGTH + /*Cmd*/1 + /*Anim Type*/1;
    if (u16MaxLen < u16ReqLength)
        return 0;

    u8Dst[0] = MAGIC0;
    u8Dst[1] = MAGIC1;
    u8Dst[2] = (uint8_t)EProtocolCmd::ChevronsLightning;
    u8Dst[3] = (uint8_t)psArg->eChevronAnim;
    return u16ReqLength;
}