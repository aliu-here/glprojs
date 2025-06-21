#include <unordered_map>
#include <variant>
#include <string>
#include <optional>
#include <vector>

namespace jsonparse {
    class jsonobject;
    using value = std::variant<int, double, std::string, jsonobject,
                               std::vector<int>, std::vector<double>, std::vector<std::string>, std::vector<jsonobject>>;

    class jsonobject {
        std::unordered_map<std::string, value> object;

        public:
            auto begin() {
                return object.begin();
            }

            auto end() {
                return object.end();
            }

            std::optional<value> find(std::string key) {
                if (object.find(key) == object.end())
                    return {};
                return object[key];
            }

            template <typename T>
            void add(std::string key, T val) {
                value tmp;
                tmp.emplace<T>(val);
                object[key] = tmp;
            }
    };
}
