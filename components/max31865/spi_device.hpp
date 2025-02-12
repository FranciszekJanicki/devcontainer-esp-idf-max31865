#ifndef SPI_DEVICE_HPP
#define SPI_DEVICE_HPP

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "utility.hpp"
#include <algorithm>
#include <array>
#include <bitset>
#include <cstdint>
#include <ranges>
#include <utility>

namespace Utility {

    struct SPIDevice {
    public:
        SPIDevice() noexcept = default;
        SPIDevice(spi_device_handle_t const spi_device, gpio_num_t const chip_select) noexcept;

        SPIDevice(SPIDevice const& other) = delete;
        SPIDevice(SPIDevice&& other) noexcept = default;

        SPIDevice& operator=(SPIDevice const& other) = delete;
        SPIDevice& operator=(SPIDevice&& other) noexcept = default;

        ~SPIDevice() noexcept = default;

        template <std::size_t SIZE>
        void transmit_dwords(std::array<std::uint32_t, SIZE> const& dwords) const noexcept;
        void transmit_dword(std::uint32_t const dword) const noexcept;

        template <std::size_t SIZE>
        void transmit_words(std::array<std::uint16_t, SIZE> const& words) const noexcept;
        void transmit_word(std::uint16_t const word) const noexcept;

        template <std::size_t SIZE>
        void transmit_bytes(std::array<std::uint8_t, SIZE> const& bytes) const noexcept;
        void transmit_byte(std::uint8_t const byte) const noexcept;

        template <std::size_t SIZE>
        std::array<std::uint32_t, SIZE> receive_dwords() const noexcept;
        std::uint32_t receive_dword() const noexcept;

        template <std::size_t SIZE>
        std::array<std::uint16_t, SIZE> receive_words() const noexcept;
        std::uint16_t receive_word() const noexcept;

        template <std::size_t SIZE>
        std::array<std::uint8_t, SIZE> receive_bytes() const noexcept;
        std::uint8_t receive_byte() const noexcept;

        template <std::size_t SIZE>
        std::array<std::uint32_t, SIZE> read_dwords(std::uint8_t const reg_address) const noexcept;
        std::uint32_t read_dword(std::uint8_t const reg_address) const noexcept;

        template <std::size_t SIZE>
        std::array<std::uint16_t, SIZE> read_words(std::uint8_t const reg_address) const noexcept;
        std::uint16_t read_word(std::uint8_t const reg_address) const noexcept;

        template <std::size_t SIZE>
        std::array<std::uint8_t, SIZE> read_bytes(std::uint8_t const reg_address) const noexcept;
        std::uint8_t read_byte(std::uint8_t const reg_address) const noexcept;

        std::uint8_t
        read_bits(std::uint8_t const reg_address, std::uint8_t const position, std::size_t const size) const noexcept;
        bool read_bit(std::uint8_t const reg_address, std::uint8_t const position) const noexcept;

        template <std::size_t SIZE>
        void write_dwords(std::uint8_t const reg_address, std::array<std::uint32_t, SIZE> const& dwords) const noexcept;
        void write_dword(std::uint8_t const reg_address, std::uint32_t const dword) const noexcept;

        template <std::size_t SIZE>
        void write_words(std::uint8_t const reg_address, std::array<std::uint16_t, SIZE> const& words) const noexcept;
        void write_word(std::uint8_t const reg_address, std::uint16_t const word) const noexcept;

        template <std::size_t SIZE>
        void write_bytes(std::uint8_t const reg_address, std::array<std::uint8_t, SIZE> const& bytes) const noexcept;
        void write_byte(std::uint8_t const reg_address, std::uint8_t const byte) const noexcept;

        void write_bits(std::uint8_t const reg_address,
                        std::uint8_t const bits,
                        std::uint8_t const position,
                        std::size_t const size) const noexcept;
        void write_bit(std::uint8_t const reg_address, bool const bit, std::uint8_t const position) const noexcept;

    private:
        static constexpr std::uint32_t TIMEOUT{100U};

        void initialize() noexcept;
        void deinitialize() noexcept;

        bool initialized_{false};

        spi_device_handle_t spi_device_{nullptr};
        gpio_num_t chip_select_{};
    };

    template <std::size_t SIZE>
    void SPIDevice::transmit_dwords(std::array<std::uint32_t, SIZE> const& dwords) const noexcept
    {
        this->transmit_bytes(dwords_to_bytes(dwords));
    }

    template <std::size_t SIZE>
    void SPIDevice::transmit_words(std::array<std::uint16_t, SIZE> const& words) const noexcept
    {
        this->transmit_bytes(words_to_bytes(words));
    }

