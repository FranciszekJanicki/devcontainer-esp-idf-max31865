#include "max31865.hpp"
#include "max31865_register_map.hpp"
#include "spi.hpp"
#include <cmath>
#include <optional>

using RA = MAX31865::RA;
using CONFIG = MAX31865::CONFIG;
using Raw = MAX31865::MAX31865::Raw;
using Scaled = MAX31865::MAX31865::Scaled;
using OptionalRaw = MAX31865::MAX31865::OptionalRaw;
using OptionalScaled = MAX31865::MAX31865::OptionalScaled;

namespace MAX31865 {

    MAX31865::MAX31865(gpio_num_t const chip_select, Scaled const threshold_min, Scaled const threshold_max) noexcept :
        chip_select{chip_select}
    {
        this->initialize(threshold_min, threshold_max);
    }

    MAX31865::~MAX31865() noexcept
    {
        this->deinitialize();
    }

    OptionalRaw MAX31865::get_temperature_raw() noexcept
    {
        if (!this->initialized) {
            return OptionalRaw{std::nullopt};
        }

        this->set_vbias(true);
        vTaskDelay(pdMS_TO_TICKS(10));

        this->start_one_shot_conversion();
        vTaskDelay(pdMS_TO_TICKS(50));

        if (SPI::read_byte(this->spi_device, this->chip_select, RA::CONFIG_REG) & (1U << CONFIG::ONESHOT_BIT)) {
            return OptionalRaw{std::nullopt};
        }

        auto const rtd{this->get_rtd_registers()};

        this->set_vbias(false);

        if (rtd & 1U) {
            return OptionalRaw{std::nullopt};
        }

        return OptionalRaw{rtd >> 1U};
    }

    OptionalScaled MAX31865::get_temperature_scaled() noexcept
    {
        auto raw{this->get_temperature_raw()};
        return (raw.has_value() ? OptionalScaled{raw_to_scaled(raw.value())} : OptionalScaled{std::nullopt});
    }

    Scaled MAX31865::raw_to_scaled(Raw const raw) noexcept
    {
        return (raw - RTD_CONVERT_INTERCEPT) / RTD_CONVERT_SLOPE;
    }

    Raw MAX31865::scaled_to_raw(Scaled const scaled) noexcept
    {
        return scaled * RTD_CONVERT_SLOPE + RTD_CONVERT_INTERCEPT;
    }

    void MAX31865::initialize(Scaled const threshold_min, Scaled const threshold_max) noexcept
    {
        this->initialize_gpio();
        this->initialize_spi();
        this->set_config();
        this->set_high_fault_registers(scaled_to_raw(threshold_max));
        this->set_low_fault_registers(scaled_to_raw(threshold_min));
        this->initialized = true;
    }

    void MAX31865::initialize_spi() noexcept
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
        spi_bus_add_device(SPI3_HOST, &config, &this->spi_device);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    void MAX31865::initialize_gpio() noexcept
    {
        gpio_config_t const config{.pin_bit_mask = 1UL << static_cast<std::uint32_t>(this->chip_select),
                                   .mode = GPIO_MODE_OUTPUT,
                                   .pull_up_en = GPIO_PULLUP_DISABLE,
                                   .pull_down_en = GPIO_PULLDOWN_DISABLE,
                                   .intr_type = GPIO_INTR_DISABLE};
        gpio_config(&config);
        gpio_set_level(this->chip_select, 1);
    }

    void MAX31865::deinitialize_spi() noexcept
    {
        spi_bus_remove_device(this->spi_device);
        gpio_set_level(this->chip_select, 0);
    }

    void MAX31865::deinitialize() noexcept
    {
        this->deinitialize_spi();
        this->initialized = false;
    }

    void MAX31865::set_vbias(bool const vbias) const noexcept
    {
        auto cfg = this->get_config_register();
        vbias ? cfg |= 1U << CONFIG::VBIAS_BIT : cfg &= ~(1U << CONFIG::VBIAS_BIT);
        cfg |= 1U << CONFIG::FAULTSTATUS_BIT;
        cfg |= 0U << CONFIG::FAULTDETECTION_BIT_0;
        cfg |= 0U << CONFIG::FAULTDETECTION_BIT_1;
        cfg |= 0U << CONFIG::ONESHOT_BIT;
        this->set_config_register(cfg);
    }

    void MAX31865::set_config_register(std::uint8_t const config) const noexcept
    {
        SPI::write_byte(this->spi_device, this->chip_select, RA::CONFIG_REG | REG_WRITE_OFFSET, config);
    }

    std::uint8_t MAX31865::get_config_register() const noexcept
    {
        return SPI::read_byte(this->spi_device, this->chip_select, RA::CONFIG_REG & (REG_WRITE_OFFSET - 1U));
    }

    void MAX31865::set_config() const noexcept
    {
        this->set_config_register(NWires::Four << CONFIG::NWIRES_BIT | Filter::Hz60 << CONFIG::MAINSFILTER_BIT |
                                  0U << CONFIG::FAULTDETECTION_BIT_0 | 0U << CONFIG::FAULTDETECTION_BIT_1 |
                                  0U << CONFIG::CONVERSIONMODE_BIT | 0U << CONFIG::VBIAS_BIT);
    }

    void MAX31865::set_high_fault_registers(std::uint16_t const high_fault) const noexcept
    {
        SPI::write_bytes(this->spi_device,
                         this->chip_select,
                         RA::HIGH_FAULT_REG_H | REG_WRITE_OFFSET,
                         std::array<std::uint8_t, 2>{static_cast<std::uint8_t>((high_fault << 1) >> 8),
                                                     static_cast<std::uint8_t>(high_fault << 1)});
    }

    void MAX31865::set_low_fault_registers(std::uint16_t const low_fault) const noexcept
    {
        SPI::write_bytes(this->spi_device,
                         this->chip_select,
                         RA::LOW_FAULT_REG_H | REG_WRITE_OFFSET,
                         std::array<std::uint8_t, 2>{static_cast<std::uint8_t>((low_fault << 1) >> 8),
                                                     static_cast<std::uint8_t>(low_fault << 1)});
    }

    std::uint16_t MAX31865::get_rtd_registers() const noexcept
    {
        auto const buffer{
            SPI::read_bytes<2>(this->spi_device, this->chip_select, RA::RTD_REG_H & (REG_WRITE_OFFSET - 1U))};
        return (static_cast<std::uint16_t>(buffer[0]) << 8U) | static_cast<std::uint16_t>(buffer[1]);
    }

    void MAX31865::start_one_shot_conversion() const noexcept
    {
        this->set_config_register(this->get_config_register() | (1U << CONFIG::ONESHOT_BIT));
    }

}; // namespace MAX31865