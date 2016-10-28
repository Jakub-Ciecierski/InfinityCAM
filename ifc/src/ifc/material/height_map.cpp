#include <iostream>
#include <factory/texture_factory.h>
#include "ifc/material/height_map.h"

#include "shaders/textures/texture_loader.h"

namespace ifc {

HeightMap::HeightMap(int width, int height, float max_height){
    texture_data_.width = width;
    texture_data_.height = height;
    texture_data_.max_height = max_height;

    int count = width*height;
    positions_.resize(count);

    texture_data_.data_.resize(count);

    for(int i = 0; i < count;i++){
        texture_data_.data_[i] = 1;
    }

    texture_data_.texture
            = TextureLoader().
            CreateEmptyTexture(TextureTypes::DISPLACEMENT,
                               TextureInternalFormat::R,
                               TexturePixelType::FLOAT,
                               width, height);
    texture_data_.texture.AddParameter(TextureParameter{
            GL_TEXTURE_MIN_FILTER, GL_LINEAR
    });
    texture_data_.texture.AddParameter(TextureParameter{
            GL_TEXTURE_MAG_FILTER, GL_LINEAR
    });
    Update();
}

HeightMap::~HeightMap(){
    texture_data_.texture.Delete();
}

float HeightMap::GetHeight(int i){
    return texture_data_.data_[i] * texture_data_.max_height;
}

void HeightMap::SetHeight(int i, int j, float height){
    int k = i*texture_data_.width + j;
    if(height > texture_data_.max_height)
        height = texture_data_.max_height;

    texture_data_.data_[k] = height / texture_data_.max_height;
}

bool HeightMap::SetHeight(int i, float height){
    if(height > GetHeight(i))
        return false;

    texture_data_.data_[i] = height / texture_data_.max_height;
    return true;
    //texture_data_.data_[i] = height;
}

void HeightMap::Update(){
    texture_data_.texture.UpdateData(
            texture_data_.data_.data(),
            texture_data_.width,
            texture_data_.height
    );
}

}