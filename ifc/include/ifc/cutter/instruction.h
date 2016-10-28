#ifndef PROJECT_INSTRUCTION_H
#define PROJECT_INSTRUCTION_H

#include <string>
#include <math/math_ifx.h>

namespace ifc {

/**
 * NORMAL = G01
 * FAST   = G00
 */
enum class InstructionSpeedMode{
    NORMAL, FAST
};

class Instruction {
public:

    Instruction(std::string instruction_str);
    ~Instruction();

    int id() const {return id_;}
    InstructionSpeedMode speed_mode() const {return speed_mode_;};
    const glm::vec3& position() const {return position_;}

    std::string ToString();

private:
    void Parse(std::string instruction_str);
    int GetID(std::string instruction_str);
    InstructionSpeedMode GetInstructionSpeedMode(std::string instruction_str);
    float GetPosition(std::string instruction_str, char dim);

    int id_;
    InstructionSpeedMode speed_mode_;
    glm::vec3 position_;

    std::string raw_;
};
}

#endif //PROJECT_INSTRUCTION_H
