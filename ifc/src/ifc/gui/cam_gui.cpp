#include <rendering/scene/scene.h>
#include <rendering/instanced_render_object.h>
#include <rendering/renderer.h>

#include <gui/imgui/imgui.h>

#include <engine_gui/engine_gui.h>
#include <engine_gui/factory/engine_gui_factory.h>

#include <ifc/factory/cutter_factory.h>
#include <ifc/gui/cam_gui.h>

namespace ifc {

CAMGUI::CAMGUI(GLFWwindow* window,
               std::shared_ptr<ifx::Renderer> renderer,
               std::shared_ptr<ifx::Scene> scene,
               std::shared_ptr<ifx::RenderObject> plane,
               std::shared_ptr<CutterSimulation> simulation)
        : ifx::GUI(window){
    simulation_gui_.reset(new SimulationGUI(scene, plane, simulation));
    path_generation_gui_.reset(new PathGenerationGUI(
            scene, simulation));
    engine_gui_ = ifx::EngineGUIFactory().CreateEngineGUI(renderer);
}

CAMGUI::~CAMGUI(){}

void CAMGUI::Render() {
    NewFrame();

    simulation_gui_->Render();
    path_generation_gui_->Render();
    engine_gui_->Render();

    ImGui::Render();
}


}