#include <ifc/gui/path_generation_gui.h>

#include <gui/imgui/imgui.h>
#include <ifc/cutter/cutter.h>
#include <ifc/factory/cad_model_loader.h>
#include <ifc/path_generation/path_generator.h>
#include <ifc/material/material_box.h>
#include <ifc/cutter/cutter_simulation.h>
#include <rendering/scene/scene.h>

namespace ifc {

PathGenerationGUI::PathGenerationGUI(
        std::shared_ptr<ifx::Scene> scene,
        std::shared_ptr<CutterSimulation> simulation)
        : scene_(scene),
          simulation_(simulation){}

PathGenerationGUI::~PathGenerationGUI(){}

void PathGenerationGUI::Render(){
    RenderMainWindow();
}


void PathGenerationGUI::RenderMainWindow(){
    ImGui::SetNextWindowSize(ImVec2(350,400));
    ImGui::Begin("Path Generation");

    if(ImGui::TreeNode("Model")) {
        RenderLoadModel();
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Generate Paths")){
        RenderPathGeneration();
        ImGui::TreePop();
    }


    ImGui::End();
}

void PathGenerationGUI::RenderLoadModel(){
    ImGui::PushItemWidth(150);
    int size = 1024;
    static char filepath[1024] = "ciecierskij_model.mg";

    if (ImGui::Button("Load Model")) {
        CADModelLoader cad_model_loader;
        cad_model_loader_result_ = cad_model_loader.Load(filepath);
        scene_->AddRenderObject(cad_model_loader_result_->render_object);

    }
    ImGui::SameLine();
    ImGui::InputText("filepath", filepath, size);

    ImGui::PopItemWidth();
}

void PathGenerationGUI::RenderPathGeneration(){
    ImGui::PushItemWidth(150);
    int size = 1024;
    static char filepath[1024] = "jc_t1";

    if(ImGui::TreeNode("Roughing Path")) {
        if (ImGui::Button("Generate")) {
            if(cad_model_loader_result_){
                path_generator_.reset(new PathGenerator(
                        cad_model_loader_result_,
                        simulation_->material_box()));
                auto cutter = path_generator_->GenerateRoughingPath();
                cutter->SaveToFile(filepath);
            }
        }
        ImGui::SameLine();
        ImGui::InputText("filename", filepath, size);
        ImGui::TreePop();
    }
    ImGui::PopItemWidth();
}

}
