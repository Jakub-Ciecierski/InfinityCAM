#ifndef PROJECT_CUTTER_LOADER_H
#define PROJECT_CUTTER_LOADER_H

#include <ifc/cutter/cutter.h>
#include <ifc/cutter/instruction.h>

#include <memory>
#include <string>
#include <vector>

namespace ifc {

class CutterLoader {
public:

    CutterLoader(std::string path);
    ~CutterLoader();

    std::shared_ptr<Cutter> Load();

private:
    CutterType GetType();
    float GetDiamater();
    std::vector<Instruction> GetInstructions();

    std::string path_;

};
}

#endif //PROJECT_CUTTER_LOADER_H
