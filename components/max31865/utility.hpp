#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <array>
#include <bit>
#include <bitset>
#include <cstdint>

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

    template <std::size_t BITS>
    [[nodiscard]] constexpr auto bits_to_bytes(Bits<BITS> const& bits) noexcept -> Bytes<BITS / 8>
    {
        static_assert(BITS % 8 == 0);
        Bytes<BITS / 8> bytes{};
        for (std::size_t i{}; i < bytes.size(); ++i) {
            for (std::size_t j{}; j < 8; ++j) {
                if (bits[i * 8 + j]) {
                    bytes[i] |= (1 << j);
                }
            }
        }
        return bytes;
    }

    template <std::size_t BYTES>
    [[nodiscard]] constexpr auto bytes_to_bits(Bytes<BYTES> const& bytes) noexcept -> Bits<8 * BYTES>
    {
        Bits<8 * BYTES> bits{};
        for (std::size_t i{}; i < bytes.size(); ++i) {
            for (std::size_t j{}; j < 8; ++j) {
                bits[i * 8 + j] = (bytes[i] & (1 << j)) != 0;
            }
        }
        return bits;
    }

    template <std::size_t BYTES>
    [[nodiscard]] constexpr auto bytes_to_words(Bytes<BYTES> const& bytes) noexcept -> Words<BYTES / 2>
    {
        static_assert(BYTES % 2 == 0);
        Words<BYTES / 2> words{};
        for (std::size_t i{}; i < words.size(); ++i) {
            words[i] = static_cast<Word>(bytes[2 * i] << 8) | static_cast<Word>(bytes[2 * i + 1]);
        }
        return words;
    }

    template <std::size_t WORDS>
    [[nodiscard]] constexpr auto words_to_bytes(Words<WORDS> const& words) noexcept -> Bytes<2 * WORDS>
    {
        Bytes<2 * WORDS> bytes{};
        for (std::size_t i{}; i < words.size(); ++i) {
            bytes[2 * i] = static_cast<Byte>(words[i] >> 8);
            bytes[2 * i + 1] = static_cast<Byte>(words[i]);
        }
        return bytes;
    }

    template <std::size_t WORDS>
    [[nodiscard]] constexpr auto words_to_dwords(Words<WORDS> const& words) noexcept -> DWords<WORDS / 2>
    {
        static_assert(WORDS % 2 == 0);

        DWords<WORDS / 2> dwords{};
        for (std::size_t i{}; i < dwords.size(); ++i) {
            dwords[i] = static_cast<DWord>(words[2 * i] << 16) | static_cast<DWord>(words[2 * i + 1]);
        }
        return dwords;
    }

    template <std::size_t DWORDS>
    [[nodiscard]] constexpr auto dwords_to_words(DWords<DWORDS> const& dwords) noexcept -> Words<2 * DWORDS>
    {
        Words<2 * DWORDS> words{};
        for (std::size_t i{}; i < words.size(); ++i) {
            words[2 * i] = static_cast<Word>(dwords[i] >> 16);
            words[2 * i + 1] = static_cast<Word>(dwords[i]);
        }
        return words;
    }

    template <std::size_t WORDS>
    [[nodiscard]] constexpr auto words_to_bits(Words<WORDS> const& words) noexcept -> Bits<16 * WORDS>
    {
        return bytes_to_bits(words_to_bytes(words));
    }

    template <std::size_t DWORDS>
    [[nodiscard]] constexpr auto dwords_to_bits(DWords<DWORDS> const& dwords) noexcept -> Bits<32 * DWORDS>
    {
        return words_to_bits(dwords_to_words(dwords));
    }

    template <std::size_t DWORDS>
    [[nodiscard]] constexpr auto dwords_to_bytes(DWords<DWORDS> const& dwords) noexcept -> Bytes<4 * DWORDS>
    {
        return words_to_bytes(dwords_to_words(dwords));
    }

    template <std::size_t BITS>
    [[nodiscard]] constexpr auto bits_to_words(Bits<BITS> const& bits) noexcept -> Words<BITS / 16>
    {
        return bytes_to_words(bits_to_bytes(bits));
    }

    template <std::size_t BITS>
    [[nodiscard]] constexpr auto bits_to_dwords(Bits<BITS> const& bits) noexcept -> DWords<BITS / 32>
    {
        return words_to_dwords(bits_to_words(bits));
    }

    template <std::size_t BYTES>
    [[nodiscard]] constexpr auto bytes_to_dwords(Bytes<BYTES> const& bytes) noexcept -> DWords<BYTES / 4>
    {
        return words_to_dwords(bytes_to_words(bytes));
    }

    template <std::size_t BYTES>
    [[nodiscard]] constexpr auto endian_swap(Bytes<BYTES> const& bytes) noexcept -> Bytes<BYTES>
    {
        Bytes<BYTES> result{};
        for (auto& byte : result) {
            byte = std::byteswap(byte);
        }
        return result;
    }

    template <std::size_t WORDS>
    [[nodiscard]] constexpr auto endian_swap(Words<WORDS> const& words) noexcept -> Words<WORDS>
    {
        return bytes_to_words(endian_swap(words_to_bytes(words)));
    }

    template <std::size_t DWORDS>
    [[nodiscard]] constexpr auto endian_swap(DWords<DWORDS> const& dwords) noexcept -> DWords<DWORDS>
    {
        return bytes_to_dwords(endian_swap(dwords_to_bytes(dwords)));
    }

    [[nodiscard]] constexpr auto set_bits(Byte& byte,
                                          Byte const write_data,
                                          std::size_t const write_size,
                                          std::uint8_t const write_position) noexcept -> void
    {
        Byte mask = ((1U << write_size) - 1) << (write_position - write_size + 1);
        Byte temp = (write_data << (write_position - write_size + 1)) & mask;
        byte &= ~mask;
        byte |= temp;
    }

    [[nodiscard]] constexpr auto set_bit(Byte& byte, Bit const write_data, std::uint8_t const write_position) noexcept
        -> void
    {
        write_data ? (byte |= (1U << write_position)) : (byte &= ~(1U << write_position));
    }

    [[nodiscard]] constexpr auto
    get_bits(Byte byte, std::size_t const read_size, std::uint8_t const read_position) noexcept -> Byte
    {
        Byte mask = ((1U << read_size) - 1) << (read_position - read_size + 1);
        byte &= mask;
        byte >>= (read_position - read_size + 1);
        return byte;
    }

    [[nodiscard]] constexpr auto get_bit(Byte byte, std::uint8_t const read_position) noexcept -> Bit
    {
        return (byte & (1U << read_position)) ? 1 : 0;
    }

}; // namespace Utility

#endif // UTILITY_HPP