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
    static char filepath[1024] = "ciecierskij.mg";

    if (ImGui::Button("Load Model")) {
        CADModelLoader cad_model_loader;
        cad_model_loader_result_ = cad_model_loader.Load(filepath);

        scene_->AddRenderObject(
                cad_model_loader_result_->cad_model->render_object);
        scene_->AddRenderObject(
                cad_model_loader_result_->interection_model->render_object);

    }
    ImGui::SameLine();
    ImGui::InputText("filepath", filepath, size);

    ImGui::PopItemWidth();
}

void PathGenerationGUI::RenderPathGeneration(){
    ImGui::PushItemWidth(150);
    const int size = 1024;
    static char filepath_1[size] = "jc_t1";
    static char filepath_2[size] = "jc_t2";
    static char filepath_3[size] = "jc_t3";
    static char filepath_4[size] = "jc_t4";

    if (ImGui::Button("Generate All")) {
        if(cad_model_loader_result_){
            path_generator_.reset(new PathGenerator(
                    cad_model_loader_result_,
                    simulation_->material_box(),
                    scene_));
            auto paths = path_generator_->GenerateAll();

            paths.rough_cutter->SaveToFile(filepath_1);
            paths.flat_heighmap_cutter->SaveToFile(filepath_2);
            if(paths.flat_intersection_cutter)
                paths.flat_intersection_cutter->SaveToFile(filepath_3);
            if(paths.parametrization_cutter)
                paths.parametrization_cutter->SaveToFile(filepath_4);
        }
    }

    if(ImGui::TreeNode("Roughing Path")) {
        if (ImGui::Button("Generate")) {
            if(cad_model_loader_result_){
                path_generator_.reset(new PathGenerator(
                        cad_model_loader_result_,
                        simulation_->material_box(),
                        scene_));
                auto cutter = path_generator_->GenerateRoughingPath();
                cutter->SaveToFile(filepath_1);
            }
        }
        ImGui::SameLine();
        ImGui::InputText("filename", filepath_1, size);
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Flat Heightmap Path")) {

        if (ImGui::Button("Generate")) {
            if(cad_model_loader_result_){
                path_generator_.reset(new PathGenerator(
                        cad_model_loader_result_,
                        simulation_->material_box(),
                        scene_));
                auto cutter = path_generator_->GenerateFlatHeightmapPath();
                cutter->SaveToFile(filepath_2);
            }
        }
        ImGui::SameLine();
        ImGui::InputText("filename", filepath_2, size);
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Flat Intersection Box Path")) {

        if (ImGui::Button("Generate")) {
            if(cad_model_loader_result_){
                path_generator_.reset(new PathGenerator(
                        cad_model_loader_result_,
                        simulation_->material_box(),
                        scene_));
                auto cutter = path_generator_->GenerateFlatIntersectionPath();
                if(cutter)
                    cutter->SaveToFile(filepath_3);
            }
        }
        ImGui::SameLine();
        ImGui::InputText("filename", filepath_3, size);
        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Parametrization Path")) {

        if (ImGui::Button("Generate")) {
            if(cad_model_loader_result_){
                path_generator_.reset(new PathGenerator(
                        cad_model_loader_result_,
                        simulation_->material_box(),
                        scene_));
                auto cutter = path_generator_->GenerateParametrizationPath();
                if(cutter)
                    cutter->SaveToFile(filepath_4);
            }
        }
        ImGui::SameLine();
        ImGui::InputText("filename", filepath_4, size);
        ImGui::TreePop();
    }

    ImGui::PopItemWidth();
}

}
