#include "max31865_task.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "max31865.hpp"
#include <array>

namespace MAX31865 {

    static std::array<StackType_t, 4096UL> task_stack;
    static StaticTask_t static_task;
    static constexpr auto TAG{"MAX31865"};

    void start_task() noexcept
    {
        xTaskCreateStaticPinnedToCore(&task, "MAX31865", 4096UL, nullptr, 1, task_stack.data(), &static_task, 1);
    }

    void task(void* parameter) noexcept
    {
        MAX31865 max31865{gpio_num_t::GPIO_NUM_0, 20.0F, 180.0F};

        while (true) {
            ESP_LOGI(TAG, "temperature: %f\n\r", max31865.get_temperature_scaled().value());
        }
    }

}; // namespace MAX31865