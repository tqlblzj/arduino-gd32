/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2010 LeafLabs LLC.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *****************************************************************************/
#include "arduino.h"
#include "gd32vw55x_platform.h"
#include "wrapper_os.h"
#include "util.h"
#include "user_setting.h"
#include "wifi_init.h"


// Force init to be called *first*, i.e. before static object allocation.
// Otherwise, statically allocated objects that need libmaple may fail.
__attribute__((constructor(101))) void premain()
{
    init();
}
static void arduino_main_task(void *param)
{
    setup();

    while(1) {
        loop();
    }
}

/*!
    \brief      Init applications.
                This function is called to initialize all the applications.
    \param[in]  none.
    \param[out] none.
    \retval     none.
*/
static void application_init(void)
{
    #ifdef CFG_WLAN_SUPPORT
        if (wifi_init()) {
            printf("wifi init failed\r\n");
        }
    #endif
    if (sys_task_create_dynamic((const uint8_t *)"arduino_main_task",
            5120, OS_TASK_PRIORITY(3), arduino_main_task, NULL) == NULL) {
        printf("arduino_main_task create failed\r\n");
    }
}

int main(void)
{
    sys_os_init();

    platform_init();

    application_init();

    sys_os_start();

    return 0;
}
