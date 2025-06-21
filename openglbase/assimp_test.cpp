#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

int main()
{
    Assimp::Importer in;
    const aiScene* scene = in.ReadFile("/home/aliu/models/stanford-bunny.obj", aiProcess_Triangulate);
}
