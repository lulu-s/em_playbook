#include "esp32/ulp.h"
// include ulp header you will create
#include "ulp_main.h"
// must include ulptool helper functions also
#include "ulptool.h"
#include "driver/rtc_io.h"
#include "driver/gpio.h"

#include <Wire.h>
#include <SparkFun_CAP1203_Registers.h>
#include <SparkFun_CAP1203_Types.h>
CAP1203 sensor;

// Unlike the esp-idf always use these binary blob names
extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[] asm("_binary_ulp_main_bin_end");

static void init_run_ulp(uint32_t usec);

void printBin(uint16_t inByte)
{
    for (int b = 16; b >= 0; b--)
    {
        Serial.print(bitRead(inByte, b));
    }
}

void setup()
{
    Serial.begin(115200);
    Wire.begin();
    delay(100);

    if (sensor.begin() == false)
    {
        Serial.println("Not connected. Please check connections and read the hookup guide.");
    }
    sensor.setSensitivity(SENSITIVITY_128X);
    sensor.setInterruptEnabled();
    Serial.println("ready sensor");
    sensor.clearInterrupt();
    sensor.setInterruptEnabled();

    Serial.println("Wake..");

    // esp_sleep_enable_ext1_wakeup(1 << 26, ESP_EXT1_WAKEUP_ALL_LOW);
    // esp_sleep_enable_timer_wakeup(1000 * 1000 * 5);

}

void loop()
{
    if(millis() > 500) {
        init_run_ulp(500);
    }
}

static void init_run_ulp(uint32_t usec)
{
    ulp_set_wakeup_period(0, usec);
    rtc_gpio_init(GPIO_NUM_26);
    rtc_gpio_set_direction(GPIO_NUM_26, RTC_GPIO_MODE_INPUT_ONLY);

    rtc_gpio_init(GPIO_NUM_27);
    rtc_gpio_set_direction(GPIO_NUM_27, RTC_GPIO_MODE_INPUT_ONLY);

    esp_sleep_enable_ulp_wakeup();
    ulp__switch = 0;
    ulp__touch = 0;
    esp_err_t err = ulptool_load_binary(0, ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
    err = ulp_run((&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t));
    if (err)
        Serial.println("Error Starting ULP Coprocessor");
    
    Serial.println("Prep Sleep");
    esp_deep_sleep_start();
}
