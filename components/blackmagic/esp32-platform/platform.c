#include <stdint.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "general.h"
#include <esp_log.h>
#include <driver/gpio.h>
#include <rom/ets_sys.h>

#include <esp_rom_gpio.h>
#include <esp_timer.h>
#include <esp_private/esp_clk.h>

uint32_t swd_delay_cnt = 0;
// static const char* TAG = "gdb-platform";

void inline platform_swdio_mode_float(void) {
    gpio_set_direction((gpio_num_t)SWDIO_PIN, GPIO_MODE_INPUT);
}

void inline platform_swdio_mode_drive(void) {
    gpio_set_direction((gpio_num_t)SWDIO_PIN, GPIO_MODE_OUTPUT);
}

void inline platform_gpio_set_level(int32_t gpio_num, uint32_t value) {
    gpio_set_level((gpio_num_t)gpio_num, value);
}

void inline platform_gpio_set(int32_t gpio_num) {
    gpio_set_level((gpio_num_t)gpio_num, 1);
}

void inline platform_gpio_clear(int32_t gpio_num) {
    gpio_set_level((gpio_num_t)gpio_num, 0);
}

int inline platform_gpio_get_level(int32_t gpio_num) {
    return gpio_get_level((gpio_num_t)gpio_num);
}

// init platform
void platform_init() {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = ((1ULL << SWCLK_PIN) | (1ULL << SWDIO_PIN)),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };

    gpio_config(&io_conf);

    platform_max_frequency_set(SWD_DEFAULT_FREQUENCY);
}

// set reset target pin level
void platform_srst_set_val(bool assert) {
    (void)assert;
}

// get reset target pin level
bool platform_srst_get_val(void) {
    return false;
}

// target voltage
const char* platform_target_voltage(void) {
    return NULL;
}

// platform time counter
uint32_t platform_time_ms(void) {
    int64_t time_milli = esp_timer_get_time() / 1000;
    return ((uint32_t)time_milli);
}

// delay ms
void platform_delay(uint32_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

// hardware version
int platform_hwversion(void) {
    return 0;
}

// set timeout
void platform_timeout_set(platform_timeout_s* t, uint32_t ms) {
    t->time = platform_time_ms() + ms;
}

// check timeout
bool platform_timeout_is_expired(const platform_timeout_s* t) {
    return platform_time_ms() > t->time;
}

// set interface freq
void platform_max_frequency_set(uint32_t freq) {
    // Gunakan fallback jika esp_clk_cpu_freq() mengembalikan 0
    uint32_t cpu_freq = esp_clk_cpu_freq();
    if (cpu_freq == 0) {
        cpu_freq = 160000000; // 160 MHz default C3
    }

    if(freq < 50000) return;

    int32_t cnt =
        (cpu_freq - SWD_TOTAL_CYCLES * (int32_t)freq) / (SWD_CYCLES_PER_CLOCK * (int32_t)freq);

    if(cnt < 0) cnt = 0;

    swd_delay_cnt = cnt;
}

// get interface freq
uint32_t platform_max_frequency_get(void) {
    return esp_clk_cpu_freq() / (swd_delay_cnt * SWD_CYCLES_PER_CLOCK + SWD_TOTAL_CYCLES);
}

void platform_nrst_set_val(bool assert) {
    (void)assert;
}

bool platform_nrst_get_val() {
    return false;
}

void platform_target_clk_output_enable(bool enable) {
    (void)enable;
}
