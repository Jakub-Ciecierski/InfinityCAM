#include "ifc/cutter/cutter_simulation.h"

#include <rendering/instanced_render_object.h>
#include <GLFW/glfw3.h>

namespace ifc {

CutterSimulation::CutterSimulation(ifx::Scene* scene) :
        scene_(scene),
        time_delta_(0.001),
        current_update_time_(0),
        last_update_time_(0),
        total_time_s_(0),
        line_delta_(0.5){
    Pause();
}

CutterSimulation::~CutterSimulation(){}

void CutterSimulation::SetMaterialBox(
        std::shared_ptr<MaterialBox> material_box){
    if(material_box_){
        scene_->DeleteRenderObject(
                material_box_->box_render_object().get());
    }

    material_box_ = material_box;

    scene_->AddRenderObject(material_box_->box_render_object());
}

void CutterSimulation::SetCutter(std::shared_ptr<Cutter> cutter){
    if(cutter_){
        scene_->DeleteRenderObject(cutter_->render_object().get());
    }
    cutter_ = cutter;
    if(cutter_)
        scene_->AddRenderObject(cutter_->render_object());
}

void CutterSimulation::Update() {
    if(cutter_ && cutter_->Finished()){
        Pause();
        return;
    }
    if(!CanUpdate())
        return;
    if(!SatisfiesTimeDelta())
        return;

    cutter_->Update(material_box_.get(), line_delta_);
    material_box_->Update();

    if(cutter_ && cutter_->last_status() != CutterStatus::NONE)
        Pause();
}

bool CutterSimulation::SatisfiesTimeDelta(){
    current_update_time_ = glfwGetTime();
    if(!running_){
        last_update_time_ = current_update_time_;
        return false;
    }
    double time_delta = current_update_time_ - last_update_time_;
    total_time_s_ += time_delta;

    bool value = time_delta >= time_delta_;
    if(value)
        last_update_time_ = current_update_time_;
    return value;
}

bool CutterSimulation::CanUpdate(){
    return material_box_ && cutter_;
}

void CutterSimulation::UpdateAllAtOnce(){
    if(cutter_ && cutter_->Finished()){
        Pause();
        return;
    }
    if(!CanUpdate())
        return;

    while(!cutter_->Finished()){
        cutter_->Update(material_box_.get(), line_delta_);
        material_box_->Update();
    }
}

}