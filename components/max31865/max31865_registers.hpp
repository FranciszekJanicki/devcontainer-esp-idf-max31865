#ifndef MAX31865_REGISTERS_HPP
#define MAX31865_REGISTERS_HPP

#include <cstdint>

#define packed __attribute__((packed))

namespace MAX31865 {

    enum struct RA : std::uint8_t {
        CONFIG = 0x00,
        RTD_H = 0x01,
        RTD_L = 0x02,
        HIGH_FAULT_H = 0x03,
        HIGH_FAULT_L = 0x04,
        LOW_FAULT_H = 0x05,
        LOW_FAULT_L = 0x06,
        FAULT_STATUS = 0x07,
    };

    struct CONFIG {
        std::uint8_t vbias : 1;
        std::uint8_t conv_mode : 1;
        std::uint8_t oneshot : 1;
        std::uint8_t nwires : 1;
        std::uint8_t fault_detect : 2;
        std::uint8_t fault_clear : 1;
        std::uint8_t mainsfilter : 1;
    } packed;

    struct RTD {
        std::uint8_t rtd_h : 8;
        std::uint8_t rtd_l : 8;
    } packed;

    struct HIGH_FAULT {
        std::uint8_t high_fault_h : 8;
        std::uint8_t high_fault_l : 8;
    } packed;

    struct LOW_FAULT {
        std::uint8_t low_fault_h : 8;
        std::uint8_t low_fault_l : 8;
    } packed;

    struct FAULT_STATUS {
        std::uint8_t fault_status : 8;
    } packed;

}; // namespace MAX31865

#endif // MAX31865_REGISTERS_HPP