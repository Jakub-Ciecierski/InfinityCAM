#ifndef PROJECT_FLAT_AROUND_INTERSECTION_PATH_H
#define PROJECT_FLAT_AROUND_INTERSECTION_PATH_H

#include <math/math_ifx.h>

#include <memory>
#include <vector>
#include <string>

class SurfaceC2Cylind;
class SurfaceC2Rect;
struct TracePoint;

namespace ifx{
class RenderObject;
class Scene;
}

namespace ifc {

struct CADModelLoaderResult;

class MaterialBox;
class Cutter;
class Instruction;

struct BoxIntersectionData{
    std::vector<TracePoint> trace_points;
    std::shared_ptr<SurfaceC2Cylind> surface;
    std::shared_ptr<ifx::RenderObject> render_object;

    std::shared_ptr<BoxIntersectionData> equally_distanced;
    std::shared_ptr<BoxIntersectionData> double_equally_distanced;
};

struct CutterTrajectory{
    std::vector<glm::vec3> positions;
};

struct BoxIntersectionsData{
    std::shared_ptr<BoxIntersectionData> base_top_intersection;
    std::shared_ptr<BoxIntersectionData> base_bottom_intersection;
    std::shared_ptr<BoxIntersectionData> drill_left_intersection;
    std::shared_ptr<BoxIntersectionData> drill_right_intersection;
    std::shared_ptr<BoxIntersectionData> hand_top_intersection;
    std::shared_ptr<BoxIntersectionData> hand_bottom_intersection;
};

enum class NormalDirection{
    UP, DOWN
};

/**
 * Flat around intersection: Surface vs Material box surface.
 * Flat 10mm.
 */
class FlatAroundIntersectionPath {
public:

    FlatAroundIntersectionPath(
            std::shared_ptr<CADModelLoaderResult> model_loader_result,
            std::shared_ptr<MaterialBox> material_box,
            std::shared_ptr<ifx::Scene> scene);
    ~FlatAroundIntersectionPath();

    BoxIntersectionsData* intersections_data() {return &intersections_data_;};
    std::vector<glm::vec3>& inside_hand_positions() {
        return inside_hand_positions_;};
    bool generated() {return generated_;}

    std::shared_ptr<Cutter> Generate();
private:
    void ComputeIntersections();
    std::vector<TracePoint> ComputeIntersection(
            std::shared_ptr<SurfaceC2Cylind> surface1,
            std::shared_ptr<SurfaceC2Rect> surface2,
            glm::vec3 start_pos);

    /**
     * Compute all needed intersections.
     * They differ in taking different surface and starting point
     */
    std::shared_ptr<BoxIntersectionData> ComputeBaseTopIntersection(
            NormalDirection normal_direction);
    std::shared_ptr<BoxIntersectionData> ComputeBaseBottomIntersection(
            NormalDirection normal_direction);

    std::shared_ptr<BoxIntersectionData> ComputeDrillLeftIntersection(
            NormalDirection normal_direction);
    std::shared_ptr<BoxIntersectionData> ComputeDrillRightIntersection(
            NormalDirection normal_direction);

    std::shared_ptr<BoxIntersectionData> ComputeHandTopIntersection(
            NormalDirection normal_direction);
    std::shared_ptr<BoxIntersectionData> ComputeHandBottomIntersection(
            NormalDirection normal_direction);

    std::shared_ptr<BoxIntersectionData>
            ComputeEqualDistancedIntersections(
            std::shared_ptr<BoxIntersectionData> intersection_data,
            NormalDirection normal_direction,
            float distance_scalar = 1.0f);

    std::shared_ptr<ifx::RenderObject> CreateRenderObject(
            const std::vector<TracePoint>& trace_points,
            std::string name);

    CutterTrajectory CreateTrajectory(
            BoxIntersectionsData& intersections_data_);
    std::vector<glm::vec3> CreateBaseHandTrajectory(
            BoxIntersectionsData& intersections_data_);
    std::vector<glm::vec3> CreateBaseTopBottomTrajectory(
            BoxIntersectionsData& intersections_data_);
    std::vector<glm::vec3> CreateBaseDrillLeftTrajectory(
            BoxIntersectionsData& intersections_data_);
    std::vector<glm::vec3> CreateDrillLeftRightTrajectory(
            BoxIntersectionsData& intersections_data_);
    std::vector<glm::vec3> CreateDrillRightBaseBottomTrajectory(
            BoxIntersectionsData& intersections_data_);
    std::vector<glm::vec3> CreateBaseBottomTopTrajectory(
            BoxIntersectionsData& intersections_data_);

    std::vector<glm::vec3> CreateInsideHandTrajectory(
            BoxIntersectionsData& intersections_data_);

    glm::vec2 GetClosestPointsIndices(
            std::vector<TracePoint>& trace_points1,
            std::vector<TracePoint>& trace_points2,
            int start2, int finish2);

    std::shared_ptr<Cutter> CreatePath(CutterTrajectory& trajectory);

    glm::vec3 GetInstructionPosition(const glm::vec3& v);

    std::shared_ptr<CADModelLoaderResult> model_loader_result_;
    std::shared_ptr<MaterialBox> material_box_;

    std::shared_ptr<ifx::Scene> scene_;

    BoxIntersectionsData intersections_data_;

    const float diameter_ = 10.0f;
    const float radius_ = diameter_ / 2.0f;

    bool generated_;

    std::vector<glm::vec3> inside_hand_positions_;
};
}

#endif //PROJECT_FLAT_AROUND_INTERSECTION_PATH_H
