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
#include <factory/texture_factory.h>

#include <algorithm>

namespace ifc{

ParametrizationPath::ParametrizationPath(
        std::shared_ptr<CADModelLoaderResult> model_loader_result,
        std::shared_ptr<MaterialBox> material_box,
        std::shared_ptr<ifx::Scene> scene) :
        model_loader_result_(model_loader_result),
        material_box_(material_box),
        scene_(scene),
        id_(0) { }

ParametrizationPath::~ParametrizationPath(){ }

std::shared_ptr<Cutter> ParametrizationPath::Generate(
        std::vector<glm::vec3>& positions){
    std::cout << "4) ParametrizationPath" << std::endl;
    ComputeIntersections();
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
    trace_points = TrimTracePoints(trace_points);
    auto render_object = CreateRenderObject(trace_points, "Base-Hand-R");
    scene_->AddRenderObject(render_object);

    auto data = std::shared_ptr<IntersectionData>(new IntersectionData());
    data->trace_points = trace_points;
    data->surface1 = base_surface;
    data->surface2 = hand_surface;
    data->render_object = render_object;

    ComputeEqualDistanceTracePoints(data);
    auto render_object2 = CreateRenderObject(data->eq_distanced_trace_points,
                                            "Base-Hand-R-Eq");
    scene_->AddRenderObject(render_object2);

    auto render_object3 = CreateRenderObject(data->eq_distanced_trace_points1,
                                             "Base-Hand-R-Eq1");
    scene_->AddRenderObject(render_object3);

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
    trace_points = TrimTracePoints(trace_points);
    auto render_object = CreateRenderObject(trace_points, "Base-Hand-L");
    scene_->AddRenderObject(render_object);

    auto data = std::shared_ptr<IntersectionData>(new IntersectionData());
    data->trace_points = trace_points;
    data->surface1 = base_surface;
    data->surface2 = hand_surface;
    data->render_object = render_object;

    ComputeEqualDistanceTracePoints(data);
    auto render_object2 = CreateRenderObject(data->eq_distanced_trace_points,
                                             "Base-Hand-L-Eq");
    scene_->AddRenderObject(render_object2);

    auto render_object3 = CreateRenderObject(data->eq_distanced_trace_points1,
                                             "Base-Hand-L-Eq1");
    scene_->AddRenderObject(render_object3);

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
    trace_points = TrimTracePoints(trace_points);
    auto render_object = CreateRenderObject(trace_points, "Base-Drill");
    scene_->AddRenderObject(render_object);

    auto data = std::shared_ptr<IntersectionData>(new IntersectionData());
    data->trace_points = trace_points;
    data->surface1 = base_surface;
    data->surface2 = drill_surface;
    data->render_object = render_object;

    ComputeEqualDistanceTracePoints(data, -1);
    auto render_object2 = CreateRenderObject(data->eq_distanced_trace_points,
                                             "Base-Drill-Eq");
    scene_->AddRenderObject(render_object2);

    auto render_object3 = CreateRenderObject(data->eq_distanced_trace_points1,
                                             "Base-Drill-Eq1");
    scene_->AddRenderObject(render_object3);


    auto render_object4 = CreateRenderObject(data->eq_distanced_trace_points2,
                                             "Base-Drill-Eq2");
    scene_->AddRenderObject(render_object4);

    return data;
}

std::shared_ptr<Cutter> ParametrizationPath::CreatePath(
        std::vector<glm::vec3>& positions){

    auto base_hand_left_intructions
            = CreateIntersectionCurveInstructions(
                    intersections_data_.base_hand_left_);
    auto base_hand_right_intructions
            = CreateIntersectionCurveInstructions(
                    intersections_data_.base_hand_right_);
    auto base_drill_intructions
            = CreateIntersectionCurveInstructions(
                    intersections_data_.base_drill_);

    auto base_instructions = CreateBaseIntructions();
    auto hand_instructions = CreateHandIntructions();
    auto drill_instructions = CreateDrillIntructions();
    auto inside_hand_instructions = CreateInsideHandInstructions(positions);

    std::vector<Instruction> instructions;
/*
    instructions.insert(instructions.end(),
                        base_hand_left_intructions.begin(),
                        base_hand_left_intructions.end());

    instructions.insert(instructions.end(),
                        base_hand_right_intructions.begin(),
                        base_hand_right_intructions.end());

    instructions.insert(instructions.end(),
                        base_drill_intructions.begin(),
                        base_drill_intructions.end());
*/



    instructions.insert(instructions.end(),
                        base_instructions.begin(),
                        base_instructions.end());
    instructions.insert(instructions.end(),
                        drill_instructions.begin(),
                        drill_instructions.end());

    instructions.insert(instructions.end(),
                        hand_instructions.begin(),
                        hand_instructions.end());

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

    glm::vec3 init_pos1 = glm::vec3(
            -material_box_->dimensions().x/2.0f - safety_adder,
            -material_box_->dimensions().z/2.0f - safety_adder,
            save_height);

    glm::vec3 init_pos2 = glm::vec3(
            GLToMillimeters(trajectory[0].x),
            GLToMillimeters(trajectory[0].z),
            save_height);

    instructions.push_back(Instruction(id_++, init_pos1));
    instructions.push_back(Instruction(id_++, init_pos2));
    for(unsigned int i = 0; i < trajectory.size(); i++){
        instructions.push_back(Instruction(id_++,
                                           GetInstructionPosition(
                                                   trajectory[i])));
    }

    glm::vec3 last_pos = instructions[instructions.size()-1].position();
    last_pos.z = save_height;
    instructions.push_back(Instruction(id_++, last_pos));

    return instructions;
}

std::vector<Instruction> ParametrizationPath::CreateHandIntructions(){
    std::vector<glm::vec3> trajectory = CreateHandTrajectory();

    const float safety_adder = 15.0f;
    std::vector<Instruction> instructions;
    const float save_height = material_box_->dimensions().depth + safety_adder;
    const float start_height = material_box_->dimensions().max_depth;

    glm::vec3 init_pos1 = glm::vec3(
            -material_box_->dimensions().x/2.0f - safety_adder,
            -material_box_->dimensions().z/2.0f - safety_adder,
            save_height);

    glm::vec3 init_pos2 = glm::vec3(
            GLToMillimeters(trajectory[0].x),
            GLToMillimeters(trajectory[0].z),
            save_height);

    instructions.push_back(Instruction(id_++, init_pos1));
    instructions.push_back(Instruction(id_++, init_pos2));
    for(unsigned int i = 0; i < trajectory.size(); i++){
        instructions.push_back(Instruction(id_++,
                                           GetInstructionPosition(
                                                   trajectory[i])));
    }

    glm::vec3 last_pos = instructions[instructions.size()-1].position();
    last_pos.z = save_height;
    instructions.push_back(Instruction(id_++, last_pos));

    return instructions;
}

std::vector<Instruction> ParametrizationPath::CreateDrillIntructions(){
    std::vector<glm::vec3> trajectory = CreateDrillTrajectory();

    const float safety_adder = 15.0f;
    std::vector<Instruction> instructions;
    const float save_height = material_box_->dimensions().depth + safety_adder;
    const float start_height = material_box_->dimensions().max_depth;

    glm::vec3 init_pos1 = glm::vec3(
            -material_box_->dimensions().x/2.0f - safety_adder,
            -material_box_->dimensions().z/2.0f - safety_adder,
            save_height);

    glm::vec3 init_pos2 = glm::vec3(
            GLToMillimeters(trajectory[0].x),
            GLToMillimeters(trajectory[0].z),
            save_height);

    instructions.push_back(Instruction(id_++, init_pos1));
    instructions.push_back(Instruction(id_++, init_pos2));
    for(unsigned int i = 0; i < trajectory.size(); i++){
        instructions.push_back(Instruction(id_++,
                                           GetInstructionPosition(
                                                   trajectory[i])));
    }

    glm::vec3 last_pos = instructions[instructions.size()-1].position();
    last_pos.z = save_height;
    instructions.push_back(Instruction(id_++, last_pos));

    return instructions;
}

std::vector<Instruction> ParametrizationPath::CreateInsideHandInstructions(
        std::vector<glm::vec3>& positions){
    const float safety_adder = 15.0f;
    std::vector<Instruction> instructions;
    if(positions.size() == 0)
        return instructions;

    const float save_height = material_box_->dimensions().depth + safety_adder;
    glm::vec3 pos0 = GetInstructionPosition(positions[0]);
    pos0.z = save_height;
    glm::vec3 pos1 = GetInstructionPosition(positions[0]);

    instructions.push_back(Instruction(id_++, pos0));
    instructions.push_back(Instruction(id_++, pos1));

    for(unsigned int i = 0; i < positions.size(); i++){
        instructions.push_back(Instruction(id_++,
                                           GetInstructionPosition
                                                   (positions[i])));
    }
    glm::vec3 pos_last = GetInstructionPosition(positions[positions.size()-1]);
    pos_last.z = save_height;

    instructions.push_back(Instruction(id_++, pos_last));

    return instructions;
}


std::vector<Instruction>
ParametrizationPath::CreateIntersectionCurveInstructions(
        std::shared_ptr<IntersectionData> data){
    const float max_height = MillimetersToGL(
            material_box_->dimensions().depth -
            material_box_->dimensions().max_depth);

    const float safety_adder = 15.0f;
    std::vector<Instruction> instructions;
    auto trace_points = data->eq_distanced_trace_points3;
    //auto trace_points = data->eq_distanced_trace_points2;
    if(trace_points.size() == 0)
        return instructions;

    const float save_height = material_box_->dimensions().depth + safety_adder;
    glm::vec3 pos0 = GetInstructionPosition(trace_points[0].point);
    pos0.z = save_height;
    glm::vec3 pos1 = GetInstructionPosition(trace_points[0].point);

    instructions.push_back(Instruction(id_++, pos0));
    instructions.push_back(Instruction(id_++, pos1));

    for(unsigned int i = 0; i < trace_points.size(); i++){
        auto pos = trace_points[i].point;
        pos -= glm::vec3(0, MillimetersToGL(radius_), 0);
        if(pos.y <= max_height)
            pos.y = max_height;
        instructions.push_back(Instruction(id_++,
                                           GetInstructionPosition(pos)));
/*
        instructions.push_back(Instruction(id_++,
                                           GetInstructionPosition
                                                   (trace_points[i].point)));
                                                   */
    }
    glm::vec3 pos_last
            = GetInstructionPosition(
                    trace_points[trace_points.size()-1].point);
    pos_last.z = save_height;

    instructions.push_back(Instruction(id_++, pos_last));

    return instructions;
}

std::vector<glm::vec3> ParametrizationPath::CreateBaseTrajectory(){
    std::cout << "Base Trajectory " << std::endl;

    auto base_drill_points
            = intersections_data_.base_drill_->eq_distanced_trace_points1;
    auto base_left_points
            = intersections_data_.base_hand_left_->eq_distanced_trace_points1;
    auto base_right_points
            = intersections_data_.base_hand_right_->eq_distanced_trace_points1;

    std::vector<glm::vec3> positions;
    const float max_height = MillimetersToGL(
            material_box_->dimensions().depth -
                    material_box_->dimensions().max_depth);
    auto base_surface = model_loader_result_->cad_model->surfaces[0];
    float du = 0.009 * 0.5f;
    float dv = 0.005;
    const float start = 0.0f;
    const float end = 1.0f;

    //for(float u = start; u < 0.5f; u+=du){
    float start_u = 0.5f - (12.0f*du);
    for(float u = start_u; u >= 0; u-=du){
        std::vector<glm::vec3> row_positions;

        bool has_collided = false;

        for(float v = 0; v < end; v+=dv){
        //for(float v = end; v >= 0; v-=dv){
            glm::vec3 pos = base_surface->compute(u,v);

            if(pos.y < max_height)
                continue;

            glm::vec3 surf_du = base_surface->computeDu(u,v);
            glm::vec3 surf_dv = base_surface->computeDv(u,v);
            glm::vec3 norm = glm::cross(surf_dv, surf_du);

            if(ifx::Magnitude(norm) < 0.2){
                norm = glm::vec3(0,1,0);
            }
            norm = glm::normalize(norm);


            glm::vec3 colliding_point1;
            glm::vec3 colliding_point2;
            glm::vec3 colliding_point3;

            glm::vec3 center;
            center = pos + MillimetersToGL(radius_) * norm;
/*
            if (row_positions.size() != 0){
                auto last_pos = row_positions[row_positions.size() - 1];
                bool ends_colliding = has_collided && last_pos.y == max_height;

                center = last_pos;
                center += glm::vec3(0, MillimetersToGL(radius_), 0);
            }*/

            auto is_colliding1
                    = IsColliding(base_drill_points, center, &colliding_point1);
            auto is_colliding2
                    = IsColliding(base_left_points, center, &colliding_point2);
            auto is_colliding3
                    = IsColliding(base_right_points, center, &colliding_point3);

            //if(is_colliding1)
                //break;
/*
            float dy = MillimetersToGL(radius_) * 0.05;
            float save_height_adder = dy;
            while(IsColliding(base_drill_points, center)){
                center += glm::vec3(0, save_height_adder,0);
                save_height_adder += dy;
            }
*/

            pos += MillimetersToGL(radius_) * norm;
            pos -= glm::vec3(0, MillimetersToGL(radius_), 0);

            if(is_colliding1){
                has_collided = true;
                pos = colliding_point1;
            }


            if(pos.y <= max_height)
                pos.y = max_height;


/*
            if (is_colliding1)
                pos = colliding_point1;
            if (is_colliding2)
                pos = colliding_point2;
            if (is_colliding3)
                pos = colliding_point3;
*/
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
    auto base_hand_left_points
            = intersections_data_.base_hand_left_->eq_distanced_trace_points2;

    auto base_hand_right_points
            = intersections_data_.base_hand_right_->eq_distanced_trace_points2;

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

        // <find non-colliding v>
        float start_v = 0.05;
        for(; start_v < end; start_v+=dv){
            glm::vec3 pos = surface->compute(u,start_v);
            if(pos.y <= max_height)
                continue;

            glm::vec3 surf_du = surface->computeDu(u,start_v);
            glm::vec3 surf_dv = surface->computeDv(u,start_v);
            glm::vec3 norm = glm::normalize(glm::cross(surf_dv, surf_du));

            auto center = pos + MillimetersToGL(radius_) * norm;
            if(!IsColliding(base_hand_left_points, center))
                break;
        }
        std::cout << "start_v: " << start_v << std::endl;
        // </find non-colliding v>
        //start_v = 0.05;

        for(float v = start_v; v < end; v+=dv){
            glm::vec3 pos = surface->compute(u,v);
            if(pos.y <= max_height)
                continue;

            glm::vec3 surf_du = surface->computeDu(u,v);
            glm::vec3 surf_dv = surface->computeDv(u,v);
            glm::vec3 norm = glm::normalize(glm::cross(surf_dv, surf_du));

            auto center = pos + MillimetersToGL(radius_) * norm;
            if(IsColliding(base_hand_right_points, center))
                break;

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
    auto base_drill_points
            = intersections_data_.base_drill_->eq_distanced_trace_points2;
    std::cout << "Drill Trajectory " << std::endl;
    std::vector<glm::vec3> positions;
    const float max_height = MillimetersToGL(
            material_box_->dimensions().depth -
            material_box_->dimensions().max_depth);
    auto surface = model_loader_result_->cad_model->surfaces[2];
    float du = 0.009f * 1.5f;
    float dv = 0.009f * 1.5f;
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

            auto center = pos + MillimetersToGL(radius_) * norm;
            if(IsColliding(base_drill_points, center))
                break;

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

std::vector<TracePoint> ParametrizationPath::TrimTracePoints(
        const std::vector<TracePoint>& trace_points){
    if(trace_points.size() == 0)
        return std::vector<TracePoint>();

    float max_height = MillimetersToGL(material_box_->dimensions().depth
                       - material_box_->dimensions().max_depth);
    std::vector<TracePoint> trimmed_trace_points;
    for(unsigned int i = 0; i < trace_points.size(); i++){
        if(trace_points[i].point.y > max_height)
            trimmed_trace_points.push_back(trace_points[i]);
    }

    float max = 0.0f;
    int max_index = 0;
    for(unsigned int i = 0; i < trimmed_trace_points.size()-1; i++){
        float z1 = trimmed_trace_points[i].point.y;
        float z2 = trimmed_trace_points[i+1].point.y;
        float distance = z1 - z2;
        distance = std::sqrt(distance*distance);

        if(distance > max){
            max = distance;
            max_index = i;
        }
    }

    int beg_index = max_index;
    int end_index = max_index + 1;

    std::cout << "beg_index: " << beg_index << std::endl;
    std::cout << "end_index: " << end_index << std::endl;
    std::cout << ifx::EuclideanDistance(trace_points[beg_index].point,
                                        trace_points[end_index].point)
    << std::endl;
    std::cout << trimmed_trace_points.size() << std::endl;

    std::vector<TracePoint> ordered_trace_points;
    for(unsigned int i = beg_index; i < trimmed_trace_points.size(); i++){
        ordered_trace_points.push_back(trimmed_trace_points[i]);
    }
    for(unsigned int i = 0; i <= end_index; i++){
        ordered_trace_points.push_back(trimmed_trace_points[i]);
    }
    /*
    for(unsigned int i = beg_index; i < trimmed_trace_points.size(); i++){
        ordered_trace_points.push_back(trimmed_trace_points[i]);
    }
    for(unsigned int i = 0; i <= end_index; i++){
        ordered_trace_points.push_back(trimmed_trace_points[i]);
    }*/

    //return trimmed_trace_points;
    return ordered_trace_points;
}

bool ParametrizationPath::IsColliding(
        const std::vector<TracePoint>& trace_points, const glm::vec3& point) {
    auto center = point;
    float distance = MillimetersToGL(radius_);

    for (auto& trace_point : trace_points){
        if(ifx::EuclideanDistance(trace_point.point, center) <=
                distance){
            return true;
        }
    }
    return false;
}

bool ParametrizationPath::IsColliding(
        const std::vector<TracePoint>& trace_points,
        const glm::vec3& point, glm::vec3* colliding_point) {
    if(!IsColliding(trace_points, point))
        return false;

    auto center = point;


    float distance_min = 999999.0f;
    for(unsigned int i = 0; i < trace_points.size(); i++){

        float current_distance
                = ifx::EuclideanDistance(trace_points[i].point, center);

        if(current_distance < distance_min){
            distance_min = current_distance;
            *colliding_point = trace_points[i].point;
        }
    }

    /*
    float distance = MillimetersToGL(radius_);
    for (auto& trace_point : trace_points){
        if(ifx::EuclideanDistance(trace_point.point, center) <=
           distance){
            *colliding_point = trace_point.point;
            return true;
        }
    }*/
    return true;
}

void ParametrizationPath::ComputeEqualDistanceTracePoints(
        std::shared_ptr<IntersectionData> data, int direction){
    const float max_height = MillimetersToGL(
            material_box_->dimensions().depth -
            material_box_->dimensions().max_depth);

    for(auto& trace_point : data->trace_points){
        glm::vec3 surf_du1 = data->surface1->computeDu(
                trace_point.params.x, trace_point.params.y);
        glm::vec3 surf_dv1 = data->surface1->computeDv(
                trace_point.params.x, trace_point.params.y);
        glm::vec3 norm1 = glm::cross(surf_dv1, surf_du1);

        norm1 = glm::normalize(norm1);

        glm::vec3 surf_du2 = data->surface2->computeDu(
                trace_point.params.z, trace_point.params.w);
        glm::vec3 surf_dv2 = data->surface2->computeDv(
                trace_point.params.z, trace_point.params.w);
        glm::vec3 norm2 = glm::cross(surf_dv2, surf_du2);
        if(direction == -1)
            norm2 = glm::cross(surf_du2, surf_dv2);
        norm2 = glm::normalize(norm2);

        auto eq_distanced_trace_point = trace_point;

        eq_distanced_trace_point.point
                += (MillimetersToGL(radius_) * norm1) +
                   (MillimetersToGL(radius_) * norm2);
        if(eq_distanced_trace_point.point.y <= max_height)
            eq_distanced_trace_point.point.y = max_height;
        data->eq_distanced_trace_points.push_back(eq_distanced_trace_point);


        auto eq_distanced_trace_point1 = trace_point;
        eq_distanced_trace_point1.point
                += (MillimetersToGL(radius_) * norm1) +
                (MillimetersToGL(radius_) * 0.5f * norm2);
        if(eq_distanced_trace_point1.point.y <= max_height)
            eq_distanced_trace_point1.point.y = max_height;
        data->eq_distanced_trace_points1.push_back(eq_distanced_trace_point1);



        auto eq_distanced_trace_point2 = trace_point;
        eq_distanced_trace_point2.point
                += (MillimetersToGL(radius_) * 0.5f * norm2);
        if(eq_distanced_trace_point2.point.y <= max_height)
            eq_distanced_trace_point2.point.y = max_height;
        data->eq_distanced_trace_points2.push_back(eq_distanced_trace_point2);



        auto eq_distanced_trace_point3 = trace_point;
        eq_distanced_trace_point3.point
                += (MillimetersToGL(radius_) * 1.0f * norm1) +
                   (MillimetersToGL(radius_) * 1.0f * norm2);
        if(eq_distanced_trace_point3.point.y <= max_height)
            eq_distanced_trace_point3.point.y = max_height;
        data->eq_distanced_trace_points3.push_back(eq_distanced_trace_point3);
    }
}

}