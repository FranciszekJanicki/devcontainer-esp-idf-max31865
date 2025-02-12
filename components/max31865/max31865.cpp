#include "max31865.hpp"
#include "max31865_registers.hpp"
#include "spi_device.hpp"
#include <cmath>
#include <optional>
#include <utility>

using SPIDevice = Utility::SPIDevice;

namespace MAX31865 {

    MAX31865::MAX31865(SPIDevice&& spi_device,
                       float const threshold_min,
                       float const threshold_max,
                       NWires const nwires,
                       FaultDetect const fault_detect,
                       FaultClear const fault_clear,
                       Filter const filter,
                       ConvMode const conv_mode) noexcept :
        spi_device_{std::forward<SPIDevice>(spi_device)}
    {
        this->initialize(threshold_min, threshold_max, nwires, fault_detect, fault_clear, filter, conv_mode);
    }

    MAX31865::~MAX31865() noexcept
    {
        this->deinitialize();
    }

    std::optional<std::int16_t> MAX31865::get_temperature_raw() noexcept
    {
        if (!this->initialized_) {
            return std::optional<std::int16_t>{std::nullopt};
        }

        this->set_vbias(true);
        vTaskDelay(pdMS_TO_TICKS(10));

        if (this->get_config_register().conv_mode == std::to_underlying(ConvMode::ONESHOT)) {
            this->start_one_shot_conversion();
            vTaskDelay(pdMS_TO_TICKS(50));
        }

        if (this->get_config_register().oneshot) {
            return std::optional<std::int16_t>{std::nullopt};
        }

        auto const rtd{std::bit_cast<std::int16_t>(this->get_rtd_registers())};

        this->set_vbias(false);

        if (rtd & 1U) {
            return std::optional<std::int16_t>{std::nullopt};
        }

        return std::optional<std::int16_t>{rtd >> 1U};
    }

    std::optional<float> MAX31865::get_temperature_scaled() noexcept
    {
        return this->get_temperature_raw().transform(&raw_to_scaled);
    }

    float MAX31865::raw_to_scaled(std::int16_t const raw) noexcept
    {
        return (raw - RTD_CONVERT_INTERCEPT) / RTD_CONVERT_SLOPE;
    }

    std::int16_t MAX31865::scaled_to_raw(float const scaled) noexcept
    {
        return scaled * RTD_CONVERT_SLOPE + RTD_CONVERT_INTERCEPT;
    }

    void MAX31865::initialize(float const threshold_min,
                              float const threshold_max,
                              NWires const nwires,
                              FaultDetect const fault_detect,
                              FaultClear const fault_clear,
                              Filter const filter,
                              ConvMode const conv_mode) noexcept
    {
        this->set_config(nwires, fault_detect, fault_clear, filter, conv_mode);
        this->set_high_fault(threshold_max);
        this->set_low_fault(threshold_min);
        this->initialized_ = true;
    }

    void MAX31865::deinitialize() noexcept
    {
        this->initialized_ = false;
    }

    void MAX31865::set_vbias(bool const vbias) const noexcept
    {
        auto cfg{this->get_config_register()};
        cfg.vbias = vbias;
        cfg.fault_clear = 0U;
        cfg.fault_detect = 0U;
        cfg.oneshot = 0U;
        this->set_config_register(cfg);
    }

    void MAX31865::set_config_register(CONFIG const config) const noexcept
    {
        this->spi_device_.write_byte(std::to_underlying(RA::CONFIG) | REG_WRITE_OFFSET,
                                     std::bit_cast<std::uint8_t>(config));
    }

    CONFIG MAX31865::get_config_register() const noexcept
    {
        return std::bit_cast<CONFIG>(
            this->spi_device_.read_byte(std::to_underlying(RA::CONFIG) & (REG_WRITE_OFFSET - 1U)));
    }

    void MAX31865::set_config(NWires const nwires,
                              FaultDetect const fault_detect,
                              FaultClear const fault_clear,
                              Filter const filter,
                              ConvMode const conv_mode) const noexcept
    {
        this->set_config_register(CONFIG{.vbias = 0U,
                                         .conv_mode = std::to_underlying(conv_mode),
                                         .oneshot = 0U,
                                         .nwires = std::to_underlying(nwires),
                                         .fault_detect = std::to_underlying(fault_detect),
                                         .fault_clear = std::to_underlying(fault_clear),
                                         .mainsfilter = std::to_underlying(filter)});
    }

    void MAX31865::set_high_fault(std::int16_t const threshold_max) const noexcept
    {
        this->set_high_fault_registers(std::bit_cast<HIGH_FAULT>(scaled_to_raw(threshold_max)));
    }

    void MAX31865::set_low_fault(std::int16_t const threshold_min) const noexcept
    {
        this->set_low_fault_registers(std::bit_cast<LOW_FAULT>(scaled_to_raw(threshold_min)));
    }

    void MAX31865::set_high_fault_registers(HIGH_FAULT const high_fault) const noexcept
    {
        this->spi_device_.write_word(std::to_underlying(RA::HIGH_FAULT_H) | REG_WRITE_OFFSET,
                                     std::bit_cast<std::uint16_t>(high_fault));
    }

    HIGH_FAULT MAX31865::get_high_fault_registers() const noexcept
    {
        return std::bit_cast<HIGH_FAULT>(
            this->spi_device_.read_word(std::to_underlying(RA::HIGH_FAULT_H) & (REG_WRITE_OFFSET - 1U)));
    }

    void MAX31865::set_low_fault_registers(LOW_FAULT const low_fault) const noexcept
    {
        this->spi_device_.write_word(std::to_underlying(RA::LOW_FAULT_H) | REG_WRITE_OFFSET,
                                     std::bit_cast<std::uint16_t>(low_fault));
    }

    LOW_FAULT MAX31865::get_low_fault_registers() const noexcept
    {
        return std::bit_cast<LOW_FAULT>(
            this->spi_device_.read_word(std::to_underlying(RA::LOW_FAULT_H) & (REG_WRITE_OFFSET - 1U)));
    }

    RTD MAX31865::get_rtd_registers() const noexcept
    {
        return std::bit_cast<RTD>(this->spi_device_.read_word(std::to_underlying(RA::RTD_H) & (REG_WRITE_OFFSET - 1U)));
    }

    void MAX31865::start_one_shot_conversion() const noexcept
    {
        auto cfg{this->get_config_register()};
        cfg.oneshot = 1U;
        this->set_config_register(cfg);
    }

}; // namespace MAX31865