#include "GateControl.hpp"
#include "Wormhole/Wormhole.hpp"
#include "FWConfig.hpp"

#define TAG "GateControl"

GateControl::GateControl() :
    m_bIsCancelAction(false)
{
    // Constructor code here
}

void GateControl::Init()
{

}

void GateControl::StartTask()
{
	if (xTaskCreatePinnedToCore(TaskRunning, "GateControl", FWCONFIG_GATECONTROL_STACKSIZE, (void*)this, FWCONFIG_GATECONTROL_PRIORITY_DEFAULT, &m_sGateControlHandle, FWCONFIG_GATECONTROL_COREID) != pdPASS )
	{
		ESP_ERROR_CHECK(ESP_FAIL);
	}
}

void GateControl::QueueAction()
{

}

void GateControl::AbortAction()
{
    m_bIsCancelAction = true;
}

void GateControl::TaskRunning(void* pArg)
{
    GateControl* gc = (GateControl*)pArg;

    // Get a stargate instance based on parameters
    BaseGate& baseGate = GateFactory::Get(GateGalaxy::MilkyWay);

    Wormhole wormhole(Wormhole::EType::NormalSG1);

    // Do homing on startup
    //baseGate.UnlockGate(); //Only apply on universe
    //baseGate.GoHome();
    //baseGate.LockGate(); //Only apply on universe

    // Dialing
    while(true)
    {
        gc->m_bIsCancelAction = false;

        //baseGate.UnlockGate(); //Only apply on universe
        //baseGate.GoHome();

        const Chevron chevrons[] = { Chevron::Chevron1, Chevron::Chevron2, Chevron::Chevron3, Chevron::Chevron4, Chevron::Chevron5, Chevron::Chevron6, Chevron::Chevron7_Master };
        const uint8_t symbols[] = { 5, 10, 20, 30, 3, 16, 1 };

        for(int32_t i = 0; i < (sizeof(symbols)/sizeof(symbols[0])); i++)
        {
            const uint8_t symbol = symbols[i];
            const Chevron currentChevron = chevrons[i];

            //baseGate.MoveToSymbol(symbol, currentChevron);
            // Chevrons
            //baseGate.LockChevron();
            vTaskDelay(pdMS_TO_TICKS(500));
            //baseGate.UnlockChevron();

            //baseGate.LightUpSymbol(20, true); //Only apply on universe
            vTaskDelay(pdMS_TO_TICKS(500));
            //baseGate.LightUpChevron(currentChevron, true);
        }

        // Run wormhole animations
        wormhole.OpenAnimation();
        wormhole.Run(&gc->m_bIsCancelAction);
        wormhole.CloseAnimation();

        // Shutdown lightning and all
        //baseGate.ShutdownGate();

        //baseGate.GoHome();

        //baseGate.LockGate(); //Only apply on universe

        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}