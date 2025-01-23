#include "spi_device.hpp"
#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "utility.hpp"

namespace Utility {

    SPIDevice::SPIDevice(spi_device_handle_t const spi_device, gpio_num_t const chip_select) noexcept :
        spi_device_{spi_device}, chip_select_{chip_select}
    {
        this->initialize();
    }

    auto SPIDevice::transmit_dword(DWord const transmit_data) const noexcept -> void
    {
        this->transmit_dwords(DWords<1UL>{transmit_data});
    }

    auto SPIDevice::transmit_word(Word const transmit_data) const noexcept -> void
    {
        this->transmit_words(Words<1UL>{transmit_data});
    }

    auto SPIDevice::transmit_byte(Byte const transmit_data) const noexcept -> void
    {
        this->transmit_bytes(Bytes<1UL>{transmit_data});
    }

    auto SPIDevice::receive_dword() const noexcept -> DWord
    {
        return this->receive_dwords<1UL>()[0];
    }

    auto SPIDevice::receive_word() const noexcept -> Word
    {
        return this->receive_words<1UL>()[0];
    }

    auto SPIDevice::receive_byte() const noexcept -> Byte
    {
        return this->receive_bytes<1UL>()[0];
    }

    auto SPIDevice::read_dword(std::uint8_t const reg_address) const noexcept -> DWord
    {
        return this->read_dwords<1UL>(reg_address)[0];
    }

    auto SPIDevice::read_word(std::uint8_t const reg_address) const noexcept -> Word
    {
        return this->read_words<1UL>(reg_address)[0];
    }

    auto SPIDevice::read_byte(std::uint8_t const reg_address) const noexcept -> Byte
    {
        return this->read_bytes<1UL>(reg_address)[0];
    }

    auto SPIDevice::read_bits(std::uint8_t const reg_address,
                              std::uint8_t const read_position,
                              std::size_t const read_size) const noexcept -> Byte
    {
        return get_bits(this->read_byte(reg_address), read_size, read_position);
    }

    auto SPIDevice::read_bit(std::uint8_t const reg_address, std::uint8_t const read_position) const noexcept -> Bit
    {
        return get_bit(this->read_byte(reg_address), read_position);
    }

    auto SPIDevice::write_dword(std::uint8_t const reg_address, DWord const write_data) const noexcept -> void
    {
        this->write_dwords(reg_address, DWords<1UL>{write_data});
    }

    auto SPIDevice::write_word(std::uint8_t const reg_address, Word const write_data) const noexcept -> void
    {
        this->write_words(reg_address, Words<1UL>{write_data});
    }

    auto SPIDevice::write_byte(std::uint8_t const reg_address, Byte const write_data) const noexcept -> void
    {
        this->write_bytes(reg_address, Bytes<1UL>{write_data});
    }

    auto SPIDevice::write_bits(std::uint8_t const reg_address,
                               Byte const write_data,
                               std::uint8_t const write_position,
                               std::size_t const write_size) const noexcept -> void
    {
        Byte write{this->read_byte(reg_address)};
        set_bits(write, write_data, write_size, write_position);
        this->write_byte(reg_address, write);
    }

    auto SPIDevice::write_bit(std::uint8_t const reg_address,
                              Bit const write_data,
                              std::uint8_t const write_position) const noexcept -> void
    {
        Byte write{this->read_byte(reg_address)};
        set_bit(write, write_data, write_position);
        this->write_byte(reg_address, write);
    }

    auto SPIDevice::initialize() noexcept -> void
    {
        if (this->spi_device_ != nullptr) {
            gpio_set_level(this->chip_select_, 1);
            this->initialized_ = true;
        }
    }

    auto SPIDevice::deinitialize() noexcept -> void
    {
        if (this->spi_device_ != nullptr) {
            gpio_set_level(this->chip_select_, 0);
            this->initialized_ = false;
        }
    }

}; // namespace Utility