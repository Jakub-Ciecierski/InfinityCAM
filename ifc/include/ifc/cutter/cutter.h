#ifndef PROJECT_CUTTER_H
#define PROJECT_CUTTER_H

#include <ifc/cutter/instruction.h>
#include <ifc/measures.h>
#include <ifc/material/height_map.h>
#include <ifc/material/material_box.h>

#include <math/math_ifx.h>
#include <object/render_object.h>

#include <memory>
#include <vector>

namespace ifc {

enum class CutterType{
    Sphere, Flat, UNKNOWN
};

enum class CutterStatus{
    NONE, FINISHED,MAX_DEPTH, FLAT_DIRECT_DOWN
};

/**
 * When t reaches >= t_max, next intruction should be setup.
 */
struct InstructionVectorEquation {
    glm::vec3 pos;
    glm::vec3 vec;
    float distance;

    const float t_min = 0.0f;
    const float t_max = 1.0f;
    float t = t_min; // t in [t_min, t_max]

    glm::vec3 Compute(){
        return pos + t*vec;
    }

    bool IsGoingDown(){
        return ifx::dot(glm::normalize(vec), glm::vec3{0,0,1.0f}) == -1.0f;
    }
};

/**
 * Cutter positions are in millimeters.
 */
class Cutter {
public:

    Cutter(CutterType type, float diameter,
           std::vector<Instruction>& instructions);
    ~Cutter();

    CutterType type(){return type_;}
    float diameter(){return diameter_;}
    const std::vector<Instruction>& instructions(){return instructions_;}
    const glm::vec3& current_position(){return current_position_;}
    std::shared_ptr<RenderObject> render_object(){return render_object_;}
    void render_object(std::shared_ptr<RenderObject> render_obj){
        render_object_ = render_obj;
        current_position_ = start_position_mm_;
        Move();
    }

    int current_instruction(){return current_intruction_;}
    CutterStatus last_status(){return last_status_;}

    float GetProgress();

    void Update(MaterialBox* material_box, float t_delta);
    bool Finished();

private:
    CutterStatus CheckErrors(MaterialBox* material_box);

    void MaybeChangeInstruction();
    void ChangeInstruction();

    void UpdateT(float t_delta);
    glm::vec3 ComputeCurrentPosition();
    void Move();
    void Cut(HeightMap* height_map);
    void CutSphere(HeightMap* height_map);
    void CutFlat(HeightMap* height_map);

    CutterType type_;
    // in mm
    float diameter_;
    float radius_;
    std::vector<Instruction> instructions_;
    std::shared_ptr<RenderObject> render_object_;

    int current_intruction_;
    InstructionVectorEquation current_vector_equation_;
    // position of the edge of cutter.
    glm::vec3 current_position_;

    glm::vec3 start_position_mm_;

    CutterStatus last_status_;
};
}

#endif //PROJECT_CUTTER_H
