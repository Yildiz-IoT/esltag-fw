

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sys/printk.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

LOG_MODULE_REGISTER(AdvancedTag,LOG_LEVEL_INF);



int main(void)
{
        printk("Advanced Tag eTag App started on %s!\n", CONFIG_BOARD);


        LOG_INF("A log message in info level!");   
        LOG_DBG("A log message in debug level!");
        LOG_WRN("A log message in warning level!");
        LOG_ERR("A log message in Error level!");

        return 0;
}
