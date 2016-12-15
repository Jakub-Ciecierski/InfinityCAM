#include "ifc/path_generation/paths/parametrization_path.h"

#include <ifc/cutter/cutter.h>
#include <infinity_cad/rendering/render_objects/surfaces/surface_c2_cylind.h>
#include <infinity_cad/geometry/intersection/intersection.h>

#include <ifc/path_generation/paths/flat_around_intersection_path.h>
#include <ifc/material/material_box.h>
#include <ifc/factory/cad_model_loader.h>
#include <rendering/scene/scene.h>
#include <factory/program_factory.h>
#include <math/print_math.h>

namespace ifc{

ParametrizationPath::ParametrizationPath(
        std::shared_ptr<CADModelLoaderResult> model_loader_result,
        std::shared_ptr<MaterialBox> material_box,
        std::shared_ptr<ifx::Scene> scene) :
        model_loader_result_(model_loader_result),
        material_box_(material_box),
        scene_(scene){ }

ParametrizationPath::~ParametrizationPath(){ }

std::shared_ptr<Cutter> ParametrizationPath::Generate(
        std::vector<glm::vec3>& positions){
    std::cout << "4) ParametrizationPath" << std::endl;
    //ComputeIntersections();
    return CreatePath(positions);
}

void ParametrizationPath::ComputeIntersections(){
    std::cout << "Computing Intersections" << std::endl;
    intersections_data_.base_hand_left_ = ComputeBaseHandLeftIntersection();
    intersections_data_.base_hand_right_ = ComputeBaseHandRightIntersection();
    intersections_data_.base_drill_ = ComputeBaseDrillIntersection();
}

std::vector<TracePoint> ParametrizationPath::ComputeIntersection(
        std::shared_ptr<SurfaceC2Cylind> surface1,
        std::shared_ptr<SurfaceC2Cylind> surface2,
        glm::vec3 start_pos){
    glm::vec4 init_point4 = glm::vec4(
            start_pos.x, start_pos.y, start_pos.z, 1.0f);
    init_point4
            = model_loader_result_->cad_model->render_object->GetModelMatrix()
              * init_point4;
    glm::vec3 init_point = glm::vec3(init_point4.x,
                                     init_point4.y,
                                     init_point4.z);
    auto intersection = std::unique_ptr<Intersection>(new Intersection(
            surface1.get(),
            surface2.get()));
    intersection->start(init_point);

    const std::vector<TracePoint>& trace_points
            = intersection->getTracePoints();

    return trace_points;
}

std::shared_ptr<IntersectionData>
        ParametrizationPath::ComputeBaseHandRightIntersection(){
    auto base_surface = model_loader_result_->cad_model->surfaces[0];
    auto hand_surface = model_loader_result_->cad_model->surfaces[1];
    std::vector<TracePoint> trace_points
            = ComputeIntersection(
                    base_surface,
                    hand_surface,
                    glm::vec3(0.12, 1.78, 0.31));
    auto render_object = CreateRenderObject(trace_points, "Base-Hand-R");
    scene_->AddRenderObject(render_object);

    auto data = std::shared_ptr<IntersectionData>(new IntersectionData());
    data->trace_points = trace_points;
    data->surface = base_surface;
    data->render_object = render_object;

    return data;
}

std::shared_ptr<IntersectionData>
        ParametrizationPath::ComputeBaseHandLeftIntersection(){
    auto base_surface = model_loader_result_->cad_model->surfaces[0];
    auto hand_surface = model_loader_result_->cad_model->surfaces[1];
    std::vector<TracePoint> trace_points
            = ComputeIntersection(
                    base_surface,
                    hand_surface,
                    glm::vec3(0.00, 1.72, -0.91));
    auto render_object = CreateRenderObject(trace_points, "Base-Hand-L");
    scene_->AddRenderObject(render_object);

    auto data = std::shared_ptr<IntersectionData>(new IntersectionData());
    data->trace_points = trace_points;
    data->surface = base_surface;
    data->render_object = render_object;

    return data;
}

std::shared_ptr<IntersectionData>
        ParametrizationPath::ComputeBaseDrillIntersection(){
    auto base_surface = model_loader_result_->cad_model->surfaces[0];
    auto drill_surface = model_loader_result_->cad_model->surfaces[2];
    std::vector<TracePoint> trace_points
            = ComputeIntersection(
                    base_surface,
                    drill_surface,
                    glm::vec3(0.12, 0.21, 0.77));
    auto render_object = CreateRenderObject(trace_points, "Base-Drill");
    scene_->AddRenderObject(render_object);

    auto data = std::shared_ptr<IntersectionData>(new IntersectionData());
    data->trace_points = trace_points;
    data->surface = base_surface;
    data->render_object = render_object;

    return data;
}

std::shared_ptr<Cutter> ParametrizationPath::CreatePath(
        std::vector<glm::vec3>& positions){
    // TODO common id
    auto base_instructions = CreateBaseIntructions();
    auto hand_instructions = CreateHandIntructions();
    auto drill_instructions = CreateDrillIntructions();
    auto inside_hand_instructions = CreateInsideHandInstructions(positions);

    std::vector<Instruction> instructions;

    instructions.insert(instructions.end(),
                        base_instructions.begin(),
                        base_instructions.end());

    instructions.insert(instructions.end(),
                        hand_instructions.begin(),
                        hand_instructions.end());
    instructions.insert(instructions.end(),
                        drill_instructions.begin(),
                        drill_instructions.end());

    instructions.insert(instructions.end(),
                        inside_hand_instructions.begin(),
                        inside_hand_instructions.end());

    return std::shared_ptr<Cutter>(new Cutter(CutterType::Sphere,
                                              diameter_,
                                              instructions));
}

std::vector<Instruction> ParametrizationPath::CreateBaseIntructions(){
    std::vector<glm::vec3> trajectory = CreateBaseTrajectory();

    const float safety_adder = 15.0f;
    std::vector<Instruction> instructions;
    const float save_height = material_box_->dimensions().depth + safety_adder;
    const float start_height = material_box_->dimensions().max_depth;
    int id = 0;
    glm::vec3 init_pos1 = glm::vec3(
            -material_box_->dimensions().x/2.0f - safety_adder,
            -material_box_->dimensions().z/2.0f - safety_adder,
            save_height);

    glm::vec3 init_pos2 = glm::vec3(
            GLToMillimeters(trajectory[0].x),
            GLToMillimeters(trajectory[0].z),
            save_height);

    instructions.push_back(Instruction(id++, init_pos1));
    instructions.push_back(Instruction(id++, init_pos2));
    for(unsigned int i = 0; i < trajectory.size(); i++){
        instructions.push_back(Instruction(id++,
                                           GetInstructionPosition(
                                                   trajectory[i])));
    }

    glm::vec3 last_pos = instructions[instructions.size()-1].position();
    last_pos.z = save_height;
    instructions.push_back(Instruction(id++, last_pos));

    return instructions;
}

std::vector<Instruction> ParametrizationPath::CreateHandIntructions(){
    std::vector<glm::vec3> trajectory = CreateHandTrajectory();

    const float safety_adder = 15.0f;
    std::vector<Instruction> instructions;
    const float save_height = material_box_->dimensions().depth + safety_adder;
    const float start_height = material_box_->dimensions().max_depth;
    int id = 0;
    glm::vec3 init_pos1 = glm::vec3(
            -material_box_->dimensions().x/2.0f - safety_adder,
            -material_box_->dimensions().z/2.0f - safety_adder,
            save_height);

    glm::vec3 init_pos2 = glm::vec3(
            GLToMillimeters(trajectory[0].x),
            GLToMillimeters(trajectory[0].z),
            save_height);

    instructions.push_back(Instruction(id++, init_pos1));
    instructions.push_back(Instruction(id++, init_pos2));
    for(unsigned int i = 0; i < trajectory.size(); i++){
        instructions.push_back(Instruction(id++,
                                           GetInstructionPosition(
                                                   trajectory[i])));
    }

    glm::vec3 last_pos = instructions[instructions.size()-1].position();
    last_pos.z = save_height;
    instructions.push_back(Instruction(id++, last_pos));

    return instructions;
}

std::vector<Instruction> ParametrizationPath::CreateDrillIntructions(){
    std::vector<glm::vec3> trajectory = CreateDrillTrajectory();

    const float safety_adder = 15.0f;
    std::vector<Instruction> instructions;
    const float save_height = material_box_->dimensions().depth + safety_adder;
    const float start_height = material_box_->dimensions().max_depth;
    int id = 0;
    glm::vec3 init_pos1 = glm::vec3(
            -material_box_->dimensions().x/2.0f - safety_adder,
            -material_box_->dimensions().z/2.0f - safety_adder,
            save_height);

    glm::vec3 init_pos2 = glm::vec3(
            GLToMillimeters(trajectory[0].x),
            GLToMillimeters(trajectory[0].z),
            save_height);

    instructions.push_back(Instruction(id++, init_pos1));
    instructions.push_back(Instruction(id++, init_pos2));
    for(unsigned int i = 0; i < trajectory.size(); i++){
        instructions.push_back(Instruction(id++,
                                           GetInstructionPosition(
                                                   trajectory[i])));
    }

    glm::vec3 last_pos = instructions[instructions.size()-1].position();
    last_pos.z = save_height;
    instructions.push_back(Instruction(id++, last_pos));

    return instructions;
}

std::vector<Instruction> ParametrizationPath::CreateInsideHandInstructions(
        std::vector<glm::vec3>& positions){
    const float safety_adder = 15.0f;
    std::vector<Instruction> instructions;
    if(positions.size() == 0)
        return instructions;

    int id = 0;
    const float save_height = material_box_->dimensions().depth + safety_adder;
    glm::vec3 pos0 = GetInstructionPosition(positions[0]);
    pos0.z = save_height;
    glm::vec3 pos1 = GetInstructionPosition(positions[0]);

    instructions.push_back(Instruction(id++, pos0));
    instructions.push_back(Instruction(id++, pos1));

    for(unsigned int i = 0; i < positions.size(); i++){
        instructions.push_back(Instruction(id++,
                                           GetInstructionPosition
                                                   (positions[i])));
    }
    glm::vec3 pos_last = GetInstructionPosition(positions[positions.size()-1]);
    pos_last.z = save_height;

    instructions.push_back(Instruction(id++, pos_last));

    return instructions;
}

std::vector<glm::vec3> ParametrizationPath::CreateBaseTrajectory(){
    std::cout << "Base Trajectory " << std::endl;
    std::vector<glm::vec3> positions;
    const float max_height = MillimetersToGL(
            material_box_->dimensions().depth -
                    material_box_->dimensions().max_depth);
    auto base_surface = model_loader_result_->cad_model->surfaces[0];
    float du = 0.009;
    float dv = 0.005;
    const float start = 0.0f;
    const float end = 1.0f;

    for(float u = start; u < 0.5f; u+=du){
        std::vector<glm::vec3> row_positions;
        for(float v = 0; v < end; v+=dv){
            glm::vec3 pos = base_surface->compute(u,v);
            if(pos.y <= max_height)
                continue;
            glm::vec3 surf_du = base_surface->computeDu(u,v);
            glm::vec3 surf_dv = base_surface->computeDv(u,v);
            //glm::vec3 norm = glm::normalize(glm::cross(surf_dv, surf_du));
            glm::vec3 norm = glm::cross(surf_dv, surf_du);

            if(ifx::Magnitude(norm) < 0.2){
                ifx::PrintVec3(norm);
                norm = glm::vec3(0,1,0);
            }

            norm = glm::normalize(norm);

            pos += MillimetersToGL(radius_) * norm;
            pos -= glm::vec3(0, MillimetersToGL(radius_), 0);
            if(pos.y <= max_height)
                pos.y = max_height;

            row_positions.push_back(pos);
        };
        for(int i = 0; i < row_positions.size(); i++){
            positions.push_back(row_positions[i]);
        }
        for(int i = row_positions.size() - 1; i > 0; i--){
            positions.push_back(row_positions[i]);
        }
    }
    return positions;
}

std::vector<glm::vec3> ParametrizationPath::CreateHandTrajectory(){
    std::cout << "Hand Trajectory " << std::endl;
    std::vector<glm::vec3> positions;
    const float max_height = MillimetersToGL(
            material_box_->dimensions().depth -
            material_box_->dimensions().max_depth);
    auto surface = model_loader_result_->cad_model->surfaces[1];
    float du = 0.01;
    float dv = 0.01;
    const float start = 0.0f;
    const float end = 1.0f;

    for(float u = start; u < 0.5f; u+=du){
        std::vector<glm::vec3> row_positions;
        for(float v = 0; v < end; v+=dv){
            glm::vec3 pos = surface->compute(u,v);
            if(pos.y <= max_height)
                continue;
            glm::vec3 surf_du = surface->computeDu(u,v);
            glm::vec3 surf_dv = surface->computeDv(u,v);
            glm::vec3 norm = glm::normalize(glm::cross(surf_dv, surf_du));
            pos += MillimetersToGL(radius_) * norm;
            pos -= glm::vec3(0, MillimetersToGL(radius_), 0);
            if(pos.y <= max_height)
                pos.y = max_height;

            row_positions.push_back(pos);
        };
        for(int i = 0; i < row_positions.size(); i++){
            positions.push_back(row_positions[i]);
        }
        for(int i = row_positions.size() - 1; i > 0; i--){
            positions.push_back(row_positions[i]);
        }
    }
    return positions;
}

std::vector<glm::vec3> ParametrizationPath::CreateDrillTrajectory(){
    std::cout << "Drill Trajectory " << std::endl;
    std::vector<glm::vec3> positions;
    const float max_height = MillimetersToGL(
            material_box_->dimensions().depth -
            material_box_->dimensions().max_depth);
    auto surface = model_loader_result_->cad_model->surfaces[2];
    float du = 0.009;
    float dv = 0.009;
    const float start = 0.0f;
    const float end = 1.0f;

    for(float u = 0; u < 0.5f; u+=du)
    {
        std::vector<glm::vec3> row_positions;
        for(float v = 0; v < end; v+=dv){
            glm::vec3 pos = surface->compute(u,v);
            if(pos.y <= max_height)
                continue;
            glm::vec3 surf_du = surface->computeDu(u,v);
            glm::vec3 surf_dv = surface->computeDv(u,v);
            glm::vec3 norm = glm::normalize(glm::cross(surf_du, surf_dv));
            pos += MillimetersToGL(radius_) * norm;
            pos -= glm::vec3(0, MillimetersToGL(radius_), 0);
            if(pos.y <= max_height)
                pos.y = max_height;

            row_positions.push_back(pos);
        };
        for(int i = 0; i < row_positions.size(); i++){
            positions.push_back(row_positions[i]);
        }
        for(int i = row_positions.size() - 1; i > 0; i--){
            positions.push_back(row_positions[i]);
        }
    }
    return positions;
}

std::shared_ptr<ifx::RenderObject> ParametrizationPath::CreateRenderObject(
        const std::vector<TracePoint>& trace_points,
        std::string name){
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    vertices.resize(trace_points.size());
    indices.resize(trace_points.size());
    for(unsigned int i = 0; i < trace_points.size();i++){
        vertices[i] = Vertex{
                trace_points[i].point,
                glm::vec3(0,1,0),
                glm::vec2(0,0)
        };
        indices[i] = i;
    }
    auto mesh = std::unique_ptr<ifx::Mesh>(new ifx::Mesh(vertices, indices));
    mesh->primitive_draw_mode(ifx::PrimitiveDrawMode::LINES);

    std::vector<std::unique_ptr<ifx::Mesh>> meshes;
    meshes.push_back(std::move(mesh));

    auto model = ifx::Model::MakeModel(ifx::NO_FILEPATH, std::move(meshes));
    auto render_object = std::shared_ptr<ifx::RenderObject>(
            new ifx::RenderObject(ObjectID(1, name), model));
    render_object->addProgram(ifx::ProgramFactory().LoadMainProgram());

    return render_object;
}

glm::vec3 ParametrizationPath::GetInstructionPosition(
        const glm::vec3& v){
    glm::vec3 vi;
    vi.x = GLToMillimeters(v.x);
    vi.y = GLToMillimeters(v.z);
    vi.z = GLToMillimeters(v.y);
    return vi;
}


}