#ifndef PROJECT_CAM_GUI_H
#define PROJECT_CAM_GUI_H

#include <gui/gui.h>
#include <ifc/material/material_box.h>
#include <ifc/cutter/cutter_simulation.h>
#include <ifc/gui/simulation_gui.h>
#include <ifc/gui/path_generation_gui.h>

#include <memory>

namespace ifx{
class Scene;
class SceneWindowGUI;
class RenderObject;
}

namespace ifc {

class CAMGUI : public ifx::GUI{
public:
    CAMGUI(GLFWwindow* window,
           std::shared_ptr<ifx::Scene> scene,
           std::shared_ptr<ifx::RenderObject> plane,
           std::shared_ptr<CutterSimulation> simulation);
    ~CAMGUI();

    virtual void Render() override;

private:
    std::unique_ptr<SimulationGUI> simulation_gui_;
    std::unique_ptr<PathGenerationGUI> path_generation_gui_;
    std::unique_ptr<ifx::SceneWindowGUI> scene_window_gui_;
};

}

#endif //PROJECT_CAM_GUI_H
