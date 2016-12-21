#include <rendering/scene/scene.h>
#include <rendering/instanced_render_object.h>

#include <gui/imgui/imgui.h>
#include <ifc/factory/cutter_factory.h>

#include "ifc/gui/simulation_gui.h"

namespace ifc {

SimulationGUI::SimulationGUI(std::shared_ptr<ifx::Scene> scene,
               std::shared_ptr<ifx::RenderObject> plane,
               std::shared_ptr<CutterSimulation> simulation) :
          scene_(scene),
          plane_(plane),
          simulation_(simulation){
    SetDefaultParameters();

    // TODO
    ResetSimulation();
}

SimulationGUI::~SimulationGUI(){}

void SimulationGUI::Render() {
    RenderMainWindow();
    RenderDebugWindow();
}

void SimulationGUI::SetDefaultParameters(){
    material_box_create_params_.dimensions.x = 150;
    material_box_create_params_.dimensions.z = 150;
    material_box_create_params_.dimensions.depth = 50;
    material_box_create_params_.dimensions.max_depth = 30;

    material_box_create_params_.precision.x = 500;
    material_box_create_params_.precision.z = 500;
}

void SimulationGUI::RenderMainWindow(){
    ImGui::SetNextWindowSize(ImVec2(350,600));
    ImGui::Begin("CAM");

    ImGui::PushItemWidth(150);
    if(ImGui::CollapsingHeader("Simulation"))
        RenderSimulationInfoSection();
    if(ImGui::CollapsingHeader("Cutter"))
        RenderCutterSection();
    if(ImGui::CollapsingHeader("Material Box"))
        RenderMaterialBoxSection();
    ImGui::End();
}

void SimulationGUI::RenderDebugWindow(){
    ImGui::SetNextWindowSize(ImVec2(350, 350));
    ImGui::Begin("Debug");
    if(ImGui::CollapsingHeader("Debug"))
        RenderDebugSection();
    ImGui::End();
}

void SimulationGUI::RenderMenuBar(){
    if (ImGui::BeginMenuBar()){
        if (ImGui::BeginMenu("Menu")){
            bool l;
            ImGui::MenuItem("Main menu bar", NULL, &l);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void SimulationGUI::RenderSimulationInfoSection(){
    ImGui::BulletText("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::BulletText("Time: %.2f [s]", simulation_->total_time_s());

    if(!simulation_->CanUpdate())
        RenderSimulationRequirements();
    else
        RenderSimulationControl();
    RenderSimulationError();
}

void SimulationGUI::RenderSimulationRequirements(){
    ImGui::TextColored(ImVec4(1.0f,0.3f,0.3f,1.0f), "Please create:");
    if(!simulation_->material_box())
        ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "Material");
    if(!simulation_->cutter())
        ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "Cutter");
}

void SimulationGUI::RenderSimulationControl(){
    std::string play_str = simulation_->IsRunning() ? "Pause" : "Play";
    if(ImGui::Button(play_str.c_str()))
        simulation_->SetRunning(!simulation_->IsRunning());
    ImGui::SameLine();
    if(ImGui::Button(">>"))
        simulation_->UpdateAllAtOnce();
    ImGui::SameLine();
    ImGui::SliderFloat("Time delta [s]",
                       simulation_->time_delta_ptr(),
                       0.0001f, 0.1f, "%.4f");
    ImGui::SliderFloat("Line delta",
                       simulation_->line_delta_ptr(),
                       0.0001f, 1.0f, "%.4f");

    if(simulation_->cutter()){
        ImGui::ProgressBar(simulation_->cutter()->GetProgress(),
                           ImVec2(0.0f,0.0f));
    }
}

void SimulationGUI::RenderSimulationError(){
    if(simulation_->cutter()){
        CutterStatus status = simulation_->cutter()->last_status();
        if(status == CutterStatus::MAX_DEPTH ||
           status == CutterStatus::FLAT_DIRECT_DOWN){
            std::string error_str
                    = status == CutterStatus::MAX_DEPTH ?
                      "Error: Max Depth reached" :
                      "Error: Flat Cutter went directly down";
            ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f),
                               error_str.c_str(), "");
        }
    }
}

void SimulationGUI::RenderCutterSection(){
    RenderLoadCutter();
    RenderShowTrajectoryCutter();
}

void SimulationGUI::RenderLoadCutter(){
    int size = 1024;
    //"jakub/programming/InfinityCAM/ifc/res/paths/t1.k16";
    static char filepath[1024] = "jc_t4.k8";

    if(ImGui::TreeNode("Cutter")){
        if (ImGui::Button("Load Cutter")) {
            simulation_->SetCutter(CutterFactory().CreateCutter(
                    std::string(filepath)
            ));
        }
        ImGui::SameLine();
        ImGui::InputText("filepath", filepath, size);
        ImGui::TreePop();
    }
    if(simulation_->cutter()){
        CutterType type = simulation_->cutter()->type();
        std::string type_str = "Type: Flat";
        if(type == CutterType::Sphere)
            type_str = "Type: Sphere";
        ImGui::BulletText(type_str.c_str(), "");
        ImGui::BulletText("Diamater: %.1f [mm]",
                          simulation_->cutter()->diameter());
    }
}

void SimulationGUI::RenderShowTrajectoryCutter(){
    static bool v = simulation_->show_trajectory();
    ImGui::Checkbox("Show Trajectory", &v);
    simulation_->show_trajectory(v);
}

void SimulationGUI::RenderMaterialBoxSection(){
    std::string reset_button_text = "Create";
    if(simulation_->material_box())
        reset_button_text = "Reset";
    if (ImGui::Button(reset_button_text.c_str())) {
        ResetSimulation();
    }

    if(ImGui::TreeNode("Dimensions [mm]")){
        RenderMaterialBoxDimensions();
        ImGui::TreePop();
    }
    if(ImGui::TreeNode("Precision (density of material box)")){
        ImGui::TreePop();
        RenderMaterialBoxPrecision();
    }
}
void SimulationGUI::RenderMaterialBoxDimensions(){
    ImGui::SliderFloat("x",
                       &material_box_create_params_.dimensions.x,
                       1.0f, 2048.0f);
    ImGui::SliderFloat("z",
                       &material_box_create_params_.dimensions.z,
                       1.0f, 2048.0f);
    ImGui::SliderFloat("depth (y)",
                       &material_box_create_params_.dimensions.depth,
                       1.0f, 2048.0f);
    ImGui::SliderFloat("max depth (Cutter can not go deeper)",
                       &material_box_create_params_.dimensions.max_depth,
                       1.0f, 2048.0f);
}

void SimulationGUI::RenderMaterialBoxPrecision(){
    ImGui::SliderInt("x and z",
                     &material_box_create_params_.precision.x,
                     1, 2048);
    material_box_create_params_.precision.z
            = material_box_create_params_.precision.x;
    /*
ImGui::SliderInt("z",
               &material_box_create_params_.precision.z,
               1, 2048);*/
}

void SimulationGUI::ResetSimulation(){
    simulation_->Pause();
    simulation_->SetCutter(std::shared_ptr<Cutter>());
    simulation_->SetMaterialBox(
            std::shared_ptr<MaterialBox>(
                    new MaterialBox(material_box_create_params_)));
    plane_->moveTo(glm::vec3(
            plane_->getPosition().x,
            MillimetersToGL(
                    material_box_create_params_.dimensions.depth
                    - material_box_create_params_.dimensions.max_depth),
            plane_->getPosition().z));

}

void SimulationGUI::RenderDebugSection(){
    RenderPlane();
    RenderExample();
    RenderCameraInfo();
    RenderPolygonMode();
}

void SimulationGUI::RenderPlane(){
    static bool do_plane_render = true;
    ImGui::Checkbox("Render Plane", &do_plane_render);
    plane_->do_render(do_plane_render);
}

void SimulationGUI::RenderExample(){
    static bool gui_example = false;
    ImGui::Checkbox("Render GUI Example", &gui_example);
    if(gui_example)
        ImGui::ShowTestWindow();
}

void SimulationGUI::RenderCameraInfo(){
    const glm::vec3& pos = scene_->camera()->getPosition();
    std::string pos_str = "Camera position: ";
    pos_str += std::to_string(pos.x);
    pos_str +=  ", ";
    pos_str += std::to_string(pos.y);
    pos_str +=  ", ";
    pos_str += std::to_string(pos.z);
    ImGui::BulletText(pos_str.c_str(), "");
}

void SimulationGUI::RenderPolygonMode(){
    static bool render_textures = true;
    ImGui::Checkbox("Render Textures", &render_textures);
}

}