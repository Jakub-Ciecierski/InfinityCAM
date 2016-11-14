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

// In GL coordinates
struct PositionInfo{
    float single_box_scale_x;
    float single_box_scale_z;
    float const_single_box_scale_x;
    float const_single_box_scale_z;
};

class HeightMap {
public:

    HeightMap(int width, int height,
              float width_mm, float height_mm,
              float max_height);
    ~HeightMap();

    HeightMapTextureData* texture_data(){return &texture_data_;}
    std::vector<glm::vec2>& positions(){return positions_;}
    std::vector<float>& heights(){return texture_data_.data_;}
    void positions(std::vector<glm::vec2>& positions){
        positions_ = positions;
    }
    PositionInfo position_info(){return position_info_;}
    void position_info(PositionInfo position_info){
        position_info_ = position_info;
    }

    float row_width(){return row_width_;}
    float column_width(){return column_width_;}

    float GetHeight(int i, int j);
    const glm::vec2& GetPosition(int i, int j);
    bool SetHeight(int i, int j, float height);

    glm::vec2 GetIndices(const glm::vec2& pos);

    float GetHeight(int i);
    bool SetHeight(int i, float height);
    void Update();

private:
    int Index(int i, int j);

    float row_width_;
    float column_width_;

    HeightMapTextureData texture_data_;

    // positions are in GL coordinates.
    std::vector<glm::vec2> positions_;

    PositionInfo position_info_;
};
}

#endif //PROJECT_HEIGHT_MAP_H
