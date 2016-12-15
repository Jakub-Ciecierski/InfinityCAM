#include "ifc/path_generation/paths/roughing_path.h"

#include <ifc/path_generation/height_map_paths.h>
#include <ifc/material/material_box.h>
#include <ifc/material/height_map.h>
#include <ifc/measures.h>
#include <ifc/factory/cad_model_loader.h>
#include <ifc/cutter/instruction.h>
#include <object/render_object.h>
#include <infinity_cad/rendering/render_objects/surfaces/surface_c2_cylind.h>
#include <ifc/cutter/cutter.h>

namespace ifc {

RoughingPath::RoughingPath(
        std::shared_ptr<CADModelLoaderResult> model_loader_result,
        std::shared_ptr<MaterialBox> material_box)
        : model_loader_result_(model_loader_result),
          material_box_(material_box){}

RoughingPath::~RoughingPath(){}

std::shared_ptr<Cutter> RoughingPath::Generate(
        std::shared_ptr<HeightMapPath> height_map_path){
    //DEBUG_AddHeightMapTexture(height_map_path, material_box_);

    std::cout << "1) Generating RoughingPath" << std::endl;
    auto cutter = CreatePath(height_map_path,
                             material_box_);
    return cutter;
}

std::shared_ptr<Cutter> RoughingPath::CreatePath(
        std::shared_ptr<HeightMapPath> height_map_path,
        std::shared_ptr<MaterialBox> material_box){
    int n = height_map_path->row_count;
    int m = height_map_path->column_count;

    const float save_height = material_box->dimensions().depth + 10.0f;

    const float diameter = 16.0f;
    const float radius = diameter / 2.0f;
    const float epsilon = 2.0f;

    int look_ahead_radius_row
            = (radius + epsilon) / height_map_path->row_width;
    int look_ahead_radius_column
            = (radius + epsilon) / height_map_path->column_width;
    int skip_columns = (radius-epsilon) / height_map_path->column_width;
    int skip_rows = (radius-epsilon) / height_map_path->row_width;

    std::cout << "skip_columns: " << skip_columns << std::endl;
    float start_height = material_box->dimensions().depth;

    auto instructions = CreatePathZigZag(height_map_path,
                                            save_height, start_height,
                                            radius, n, m,
                                            skip_rows, skip_columns,
                                            look_ahead_radius_row,
                                            look_ahead_radius_column);

    auto cutter = std::shared_ptr<Cutter>(new Cutter(CutterType::Sphere,
                                                     diameter,
                                                     instructions));
    return cutter;
}

std::vector<Instruction> RoughingPath::CreatePathFromAbove(
        std::shared_ptr<HeightMapPath> height_map_path,
        float save_height, float start_height, float radius,
        int n, int m,
        int skip_rows, int skip_columns,
        int look_ahead_radius_row, int look_ahead_radius_column){
    std::vector<Instruction> instructions;

    int id = 0;
    instructions.push_back(
            CreateInstruction(id++, height_map_path->Position(0,0),
                              save_height));
    for(int i = 0; i < n; i+=skip_rows){
        for(int j = 0; j < m-1; j+=skip_columns){
            float max_height = MaxHeightInVicinity(i,j,n,m,
                                                   look_ahead_radius_row,
                                                   look_ahead_radius_column,
                                                   height_map_path);
            instructions.push_back(
                    CreateInstruction(id++, height_map_path->Position(i,j),
                                      GLToMillimeters(max_height)));
            instructions.push_back(
                    CreateInstruction(id++, height_map_path->Position(i,j),
                                      save_height));
            instructions.push_back(
                    CreateInstruction(id++, height_map_path->Position(i,j+1),
                                      save_height));
        }
        instructions.push_back(
                CreateInstruction(id++, height_map_path->Position(i,0),
                                  save_height));
    }
    return instructions;
}


std::vector<Instruction> RoughingPath::CreatePathZigZag(
        std::shared_ptr<HeightMapPath> height_map_path,
        float save_height, float start_height, float radius,
        int n, int m,
        int skip_rows, int skip_columns,
        int look_ahead_radius_row, int look_ahead_radius_column){
    std::vector<Instruction> instructions;

    int id = 0;
    float max_height = MaxHeightInVicinity(0,0,n,m,
                                           look_ahead_radius_row,
                                           look_ahead_radius_column,
                                           height_map_path);
    instructions.push_back(
            CreateInstruction(id++,
                              height_map_path->Position(0,0)
                              + MillimetersToGL(glm::vec2(-10,0)),
                              save_height));
    instructions.push_back(
            CreateInstruction(id++,
                              height_map_path->Position(0,0)
                              + MillimetersToGL(glm::vec2(-10,0)),
                              max_height));
    /*
    int direction = 1;
    for(int i = 0; i < n; i+=skip_rows){
        for (int j = m-1; j > 1; j -= skip_columns) {
            float max_height1 = MaxHeightInVicinity(i, j, n, m,
                                                    look_ahead_radius_row,
                                                    look_ahead_radius_column,
                                                    height_map_path);
            float max_height2 = MaxHeightInVicinity(i, j - skip_columns, n,
                                                    m,
                                                    look_ahead_radius_row,
                                                    look_ahead_radius_column,
                                                    height_map_path);
            instructions.push_back(
                    CreateInstruction(id++, height_map_path->Position(i, j),
                                      GLToMillimeters(max_height1)));
            instructions.push_back(
                    CreateInstruction(id++,
                                      height_map_path->Position(
                                              i, j - 1),
                                      GLToMillimeters(max_height2)));
            direction = 1;
        }
    }
    */








    int direction = 1;
    for(int i = 0; i < n; i+=skip_rows){
        if(direction == 1){
        for(int j = 0; j < m-1; j+=skip_columns){
            float max_height1 = MaxHeightInVicinity(i,j,n,m,
                                                   look_ahead_radius_row,
                                                   look_ahead_radius_column,
                                                   height_map_path);
            float max_height2 = MaxHeightInVicinity(i,j+skip_columns,n,m,
                                                    look_ahead_radius_row,
                                                    look_ahead_radius_column,
                                                    height_map_path);
            instructions.push_back(
                    CreateInstruction(id++, height_map_path->Position(i,j),
                                      GLToMillimeters(max_height1)));
            instructions.push_back(
                    CreateInstruction(id++,
                                      height_map_path->Position(
                                              i,j + 1),
                                      GLToMillimeters(max_height2)));
            direction = -1;
        }}
        else if(direction == -1) {
            for (int j = m-1; j > 1; j -= skip_columns) {
                float max_height1 = MaxHeightInVicinity(i, j, n, m,
                                                        look_ahead_radius_row,
                                                        look_ahead_radius_column,
                                                        height_map_path);
                float max_height2 = MaxHeightInVicinity(i, j - skip_columns, n,
                                                        m,
                                                        look_ahead_radius_row,
                                                        look_ahead_radius_column,
                                                        height_map_path);
                instructions.push_back(
                        CreateInstruction(id++, height_map_path->Position(i, j),
                                          GLToMillimeters(max_height1)));
                instructions.push_back(
                        CreateInstruction(id++,
                                          height_map_path->Position(
                                                  i, j - 1),
                                          GLToMillimeters(max_height2)));
                direction = 1;
            }
        }
    }
    return instructions;
}

float RoughingPath::MaxHeightInVicinity(
        int i, int j, int n, int m,
        int look_ahead_radius_row,
        int look_ahead_radius_column,
        std::shared_ptr<HeightMapPath> height_map_path){
    float max = -1.0f;
    // Go back if encoutered obstacle
    for(int ii = -look_ahead_radius_row; ii < look_ahead_radius_row; ii++){
        if(ii + i < 0 || ii + i >= n)
            continue;
        for(int jj = -look_ahead_radius_column; jj < look_ahead_radius_column; jj++){
            if(jj + j < 0 || jj + j >= m)
                continue;

            float height = height_map_path->GetHeight(ii+i,jj+j);
            if(height > max)
                max = height;
        }
    }
    return max;
}

Instruction RoughingPath::CreateInstruction(int id,
                                            const glm::vec2& v,
                                            float height){
    glm::vec3 pos2 = glm::vec3(GLToMillimeters(v.x),
                               GLToMillimeters(v.y),
                               height);
    return Instruction(id, pos2);
}

void RoughingPath::DEBUG_AddHeightMapTexture(
        std::shared_ptr<HeightMapPath> height_map_path,
        std::shared_ptr<MaterialBox> material_box_){
    auto object = material_box_->box_render_object();

    debug_texture_
            = ifx::Texture2D::MakeTexture2DEmpty(
                    ifx::NO_FILEPATH,
                    ifx::TextureTypes::DISPLACEMENT,
                    ifx::TextureInternalFormat::R,
                    ifx::TexturePixelType::FLOAT,
                    material_box_->height_map()->texture_data()->width,
                    material_box_->height_map()->texture_data()->height);
    debug_texture_->InitData(
            (void*) height_map_path->heights.data(),
            material_box_->height_map()->texture_data()->width,
            material_box_->height_map()->texture_data()->height);

    debug_texture_->AddParameter(ifx::TextureParameter{
            GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE
    });
    debug_texture_->AddParameter(ifx::TextureParameter{
            GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE
    });
    debug_texture_->AddParameter(ifx::TextureParameter{
            GL_TEXTURE_MIN_FILTER, GL_LINEAR
    });
    debug_texture_->AddParameter(ifx::TextureParameter{
            GL_TEXTURE_MAG_FILTER, GL_LINEAR
    });

    auto texture = debug_texture_;
    object ->SetBeforeRender([texture](
            const Program* program){
        glActiveTexture(GL_TEXTURE1);
        texture->Bind();
        glUniform1i(glGetUniformLocation(program->getID(),
                                         "height_map"), 1);
    });
    object ->SetAfterRender([texture](const Program* program){
        texture->Unbind();
    });
}

}