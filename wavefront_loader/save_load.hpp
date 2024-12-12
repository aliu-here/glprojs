#ifndef SAVE_LOAD_OBJ
#define SAVE_LOAD_OBJ
#include <string>
#include "mesh.hpp"
namespace loader {
    void save_model(model model, std::string savepath);
    model load_model(std::string path);
}
#endif
