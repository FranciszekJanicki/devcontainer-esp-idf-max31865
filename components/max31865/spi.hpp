#ifndef SPI_HPP
#define SPI_HPP

#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "utility.hpp"
#include <algorithm>
#include <ranges>

using namespace Utility;

namespace MAX31865::SPI {

    template <Size WRITE_SIZE>
    inline void write_bytes(spi_device_handle_t const spi_device,
                            gpio_num_t const chip_select,
                            std::uint8_t const reg_address,
                            Bytes<WRITE_SIZE> const& write_data) noexcept
    {
        spi_transaction_t transaction{};
        transaction.length = 8 * WRITE_SIZE;
        transaction.rxlength = 0;
        transaction.addr = reg_address;
        transaction.flags = SPI_TRANS_USE_TXDATA;

        std::ranges::copy(write_data, transaction.tx_data);

        gpio_set_level(chip_select, 0);
        spi_device_polling_transmit(spi_device, &transaction);
        gpio_set_level(chip_select, 1);
    }

    inline void write_byte(spi_device_handle_t const spi_device,
                           gpio_num_t const chip_select,
                           std::uint8_t const reg_address,
                           Byte const write_data) noexcept
    {
        write_bytes(spi_device, chip_select, reg_address, Bytes<1UL>{write_data});
    }

    template <Size READ_SIZE>
    [[nodiscard]] inline Bytes<READ_SIZE> read_bytes(spi_device_handle_t const spi_device,
                                                     gpio_num_t const chip_select,
                                                     std::uint8_t const reg_address) noexcept
    {
        spi_transaction_t transaction{};
        transaction.length = 0;
        transaction.rxlength = 8 * READ_SIZE;
        transaction.addr = reg_address;
        transaction.flags = SPI_TRANS_USE_RXDATA;

        gpio_set_level(chip_select, 0);
        spi_device_polling_transmit(spi_device, &transaction);
        gpio_set_level(chip_select, 1);

        Bytes<READ_SIZE> read{};
        std::ranges::copy(transaction.rx_data, read.data());
        return read;
    }

    [[nodiscard]] inline Byte read_byte(spi_device_handle_t const spi_device,
                                        gpio_num_t const chip_select,
                                        std::uint8_t const reg_address) noexcept
    {
        return read_bytes<1UL>(spi_device, chip_select, reg_address)[0];
    }

}; // namespace MAX31865::SPI

#endif // SPI_HPP