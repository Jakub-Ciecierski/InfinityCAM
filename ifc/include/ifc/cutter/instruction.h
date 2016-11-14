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
    Instruction(int id,
                const glm::vec3& position,
                InstructionSpeedMode speed_mode = InstructionSpeedMode::NORMAL);
    ~Instruction();

    int id() const {return id_;}
    InstructionSpeedMode speed_mode() const {return speed_mode_;};
    const glm::vec3& position() const {return position_;}
    std::string raw_instruction(){return raw_;}

    std::string ToString();

private:
    /**
     * Parses raw string to construct instruction data.
     */
    void Parse(std::string instruction_str);
    int GetID(std::string instruction_str);
    InstructionSpeedMode GetInstructionSpeedMode(std::string instruction_str);
    float GetPosition(std::string instruction_str, char dim);

    /**
     * Constructs raw string from input data.
     */
    std::string ConstructRaw(int id,
                             const glm::vec3& position,
                             InstructionSpeedMode speed_mode);
    std::string AppendID(std::string current_raw, int id);
    std::string AppendSpeedMode(std::string current_raw,
                                InstructionSpeedMode speed_mode);
    std::string AppendPosition(std::string current_raw,
                               const glm::vec3& position);
    std::string ToStringWithPrecision(float d);

    int id_;
    glm::vec3 position_;
    InstructionSpeedMode speed_mode_;

    std::string raw_;
};
}

#endif //PROJECT_INSTRUCTION_H
