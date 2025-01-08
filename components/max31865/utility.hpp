#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <array>
#include <bitset>
#include <cstdint>

namespace Utility {

    using Size = std::size_t;

    using Bit = bool;
    template <Size SIZE>
    using Bits = std::bitset<SIZE>;

    using Byte = std::uint8_t;
    template <Size SIZE>
    using Bytes = std::array<Byte, SIZE>;

    using Word = std::uint16_t;
    template <Size SIZE>
    using Words = std::array<Word, SIZE>;

    using DWord = std::uint32_t;
    template <Size SIZE>
    using DWords = std::array<DWord, SIZE>;

    template <Size NUM_BITS>
    inline Bytes<NUM_BITS / 8> bits_to_bytes(Bits<NUM_BITS> const& bits) noexcept
    {
        static_assert(NUM_BITS % 8 == 0);
        Bytes<NUM_BITS / 8> bytes{};
        for (auto i{0}; i < bytes.size(); ++i) {
            for (auto j{0}; j < 8; ++j) {
                if (bits[i * 8 + j]) {
                    bytes[i] |= (1U << j);
                } else {
                    bytes[i] &= ~(1U << j);
                }
            }
        }
        return bytes;
    }

    template <Size NUM_BYTES>
    inline Bits<8 * NUM_BYTES> bytes_to_bits(Bytes<NUM_BYTES> const& bytes) noexcept
    {
        Bits<8 * NUM_BYTES> bits{};
        for (auto i{0}; i < bytes.size(); ++i) {
            for (auto j{0}; j < 8; ++j) {
                bits[i * 8 + j] = (bytes[i] & (1 << j)) != 0;
            }
        }
        return bits;
    }

    template <Size NUM_BYTES>
    inline Words<NUM_BYTES / 2> bytes_to_words(Bytes<NUM_BYTES> const& bytes) noexcept
    {
        static_assert(NUM_BYTES % 2 == 0);
        Words<NUM_BYTES / 2> words{};
        for (auto i{0}; i < words.size(); ++i) {
            words[i] = static_cast<Word>(bytes[2 * i] << 8) | static_cast<Word>(bytes[2 * i + 1]);
        }
        return words;
    }

    template <Size NUM_WORDS>
    inline Bytes<2 * NUM_WORDS> words_to_bytes(Words<NUM_WORDS> const& words) noexcept
    {
        Bytes<2 * NUM_WORDS> bytes{};
        for (auto i{0}; i < words.size(); ++i) {
            bytes[2 * i] = static_cast<Byte>(words[i] >> 8);
            bytes[2 * i + 1] = static_cast<Byte>(words[i]);
        }
        return bytes;
    }

    template <Size NUM_WORDS>
    inline DWords<NUM_WORDS / 2> words_to_dwords(Words<NUM_WORDS> const& words) noexcept
    {
        static_assert(NUM_WORDS % 2 == 0);

        DWords<NUM_WORDS / 2> dwords{};
        for (auto i{0}; i < dwords.size(); ++i) {
            dwords[i] = static_cast<DWord>(words[2 * i] << 16) | static_cast<DWord>(words[2 * i + 1]);
        }
        return dwords;
    }

    template <Size NUM_DWORDS>
    inline Words<2 * NUM_DWORDS> dwords_to_words(DWords<NUM_DWORDS> const& dwords) noexcept
    {
        Words<2 * NUM_DWORDS> words{};
        for (auto i{0}; i < words.size(); ++i) {
            words[2 * i] = static_cast<Word>(words[i] >> 16);
            words[2 * i + 1] = static_cast<Word>(words[i]);
        }
        return words;
    }

    template <Size NUM_WORDS>
    inline Bits<16 * NUM_WORDS> words_to_bits(Words<NUM_WORDS> const& words) noexcept
    {
        return bytes_to_bits(words_to_bytes(words));
    }

    template <Size NUM_DWORDS>
    inline Bits<32 * NUM_DWORDS> dwords_to_bits(DWords<NUM_DWORDS> const& dwords) noexcept
    {
        return words_to_bits(dwords_to_words(dwords));
    }

    template <Size NUM_DWORDS>
    inline Bytes<4 * NUM_DWORDS> dwords_to_bytes(DWords<NUM_DWORDS> const& dwords) noexcept
    {
        return words_to_bytes(dwords_to_words(dwords));
    }

    template <Size NUM_BITS>
    inline Words<NUM_BITS / 16> bits_to_words(Bits<NUM_BITS> const& bits) noexcept
    {
        return bytes_to_words(bits_to_bytes(bits));
    }

    template <Size NUM_BITS>
    inline DWords<NUM_BITS / 32> bits_to_dwords(Bits<NUM_BITS> const& bits) noexcept
    {
        return words_to_dwords(bits_to_words(bits));
    }

    template <Size NUM_BYTES>
    inline DWords<NUM_BYTES / 4> bytes_to_dwords(Bytes<NUM_BYTES> const& bytes) noexcept
    {
        return words_to_dwords(bytes_to_words(bytes));
    }

}; // namespace Utility

#endif // UTILITY_HPP