#include "ifc/cutter/cutter.h"

#include <object/render_object.h>
#include <ifc/measures.h>
#include <fstream>

namespace ifc {

Cutter::Cutter(CutterType type, float diameter,
               std::vector<Instruction> &instructions) :
        type_(type),
        diameter_(diameter),
        radius_(diameter / 2.0f),
        instructions_(instructions),
        current_intruction_(-1),
        start_position_mm_(glm::vec3(0, 0, 150)),
        last_status_(CutterStatus::NONE) {
    ChangeInstruction();
}

Cutter::~Cutter() { }

float Cutter::GetProgress() {
    return (float) (current_intruction_) / (float) (instructions_.size() - 1);
}

void Cutter::Update(MaterialBox *material_box, float t_delta) {
    if (Finished()) {
        return;
    }
    CutterStatus error = CheckErrors(material_box);
    last_status_ = error;
    if (error != CutterStatus::NONE) return;

    MaybeChangeInstruction();
    UpdateT(t_delta);
    ComputeCurrentPosition();
    Move();
    Cut(material_box->height_map());
}

bool Cutter::Finished() {
    if (instructions_.size() < 1) return true;

    int size = instructions_.size();
    return (current_intruction_ + 1 >= size);
}

bool Cutter::SaveToFile(std::string filename) {
    std::ofstream file;
    filename += ".";
    filename += GetFileExtention(type_, diameter_);

    file.open(filename);
    if(!file.is_open())
        return false;
    for(auto& instruction : instructions_){
        auto pos = instruction.position();
        pos.x = -pos.x;
        Instruction fixed_coord_instruction(instruction.id(), pos,
                                            instruction.speed_mode());
        file << fixed_coord_instruction.raw_instruction() << std::endl;
    }

     file.close();

    return true;
}

std::string Cutter::GetFileExtention(CutterType type,
                                     float diameter) {
    std::string extension = "";
    if (type == CutterType::Sphere)
        extension += "k";
    else if (type == CutterType::Flat)
        extension += "f";
    else
        extension += "unknown";

    extension += std::to_string((int)diameter);

    return extension;
}

CutterStatus Cutter::CheckErrors(MaterialBox* material_box){
    if(current_position_.x >= -material_box->dimensions().x / 2.0f
       && current_position_.x <= material_box->dimensions().x / 2.0f
       && current_position_.y >= -material_box->dimensions().z / 2.0f
       && current_position_.y <= material_box->dimensions().z / 2.0f){
        if(current_position_.z <
                    material_box->dimensions().depth -
                material_box->dimensions().max_depth - 1.0f){
            std::cout << "Error MAX_DEPTH" << std::endl;
            return CutterStatus::MAX_DEPTH;
        }
    }
    return CutterStatus::NONE;
}

void Cutter::MaybeChangeInstruction(){
    if(current_vector_equation_.t < current_vector_equation_.t_max)
        return;
    ChangeInstruction();
}

void Cutter::ChangeInstruction(){
    current_intruction_++;
    if(Finished())
        return;

    current_vector_equation_.t = current_vector_equation_.t_min;
    glm::vec3 pos1 = instructions_[current_intruction_].position();
    glm::vec3 pos2 = instructions_[current_intruction_+1].position();

    current_vector_equation_.pos = pos1;
    current_vector_equation_.vec = pos2 - pos1;
    current_vector_equation_.distance = ifx::EuclideanDistance(pos1, pos2);
}

void Cutter::UpdateT(float t_delta){
    current_vector_equation_.t
            += t_delta * (1.0f / current_vector_equation_.distance);

    if(current_vector_equation_.t > current_vector_equation_.t_max)
        current_vector_equation_.t = current_vector_equation_.t_max;
}

void Cutter::ComputeCurrentPosition(){
    current_position_ = current_vector_equation_.Compute();
}

void Cutter::Move(){
    glm::vec3 vec_gl = MillimetersToGL(current_position_);

    // -x so that cutter moves properly
    glm::vec3 vec_swap_zy = glm::vec3(vec_gl.x,
                                      vec_gl.z + MillimetersToGL(radius_),
                                      vec_gl.y);
    // + MillimetersToGL(radius_)
    render_object_->moveTo(vec_swap_zy);
}

void Cutter::Cut(HeightMap* height_map){
    if(type_ == CutterType::Sphere)
        CutSphere(height_map);
    if(type_ == CutterType::Flat)
        CutFlat(height_map);
}

void Cutter::CutSphere(HeightMap* height_map) {
    int n = height_map->texture_data()->width;
    int m = height_map->texture_data()->height;
    glm::vec3 cutter_center = glm::vec3(0, 0, radius_) + current_position_;
    glm::vec2 test_cutter_center = glm::vec2(
            current_position_.x, current_position_.y);

    const float epsilon = 0.5f * radius_;
    int look_ahead_radius_row
            = (radius_ + epsilon) * height_map->row_width();
    int look_ahead_radius_column
            = (radius_+ epsilon) * height_map->column_width();

    glm::vec2 corresponding_index = height_map->GetIndices(
            MillimetersToGL(glm::vec2(cutter_center.x, cutter_center.y)));
    int start_i = corresponding_index.x;
    int start_j = corresponding_index.y;
    for(int ii = -look_ahead_radius_row; ii < look_ahead_radius_row; ii++){
        if(ii + start_i < 0 || ii + start_i >= n)
            continue;
        for(int jj = -look_ahead_radius_column;
            jj < look_ahead_radius_column; jj++){
            if(jj + start_j < 0 || jj + start_j >= m)
                continue;
            int i = ii + start_i;
            int j = jj + start_j;
            glm::vec2 box_pos2_mm
                    = GLToMillimeters(height_map->GetPosition(i, j));
            glm::vec3 box_top_mm = glm::vec3(box_pos2_mm.x,
                                             box_pos2_mm.y,
                                             height_map->GetHeight(i, j));
            /*
            float distance = ifx::EuclideanDistance(cutter_center,
                                                    box_top_mm);*/
            float distance = ifx::EuclideanDistance(test_cutter_center,
                                                    box_pos2_mm);
            float delta = radius_ - distance;
            if (delta > 0) {
                ifx::LineIntersection line;
                ifx::SphereIntersection sphere;
                line.origin = glm::vec3(box_top_mm.x, box_top_mm.y, 0);
                line.direction = glm::vec3(0.0f, 0.0f, 1.0f);
                sphere.radius = radius_;
                sphere.center = cutter_center;

                ifx::LineSphereIntersection intersection
                        = ifx::Intersection(line, sphere);
                float d = intersection.d1 <= intersection.d2
                          ? intersection.d1 : intersection.d2;
                if (d == intersection.NO_SOLUTION)
                    continue;

                height_map->SetHeight(i, j, d);

            }
        }
    }
}

void Cutter::CutFlat(HeightMap* height_map){
    int n = height_map->texture_data()->width;
    int m = height_map->texture_data()->height;
    glm::vec3 cutter_center = glm::vec3(0, 0, radius_) + current_position_;
    glm::vec2 test_cutter_center = glm::vec2(
            current_position_.x, current_position_.y);

    const float epsilon = 0.5f * radius_;
    int look_ahead_radius_row
            = (radius_ + epsilon) * height_map->row_width();
    int look_ahead_radius_column
            = (radius_+ epsilon) * height_map->column_width();

    glm::vec2 corresponding_index = height_map->GetIndices(
            MillimetersToGL(glm::vec2(cutter_center.x, cutter_center.y)));
    int start_i = corresponding_index.x;
    int start_j = corresponding_index.y;
    for(int ii = -look_ahead_radius_row; ii < look_ahead_radius_row; ii++){
        if(ii + start_i < 0 || ii + start_i >= n)
            continue;
        for(int jj = -look_ahead_radius_column;
            jj < look_ahead_radius_column; jj++){
            if(jj + start_j < 0 || jj + start_j >= m)
                continue;
            int i = ii + start_i;
            int j = jj + start_j;
            glm::vec2 box_pos2_mm
                    = GLToMillimeters(height_map->GetPosition(i, j));
            glm::vec3 box_top_mm = glm::vec3(box_pos2_mm.x,
                                             box_pos2_mm.y,
                                             height_map->GetHeight(i, j));
            /*
            float distance = ifx::EuclideanDistance(cutter_center,
                                                    box_top_mm);*/
            float distance = ifx::EuclideanDistance(test_cutter_center,
                                                    box_pos2_mm);
            float delta = radius_ - distance;
            if (delta > 0) {
                ifx::LineIntersection line;
                ifx::SphereIntersection sphere;
                line.origin = glm::vec3(box_top_mm.x, box_top_mm.y, 0);
                line.direction = glm::vec3(0.0f, 0.0f, 1.0f);
                sphere.radius = radius_;
                sphere.center = cutter_center;

                ifx::LineSphereIntersection intersection
                        = ifx::Intersection(line, sphere);
                float d = intersection.d1 <= intersection.d2
                          ? intersection.d1 : intersection.d2;
                if (d == intersection.NO_SOLUTION)
                    continue;

                height_map->SetHeight(i, j, current_position_.z);

            }
        }
    }
}

}
