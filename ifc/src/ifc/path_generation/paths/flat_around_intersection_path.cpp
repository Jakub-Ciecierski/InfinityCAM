#include "ifc/path_generation/paths/flat_around_intersection_path.h"

#include <ifc/material/material_box.h>
#include <ifc/factory/cad_model_loader.h>
#include <ifc/cutter/cutter.h>

#include <infinity_cad/geometry/intersection/intersection.h>
#include <infinity_cad/rendering/render_objects/surfaces/surface_c2_rect.h>
#include <infinity_cad/rendering/render_objects/surfaces/surface_c2_cylind.h>

#include <rendering/scene/scene.h>
#include <object/render_object.h>
#include <factory/program_factory.h>

namespace ifc {

FlatAroundIntersectionPath::FlatAroundIntersectionPath(
        std::shared_ptr<CADModelLoaderResult> model_loader_result,
        std::shared_ptr<MaterialBox> material_box,
        std::shared_ptr<ifx::Scene> scene) :
        model_loader_result_(model_loader_result),
        material_box_(material_box),
        scene_(scene),
        generated_(false){}

FlatAroundIntersectionPath::~FlatAroundIntersectionPath(){}

std::shared_ptr<Cutter> FlatAroundIntersectionPath::Generate(){
    generated_ = true;
    std::cout << "3) Generating FlatAroundIntersectionPath" << std::endl;

    ComputeIntersections();
    CutterTrajectory trajectory = CreateTrajectory(intersections_data_);

    // Used in final stage
    inside_hand_positions_ = CreateInsideHandTrajectory(intersections_data_);

    return CreatePath(trajectory);
}

void FlatAroundIntersectionPath::ComputeIntersections(){
    intersections_data_.base_top_intersection = ComputeBaseTopIntersection(
            NormalDirection::DOWN);
    intersections_data_.base_bottom_intersection = ComputeBaseBottomIntersection(
            NormalDirection::DOWN);
    intersections_data_.hand_top_intersection = ComputeHandTopIntersection(
            NormalDirection::DOWN);
    intersections_data_.hand_bottom_intersection
            = ComputeHandBottomIntersection(NormalDirection::DOWN);
    intersections_data_.drill_left_intersection = ComputeDrillLeftIntersection(
            NormalDirection::UP);
    intersections_data_.drill_right_intersection = ComputeDrillRightIntersection(
            NormalDirection::UP);
}

std::vector<TracePoint> FlatAroundIntersectionPath::ComputeIntersection(
        std::shared_ptr<SurfaceC2Cylind> surface1,
        std::shared_ptr<SurfaceC2Rect> surface2,
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

std::shared_ptr<BoxIntersectionData>
        FlatAroundIntersectionPath::ComputeBaseTopIntersection(
                NormalDirection normal_direction){
    auto surface = model_loader_result_->cad_model->surfaces[0];
    std::vector<TracePoint> trace_points
            = ComputeIntersection(
                    surface,
                    model_loader_result_->interection_model->surface,
                    glm::vec3(0, 1.89, 0.88));
    //0, 1.68, -0.28))
    auto render_object = CreateRenderObject(trace_points, "Base Top");
    scene_->AddRenderObject(render_object);

    auto data = std::shared_ptr<BoxIntersectionData>(new BoxIntersectionData());
    data->trace_points = trace_points;
    data->surface = surface;
    data->render_object = render_object;

    data->equally_distanced
            = ComputeEqualDistancedIntersections(data, normal_direction);
    data->double_equally_distanced
            = ComputeEqualDistancedIntersections(data, normal_direction, 2.0f);

    return data;
}

std::shared_ptr<BoxIntersectionData>
        FlatAroundIntersectionPath::ComputeBaseBottomIntersection(
                NormalDirection normal_direction){
    auto surface = model_loader_result_->cad_model->surfaces[0];
    std::vector<TracePoint> trace_points
            = ComputeIntersection(
                    surface,
                    model_loader_result_->interection_model->surface,
                    glm::vec3(0, 0.47, -0.35));
    auto render_object = CreateRenderObject(trace_points, "Base Bottom");
    scene_->AddRenderObject(render_object);

    auto data = std::shared_ptr<BoxIntersectionData>(new BoxIntersectionData());
    data->trace_points = trace_points;
    data->surface = surface;
    data->render_object = render_object;

    data->equally_distanced
            = ComputeEqualDistancedIntersections(data, normal_direction);

    return data;
}

std::shared_ptr<BoxIntersectionData>
FlatAroundIntersectionPath::ComputeDrillLeftIntersection(
        NormalDirection normal_direction){
    auto surface = model_loader_result_->cad_model->surfaces[2];
    std::vector<TracePoint> trace_points
            = ComputeIntersection(
                    surface,
                    model_loader_result_->interection_model->surface,
                    glm::vec3(0, -0.25, 0.95));
    // glm::vec3(0, -0.74, 0.83)
    auto render_object = CreateRenderObject(trace_points, "Drill Left");
    scene_->AddRenderObject(render_object);

    auto data = std::shared_ptr<BoxIntersectionData>(new BoxIntersectionData());
    data->trace_points = trace_points;
    data->surface = surface;
    data->render_object = render_object;

    data->equally_distanced
            = ComputeEqualDistancedIntersections(data, normal_direction);

    return data;
}

std::shared_ptr<BoxIntersectionData>
FlatAroundIntersectionPath::ComputeDrillRightIntersection(
        NormalDirection normal_direction){
    auto surface = model_loader_result_->cad_model->surfaces[2];
    std::vector<TracePoint> trace_points
            = ComputeIntersection(
                    surface,
                    model_loader_result_->interection_model->surface,
                    glm::vec3(0, -0.25, 0.69));
    // glm::vec3(0, -0.74, 0.83)
    auto render_object = CreateRenderObject(trace_points, "Drill Right");
    scene_->AddRenderObject(render_object);

    auto data = std::shared_ptr<BoxIntersectionData>(new BoxIntersectionData());
    data->trace_points = trace_points;
    data->surface = surface;
    data->render_object = render_object;

    data->equally_distanced
            = ComputeEqualDistancedIntersections(data, normal_direction);

    return data;
}

std::shared_ptr<BoxIntersectionData>
        FlatAroundIntersectionPath::ComputeHandTopIntersection(
                NormalDirection normal_direction){
    auto surface = model_loader_result_->cad_model->surfaces[1];
    std::vector<TracePoint> trace_points
            = ComputeIntersection(
                    surface,
                    model_loader_result_->interection_model->surface,
                    glm::vec3(0, 2.60, -0.17));
    auto render_object = CreateRenderObject(trace_points, "Hand Top");
    scene_->AddRenderObject(render_object);

    auto data = std::shared_ptr<BoxIntersectionData>(new BoxIntersectionData());
    data->trace_points = trace_points;
    data->surface = surface;
    data->render_object = render_object;

    data->equally_distanced
            = ComputeEqualDistancedIntersections(data, normal_direction);

    return data;
}
std::shared_ptr<BoxIntersectionData>
        FlatAroundIntersectionPath::ComputeHandBottomIntersection(
                NormalDirection normal_direction){
    auto surface = model_loader_result_->cad_model->surfaces[1];
    std::vector<TracePoint> trace_points
            = ComputeIntersection(
                    surface,
                    model_loader_result_->interection_model->surface,
                    glm::vec3(0, 2.29, -0.17));
    auto render_object = CreateRenderObject(trace_points, "Hand Bottom");
    scene_->AddRenderObject(render_object);

    auto data = std::shared_ptr<BoxIntersectionData>(new BoxIntersectionData());
    data->trace_points = trace_points;
    data->surface = surface;
    data->render_object = render_object;

    data->equally_distanced
            = ComputeEqualDistancedIntersections(data, normal_direction);

    data->double_equally_distanced
            = ComputeEqualDistancedIntersections(data, normal_direction, 2.0f);

    return data;
}


std::shared_ptr<BoxIntersectionData>
FlatAroundIntersectionPath::ComputeEqualDistancedIntersections(
        std::shared_ptr<BoxIntersectionData> intersection_data,
        NormalDirection normal_direction,
        float distance_scalar){
    auto equally_distanced_data = std::make_shared<BoxIntersectionData>();
    int size = intersection_data->trace_points.size();
    equally_distanced_data->trace_points = intersection_data->trace_points;
    equally_distanced_data->surface = intersection_data->surface;

    for(int i = 0; i < size; i++){
        glm::vec4& params = equally_distanced_data->trace_points[i].params;
        glm::vec3 du = equally_distanced_data->surface->computeDu(params.x,
                                                                  params.y);
        glm::vec3 dv = equally_distanced_data->surface->computeDv(params.x,
                                                                  params.y);
        glm::vec3 normal;
        if(normal_direction == NormalDirection::UP)
            normal = glm::normalize(glm::cross(du,dv));
        if(normal_direction == NormalDirection::DOWN)
            normal = glm::normalize(glm::cross(dv,du));

        equally_distanced_data->trace_points[i].point
                += distance_scalar * MillimetersToGL(radius_) * normal;
    }
    std::string name = intersection_data->render_object->id().name();
    name += "_ed";
    equally_distanced_data->render_object
            = CreateRenderObject(equally_distanced_data->trace_points, name);
    scene_->AddRenderObject(equally_distanced_data->render_object);

    return equally_distanced_data;
}

std::shared_ptr<ifx::RenderObject> FlatAroundIntersectionPath::CreateRenderObject(
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

CutterTrajectory FlatAroundIntersectionPath::CreateTrajectory(
        BoxIntersectionsData& intersections_data_){
    CutterTrajectory trajectory;

    std::vector<glm::vec3> base_hand_positions
            = CreateBaseHandTrajectory(intersections_data_);
    std::vector<glm::vec3> base_top_bottom_positions
            = CreateBaseTopBottomTrajectory(intersections_data_);
    std::vector<glm::vec3> base_bottom_drill_left
            = CreateBaseDrillLeftTrajectory(intersections_data_);
    std::vector<glm::vec3> drill_left_right
            = CreateDrillLeftRightTrajectory(intersections_data_);
    std::vector<glm::vec3> drill_right_base
            = CreateDrillRightBaseBottomTrajectory(intersections_data_);
    std::vector<glm::vec3> base_bottom_top
            = CreateBaseBottomTopTrajectory(intersections_data_);

    trajectory.positions.insert(trajectory.positions.end(),
                                base_hand_positions.begin(),
                                base_hand_positions.end());
    trajectory.positions.insert(trajectory.positions.end(),
                                base_top_bottom_positions.begin(),
                                base_top_bottom_positions.end());
    trajectory.positions.insert(trajectory.positions.end(),
                                base_bottom_drill_left.begin(),
                                base_bottom_drill_left.end());
    trajectory.positions.insert(trajectory.positions.end(),
                                drill_left_right.begin(),
                                drill_left_right.end());
    trajectory.positions.insert(trajectory.positions.end(),
                                drill_right_base.begin(),
                                drill_right_base.end());
    trajectory.positions.insert(trajectory.positions.end(),
                                base_bottom_top.begin(),
                                base_bottom_top.end());

    for(int i = 0; i < trajectory.positions.size(); i++){
        // Avoid numerical errors, round up to e.g. 20mm
        trajectory.positions[i].y
                = MillimetersToGL(
                material_box_->dimensions().depth -
                material_box_->dimensions().max_depth);
    }

    return trajectory;
}

std::vector<glm::vec3> FlatAroundIntersectionPath::CreateBaseHandTrajectory(
        BoxIntersectionsData& intersections_data_){
    std::vector<glm::vec3> positions;
    auto base_top
            = intersections_data_.base_top_intersection->equally_distanced;
    auto hand_top
            = intersections_data_.hand_top_intersection->equally_distanced;

    glm::vec2 base_hand_indicies1
            = GetClosestPointsIndices(base_top->trace_points,
                                      hand_top->trace_points,
                                      0, hand_top->trace_points.size()/2);
    glm::vec2 base_hand_indicies2
            = GetClosestPointsIndices(base_top->trace_points,
                                      hand_top->trace_points,
                                      hand_top->trace_points.size()/2,
                                      hand_top->trace_points.size());

    for(int i = 0; i < base_hand_indicies1.x; i++){
        positions.push_back(base_top->trace_points[i].point
        );
    }
    for(int i = base_hand_indicies1.y; i < base_hand_indicies2.y; i++){
        positions.push_back(hand_top->trace_points[i].point
        );
    }
    for(int i = base_hand_indicies2.x; i < base_top->trace_points.size(); i++){
        positions.push_back(base_top->trace_points[i].point
        );
    }

    return positions;
}

std::vector<glm::vec3>
FlatAroundIntersectionPath::CreateBaseTopBottomTrajectory(
        BoxIntersectionsData& intersections_data_){
    std::vector<glm::vec3> positions;

    auto base_top
            = intersections_data_.base_top_intersection->equally_distanced;
    auto base_bottom
            = intersections_data_.base_bottom_intersection->equally_distanced;

    positions.push_back(
            base_top->trace_points[base_top->trace_points.size()-1].point);
    positions.push_back(
            base_bottom->trace_points[0].point);

    return positions;
}

std::vector<glm::vec3>
FlatAroundIntersectionPath::CreateBaseDrillLeftTrajectory(
        BoxIntersectionsData& intersections_data_){
    std::vector<glm::vec3> positions;

    auto drill_left
            = intersections_data_.drill_left_intersection->equally_distanced;
    auto base_bottom
            = intersections_data_.base_bottom_intersection->equally_distanced;

    glm::vec2 indicies
            = GetClosestPointsIndices(base_bottom->trace_points,
                                      drill_left->trace_points,
                                      0, drill_left->trace_points.size());

    for(int i = 0; i < indicies.x; i++){
        positions.push_back(base_bottom->trace_points[i].point);
    }
    for(int i = indicies.y; i > 0; i--){
        positions.push_back(drill_left->trace_points[i].point);
    }

    return positions;
}

std::vector<glm::vec3>
FlatAroundIntersectionPath::CreateDrillLeftRightTrajectory(
        BoxIntersectionsData& intersections_data_){
    auto drill_left
            = intersections_data_.drill_left_intersection->equally_distanced;
    auto drill_right
            = intersections_data_.drill_right_intersection->equally_distanced;

    std::vector<glm::vec3> positions;

    positions.push_back(drill_left->trace_points[0].point);
    positions.push_back(
            drill_right->trace_points[drill_right->trace_points.size()-1].point);

    return positions;
}

std::vector<glm::vec3>
FlatAroundIntersectionPath::CreateDrillRightBaseBottomTrajectory(
        BoxIntersectionsData& intersections_data_){
    auto drill_right
            = intersections_data_.drill_right_intersection->equally_distanced;
    auto base_bottom
            = intersections_data_.base_bottom_intersection->equally_distanced;

    std::vector<glm::vec3> positions;
    glm::vec2 indicies
            = GetClosestPointsIndices(drill_right->trace_points,
                                      base_bottom->trace_points,
                                      0, base_bottom->trace_points.size());
    for(int i = drill_right->trace_points.size()-1; i > indicies.x; i--){
        positions.push_back(drill_right->trace_points[i].point);
    }
    for(int i = indicies.y; i < base_bottom->trace_points.size();i++){
        positions.push_back(base_bottom->trace_points[i].point);
    }

    return positions;
}

std::vector<glm::vec3>
FlatAroundIntersectionPath::CreateBaseBottomTopTrajectory(
        BoxIntersectionsData& intersections_data_){
    auto base_bottom
            = intersections_data_.base_bottom_intersection->equally_distanced;
    auto base_top
            = intersections_data_.base_top_intersection->equally_distanced;

    std::vector<glm::vec3> positions;

    positions.push_back(
            base_bottom->trace_points[base_bottom->trace_points.size()-1].point);
    positions.push_back(
            base_top->trace_points[0].point);

    return positions;
}

std::vector<glm::vec3> FlatAroundIntersectionPath::CreateInsideHandTrajectory(
        BoxIntersectionsData& intersections_data_){
    auto curve1
            = intersections_data_.hand_bottom_intersection->equally_distanced;
    auto curve2
            = intersections_data_.base_top_intersection->equally_distanced;

    std::vector<glm::vec3> positions;
    glm::vec2 indicies1
            = GetClosestPointsIndices(curve1->trace_points,
                                      curve2->trace_points,
                                      0, curve2->trace_points.size() / 2);

    glm::vec2 indicies2
            = GetClosestPointsIndices(curve1->trace_points,
                                      curve2->trace_points,
                                      curve2->trace_points.size() / 2,
                                      curve2->trace_points.size());

    for(int i = indicies1.x;  i > indicies2.x; i--){
        positions.push_back(curve1->trace_points[i].point);
    }
    for(int i = indicies2.y; i > indicies1.y ;i--){
        positions.push_back(curve2->trace_points[i].point);
    }





    auto curve11
            = intersections_data_.hand_bottom_intersection->double_equally_distanced;
    auto curve22
            = intersections_data_.base_top_intersection->double_equally_distanced;

    glm::vec2 indicies11
            = GetClosestPointsIndices(curve11->trace_points,
                                      curve22->trace_points,
                                      0, curve22->trace_points.size() / 2);

    glm::vec2 indicies22
            = GetClosestPointsIndices(curve11->trace_points,
                                      curve22->trace_points,
                                      curve22->trace_points.size() / 2,
                                      curve22->trace_points.size());

    for(int i = indicies11.x;  i > indicies22.x; i--){
        positions.push_back(curve11->trace_points[i].point);
    }
    for(int i = indicies22.y; i > indicies11.y ;i--){
        positions.push_back(curve22->trace_points[i].point);
    }

    for(unsigned int i = 0; i < positions.size(); i++){
        // Avoid numerical errors, round up to e.g. 20mm
        positions[i].y
                = MillimetersToGL(
                material_box_->dimensions().depth -
                material_box_->dimensions().max_depth);
    }

    return positions;
}

glm::vec2 FlatAroundIntersectionPath::GetClosestPointsIndices(
        std::vector<TracePoint>& trace_points1,
        std::vector<TracePoint>& trace_points2,
        int start2, int finish2){
    glm::vec2 smallest_indices;
    float smallest_distance = 9999.0f;
    smallest_indices.x = 0;
    smallest_indices.y = 0;

    for(int i = 0; i < trace_points1.size(); i++){
        for(int j = start2; j < finish2; j++){
            float distance = ifx::EuclideanDistance(trace_points1[i].point,
                                                    trace_points2[j].point);
            if(distance < smallest_distance){
                smallest_distance = distance;
                smallest_indices.x = i;
                smallest_indices.y = j;
            }
        }
    }
    return smallest_indices;
}

std::shared_ptr<Cutter> FlatAroundIntersectionPath::CreatePath(
        CutterTrajectory& trajectory){
    const float safety_adder = 15.0f;
    std::vector<Instruction> instructions;
    const float save_height = material_box_->dimensions().depth + safety_adder;
    const float start_height
            = material_box_->dimensions().depth
              -material_box_->dimensions().max_depth;
    int id = 0;
    glm::vec3 init_pos1 = glm::vec3(
            -material_box_->dimensions().x/2.0f - safety_adder,
            -material_box_->dimensions().z/2.0f - safety_adder,
            save_height);
    glm::vec3 init_pos2 = glm::vec3(
            -material_box_->dimensions().x/2.0f - safety_adder,
            -material_box_->dimensions().z/2.0f - safety_adder,
            start_height);

    instructions.push_back(Instruction(id++, init_pos1));
    instructions.push_back(Instruction(id++, init_pos2));
    for(unsigned int i = 0; i < trajectory.positions.size(); i++){
        instructions.push_back(Instruction(id++,
                                           GetInstructionPosition(
                                                   trajectory.positions[i])));
    }

    glm::vec3 last_pos = instructions[instructions.size()-1].position();
    last_pos.z = save_height;
    instructions.push_back(Instruction(id++, last_pos));

    return std::shared_ptr<Cutter>(new Cutter(CutterType::Flat,
                                              diameter_,
                                              instructions));
}

glm::vec3 FlatAroundIntersectionPath::GetInstructionPosition(
        const glm::vec3& v){
    glm::vec3 vi;
    vi.x = GLToMillimeters(v.x);
    vi.y = GLToMillimeters(v.z);
    vi.z = GLToMillimeters(v.y);
    return vi;
}

}