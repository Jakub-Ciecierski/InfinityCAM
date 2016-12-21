#include "ifc/path_generation/paths/flat_around_hm_path.h"

#include <ifc/path_generation/height_map_paths.h>
#include <ifc/material/material_box.h>
#include <ifc/cutter/cutter_loader.h>

namespace ifc {

FlatAroundHMPath::FlatAroundHMPath(
        std::shared_ptr<CADModelLoaderResult> model_loader_result,
        std::shared_ptr <MaterialBox> material_box) :
        model_loader_result_(model_loader_result),
        material_box_(material_box),
        id_(0){}

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
    float current_height = start_height - radius;

    std::vector<std::vector<glm::vec3>> lines;

    for(int i = 0; i < n;){
        std::vector<glm::vec3> line_positions;
        int j = 0;
        bool go_back = false;
        for(j = m-1; j >= 0; j--){
            line_positions.push_back(glm::vec3(
                    GLToMillimeters(height_map_path->Position(i,j).x),
                    GLToMillimeters(height_map_path->Position(i,j).y),
                    current_height));

            if(ShouldGoBack(i, j, n, m,
                            look_ahead_radius_row,
                            look_ahead_radius_column,height_map_path)){
                break;
            }
        }
        lines.push_back(line_positions);

        // zic-zac
        j--;
        i+=skip_columns;
        if(i >= n)
            break;
    }

    std::vector<Instruction> instructions;

    const glm::vec2 save_position = glm::vec2(-MillimetersToGL(radius*4),0);
    instructions.push_back(
            CreateInstruction(id_++,
                              height_map_path->Position(skip_rows, 0)
                              + save_position,
                              save_height));
    instructions.push_back(
            CreateInstruction(id_++,
                              height_map_path->Position(skip_rows, 0)
                              + save_position,
                              current_height));
    int direction = 1;
    for(unsigned int i = 0; i < lines.size(); i++){
        auto &line = lines[i];
        if(direction == 1) {
            instructions.push_back(Instruction(id_++, line[0]));
            instructions.push_back(Instruction(id_++, line[line.size() - 1]));

            if (i != lines.size() - 1) {
                auto &next_line = lines[i + 1];
                instructions.push_back(
                        Instruction(id_++,next_line[next_line.size() - 1]));
            }
            direction = -1;
        }
        else{
            instructions.push_back(Instruction(id_++, line[line.size() - 1]));
            instructions.push_back(Instruction(id_++, line[0]));

            if (i != lines.size() - 1) {
                auto &next_line = lines[i + 1];
                instructions.push_back(Instruction(id_++, next_line[0]));
            }

            direction = 1;
        }
    }

    const glm::vec3& last_pos
            = instructions[instructions.size() - 1].position();

    instructions.push_back(
            CreateInstruction(id_++, MillimetersToGL(
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
    float current_height = start_height - radius;

    std::vector<std::vector<glm::vec3>> lines;

    for(int i = 0; i < n;){
        std::vector<glm::vec3> line_positions;
        int j = 0;
        bool go_back = false;
        for(j = 0; j < m; j++){
            line_positions.push_back(glm::vec3(
                    GLToMillimeters(height_map_path->Position(i,j).x),
                    GLToMillimeters(height_map_path->Position(i,j).y),
                    current_height));

            if(ShouldGoBack(i, j, n, m,
                            look_ahead_radius_row,
                            look_ahead_radius_column,height_map_path)){
                break;
            }
        }
        lines.push_back(line_positions);

        // zic-zac
        i+=skip_columns;
        if(i >= n)
            break;
    }

    std::vector<Instruction> instructions;

    const glm::vec2 save_position = glm::vec2(-MillimetersToGL(radius*4),0);
    instructions.push_back(
            CreateInstruction(id_++,
                              height_map_path->Position(skip_rows, 0)
                              + save_position,
                              save_height));
    instructions.push_back(
            CreateInstruction(id_++,
                              height_map_path->Position(skip_rows, 0)
                              + save_position,
                              current_height));
    int direction = 1;
    for(unsigned int i = 0; i < lines.size(); i++){
        auto &line = lines[i];
        if(direction == 1) {
            instructions.push_back(Instruction(id_++, line[0]));
            instructions.push_back(Instruction(id_++, line[line.size() - 1]));

            if (i != lines.size() - 1) {
                auto &next_line = lines[i + 1];
                instructions.push_back(
                        Instruction(id_++,next_line[next_line.size() - 1]));
            }
            direction = -1;
        }
        else{
            instructions.push_back(Instruction(id_++, line[line.size() - 1]));
            instructions.push_back(Instruction(id_++, line[0]));

            if (i != lines.size() - 1) {
                auto &next_line = lines[i + 1];
                instructions.push_back(Instruction(id_++, next_line[0]));
            }

            direction = 1;
        }
    }

    const glm::vec3& last_pos
            = instructions[instructions.size() - 1].position();

    instructions.push_back(
            CreateInstruction(id_++, MillimetersToGL(
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