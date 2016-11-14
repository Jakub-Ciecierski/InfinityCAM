#include <rendering/scene/scene.h>
#include <rendering/instanced_render_object.h>

#include <gui/imgui/imgui.h>
#include <rendering/scene/scene_gui.h>
#include <ifc/factory/cutter_factory.h>
#include "ifc/gui/cam_gui.h"

namespace ifc {

CAMGUI::CAMGUI(GLFWwindow* window, std::shared_ptr<ifx::Scene> scene,
               std::shared_ptr<ifx::RenderObject> plane,
               std::shared_ptr<CutterSimulation> simulation)
        : ifx::GUI(window){
    simulation_gui_.reset(new SimulationGUI(scene, plane, simulation));
    path_generation_gui_.reset(new PathGenerationGUI(
            scene, simulation));
    scene_window_gui_.reset(new ifx::SceneWindowGUI(scene));
}

CAMGUI::~CAMGUI(){}

void CAMGUI::Render() {
    NewFrame();

    simulation_gui_->Render();
    path_generation_gui_->Render();
    scene_window_gui_->Render();

    ImGui::Render();
}


}