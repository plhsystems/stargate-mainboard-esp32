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
    BaseGate baseGate = GateFactory::Get(BaseGate::Type::Kristian);

    Wormhole wormhole(Wormhole::EType::NormalSG1);

    // Dialing
    while(true)
    {
        m_bIsCancelAction = false;

        baseGate.UnlockGate(); //Only apply on universe

        baseGate.GoHome();

        BaseGate::Chevron currentChevron = BaseGate::Chevron::Chevron1;
        baseGate.MoveToSymbol(20, currentChevron);
        // Chevrons
        baseGate.LockChevron();
        vTaskDelay(pdMS_TO_TICKS(500));
        baseGate.UnlockChevron();

        baseGate.LightUpSymbol(20, true); //Only apply on universe
        vTaskDelay(pdMS_TO_TICKS(500));
        baseGate.LightUpChevron(currentChevron, true);

        // &m_bIsCancelAction
        wormhole.OpenAnimation();
        wormhole.Run(&gc->m_bIsCancelAction);
        wormhole.CloseAnimation();

        baseGate.ShutdownGate();

        baseGate.GoHome();

        baseGate.LockGate(); //Only apply on universe
    }
}