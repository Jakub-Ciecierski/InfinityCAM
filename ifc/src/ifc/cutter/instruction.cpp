#include "ifc/cutter/instruction.h"

#include <iostream>
#include <sstream>
#include <iomanip>

namespace {
float USE_PREVIOUS_POSITION = -99999.9;
const char X = 'X';
const char Y = 'Y';
const char Z = 'Z';
const char N = 'N';
const char G = 'G';
const std::string G00 = "G00";
const std::string G01 = "G01";
}

namespace ifc {

Instruction::Instruction(std::string instruction_str) :
        raw_(instruction_str){
    Parse(instruction_str);
}

Instruction::Instruction(int id,
                         const glm::vec3& position,
                         InstructionSpeedMode speed_mode) :
        id_(id),
        position_(position),
        speed_mode_(speed_mode){
    raw_ = ConstructRaw(id_, position_, speed_mode_);
}

Instruction::~Instruction(){}


void Instruction::Parse(std::string instruction_str){
    id_ = GetID(instruction_str);
    speed_mode_ = GetInstructionSpeedMode(instruction_str);

    position_.x = GetPosition(instruction_str, X);
    position_.y = GetPosition(instruction_str, Y);
    position_.z = GetPosition(instruction_str, Z);

    // Fix coordinate system
    position_.x = -position_.x;
}

int Instruction::GetID(std::string instruction_str){
    std::string s = "";
    s += N;
    std::vector<std::string> split = ifx::SplitString(instruction_str, s);
    int i = 0;
    std::string id_str = "";
    if(split.size() != 0){
        while(split[1][i] != 'G'){
            id_str += split[1][i];
            i++;
        }
    }
    return std::stoi(id_str);
}

InstructionSpeedMode Instruction::GetInstructionSpeedMode(
        std::string instruction_str){
    std::string s = "";
    s += G;
    std::vector<std::string> split = ifx::SplitString(instruction_str, s);
    int i = 0;
    std::string g_str = "G";
    if(split.size() != 0){
        while(split[1][i] != X && split[1][i] != Y && split[1][i] != Z){
            g_str += split[1][i];
            i++;
        }
    }

    if(g_str == G00)
        return InstructionSpeedMode::FAST;
    else if(g_str == G01)
        return InstructionSpeedMode::NORMAL;
    else
        throw std::invalid_argument("GetInstructionSpeedMode() Error");
}

float Instruction::GetPosition(std::string instruction_str, char dim){
    std::string s = "";
    s += dim;
    std::vector<std::string> x_split = ifx::SplitString(instruction_str, s);
    if(x_split.size() != 0){
        std::string x_str;
        int j = 0;
        while(x_split[1][j] != '.'){
            x_str += x_split[1][j];
            j++;
        }
        x_str += x_split[1][j++];
        x_str += x_split[1][j++];
        x_str += x_split[1][j++];
        x_str += x_split[1][j++];
        return std::stof(x_str);
    }else{
        return USE_PREVIOUS_POSITION;
    }
}

std::string Instruction::ConstructRaw(int id,
                                      const glm::vec3& position,
                                      InstructionSpeedMode speed_mode){
    std::string raw_instruction = "";

    raw_instruction = AppendID(raw_instruction, id);
    raw_instruction = AppendSpeedMode(raw_instruction, speed_mode);
    raw_instruction = AppendPosition(raw_instruction, position);

    return raw_instruction;
}

std::string Instruction::AppendID(std::string current_raw, int id){
    std::string raw = current_raw;
    raw += N;
    raw += std::to_string(id);
    return raw;
}

std::string Instruction::AppendSpeedMode(std::string current_raw,
                                         InstructionSpeedMode speed_mode){
    std::string raw = current_raw;
    if(speed_mode == InstructionSpeedMode::NORMAL)
        raw += G01;
    else if (speed_mode == InstructionSpeedMode::FAST)
        raw += G00;

    return raw;
}

std::string Instruction::AppendPosition(std::string current_raw,
                                        const glm::vec3& position){
    std::string raw = current_raw;

    raw += X;
    raw += ToStringWithPrecision(position.x);

    raw += Y;
    raw += ToStringWithPrecision(position.y);

    raw += Z;
    raw += ToStringWithPrecision(position.z);

    return raw;
}

std::string Instruction::ToStringWithPrecision(float d){
    std::stringstream stream;
    const int presition = 3;
    stream << std::fixed << std::setprecision(presition) << d;

    return stream.str();
}

std::string  Instruction::ToString(){
    std::stringstream ss;
    ss << "ID: " << id() << std::endl;
    const glm::vec3& pos = position();
    ss << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
    InstructionSpeedMode speed_modee = speed_mode();
    if(speed_modee == InstructionSpeedMode::FAST)
        ss << "Speed Mode: " << "Fast" << std::endl;
    else
        ss << "Speed Mode: " << "Normal" << std::endl;
    return ss.str();
}

}
