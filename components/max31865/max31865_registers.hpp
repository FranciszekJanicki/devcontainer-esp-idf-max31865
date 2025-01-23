#ifndef MAX31865_REGISTERS_HPP
#define MAX31865_REGISTERS_HPP

#include <cstdint>

namespace MAX31865 {

    enum RA : std::uint8_t {
        CONFIG_REG = 0x00,
        RTD_REG_H = 0x01,
        RTD_REG_L = 0x02,
        HIGH_FAULT_REG_H = 0x03,
        HIGH_FAULT_REG_L = 0x04,
        LOW_FAULT_REG_H = 0x05,
        LOW_FAULT_REG_L = 0x06,
        FAULT_STATUS_REG = 0x07,
    };

    enum CONFIG : std::uint8_t {
        VBIAS_BIT = 7,
        CONVERSIONMODE_BIT = 6,
        ONESHOT_BIT = 5,
        NWIRES_BIT = 4,
        FAULTDETECTION_BIT_1 = 3,
        FAULTDETECTION_BIT_0 = 2,
        FAULTSTATUS_BIT = 1,
        MAINSFILTER_BIT = 0,
    };

}; // namespace MAX31865

#endif // MAX31865_REGISTERS_HPP