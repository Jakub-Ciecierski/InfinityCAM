#include <ifc/gui/path_generation_gui.h>

#include <gui/imgui/imgui.h>
#include <ifc/factory/cad_model_loader.h>
#include <rendering/scene/scene.h>

namespace ifc {

PathGenerationGUI::PathGenerationGUI(std::shared_ptr<ifx::Scene> scene) :
        scene_(scene){}

PathGenerationGUI::~PathGenerationGUI(){}

void PathGenerationGUI::Render(){
    RenderMainWindow();
}


void PathGenerationGUI::RenderMainWindow(){
    ImGui::SetNextWindowSize(ImVec2(350,400));
    ImGui::Begin("Path Generation");

    RenderLoadModel();

    ImGui::End();
}

void PathGenerationGUI::RenderLoadModel(){
    ImGui::PushItemWidth(150);
    int size = 1024;
    static char filepath[1024] = "ciecierskij_model.mg";

    if(ImGui::TreeNode("Model")) {
        if (ImGui::Button("Load Model")) {
            CADModelLoader cad_model_loader;
            scene_->AddRenderObject(cad_model_loader.Load(filepath));
        }
        ImGui::SameLine();
        ImGui::InputText("filepath", filepath, size);
        ImGui::TreePop();
    }

    ImGui::PopItemWidth();
}

}
