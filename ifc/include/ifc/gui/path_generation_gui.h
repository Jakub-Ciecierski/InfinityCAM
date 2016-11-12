#ifndef PROJECT_PATH_GENERATION_GUI_H
#define PROJECT_PATH_GENERATION_GUI_H

#include <memory>

namespace ifx{
class Scene;
}

namespace ifc {

class PathGenerationGUI {
public:

    PathGenerationGUI(std::shared_ptr<ifx::Scene> scene);
    ~PathGenerationGUI();

    void Render();

private:
    void RenderMainWindow();
    void RenderLoadModel();

    std::shared_ptr<ifx::Scene> scene_;
};
}

#endif //PROJECT_PATH_GENERATION_GUI_H
