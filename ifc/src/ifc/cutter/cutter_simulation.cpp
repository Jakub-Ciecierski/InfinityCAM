#include "ifc/cutter/cutter_simulation.h"

#include <rendering/instanced_render_object.h>
#include <GLFW/glfw3.h>
#include <factory/program_factory.h>
#include <factory/texture_factory.h>

namespace ifc {

CutterSimulation::CutterSimulation(std::shared_ptr<ifx::Scene> scene) :
        scene_(scene),
        time_delta_(0.001),
        current_update_time_(0),
        last_update_time_(0),
        total_time_s_(0),
        line_delta_(1.0){
    Pause();
}

CutterSimulation::~CutterSimulation(){}

bool CutterSimulation::show_trajectory(){
    if(trajectory_.view)
        return trajectory_.view->do_render();
    return false;
}

void CutterSimulation::show_trajectory(bool v){
    if(trajectory_.view)
        trajectory_.view->do_render(v);

    UpdateTrajectoryView();
}

void CutterSimulation::SetMaterialBox(
        std::shared_ptr<MaterialBox> material_box){
    Pause();
    if(material_box_){
        scene_->DeleteRenderObject(
                material_box_->box_render_object().get());
    }

    material_box_ = material_box;

    scene_->AddRenderObject(material_box_->box_render_object());

    Reset();
}

void CutterSimulation::SetCutter(std::shared_ptr<Cutter> cutter){
    Pause();
    if(cutter_){
        scene_->DeleteRenderObject(cutter_->render_object().get());
    }
    cutter_ = cutter;
    if(cutter_)
        scene_->AddRenderObject(cutter_->render_object());

    Reset();
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
    UpdateTrajectory();

    if(cutter_ && cutter_->last_status() != CutterStatus::NONE)
        Pause();
}

void CutterSimulation::Reset(){
    trajectory_.positions.clear();
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
        UpdateTrajectory();
    }
}

void CutterSimulation::UpdateTrajectory(){
    glm::vec3 current_cutter_positions = cutter()->current_position();
    current_cutter_positions = MillimetersToGL(current_cutter_positions);
    float z = current_cutter_positions.z;
    current_cutter_positions.z = current_cutter_positions.y;
    current_cutter_positions.y = z;
    current_cutter_positions.y += MillimetersToGL(1);
    trajectory_.positions.push_back(current_cutter_positions);

    UpdateTrajectoryView();
}

void CutterSimulation::UpdateTrajectoryView(){
    if(trajectory_.view && !trajectory_.view->do_render())
        return;

    std::vector<Vertex> vertices(trajectory_.positions.size());
    std::vector<GLuint> indices(trajectory_.positions.size());
    for(unsigned int i = 0; i < vertices.size(); i++) {
        vertices[i] = Vertex{trajectory_.positions[i],
                             glm::vec3(0.0f, 0.0f, -1.0f),
                             glm::vec2(1.0f, 1.0f)};
        indices[i] = i;
    }
    std::unique_ptr<ifx::Mesh> mesh(new ifx::Mesh(vertices, indices));

    auto material = std::make_shared<ifx::Material>();

    auto texture_diff = ifx::TextureFactory().CreateSolidColorTexture(
            glm::vec3(255,0,0),
            ifx::TextureTypes::DIFFUSE, 1, 1 );
    auto texture_spec = ifx::TextureFactory().CreateSolidColorTexture(
            glm::vec3(255,0,0),
            ifx::TextureTypes::SPECULAR, 1, 1 );

    material->AddTexture(texture_diff);
    material->AddTexture(texture_spec);

    mesh->material(material);
    mesh->primitive_draw_mode(ifx::PrimitiveDrawMode::LINES);
    std::vector<std::unique_ptr<ifx::Mesh>> meshes;
    meshes.push_back(std::move(mesh));
    auto model = ifx::Model::MakeModel(ifx::NO_FILEPATH, std::move(meshes));

    std::shared_ptr<Program> program = ifx::ProgramFactory().LoadMainProgram();

    if(!trajectory_.view){
        trajectory_.view
                = std::shared_ptr<ifx::RenderObject>(new ifx::RenderObject(
                ObjectID(0, "Trajectory"), model));
        trajectory_.view->addProgram(program);
        trajectory_.view->SetBeforeRender(
                [](const Program *program){
                    glLineWidth(3);
                }
        );
        trajectory_.view->SetAfterRender(
                [](const Program *program){
                    glLineWidth(1);
                }
        );
        scene_->AddRenderObject(trajectory_.view);
    }else{
        std::vector<std::shared_ptr<ifx::Model>> models{model};
        trajectory_.view->models(models);
    }
}

}