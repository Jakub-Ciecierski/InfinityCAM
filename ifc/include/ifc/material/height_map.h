#ifndef PROJECT_HEIGHT_MAP_H
#define PROJECT_HEIGHT_MAP_H

#include <shaders/data/shader_data.h>
#include <shaders/textures/texture.h>

namespace ifc {

struct HeightMapTextureData{
    std::shared_ptr<ifx::Texture2D> texture;
    std::vector<float> data_;

    int width;
    int height;
    float max_height;
};

class HeightMap {
public:

    HeightMap(int width, int height, float max_height);
    ~HeightMap();

    HeightMapTextureData* texture_data(){return &texture_data_;}
    std::vector<glm::vec2>& positions(){return positions_;}
    std::vector<float>& heights(){return texture_data_.data_;}
    void positions(std::vector<glm::vec2>& positions){
        positions_ = positions;
    }
    float GetHeight(int i);
    bool SetHeight(int i, float height);
    void Update();

private:
    HeightMapTextureData texture_data_;

    // positions are in GL coordinates.
    std::vector<glm::vec2> positions_;
};
}

#endif //PROJECT_HEIGHT_MAP_H
