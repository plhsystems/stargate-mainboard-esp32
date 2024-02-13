#include "cJSON.h"
#include "WebServer.hpp"
#include "Gate/BaseGate.hpp"
#include "Gate/GateFactory.hpp"

char* WebServer::GetGalaxyInfoJSON(GateGalaxy eGateGalaxy)
{
    cJSON* pRoot = NULL;
    {
        pRoot = cJSON_CreateObject();
        if (pRoot == NULL)
            goto ERROR;

        BaseGate& bg = GateFactory::Get(eGateGalaxy);
        // TODO: Add a cache expiration value

        cJSON_AddItemToObject(pRoot, "name", cJSON_CreateString(bg.szName));
        // ------------------------------
        // Symbols
        cJSON* pSymbolsEntries = cJSON_AddArrayToObject(pRoot, "symbols");
        for(int i = 1; i <= bg.GetSymbolCount(); i++)
        {
            const GateSymbol& sym = bg.GetSymbol(i);

            cJSON* pNewFile = cJSON_CreateObject();
            cJSON_AddItemToObject(pNewFile, "id", cJSON_CreateNumber((int)sym.u8Number));
            cJSON_AddItemToObject(pNewFile, "name", cJSON_CreateString(sym.szName));
            cJSON_AddItemToArray(pSymbolsEntries, pNewFile);
        }

        // ------------------------------
        // Gate Address
        cJSON* pAddressesEntries = cJSON_AddArrayToObject(pRoot, "addresses");
        for(int i = 0; i < bg.GetAddressCount(); i++)
        {
            const GateAddress& gateAddr = bg.GetAddress(i);

            cJSON* pNewFile = cJSON_CreateObject();
            cJSON_AddItemToObject(pNewFile, "id", cJSON_CreateNumber(i+1));
            cJSON_AddItemToObject(pNewFile, "name", cJSON_CreateString(gateAddr.GetName()));
            // Address
            cJSON* pAddressEntries = cJSON_AddArrayToObject(pNewFile, "address");
            for(int i = 0; i < gateAddr.GetSymbolCount(); i++) {
                const uint8_t u8SymbolNum = gateAddr.GetSymbol(i);
                cJSON_AddItemToArray(pAddressEntries, cJSON_CreateNumber(u8SymbolNum));
            }
            cJSON_AddItemToArray(pAddressesEntries, pNewFile);
        }

        char* pStr = cJSON_PrintUnformatted(pRoot);
        cJSON_Delete(pRoot);
        return pStr;
    }
    ERROR:
    cJSON_Delete(pRoot);
    return NULL;
}
