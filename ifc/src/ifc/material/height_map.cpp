#include <iostream>
#include <factory/texture_factory.h>
#include <ifc/measures.h>
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

float HeightMap::GetHeight(int i){
    //return texture_data_.data_[i] * texture_data_.max_height;

    return GLToMillimeters(texture_data_.data_[i]);

}

bool HeightMap::SetHeight(int i, float height){
    if(height > GetHeight(i))
        return false;

    //texture_data_.data_[i] = height / texture_data_.max_height;
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

}