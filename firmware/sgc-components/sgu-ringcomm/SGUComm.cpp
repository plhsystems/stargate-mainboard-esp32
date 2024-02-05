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
    u8Datas++;
    switch(eCmd)
    {
        case EProtocolCmd::UpdateLight:
        {
            // R | G | B | LIGHT0 | LIGHT1 | ...
            if (u16Length < 4)
                return false;

            // Lights
            uint8_t u8LightCount = u16Length - 4;

            const SUpdateLightArg sUpdateLightArg =
            {
                .sColor = { .u8Red = u8Datas[0], .u8Green = u8Datas[1], .u8Blue = u8Datas[2] },
                .u8Lights = u8Datas+3,
                .u8LightCount = u8LightCount
            };

            if (refConfig.fnUpdateLightHandler != NULL) {
                refConfig.fnUpdateLightHandler(&sUpdateLightArg);
            }
            return true;
        }
        case EProtocolCmd::TurnOff:
        {
            if (refConfig.fnTurnOffHandler != NULL) {
                refConfig.fnTurnOffHandler();
            }
            return true;
        }
        case EProtocolCmd::GotoFactory:
        {
            if (refConfig.fnGotoFactoryHandler != NULL) {
                refConfig.fnGotoFactoryHandler();
            }
            return true;
        }
        case EProtocolCmd::ChevronsLightning:
        {
            // ANIM ...
            if (u16Length < 1)
                return false;

            const EChevronAnimation eChevronAnim = (EChevronAnimation)u8Datas[0];
            if (eChevronAnim >= EChevronAnimation::Count)
                return false;

            const SChevronsLightningArg sChevronsLightningArg =
            {
                .eChevronAnim = eChevronAnim
            };

            if (refConfig.fnChevronsLightningHandler != NULL) {
                refConfig.fnChevronsLightningHandler(&sChevronsLightningArg);
            }
            return true;
        }
        case EProtocolCmd::PingPong:
        {
            // PingPong
            if (u16Length < 4)
                return false;
            SPingPongArg sPongArg;
            memcpy(&sPongArg.u32PingPong, &u8Datas[0], sizeof(uint32_t));
            if (refConfig.fnPongHandler != NULL) {
                refConfig.fnPongHandler(&sPongArg);
            }
            return true;
        }
    }
    return false;
}

uint32_t SGUComm::EncTurnOff(uint8_t* u8Dst, uint16_t u16MaxLen)
{
    const uint16_t u16ReqLength = MAGIC_LENGTH+/*Cmd*/1;
    if (u16MaxLen < u16ReqLength)
        return 0;
    u8Dst[0] = MAGIC0;
    u8Dst[1] = MAGIC1;
    u8Dst[2] = (uint8_t)EProtocolCmd::TurnOff;
    return u16ReqLength;
}

uint32_t SGUComm::EncGotoFactory(uint8_t* u8Dst, uint16_t u16MaxLen)
{
    const uint16_t u16ReqLength = MAGIC_LENGTH+/*Cmd*/1;
    if (u16MaxLen < u16ReqLength)
        return 0;
    u8Dst[0] = MAGIC0;
    u8Dst[1] = MAGIC1;
    u8Dst[2] = (uint8_t)EProtocolCmd::GotoFactory;
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

uint32_t SGUComm::EncPingPong(uint8_t* u8Dst, uint16_t u16MaxLen, const SPingPongArg* psArg)
{
    const uint16_t u16ReqLength = MAGIC_LENGTH + /*Cmd*/1 + /*Anim Type*/4;
    if (u16MaxLen < u16ReqLength)
        return 0;
    u8Dst[0] = MAGIC0;
    u8Dst[1] = MAGIC1;
    u8Dst[2] = (uint8_t)EProtocolCmd::PingPong;
    memcpy(&u8Dst[3], &psArg->u32PingPong, sizeof(uint32_t));
    return u16ReqLength;
}