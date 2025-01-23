#ifndef SPI_DEVICE_HPP
#define SPI_DEVICE_HPP

#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "utility.hpp"
#include <algorithm>
#include <ranges>
#include <utility>

namespace Utility {

    struct SPIDevice {
    public:
        SPIDevice() noexcept = default;
        SPIDevice(spi_device_handle_t const spi_device, gpio_num_t const chip_select_) noexcept;

        SPIDevice(SPIDevice const& other) noexcept = delete;
        SPIDevice(SPIDevice&& other) noexcept = default;

        auto operator=(SPIDevice const& other) noexcept -> SPIDevice& = delete;
        auto operator=(SPIDevice&& other) noexcept -> SPIDevice& = default;

        ~SPIDevice() noexcept = default;

        template <std::size_t TRANSMIT_SIZE>
        auto transmit_dwords(DWords<TRANSMIT_SIZE> const& transmit_data) const noexcept -> void;
        auto transmit_dword(DWord const transmit_data) const noexcept -> void;

        template <std::size_t TRANSMIT_SIZE>
        auto transmit_words(Words<TRANSMIT_SIZE> const& transmit_data) const noexcept -> void;
        auto transmit_word(Word const transmit_data) const noexcept -> void;

        template <std::size_t TRANSMIT_SIZE>
        auto transmit_bytes(Bytes<TRANSMIT_SIZE> const& transmit_data) const noexcept -> void;
        auto transmit_byte(Byte const transmit_data) const noexcept -> void;

        template <std::size_t RECEIVE_SIZE>
        auto receive_dwords() const noexcept -> [[nodiscard]] DWords<RECEIVE_SIZE>;
        auto receive_dword() const noexcept -> [[nodiscard]] DWord;

        template <std::size_t RECEIVE_SIZE>
        auto receive_words() const noexcept -> [[nodiscard]] Words<RECEIVE_SIZE>;
        auto receive_word() const noexcept -> [[nodiscard]] Word;

        template <std::size_t RECEIVE_SIZE>
        auto receive_bytes() const noexcept -> [[nodiscard]] Bytes<RECEIVE_SIZE>;
        auto receive_byte() const noexcept -> [[nodiscard]] Byte;

        template <std::size_t READ_SIZE>
        auto read_dwords(std::uint8_t const reg_address) const noexcept -> [[nodiscard]] DWords<READ_SIZE>;
        auto read_dword(std::uint8_t const reg_address) const noexcept -> [[nodiscard]] DWord;

        template <std::size_t READ_SIZE>
        auto read_words(std::uint8_t const reg_address) const noexcept -> [[nodiscard]] Words<READ_SIZE>;
        auto read_word(std::uint8_t const reg_address) const noexcept -> [[nodiscard]] Word;

        template <std::size_t READ_SIZE>
        auto read_bytes(std::uint8_t const reg_address) const noexcept -> [[nodiscard]] Bytes<READ_SIZE>;
        auto read_byte(std::uint8_t const reg_address) const noexcept -> [[nodiscard]] Byte;

        auto read_bits(std::uint8_t const reg_address,
                       std::uint8_t const read_position,
                       std::size_t const read_size) const noexcept -> [[nodiscard]] Byte;
        auto read_bit(std::uint8_t const reg_address, std::uint8_t const read_position) const noexcept ->
            [[nodiscard]] Bit;

        template <std::size_t WRITE_SIZE>
        auto write_dwords(std::uint8_t const reg_address, DWords<WRITE_SIZE> const& write_data) const noexcept -> void;
        auto write_dword(std::uint8_t const reg_address, DWord const write_data) const noexcept -> void;

        template <std::size_t WRITE_SIZE>
        auto write_words(std::uint8_t const reg_address, Words<WRITE_SIZE> const& write_data) const noexcept -> void;
        auto write_word(std::uint8_t const reg_address, Word const write_data) const noexcept -> void;

        template <std::size_t WRITE_SIZE>
        auto write_bytes(std::uint8_t const reg_address, Bytes<WRITE_SIZE> const& write_data) const noexcept -> void;
        auto write_byte(std::uint8_t const reg_address, Byte const write_data) const noexcept -> void;

        auto write_bits(std::uint8_t const reg_address,
                        Byte const write_data,
                        std::uint8_t const write_position,
                        std::size_t const write_size) const noexcept -> void;
        auto write_bit(std::uint8_t const reg_address,
                       Bit const write_data,
                       std::uint8_t const write_position) const noexcept -> void;

    private:
        static constexpr std::uint32_t SPI_TIMEOUT{100U};