    template <std::size_t SIZE>
    void SPIDevice::transmit_bytes(std::array<std::uint8_t, SIZE> const& bytes) const noexcept
    {
        std::array<std::uint8_t, SIZE> write{bytes};
        if (this->initialized_) {
            spi_transaction_t transaction{};
            transaction.length = 8 * SIZE;
            transaction.rxlength = 0;
            transaction.flags = SPI_TRANS_USE_TXDATA;
            std::ranges::copy(write, transaction.tx_data);
            gpio_set_level(this->chip_select_, 0);
            spi_device_polling_transmit(this->spi_device_, &transaction);
            gpio_set_level(this->chip_select_, 1);
        }
    }

    template <std::size_t SIZE>
    std::array<std::uint32_t, SIZE> SPIDevice::receive_dwords() const noexcept
    {
        return bytes_to_dwords(this->receive_bytes<4 * SIZE>());
    }

    template <std::size_t SIZE>
    std::array<std::uint16_t, SIZE> SPIDevice::receive_words() const noexcept
    {
        return bytes_to_words(this->receive_bytes<2 * SIZE>());
    }

    template <std::size_t SIZE>
    std::array<std::uint8_t, SIZE> SPIDevice::receive_bytes() const noexcept
    {
        std::array<std::uint8_t, SIZE> read{};
        if (this->initialized_) {
            spi_transaction_t transaction{};
            transaction.length = 0;
            transaction.rxlength = 8 * SIZE;
            transaction.flags = SPI_TRANS_USE_RXDATA;
            gpio_set_level(this->chip_select_, 0);
            spi_device_polling_transmit(this->spi_device_, &transaction);
            gpio_set_level(this->chip_select_, 1);
            Bytes<SIZE> read{};
            std::ranges::copy(transaction.rx_data, read.data());
        }
        return read;
    }

    template <std::size_t SIZE>
    std::array<std::uint32_t, SIZE> SPIDevice::read_dwords(std::uint8_t const reg_address) const noexcept
    {
        return bytes_to_dwords(this->read_bytes<4 * SIZE>(reg_address));
    }

    template <std::size_t SIZE>
    std::array<std::uint16_t, SIZE> SPIDevice::read_words(std::uint8_t const reg_address) const noexcept
    {
        return bytes_to_words(this->read_bytes<2 * SIZE>(reg_address));
    }

    template <std::size_t SIZE>
    std::array<std::uint8_t, SIZE> SPIDevice::read_bytes(std::uint8_t const reg_address) const noexcept
    {
        std::array<std::uint8_t, SIZE> read{};
        if (this->initialized_) {
            spi_transaction_t transaction{};
            transaction.length = 0;
            transaction.rxlength = 8 * SIZE;
            transaction.addr = reg_address;
            transaction.flags = SPI_TRANS_USE_RXDATA;
            gpio_set_level(this->chip_select_, 0);
            spi_device_polling_transmit(this->spi_device_, &transaction);
            gpio_set_level(this->chip_select_, 1);
            Bytes<SIZE> read{};
            std::ranges::copy(transaction.rx_data, read.data());
        }
        return read;
    }

    template <std::size_t SIZE>
    void SPIDevice::write_dwords(std::uint8_t const reg_address,
                                 std::array<std::uint32_t, SIZE> const& dwords) const noexcept
    {
        this->write_bytes(reg_address, dwords_to_bytes(dwords));
    }

    template <std::size_t SIZE>
    void SPIDevice::write_words(std::uint8_t const reg_address,
                                std::array<std::uint16_t, SIZE> const& words) const noexcept
    {
        this->write_bytes(reg_address, words_to_bytes(words));
    }

    template <std::size_t SIZE>
    void SPIDevice::write_bytes(std::uint8_t const reg_address,
                                std::array<std::uint8_t, SIZE> const& bytes) const noexcept
    {
        std::array<std::uint8_t, 1UL + SIZE> write{reg_address};
        std::ranges::copy(bytes, write.data() + 1UL);
        if (this->initialized_) {
            spi_transaction_t transaction{};
            transaction.length = 8 * SIZE;
            transaction.rxlength = 0;
            transaction.addr = reg_address;
            transaction.flags = SPI_TRANS_USE_TXDATA;
            std::ranges::copy(write, transaction.tx_data);
            gpio_set_level(this->chip_select_, 0);
            spi_device_polling_transmit(this->spi_device_, &transaction);
            gpio_set_level(this->chip_select_, 1);
        }
    }

}; // namespace Utility

#endif // SPI_DEVICE_HPP