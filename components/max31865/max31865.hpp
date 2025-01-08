#ifndef MAX31865_HPP
#define MAX31865_HPP

#include "spi.hpp"
#include <optional>

namespace MAX31865 {

    struct MAX31865 {
    public:
        enum NWires : uint8_t {
            Three = 1,
            Two = 0,
            Four = 0,
        };

        enum FaultDetection : uint8_t {
            NoAction = 0b00,
            AutoDelay = 0b01,
            ManualDelayCycle1 = 0b10,
            ManualDelayCycle2 = 0b11,
        };

        enum Filter : uint8_t {
            Hz50 = 1,
            Hz60 = 0,
        };

        enum Error : uint8_t {
            NoError = 0,
            Voltage = 2,
            RTDInLow,
            RefLow,
            RefHigh,
            RTDLow,
            RTDHigh,
        };

        using Raw = std::uint16_t;
        using Scaled = float;
        using OptionalRaw = std::optional<Raw>;
        using OptionalScaled = std::optional<Scaled>;

        MAX31865() noexcept = default;

        MAX31865(gpio_num_t const chip_select, Scaled const threshold_min, Scaled const threshold_max) noexcept;

        MAX31865(MAX31865 const& other) noexcept = delete;
        MAX31865(MAX31865&& other) noexcept = default;

        MAX31865& operator=(MAX31865 const& other) noexcept = delete;
        MAX31865& operator=(MAX31865&& other) noexcept = default;

        ~MAX31865() noexcept;

        OptionalRaw get_temperature_raw() noexcept;
        OptionalScaled get_temperature_scaled() noexcept;

    private:
        static Scaled raw_to_scaled(Raw const raw) noexcept;
        static Raw scaled_to_raw(Scaled const scaled) noexcept;

        static constexpr Scaled RTD_CONVERT_SLOPE{30.904F};
        static constexpr Scaled RTD_CONVERT_INTERCEPT{8234.257F};
        static constexpr std::uint8_t REG_WRITE_OFFSET{0x80};

        void initialize(Scaled const threshold_min, Scaled const threshold_max) noexcept;
        void initialize_gpio() noexcept;
        void initialize_spi() noexcept;
        void deinitialize_spi() noexcept;
        void deinitialize() noexcept;

        void set_config_register(std::uint8_t const config) const noexcept;
        std::uint8_t get_config_register() const noexcept;

        void set_high_fault_registers(std::uint16_t const high_fault) const noexcept;
        void set_low_fault_registers(std::uint16_t const low_fault) const noexcept;

        std::uint16_t get_rtd_registers() const noexcept;

        void set_config() const noexcept;
        void set_vbias(bool const vbias) const noexcept;
        void start_one_shot_conversion() const noexcept;

        bool initialized{false};

        gpio_num_t chip_select{};
        spi_device_handle_t spi_device{nullptr};
    };

}; // namespace MAX31865

#endif // MAX31865_HPP