        void initialize() noexcept;
        void deinitialize() noexcept;

        bool initialized_{false};

        spi_device_handle_t spi_device_{nullptr};
        gpio_num_t chip_select_{};
    };

    template <std::size_t TRANSMIT_SIZE>
    auto SPIDevice::transmit_dwords(DWords<TRANSMIT_SIZE> const& transmit_data) const noexcept -> void
    {
        this->transmit_bytes(dwords_to_bytes(transmit_data));
    }

    template <std::size_t TRANSMIT_SIZE>
    auto SPIDevice::transmit_words(Words<TRANSMIT_SIZE> const& transmit_data) const noexcept -> void
    {
        this->transmit_bytes(words_to_bytes(transmit_data));
    }

    template <std::size_t TRANSMIT_SIZE>
    auto SPIDevice::transmit_bytes(Bytes<TRANSMIT_SIZE> const& transmit_data) const noexcept -> void
    {
        if (this->initialized_) {
            Bytes<TRANSMIT_SIZE> transmit{transmit_data};
        }
    }

    template <std::size_t RECEIVE_SIZE>
    auto SPIDevice::receive_dwords() const noexcept -> DWords<RECEIVE_SIZE>
    {
        return bytes_to_dwords(this->receive_bytes<4 * RECEIVE_SIZE>());
    }

    template <std::size_t RECEIVE_SIZE>
    auto SPIDevice::receive_words() const noexcept -> Words<RECEIVE_SIZE>
    {
        return bytes_to_words(this->receive_bytes<2 * RECEIVE_SIZE>());
    }

    template <std::size_t RECEIVE_SIZE>
    auto SPIDevice::receive_bytes() const noexcept -> Bytes<RECEIVE_SIZE>
    {
        if (this->initialized_) {
            Bytes<RECEIVE_SIZE> receive{};
            return receive;
        }
        std::unreachable();
    }

    template <std::size_t READ_SIZE>
    auto SPIDevice::read_dwords(std::uint8_t const reg_address) const noexcept -> DWords<READ_SIZE>
    {
        return bytes_to_dwords(this->read_bytes<4 * READ_SIZE>(reg_address));
    }

    template <std::size_t READ_SIZE>
    auto SPIDevice::read_words(std::uint8_t const reg_address) const noexcept -> Words<READ_SIZE>
    {
        return bytes_to_words(this->read_bytes<2 * READ_SIZE>(reg_address));
    }

    template <std::size_t READ_SIZE>
    auto SPIDevice::read_bytes(std::uint8_t const reg_address) const noexcept -> Bytes<READ_SIZE>
    {
        if (this->initialized_) {
            spi_transaction_t transaction{};
            transaction.length = 0;
            transaction.rxlength = 8 * READ_SIZE;
            transaction.addr = reg_address;
            transaction.flags = SPI_TRANS_USE_RXDATA;

            gpio_set_level(this->chip_select_, 0);
            spi_device_polling_transmit(this->spi_device_, &transaction);
            gpio_set_level(this->chip_select_, 1);

            Bytes<READ_SIZE> read{};
            std::ranges::copy(transaction.rx_data, read.data());
            return read;
        }
        std::unreachable();
    }

    template <std::size_t WRITE_SIZE>
    auto SPIDevice::write_dwords(std::uint8_t const reg_address, DWords<WRITE_SIZE> const& write_data) const noexcept
        -> void
    {
        this->write_bytes(reg_address, dwords_to_bytes(write_data));
    }

    template <std::size_t WRITE_SIZE>
    auto SPIDevice::write_words(std::uint8_t const reg_address, Words<WRITE_SIZE> const& write_data) const noexcept
        -> void
    {
        this->write_bytes(reg_address, words_to_bytes(write_data));
    }

    template <std::size_t WRITE_SIZE>
    auto SPIDevice::write_bytes(std::uint8_t const reg_address, Bytes<WRITE_SIZE> const& write_data) const noexcept
        -> void
    {
        if (this->initialized_) {
            spi_transaction_t transaction{};
            transaction.length = 8 * WRITE_SIZE;
            transaction.rxlength = 0;
            transaction.addr = reg_address;
            transaction.flags = SPI_TRANS_USE_TXDATA;
            std::ranges::copy(write_data, transaction.tx_data);

            gpio_set_level(this->chip_select_, 0);
            spi_device_polling_transmit(this->spi_device_, &transaction);
            gpio_set_level(this->chip_select_, 1);
        }
    }

}; // namespace Utility

#endif // SPI_HPP