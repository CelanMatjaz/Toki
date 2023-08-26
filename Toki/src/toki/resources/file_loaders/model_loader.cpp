#include "tkpch.h"
#include "model_loader.h"
#include "core/assert.h"
#include "resources/geometry.h"

namespace Toki {

    int mapLine(std::string line) {
        if (line[0] == '#') return -1;
        if (line[0] == 'v') {
            if (line[1] == 't') return 2;
            if (line[1] == 'n') return 3;
            return 1;
        }
        else if (line[0] == 'f') return 0;

        return -1;
    }

    std::vector<uint32_t> getCharacterIndexes(char* buffer, uint32_t len, char character = ' ') {
        std::vector<uint32_t> indexes;
        for (uint32_t i = 0; i < len; ++i) {
            if (buffer[i] == 0) break;
            if (buffer[i] == character) indexes.emplace_back(i);
        }
        return indexes;
    }

    void convertCharactersTo0(char* buffer, uint32_t len, char character = ' ') {
        for (uint32_t i = 0; i < len; ++i) {
            if (buffer[i] == character) buffer[i] = 0;
        }
    }

    glm::i32vec3 parseValuesF(char* buffer, uint32_t len) {
        auto slashIndexes = getCharacterIndexes(buffer, len, '/');

        glm::i32vec3 values = { -1, -1, -1 };
        if (slashIndexes.size() == 0) {
            values.x = std::stoi(buffer);
            return values;
        }
        else {
            convertCharactersTo0(buffer, len, '/');
            values.x = std::stoi(buffer);
            if (slashIndexes[0] < slashIndexes[1] - 1) {
                values.y = std::stoi(&buffer[slashIndexes[0] + 1]);
            }

            if (slashIndexes.size() == 2) {
                values.z = std::stoi(&buffer[slashIndexes[1] + 1]);
            }
        }

        return values;
    }

    GeometryData ModelLoader::loadFromObj(std::filesystem::path path) {
        TK_ASSERT(std::filesystem::exists(path), std::format("Model file {} does not exist", path.string()));

        std::ifstream file(path);

        TK_ASSERT(file.good(), std::format("Error opening file {}", path.string()));

        GeometryData model;

        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        std::vector<uint32_t> indexes;

        while (!file.eof()) {
            std::string buffer(100, 0);
            file.getline(buffer.data(), 100, '\n');

            // v
            if (buffer.starts_with(std::string_view("v "))) {
                std::stringstream ss{ buffer.substr(2) };
                glm::vec3 vertex;
                ss >> vertex.x >> vertex.y >> vertex.z;
                positions.emplace_back(vertex);
            }
            else if (buffer.starts_with(std::string_view("vn "))) {
                std::stringstream ss{ buffer.substr(3) };
                glm::vec3 normal;
                ss >> normal.x >> normal.y >> normal.z;
                normals.emplace_back(normal);
            }
            else if (buffer.starts_with(std::string_view("vt "))) {
                std::stringstream ss{ buffer.substr(3) };
                glm::vec2 uv;
                ss >> uv.x >> uv.y;
                uvs.emplace_back(uv);
            }
            else if (buffer.starts_with(std::string_view("f "))) {
                std::stringstream ss{ buffer.substr(2) };

                std::vector<glm::ivec3> vertexComponentIndexes;

                while (!ss.eof()) {
                    std::string temp(100, 0);
                    ss.getline(temp.data(), temp.size(), ' ');

                    int8_t firstSlash = temp.find('/');
                    int8_t secondSlash = temp.find('/', firstSlash + 1);

                    if (firstSlash == -1) { // only 1 entry per vertex
                        glm::ivec3 newVert = { std::stoi(temp), 0, 0 };
                        vertexComponentIndexes.emplace_back(newVert);
                    }
                    else if (secondSlash == -1) { // only 2 entries per vertex
                        glm::ivec3 newVert = { std::stoi(temp.substr(0, firstSlash)), std::stoi(temp.substr(firstSlash + 1)), 0 };
                        vertexComponentIndexes.emplace_back(newVert);
                    }
                    else if (secondSlash - firstSlash == 1) { // no uv per vertex
                        glm::ivec3 newVert = { std::stoi(temp.substr(0, firstSlash)), 0, std::stoi(temp.substr(firstSlash + 1)) };
                        vertexComponentIndexes.emplace_back(newVert);
                    }
                    else if (secondSlash - firstSlash > 1) { // all 3 values
                        glm::ivec3 newVert = { std::stoi(temp.substr(0, firstSlash)), std::stoi(temp.substr(firstSlash + 1, secondSlash - firstSlash)), std::stoi(temp.substr(secondSlash + 1)) };
                        vertexComponentIndexes.emplace_back(newVert);
                    }
                }

                for (const auto& c : vertexComponentIndexes) {
                    Vertex v{};
                    v.position = positions[c.x - 1];
                    if (c.y > 0) v.uv = uvs[c.y - 1];
                    if (c.z > 0) v.normal = normals[c.z - 1];

                    bool handled = false;
                    for (uint32_t i = 0; i < model.vertexData.size(); ++i) {
                        if (model.vertexData[i] == v) {
                            model.indexData.emplace_back(i);
                            handled = true;
                            break;
                        }
                    }

                    if (!handled) {
                        model.indexData.emplace_back(model.vertexData.size());
                        model.vertexData.emplace_back(v);
                    }
                }
            }
        }

        return model;
    }

}