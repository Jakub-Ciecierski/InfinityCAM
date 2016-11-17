#ifndef PROJECT_PARAMETRIZATION_PATH_H
#define PROJECT_PARAMETRIZATION_PATH_H

#include <math/math_ifx.h>
#include <memory>
#include <vector>

class SurfaceC2Cylind;
struct TracePoint;

namespace ifx{
class Scene;
class RenderObject;
};

namespace ifc {

class Cutter;
class MaterialBox;
class Instruction;
struct CADModelLoaderResult;

struct IntersectionData{
    std::vector<TracePoint> trace_points;
    std::shared_ptr<SurfaceC2Cylind> surface;
    std::shared_ptr<ifx::RenderObject> render_object;
};

struct IntersectionsData{
    std::shared_ptr<IntersectionData> base_hand_right_;
    std::shared_ptr<IntersectionData> base_hand_left_;
    std::shared_ptr<IntersectionData> base_drill_;
};

class ParametrizationPath {
public:

    ParametrizationPath(
            std::shared_ptr<CADModelLoaderResult> model_loader_result,
            std::shared_ptr<MaterialBox> material_box,
            std::shared_ptr<ifx::Scene> scene);
    ~ParametrizationPath();

    std::shared_ptr<Cutter> Generate();
private:
    void ComputeIntersections();
    std::vector<TracePoint> ComputeIntersection(
            std::shared_ptr<SurfaceC2Cylind> surface1,
            std::shared_ptr<SurfaceC2Cylind> surface2,
            glm::vec3 start_pos);

    std::shared_ptr<IntersectionData> ComputeBaseHandRightIntersection();
    std::shared_ptr<IntersectionData> ComputeBaseHandLeftIntersection();
    std::shared_ptr<IntersectionData> ComputeBaseDrillIntersection();

    std::shared_ptr<Cutter> CreatePath();

    std::vector<glm::vec3> CreateBaseTrajectory();
    std::vector<glm::vec3> CreateHandTrajectory();
    std::vector<glm::vec3> CreateDrillTrajectory();

    std::vector<Instruction> CreateBaseIntructions();
    std::vector<Instruction> CreateHandIntructions();
    std::vector<Instruction> CreateDrillIntructions();


    std::shared_ptr<ifx::RenderObject> CreateRenderObject(
            const std::vector<TracePoint>& trace_points,
            std::string name);

    glm::vec3 GetInstructionPosition(const glm::vec3& v);

    const float diameter_ = 8;
    const float radius_ = diameter_ / 2.0f;
    std::shared_ptr<CADModelLoaderResult> model_loader_result_;
    std::shared_ptr<MaterialBox> material_box_;
    std::shared_ptr<ifx::Scene> scene_;

    IntersectionsData intersections_data_;
};
}

#endif //PROJECT_PARAMETRIZATION_PATH_H
