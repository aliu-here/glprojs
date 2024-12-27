#include <fstream>
#include <bit>
#include <limits>
#include <cstdint>
#include <cstring>
#include <cmath>

namespace loader {
    constexpr bool is_little_endian()
    {
        return std::endian::native == std::endian::little;
    }

    uint32_t float_to_ieee_754(float value)  // there's std::float32_t but that's not supported by clang++ (as of now, 12/26/24)
    {
        if (value != value)
            return 0x7f800001; //NaN checking
        if (std::isinf(value)) {
            if (value < 0)
                return 0xff800000; //pos inf
            else
                return 0x7f800000; // neg inf
        }
        const uint32_t sign_shift = 31, exp_shift = 23;
        uint32_t ieee_754_fmt = 0;
        ieee_754_fmt |= (value < 0) << sign_shift;
        if (value == 0)
            return ieee_754_fmt; // only need sign
        double dbl_exp = std::log2(std::abs(value));
        int exp = std::floor(dbl_exp); //so no significands < 1
        unsigned char exp_8b;
        if (exp < -126) {
            exp_8b = 0;
        } else if (exp > 127) {
            return 0x7f800000 | ((value < 0) << sign_shift); // signed inf
        } else {
            exp_8b = exp + 127;
        }

        ieee_754_fmt |= exp_8b << exp_shift;

        double significand;
        if (exp_8b == 0)
            significand = std::abs((double)value) / (std::exp2(-126)); // check for subnormals
        else
            significand = std::abs((double)value) / (std::exp2(exp_8b - 127)) - 1;
        uint32_t shifted_significand = significand * std::exp2(23);
        ieee_754_fmt |= shifted_significand;
        return ieee_754_fmt;
    }
    
    float ieee_754_to_float(uint32_t ieee_754_fmt)
    {
        const uint32_t significand_bitmask = 0x007fffff, exp_bitmask = 0x7f800000, exp_shift = 23, sign_shift = 31;
        float out = 0;
        uint32_t significand = (ieee_754_fmt & significand_bitmask);
        out = ((double)significand * (std::exp2(-23))) + 1;
        unsigned char exp = (ieee_754_fmt & exp_bitmask) >> exp_shift;
        if (exp == 0) {
            exp = 1; //subnormals
            out -= 1; //subnormals don't have addition here
        }
        bool sign = ieee_754_fmt >> sign_shift;
        if (exp == 0 && significand == 0) { // signed zeroes
            return (sign) ? -0 : +0;
        }
        if (exp == 255) {
            if (significand == 0)
                return ((sign) ? -1 : 1) * std::numeric_limits<float>::infinity();
            else
                return NAN;
        }
        return std::exp2(exp - 127) * out * ((sign) ? -1 : 1);
    }

    inline uint32_t native_to_bigendian(uint32_t in)
    {
        if (is_little_endian())
            return std::byteswap(in);
        return in;
    }

    inline uint32_t bigendian_to_native(uint32_t in)
    {
        if (is_little_endian())
            return std::byteswap(in);
        return in;
    }



    inline void write_uint32(std::ofstream out_file, uint32_t to_write)
    {
        out_file.write(reinterpret_cast<char*>(&to_write), sizeof(uint32_t));
    }

    inline uint32_t read_uint32(std::ifstream in_file) {
        char *read_in;
        in_file.read(read_in, sizeof(uint32_t));
        uint32_t out = *reinterpret_cast<uint32_t*>(read_in);
        return out;
    }
}
