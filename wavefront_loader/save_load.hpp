#ifndef SAVE_LOAD_OBJ
#define SAVE_LOAD_OBJ
#include <cstdint>
#include <fstream>
namespace loader {
    uint32_t float_to_ieee_754(float value);
    float ieee_754_to_float(uint32_t ieee_754_fmt);
    void write_uint32(std::ofstream out_file, uint32_t to_write);
    uint32_t read_uint32(std::ifstream in_file);
}
#endif
