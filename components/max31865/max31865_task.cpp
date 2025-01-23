#include "max31865_task.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "max31865.hpp"
#include "spi_device.hpp"
#include <array>

namespace MAX31865 {

    static constexpr auto TASK_TAG{"MAX31865"};
    static constexpr auto TASK_STACK_SIZE{4096UL};
    static StackType_t task_stack[TASK_STACK_SIZE];
    static StaticTask_t static_task;

    static spi_device_handle_t max31865_spi_device{nullptr};
    static constexpr auto max31865_chip_select{gpio_num_t::GPIO_NUM_0};

    static void gpio_init() noexcept
    {
        gpio_config_t const config{.pin_bit_mask = 1UL << static_cast<std::uint32_t>(max31865_chip_select),
                                   .mode = GPIO_MODE_OUTPUT,
                                   .pull_up_en = GPIO_PULLUP_DISABLE,
                                   .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                   .intr_type = GPIO_INTR_DISABLE};
        gpio_config(&config);
    }

    static void spi_init() noexcept
    {
        spi_device_interface_config_t const config{.command_bits = 0,
                                                   .address_bits = 8,
                                                   .dummy_bits = 0,
                                                   .mode = 3,
                                                   .clock_source = SPI_CLK_SRC_DEFAULT,
                                                   .duty_cycle_pos = 0,
                                                   .cs_ena_pretrans = 0,
                                                   .cs_ena_posttrans = 0,
                                                   .clock_speed_hz = 5 * 1000 * 1000,
                                                   .input_delay_ns = 0,
                                                   .spics_io_num = -1,
                                                   .flags = SPI_DEVICE_HALFDUPLEX,
                                                   .queue_size = 1,
                                                   .pre_cb = nullptr,
                                                   .post_cb = nullptr};
        spi_bus_add_device(SPI3_HOST, &config, &max31865_spi_device);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    static void task(void* parameter) noexcept
    {
        using namespace Utility;

        spi_init();
        gpio_init();

        SPIDevice spi_device{max31865_spi_device, max31865_chip_select};
        
        MAX31865 max31865{std::move(spi_device), 20.0F, 180.0F};

        while (true) {
            ESP_LOGI(TASK_TAG, "temperature: %f", max31865.get_temperature_scaled().value());
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    void start_task() noexcept
    {
        xTaskCreateStaticPinnedToCore(&task, TASK_TAG, TASK_STACK_SIZE, nullptr, 1, task_stack, &static_task, 1);
    }

}; // namespace MAX31865