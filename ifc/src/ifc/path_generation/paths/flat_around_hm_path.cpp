#include "ifc/path_generation/paths/flat_around_hm_path.h"

#include <ifc/path_generation/height_map_paths.h>
#include <ifc/material/material_box.h>
#include <ifc/cutter/cutter_loader.h>

namespace ifc {

FlatAroundHMPath::FlatAroundHMPath(
        std::shared_ptr<CADModelLoaderResult> model_loader_result,
        std::shared_ptr <MaterialBox> material_box) :
        model_loader_result_(model_loader_result),
        material_box_(material_box){}

FlatAroundHMPath::~FlatAroundHMPath(){

}

std::shared_ptr<Cutter> FlatAroundHMPath::Generate(
        std::shared_ptr<HeightMapPath> height_map){
    std::cout << "2) Generating FlatAroundHMPath" << std::endl;
    return CreatePath(height_map, material_box_);
}

std::shared_ptr<Cutter> FlatAroundHMPath::CreatePath(
        std::shared_ptr<HeightMapPath> height_map_path,
        std::shared_ptr<MaterialBox> material_box){
    int n = height_map_path->row_count;
    int m = height_map_path->column_count;

    const float save_height = material_box->dimensions().depth + 10.0f;
    const float epsilon = 0.5f;

    int look_ahead_radius_row
            = (radius_ + epsilon) / height_map_path->row_width;
    int look_ahead_radius_column
            = (radius_ + epsilon) / height_map_path->column_width;
    int skip_columns = (radius_-epsilon) / height_map_path->column_width;
    int skip_rows = (radius_-epsilon) / height_map_path->row_width;

    float start_height
            = GLToMillimeters(height_map_path->init_height) + radius_;
    auto instructions = CreatePathFirstHalf(height_map_path,
                                            save_height, start_height,
                                            radius_, n, m,
                                            skip_rows, skip_columns,
                                            look_ahead_radius_row,
                                            look_ahead_radius_column);

    auto instructions2 = CreatePathSecondHalf(height_map_path,
                                              save_height, start_height,
                                              radius_, n, m,
                                              skip_rows, skip_columns,
                                              look_ahead_radius_row,
                                              look_ahead_radius_column);
    instructions.insert(instructions.end(),
                        instructions2.begin(),
                        instructions2.end());

    return std::shared_ptr<Cutter>(new Cutter(CutterType::Flat,
                                              diameter_,
                                              instructions));
}

std::vector<Instruction> FlatAroundHMPath::CreatePathSecondHalf(
        std::shared_ptr<HeightMapPath> height_map_path,
        float save_height, float start_height, float radius,
        int n, int m,
        int skip_rows, int skip_columns,
        int look_ahead_radius_row, int look_ahead_radius_column){
    std::vector<Instruction> instructions;

    int id = 0;
    float current_height = start_height - radius;

    const glm::vec2 save_position = glm::vec2(MillimetersToGL(radius*4),0);
    instructions.push_back(
            CreateInstruction(id++,
                              height_map_path->Position(skip_rows, m-1)
                              + save_position,
                              save_height));
    instructions.push_back(
            CreateInstruction(id++,
                              height_map_path->Position(skip_rows, m-1)
                              + save_position,
                              current_height));
    /*
    instructions.push_back(
            CreateInstruction(id++, height_map_path->Position(0,m-1),
                              save_height));
*/
    bool go_back = false;
    for(int i = 0; i < n;){
        int j = m-1;
        go_back = false;
        for(j = m-1; j > 0; j--){
            // Keep moving forward
            instructions.push_back(
                    CreateInstruction(id++, height_map_path->Position(i,j),
                                      current_height));
            go_back = ShouldGoBack(i, j, n, m,
                                   look_ahead_radius_row,
                                   look_ahead_radius_column,height_map_path);

            if(go_back){
                instructions.push_back(
                        CreateInstruction(id++,
                                          height_map_path->Position(i, m-1),
                                          current_height));
                break;
            }
        }

        const glm::vec3& last_pos
                = instructions[instructions.size() - 1].position();
        // zic-zac
        j--;
        i+=skip_columns;
        if(i >= n) break;
        instructions.push_back(
                CreateInstruction(id++,
                                  height_map_path->Position(i,m-1),
                                  current_height));
    }

    const glm::vec3& last_pos
            = instructions[instructions.size() - 1].position();

    instructions.push_back(
            CreateInstruction(id++, MillimetersToGL(
                                      glm::vec2(last_pos.x,
                                                last_pos.y)),
                              save_height));
    return instructions;
}

std::vector<Instruction> FlatAroundHMPath::CreatePathFirstHalf(
        std::shared_ptr<HeightMapPath> height_map_path,
        float save_height, float start_height, float radius,
        int n, int m,
        int skip_rows, int skip_columns,
        int look_ahead_radius_row, int look_ahead_radius_column){
    std::vector<Instruction> instructions;

    int id = 0;
    float current_height = start_height - radius;
    const glm::vec2 save_position = glm::vec2(-MillimetersToGL(radius*4),0);
    instructions.push_back(
            CreateInstruction(id++,
                              height_map_path->Position(skip_rows, 0)
                              + save_position,
                              save_height));
    instructions.push_back(
            CreateInstruction(id++,
                              height_map_path->Position(skip_rows, 0)
                              + save_position,
                              current_height));
    bool go_back = false;
    for(int i = 0; i < n;){
        int j = 0;
        go_back = false;
        for(j = 0; j < m; j++){
            // Keep moving forward
            instructions.push_back(
                    CreateInstruction(id++, height_map_path->Position(i,j),
                                      current_height));
            go_back = ShouldGoBack(i, j, n, m,
                                   look_ahead_radius_row,
                                   look_ahead_radius_column,height_map_path);

            if(go_back){
                instructions.push_back(
                        CreateInstruction(id++, height_map_path->Position(i,0),
                                          current_height));
                break;
            }
        }

        const glm::vec3& last_pos
                = instructions[instructions.size() - 1].position();
        // zic-zac
        j--;
        i+=skip_columns;
        if(i >= n) break;
        instructions.push_back(
                CreateInstruction(id++,
                                  height_map_path->Position(i,0),
                                  current_height));
    }

    const glm::vec3& last_pos
            = instructions[instructions.size() - 1].position();

    instructions.push_back(
            CreateInstruction(id++, MillimetersToGL(
                                      glm::vec2(last_pos.x,
                                                last_pos.y)),
                              save_height));
    return instructions;
}

bool FlatAroundHMPath::ShouldGoBack(int i, int j, int n, int m,
                                int look_ahead_radius_row,
                                int look_ahead_radius_column,
                                std::shared_ptr<HeightMapPath> height_map_path){
    // Go back if encoutered obstacle
    for(int ii = -look_ahead_radius_row; ii < look_ahead_radius_row; ii++){
        if(ii + i < 0 || ii + i >= n)
            continue;
        for(int jj = -look_ahead_radius_column; jj < look_ahead_radius_column; jj++){
            if(jj + j < 0 || jj + j >= m)
                continue;
            if(height_map_path->GetHeight(ii+i,jj+j) >
               height_map_path->init_height){
                return true;
            }
        }
    }
    return false;
}

Instruction FlatAroundHMPath::CreateInstruction(int id,
                                            const glm::vec2& v,
                                            float height){
    glm::vec3 pos2 = glm::vec3(GLToMillimeters(v.x),
                               GLToMillimeters(v.y),
                               height);
    return Instruction(id, pos2);
}

}