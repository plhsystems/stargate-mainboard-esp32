#include "cJSON.h"
#include "WebServer.hpp"
#include "Gate/BaseGate.hpp"
#include "Gate/GateFactory.hpp"
#include "SGUComm.hpp"
#include "../Wormhole/Wormhole.hpp"

char* WebServer::GetGalaxyInfoJSON(GateGalaxy gate_galaxy)
{
    cJSON* root = NULL;
    {
        root = cJSON_CreateObject();
        if (root == NULL)
            goto ERROR;

        BaseGate& bg = GateFactory::Get(gate_galaxy);
        // TODO: Add a cache expiration value

        cJSON_AddItemToObject(root, "name", cJSON_CreateString(bg.name));
        // ------------------------------
        // Symbols
        cJSON* symbols_entries = cJSON_AddArrayToObject(root, "symbols");
        for(int i = 1; i <= bg.GetSymbolCount(); i++)
        {
            const GateSymbol& sym = bg.GetSymbol(i);

            cJSON* new_file = cJSON_CreateObject();
            cJSON_AddItemToObject(new_file, "id", cJSON_CreateNumber((int)sym.number));
            cJSON_AddItemToObject(new_file, "name", cJSON_CreateString(sym.name));
            cJSON_AddItemToArray(symbols_entries, new_file);
        }

        // ------------------------------
        // Gate Address
        cJSON* addresses_entries = cJSON_AddArrayToObject(root, "addresses");
        for(int i = 0; i < bg.GetAddressCount(); i++)
        {
            const GateAddress& gate_addr = bg.GetAddress(i);

            cJSON* new_file = cJSON_CreateObject();
            cJSON_AddItemToObject(new_file, "id", cJSON_CreateNumber(i));
            cJSON_AddItemToObject(new_file, "name", cJSON_CreateString(gate_addr.GetName()));
            // Address
            cJSON* address_entries = cJSON_AddArrayToObject(new_file, "address");
            for(int i = 0; i < gate_addr.GetSymbolCount(); i++) {
                const uint8_t symbol_num = gate_addr.GetSymbol(i);
                cJSON_AddItemToArray(address_entries, cJSON_CreateNumber(symbol_num));
            }
            cJSON_AddItemToArray(addresses_entries, new_file);
        }

        // ------------------------------
        // Ring (animations)
        if (GateGalaxy::Universe == gate_galaxy) {
            // There is no possible ring animation on other gates
            cJSON* ring_animation_entries = cJSON_AddArrayToObject(root, "ring_animations");
            for(int i = 0; i < (int)SGUCommNS::EChevronAnimation::Count; i++)
            {
                cJSON* new_file = cJSON_CreateObject();
                cJSON_AddItemToObject(new_file, "id", cJSON_CreateNumber(i));
                cJSON_AddItemToObject(new_file, "name", cJSON_CreateString(SGUCommNS::GetAnimationText( (SGUCommNS::EChevronAnimation)i)));
                cJSON_AddItemToArray(ring_animation_entries, new_file);
            }
        }

        // ------------------------------
        // Wormholes
        cJSON* wormhole_entries = cJSON_AddArrayToObject(root, "wormhole_types");
        for(int i = 0; i < (int)Wormhole::EType::Count; i++)
        {
            const GateAddress& gate_addr = bg.GetAddress(i);

            cJSON* new_file = cJSON_CreateObject();
            cJSON_AddItemToObject(new_file, "id", cJSON_CreateNumber(i));
            cJSON_AddItemToObject(new_file, "name", cJSON_CreateString(Wormhole::GetTypeText( (Wormhole::EType) i)));
            cJSON_AddItemToArray(wormhole_entries, new_file);
        }

        char* str = cJSON_PrintUnformatted(root);
        cJSON_Delete(root);
        return str;
    }
    ERROR:
    cJSON_Delete(root);
    return NULL;
}
