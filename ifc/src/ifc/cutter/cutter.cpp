#include "ifc/cutter/cutter.h"

#include <object/render_object.h>
#include <ifc/measures.h>

namespace ifc{

Cutter::Cutter(CutterType type, float diameter,
        std::vector<Instruction>& instructions) :
    type_(type),
    diameter_(diameter),
    radius_(diameter / 2.0f),
    instructions_(instructions),
    current_intruction_(-1),
    start_position_mm_(glm::vec3(0, 0, 150)),
    last_status_(CutterStatus::NONE){
    ChangeInstruction();
}

Cutter::~Cutter(){}

float Cutter::GetProgress(){
    return (float)(current_intruction_) / (float)instructions_.size();
}

void Cutter::Update(MaterialBox* material_box, float t_delta){
    if(Finished()){
        return;
    }
    CutterStatus error = CheckErrors(material_box);
    last_status_ = error;
    if(error != CutterStatus::NONE) return;

    MaybeChangeInstruction();
    UpdateT(t_delta);
    ComputeCurrentPosition();
    Move();
    Cut(material_box->height_map());
}
bool Cutter::Finished(){
    if(instructions_.size() < 1) return true;

    int size = instructions_.size();
    return (current_intruction_ + 1 >= size);
}

CutterStatus Cutter::CheckErrors(MaterialBox* material_box){
    if(current_position_.x >= -material_box->dimensions().x / 2.0f
       && current_position_.x <= material_box->dimensions().x / 2.0f
       && current_position_.y >= -material_box->dimensions().z / 2.0f
       && current_position_.y <= material_box->dimensions().z / 2.0f){
        if(current_position_.z < material_box->dimensions().max_depth){
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

glm::vec3 Cutter::ComputeCurrentPosition(){
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

void Cutter::CutSphere(HeightMap* height_map){
    std::vector<glm::vec2>& positions = height_map->positions();

    glm::vec3 cutter_center = glm::vec3(0, 0, radius_) + current_position_;

    for(unsigned int i = 0; i < positions.size(); i++){
        glm::vec2 box_pos2_mm = GLToMillimeters(positions[i]);
        glm::vec3 box_top_mm = glm::vec3(box_pos2_mm.x,
                                         box_pos2_mm.y,
                                         height_map->GetHeight(i));
        float distance = ifx::EuclideanDistance(cutter_center, box_top_mm);
        float delta = radius_ - distance;
        if(delta > 0){
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
            if(d == intersection.NO_SOLUTION)continue;

            height_map->SetHeight(i, d);
        }
    }
}
void Cutter::CutFlat(HeightMap* height_map){
    std::vector<glm::vec2>& positions = height_map->positions();
    std::vector<float>& heights = height_map->heights();

    glm::vec3 cutter_center = glm::vec3(0, 0, radius_) + current_position_;

    for(unsigned int i = 0; i < positions.size(); i++) {
        glm::vec2 box_pos2_mm = GLToMillimeters(positions[i]);
        glm::vec3 box_top_mm = glm::vec3(box_pos2_mm.x,
                                         box_pos2_mm.y,
                                         heights[i]);
        if(box_top_mm.x >= cutter_center.x - radius_ &&
                box_top_mm.x <= cutter_center.x + radius_ &&
                box_top_mm.y >= cutter_center.y - radius_ &&
                box_top_mm.y <= cutter_center.y + radius_){

            if(height_map->SetHeight(i, current_position_.z)){
                if(current_vector_equation_.IsGoingDown()){
                    last_status_= CutterStatus::FLAT_DIRECT_DOWN;
                    return;
                }
            }
        }
    }
}

}
