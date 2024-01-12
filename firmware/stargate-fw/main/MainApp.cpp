#include <stdio.h>
#include "GateControl.hpp"

extern "C" {
    void app_main(void);
}

static GateControl m_gc;

void app_main(void)
{
    m_gc.StartTask();
}
