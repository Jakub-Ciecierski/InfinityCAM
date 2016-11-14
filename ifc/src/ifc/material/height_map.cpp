#include <iostream>
#include <factory/texture_factory.h>
#include <ifc/measures.h>
#include "ifc/material/height_map.h"

namespace ifc {

HeightMap::HeightMap(int width, int height,
                     float width_mm, float height_mm,
                     float max_height){
    texture_data_.width = width;
    texture_data_.height = height;
    texture_data_.max_height = max_height;

    int count = width*height;
    positions_.resize(count);


    row_width_ = width / width_mm;
    column_width_ = height / height_mm;

    texture_data_.data_.resize(count);

    for(int i = 0; i < count; i++){
        texture_data_.data_[i] = MillimetersToGL(max_height);
    }

    for(int i = 0; i < count; i+=width){
        texture_data_.data_[i] = 0;
        if(i != 0)
            texture_data_.data_[i-1] = 0;
    }
    for(int i = 0; i < height; i++)
        texture_data_.data_[i] = 0;
    for(int i = count-1; i > count-1 - height; i--)
        texture_data_.data_[i] = 0;

    texture_data_.texture
            = ifx::Texture2D::MakeTexture2DEmpty("ifc_height_map",
                                                 ifx::TextureTypes::DISPLACEMENT,
                                                 ifx::TextureInternalFormat::R,
                                                 ifx::TexturePixelType::FLOAT,
                                                 width, height);
    texture_data_.texture->AddParameter(ifx::TextureParameter{
            GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE
    });
    texture_data_.texture->AddParameter(ifx::TextureParameter{
            GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE
    });
    texture_data_.texture->AddParameter(ifx::TextureParameter{
            GL_TEXTURE_MIN_FILTER, GL_LINEAR
    });
    texture_data_.texture->AddParameter(ifx::TextureParameter{
            GL_TEXTURE_MAG_FILTER, GL_LINEAR
    });
    Update();
}

HeightMap::~HeightMap(){
}

float HeightMap::GetHeight(int i, int j){
    return GLToMillimeters(texture_data_.data_[Index(i,j)]);
}

const glm::vec2& HeightMap::GetPosition(int i, int j){
    return positions_[Index(i,j)];
}

bool HeightMap::SetHeight(int i, int j, float height){
    if(height > GetHeight(Index(i,j)))
        return false;
    texture_data_.data_[Index(i,j)] = MillimetersToGL(height);
    return true;
}

glm::vec2 HeightMap::GetIndices(const glm::vec2& pos){
    int i = (pos.x - position_info_.const_single_box_scale_x) /
            position_info_.single_box_scale_x;
    int j = (pos.y - position_info_.const_single_box_scale_z) /
            position_info_.single_box_scale_z;
    return glm::vec2(i,j);
}

float HeightMap::GetHeight(int i){
    return GLToMillimeters(texture_data_.data_[i]);
}

bool HeightMap::SetHeight(int i, float height){
    if(height > GetHeight(i))
        return false;
    texture_data_.data_[i] = MillimetersToGL(height);
    return true;
}

void HeightMap::Update(){
    texture_data_.texture->InitData(
            (void*)texture_data_.data_.data(),
            texture_data_.width,
            texture_data_.height
    );
}

int HeightMap::Index(int i, int j){
    //return i*texture_data_.width + j;
    return j*texture_data_.width + i;
}

}