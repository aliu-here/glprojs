#include <vector>
#include <string>
#include <stddef.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>

#ifdef DEBUG
#include <iostream>
#endif


namespace gltf_loader {
    class File;
    class Scene;
    class Node;
    class Skin;
    class Camera;
    class Mesh;
    class Accessor;
    class Material;
    class Texture;
    class Sampler;
    class Image;
    class BufferView;
    class Buffer;
    class Animation;

    class Asset {
        public:
            std::string copyright, generator, version, minVersion;
    };

    class File {
        public:
            std::vector<std::string> extensionsUsed, extensionsRequired;
            std::vector<Accessor> accessors;
            std::vector<Animation> animations;
            Asset& asset;
            std::vector<Buffer> buffers;
            std::vector<BufferView> bufferViews;
            std::vector<Camera> cameras;
            std::vector<Image> images;
            std::vector<Material> Materials;
            std::vector<Mesh> meshes;
            std::vector<Sampler> samplers;
            std::vector<Skin> skins;
            std::vector<Texture> textures;
            int scene;
    };

    class Scene {
        public:
            std::vector<Node> nodes;
    };

    class Node {
        public:
            std::string name;
            std::vector<Node> children;
            glm::mat4 matrix;
            glm::vec3 translation, scale;
            glm::quat rotation;
    };

    class Skin {

    };

    class Camera {

    };

    class Mesh {
        class Primitive {
            int indices;
            int material;
            int mode;
        };
        std::vector<Primitive> primitives;
    };

    enum componentType {
        signedByte = 5120,
        unsignedByte = 5121,
        signedShort = 5122,
        unsignedShort = 5123,
        unsignedInt = 5125,
        Float = 5126
    };

    class Accessor {
        public:
            size_t bufferView, byteOffset;
            bool normalized;
            size_t count;
            std::string type;
            componentType componentType;
            int max, min;
            std::string name;
            int byteStride;
    };

    class Material {

    };

    class Texture {

    };

    class Image {

    };

    class Sampler {

    };

    enum bufferViewTarget {
        ARRAY_BUFFER = 34962,
        ELEMENT_ARRAY_BUFFER = 34963
    };
    class BufferView {
        public:
            size_t offset, length, bufferID, byteStride;
            bufferViewTarget target;
            std::string name;
    };

    class Buffer {
        public:
            uint8_t *buffer;
            size_t byteLength;
            std::string name;
    };

    class Animation {

    };
}
