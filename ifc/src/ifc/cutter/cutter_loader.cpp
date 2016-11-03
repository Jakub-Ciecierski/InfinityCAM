#include "ifc/cutter/cutter_loader.h"

#include <math/math_ifx.h>
#include <fstream>
#include <vector>
#include <ifc/cutter/instruction.h>
#include <iostream>

namespace ifc {

CutterLoader::CutterLoader(std::string path) :
    path_(path){

}

CutterLoader::~CutterLoader(){}

std::shared_ptr<Cutter> CutterLoader::Load(){
    CutterType type = GetType();
    if(type == CutterType::UNKNOWN)
        return std::shared_ptr<Cutter>();
    float diamater = GetDiamater();
    std::vector<Instruction> instructions = GetInstructions();
    auto cutter = std::shared_ptr<Cutter>(new Cutter(type,
                                                     diamater,
                                                     instructions));
    return cutter;
}

CutterType CutterLoader::GetType(){
    std::ifstream file(path_);
    if(!file.is_open())
        return CutterType::UNKNOWN;

    std::vector<std::string> split_path = ifx::SplitString(path_, ".");
    std::string format = split_path[split_path.size()-1];

    if(format[0] == 'k')
        return CutterType::Sphere;
    else if(format[0] == 'f')
        return CutterType::Flat;
    else
        return CutterType::UNKNOWN;
}

float CutterLoader::GetDiamater(){
    std::vector<std::string> split_path = ifx::SplitString(path_, ".");
    std::string format = split_path[split_path.size()-1];

    std::string diamater_str = "";
    for(int i = 1; i < format.size(); i++){
        diamater_str += format[i];
    }

    return std::stof(diamater_str);
}

std::vector<Instruction> CutterLoader::GetInstructions(){
    std::vector<Instruction> instructions;

    std::ifstream file(path_);
    if(!file.is_open()){
        std::cout << "File not found: " << path_<< std::endl;
    }
    std::string line;
    while(std::getline(file, line)){
        instructions.push_back(Instruction(line));
    }
    return instructions;
}

}
