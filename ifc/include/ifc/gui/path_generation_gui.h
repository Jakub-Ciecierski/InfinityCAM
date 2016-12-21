#ifndef PROJECT_PATH_GENERATION_GUI_H
#define PROJECT_PATH_GENERATION_GUI_H

#include <memory>

namespace ifx{
class Scene;
}

namespace ifc {

struct CADModelLoaderResult;

class CutterSimulation;
class MaterialBox;
class PathGenerator;

class PathGenerationGUI {
public:

    PathGenerationGUI(std::shared_ptr<ifx::Scene> scene,
                      std::shared_ptr<CutterSimulation> simulation);
    ~PathGenerationGUI();

    void Render();

private:
    void RenderMainWindow();
    void RenderLoadModel();
    void RenderPathGeneration();

    void GenerateSignaturePath();

    std::shared_ptr<ifx::Scene> scene_;
    std::shared_ptr<CutterSimulation> simulation_;
    std::shared_ptr<PathGenerator> path_generator_;

    std::shared_ptr<CADModelLoaderResult> cad_model_loader_result_;
};
}

#endif //PROJECT_PATH_GENERATION_GUI_H
