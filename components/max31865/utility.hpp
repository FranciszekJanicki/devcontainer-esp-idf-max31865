#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <array>
#include <bit>
#include <bitset>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace Utility {

    using Bit = bool;
    using Byte = std::uint8_t;
    using Word = std::uint16_t;
    using DWord = std::uint32_t;

    template <std::size_t SIZE>
    using Bits = std::bitset<SIZE>;
    template <std::size_t SIZE>
    using Bytes = std::array<Byte, SIZE>;
    template <std::size_t SIZE>
    using Words = std::array<Word, SIZE>;
    template <std::size_t SIZE>
    using DWords = std::array<DWord, SIZE>;

    template <typename Value, std::size_t SIZE>
    inline std::array<std::uint8_t, SIZE * sizeof(Value)>
    values_to_bytes(std::array<Value, SIZE> const& values) noexcept
    {
        std::array<std::uint8_t, SIZE * sizeof(Value)> bytes{};
        std::memcpy(bytes.data(), values.data(), SIZE * sizeof(Value));
        return bytes;
    }

    template <typename Value, std::size_t SIZE>
    inline std::array<Value, SIZE> bytes_to_values(std::array<std::uint8_t, SIZE * sizeof(Value)> const& bytes) noexcept
    {
        std::array<Value, SIZE> values{};
        std::memcpy(values.data(), bytes.data(), SIZE * sizeof(Value));
        return values;
    }

    template <std::size_t SIZE>
    inline std::array<std::uint8_t, SIZE / 8> bits_to_bytes(std::bitset<SIZE> const& bits) noexcept
    {
        static_assert(SIZE % 8 == 0);
        std::array<std::uint8_t, SIZE / 8> bytes{};
        for (std::size_t i{}; i < bytes.size(); ++i) {
            for (std::size_t j{}; j < 8; ++j) {
                if (bits[i * 8 + j]) {
                    bytes[i] |= (1 << j);
                }
            }
        }
        return bytes;
    }

    template <std::size_t SIZE>
    inline std::bitset<8 * SIZE> bytes_to_bits(std::array<std::uint8_t, SIZE> const& bytes) noexcept

    {
        std::bitset<8 * SIZE> bits{};
        for (std::size_t i{}; i < bytes.size(); ++i) {
            for (std::size_t j{}; j < 8; ++j) {
                bits[i * 8 + j] = (bytes[i] & (1 << j)) != 0;
            }
        }
        return bits;
    }

    template <std::size_t SIZE>
    inline std::array<std::uint16_t, SIZE / 2>
    bytes_in_big_endian_to_words(std::array<std::uint8_t, SIZE> const& bytes) noexcept
    {
        static_assert(SIZE % 2 == 0);
        std::array<std::uint16_t, SIZE / 2> words{};
        for (std::size_t i{}; i < words.size(); ++i) {
            words[i] = static_cast<std::uint16_t>(bytes[2 * i] << 8) | static_cast<std::uint16_t>(bytes[2 * i + 1]);
        }
        return words;
    }

    template <std::size_t SIZE>
    inline std::array<std::uint16_t, SIZE / 2>
    bytes_in_little_endian_to_words(std::array<std::uint8_t, SIZE> const& bytes) noexcept
    {
        static_assert(SIZE % 2 == 0);
        std::array<std::uint16_t, SIZE / 2> words{};
        for (std::size_t i{}; i < words.size(); ++i) {
            words[i] = static_cast<std::uint16_t>(bytes[2 * i]) | static_cast<std::uint16_t>(bytes[2 * i + 1] << 8);
        }
        return words;
    }

    template <std::size_t SIZE>
    inline std::array<std::uint16_t, SIZE / 2> bytes_to_words(std::array<std::uint8_t, SIZE> const& bytes,
                                                              std::endian const endian = std::endian::big) noexcept
    {
        return endian == std::endian::little ? bytes_in_little_endian_to_words(bytes)
                                             : bytes_in_big_endian_to_words(bytes);
    }

    template <std::size_t SIZE>
    inline std::array<std::uint8_t, 2 * SIZE>
    words_to_big_endian_bytes(std::array<std::uint16_t, SIZE> const& words) noexcept
    {
        std::array<std::uint8_t, 2 * SIZE> bytes{};
        for (std::size_t i{}; i < words.size(); ++i) {
            bytes[2 * i] = static_cast<std::uint8_t>(words[i] >> 8);
            bytes[2 * i + 1] = static_cast<std::uint8_t>(words[i]);
        }
        return bytes;
    }

    template <std::size_t SIZE>
    inline std::array<std::uint8_t, 2 * SIZE>
    words_to_little_endian_bytes(std::array<std::uint16_t, SIZE> const& words) noexcept
    {
        std::array<std::uint8_t, 2 * SIZE> bytes{};
        for (std::size_t i{}; i < words.size(); ++i) {
            bytes[2 * i] = static_cast<std::uint8_t>(words[i]);
            bytes[2 * i + 1] = static_cast<std::uint8_t>(words[i] >> 8);
        }
        return bytes;
    }

    template <std::size_t SIZE>
    inline std::array<std::uint8_t, 2 * SIZE> words_to_bytes(std::array<std::uint16_t, SIZE> const& words,
                                                             std::endian const endian = std::endian::big) noexcept
    {
        return endian == std::endian::little ? words_to_little_endian_bytes(words) : words_to_big_endian_bytes(words);
    }

    template <std::size_t SIZE>
    inline std::array<std::uint32_t, SIZE / 4>
    bytes_in_big_endian_to_dwords(std::array<std::uint8_t, SIZE> const& bytes) noexcept
    {
        static_assert(SIZE % 4 == 0);
        std::array<std::uint32_t, SIZE / 4> dwords{};
        for (std::size_t i{}; i < dwords.size(); ++i) {
            dwords[i] =
                static_cast<std::uint32_t>(bytes[2 * i] << 24) | static_cast<std::uint32_t>(bytes[2 * i + 1] << 16) |
                static_cast<std::uint32_t>(bytes[2 * i + 2] << 8) | static_cast<std::uint32_t>(bytes[2 * i + 3]);
        }
        return dwords;
    }

    template <std::size_t SIZE>
    inline std::array<std::uint32_t, SIZE / 4>
    bytes_in_little_endian_to_dwords(std::array<std::uint8_t, SIZE> const& bytes) noexcept
    {
        static_assert(SIZE % 4 == 0);
        std::array<std::uint32_t, SIZE / 4> dwords{};
        for (std::size_t i{}; i < dwords.size(); ++i) {
            dwords[i] = static_cast<std::uint32_t>(bytes[2 * i]) | static_cast<std::uint32_t>(bytes[2 * i + 1] << 8) |
                        static_cast<std::uint32_t>(bytes[2 * i + 2] << 16) |
                        static_cast<std::uint32_t>(bytes[2 * i + 3] << 24);
        }
        return dwords;
    }

    template <std::size_t SIZE>
    inline std::array<std::uint32_t, SIZE / 4> bytes_to_dwords(std::array<std::uint8_t, SIZE> const& bytes,
                                                               std::endian const endian = std::endian::big) noexcept
    {
        return endian == std::endian::little ? bytes_in_little_endian_to_dwords(bytes)
                                             : bytes_in_big_endian_to_dwords(bytes);
    }

    template <std::size_t SIZE>
    inline std::array<std::uint8_t, 4 * SIZE>
    dwords_to_bytes_in_big_endian(std::array<std::uint32_t, SIZE> const& dwords) noexcept
    {
        std::array<std::uint8_t, 4 * SIZE> bytes{};
        for (std::size_t i{}; i < dwords.size(); ++i) {
            bytes[2 * i] = static_cast<std::uint8_t>(dwords[i] >> 24);
            bytes[2 * i + 1] = static_cast<std::uint8_t>(dwords[i] >> 16);
            bytes[2 * i + 2] = static_cast<std::uint8_t>(dwords[i] >> 8);
            bytes[2 * i + 3] = static_cast<std::uint8_t>(dwords[i]);
        }
        return bytes;
    }

    template <std::size_t SIZE>
    inline std::array<std::uint8_t, 4 * SIZE>
    dwords_to_bytes_in_little_endian(std::array<std::uint32_t, SIZE> const& dwords) noexcept
    {
        std::array<std::uint8_t, 4 * SIZE> bytes{};
        for (std::size_t i{}; i < dwords.size(); ++i) {
            bytes[2 * i] = static_cast<std::uint8_t>(dwords[i]);
            bytes[2 * i + 1] = static_cast<std::uint8_t>(dwords[i] >> 8);
            bytes[2 * i + 1] = static_cast<std::uint8_t>(dwords[i] >> 16);
            bytes[2 * i + 1] = static_cast<std::uint8_t>(dwords[i] >> 24);
        }
        return bytes;
    }

    template <std::size_t SIZE>
    inline std::array<std::uint8_t, 4 * SIZE> dwords_to_bytes(std::array<std::uint32_t, SIZE> const& dwords,
                                                              std::endian const endian = std::endian::big) noexcept
    {
        return endian == std::endian::little ? dwords_to_bytes_in_little_endian(dwords)
                                             : dwords_to_bytes_in_big_endian(dwords);
    }

    inline void set_bits(std::uint8_t& byte,
                         std::uint8_t const write_data,
                         std::size_t const size,
                         std::uint8_t const position) noexcept
    {
        std::uint8_t mask = ((1U << size) - 1) << (position - size + 1);
        std::uint8_t temp = (write_data << (position - size + 1)) & mask;
        byte &= ~mask;
        byte |= temp;
    }

    inline void set_bit(std::uint8_t& byte, bool const write_data, std::uint8_t const position) noexcept
    {
        write_data ? (byte |= (1U << position)) : (byte &= ~(1U << position));
    }

    inline std::uint8_t get_bits(std::uint8_t byte, std::size_t const size, std::uint8_t const position) noexcept
    {
        std::uint8_t mask = ((1U << size) - 1) << (position - size + 1);
        byte &= mask;
        byte >>= (position - size + 1);
        return byte;
    }

    inline bool get_bit(std::uint8_t byte, std::uint8_t const position) noexcept
    {
        return (byte & (1U << position)) ? true : false;
    }

}; // namespace Utility

#endif // UTILITY_HPP