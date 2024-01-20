#ifndef _USB_DHD_H_
#define _USB_DHD_H_

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class USB_DHD
{
    public:

    private:
    USB_DHD();

    public:
    // Singleton pattern
    USB_DHD(USB_DHD const&) = delete;
    void operator=(USB_DHD const&) = delete;

    public:
    void Init();

    void Start();

    // static void TaskRunning(void* pArg);

    static USB_DHD& getI()
    {
        static USB_DHD instance;
        return instance;
    }
    private:
    TaskHandle_t m_sTaskUSBHandle;
};

#endif
