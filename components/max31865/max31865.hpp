#ifndef MAX31865_HPP
#define MAX31865_HPP

#include "max31865_registers.hpp"
#include "spi_device.hpp"
#include <optional>

namespace MAX31865 {

    struct MAX31865 {
    public:
        enum struct NWires : std::uint8_t {
            THREE = 1,
            TWO = 0,
            FOUR = 0,
        };

        enum struct FaultDetect : std::uint8_t {
            NO_ACTION = 0b00,
            AUTO_DELAY = 0b01,
            MANUAL_DELAY_CYCLE1 = 0b10,
            MANUAL_DELAY_CYCLE2 = 0b11,
        };

        enum Filter : std::uint8_t {
            HZ50 = 1,
            HZ60 = 0,
        };

        enum struct Error : std::uint8_t {
            NO_ERROR = 0,
            VOLTAGE = 2,
            RTD_IN_LOW,
            REF_LOW,
            REF_HIGH,
            RTD_LOW,
            RTD_HIGH,
        };

        enum struct FaultClear : std::uint8_t {
            AUTO = 1U,
            MANUAL = 0U,
        };

        enum struct ConvMode : std::uint8_t {
            AUTO = 1U,
            ONESHOT = 0U,
        };

        using SPIDevice = Utility::SPIDevice;

        MAX31865() noexcept = default;
        MAX31865(SPIDevice&& spi_device,
                 float const threshold_min,
                 float const threshold_max,
                 NWires const nwires,
                 FaultDetect const fault_detect,
                 FaultClear const fault_clear,
                 Filter const filter,
                 ConvMode const conv_mode)
        noexcept;

        MAX31865(MAX31865 const& other) = delete;
        MAX31865(MAX31865&& other) noexcept = default;

        MAX31865& operator=(MAX31865 const& other) = delete;
        MAX31865& operator=(MAX31865&& other) noexcept = default;

        ~MAX31865() noexcept;

        std::optional<std::int16_t> get_temperature_raw() noexcept;
        std::optional<float> get_temperature_scaled() noexcept;

    private:
        static float raw_to_scaled(std::int16_t const raw) noexcept;
        static std::int16_t scaled_to_raw(float const scaled) noexcept;

        static constexpr float RTD_CONVERT_SLOPE{30.904F};
        static constexpr float RTD_CONVERT_INTERCEPT{8234.257F};
        static constexpr std::uint8_t REG_WRITE_OFFSET{0x80};

        void initialize(float const threshold_min,
                        float const threshold_max,
                        NWires const nwires,
                        FaultDetect const fault_detect,
                        FaultClear const fault_clear,
                        Filter const filter,
                        ConvMode const conv_mode) noexcept;
        void deinitialize() noexcept;

        void set_config_register(CONFIG const config) const noexcept;
        CONFIG get_config_register() const noexcept;

        void set_high_fault_registers(HIGH_FAULT const high_fault) const noexcept;
        HIGH_FAULT get_high_fault_registers() const noexcept;

        void set_low_fault_registers(LOW_FAULT const low_fault) const noexcept;
        LOW_FAULT get_low_fault_registers() const noexcept;

        RTD get_rtd_registers() const noexcept;

        void set_config(NWires const nwires,
                        FaultDetect const fault_detect,
                        FaultClear const fault_clear,
                        Filter const filter,
                        ConvMode const conv_mode) const noexcept;
        void set_high_fault(std::int16_t const threshold_max) const noexcept;
        void set_low_fault(std::int16_t const threshold_min) const noexcept;
        void set_vbias(bool const vbias) const noexcept;
        void start_one_shot_conversion() const noexcept;

        bool initialized_{false};

        SPIDevice spi_device_{};
    };

}; // namespace MAX31865

#endif // MAX31865_HPP
