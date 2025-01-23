#include "max31865.hpp"
#include "max31865_registers.hpp"
#include "spi_device.hpp"
#include <cmath>
#include <optional>

using RA = MAX31865::RA;
using CONFIG = MAX31865::CONFIG;
using Raw = MAX31865::MAX31865::Raw;
using Scaled = MAX31865::MAX31865::Scaled;
using OptionalRaw = MAX31865::MAX31865::OptionalRaw;
using OptionalScaled = MAX31865::MAX31865::OptionalScaled;

namespace MAX31865 {

    MAX31865::MAX31865(SPIDevice&& spi_device, Scaled const threshold_min, Scaled const threshold_max) noexcept :
        spi_device_{std::forward<SPIDevice>(spi_device)}
    {
        this->initialize(threshold_min, threshold_max);
    }

    MAX31865::~MAX31865() noexcept
    {
        this->deinitialize();
    }

    OptionalRaw MAX31865::get_temperature_raw() noexcept
    {
        if (!this->initialized_) {
            return OptionalRaw{std::nullopt};
        }

        this->set_vbias(true);
        vTaskDelay(pdMS_TO_TICKS(10));

        this->start_one_shot_conversion();
        vTaskDelay(pdMS_TO_TICKS(50));

        if (this->spi_device_.read_byte(RA::CONFIG_REG) & (1U << CONFIG::ONESHOT_BIT)) {
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
        return this->get_temperature_raw().transform(&raw_to_scaled);
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
        this->set_config();
        this->set_high_fault_registers(scaled_to_raw(threshold_max));
        this->set_low_fault_registers(scaled_to_raw(threshold_min));
        this->initialized_ = true;
    }

    void MAX31865::deinitialize() noexcept
    {
        this->initialized_ = false;
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
        this->spi_device_.write_byte(RA::CONFIG_REG | REG_WRITE_OFFSET, config);
    }

    std::uint8_t MAX31865::get_config_register() const noexcept
    {
        return this->spi_device_.read_byte(RA::CONFIG_REG & (REG_WRITE_OFFSET - 1U));
    }

    void MAX31865::set_config() const noexcept
    {
        this->set_config_register(NWires::Four << CONFIG::NWIRES_BIT | Filter::Hz60 << CONFIG::MAINSFILTER_BIT |
                                  0U << CONFIG::FAULTDETECTION_BIT_0 | 0U << CONFIG::FAULTDETECTION_BIT_1 |
                                  0U << CONFIG::CONVERSIONMODE_BIT | 0U << CONFIG::VBIAS_BIT);
    }

    void MAX31865::set_high_fault_registers(std::uint16_t const high_fault) const noexcept
    {
        this->spi_device_.write_bytes(RA::HIGH_FAULT_REG_H | REG_WRITE_OFFSET,
                                      std::array<std::uint8_t, 2>{static_cast<std::uint8_t>((high_fault << 1) >> 8),
                                                                  static_cast<std::uint8_t>(high_fault << 1)});
    }

    void MAX31865::set_low_fault_registers(std::uint16_t const low_fault) const noexcept
    {
        this->spi_device_.write_bytes(RA::LOW_FAULT_REG_H | REG_WRITE_OFFSET,
                                      std::array<std::uint8_t, 2>{static_cast<std::uint8_t>((low_fault << 1) >> 8),
                                                                  static_cast<std::uint8_t>(low_fault << 1)});
    }

    std::uint16_t MAX31865::get_rtd_registers() const noexcept
    {
        auto const buffer{this->spi_device_.read_bytes<2>(RA::RTD_REG_H & (REG_WRITE_OFFSET - 1U))};
        return (static_cast<std::uint16_t>(buffer[0]) << 8U) | static_cast<std::uint16_t>(buffer[1]);
    }

    void MAX31865::start_one_shot_conversion() const noexcept
    {
        this->set_config_register(this->get_config_register() | (1U << CONFIG::ONESHOT_BIT));
    }

}; // namespace MAX31865