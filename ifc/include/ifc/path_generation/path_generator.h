#ifndef PROJECT_PATH_GENERATOR_H
#define PROJECT_PATH_GENERATOR_H

#include <ifc/factory/cad_model_loader.h>
#include <math/math_ifx.h>

#include <iostream>
#include <memory>

class SurfaceC2Cylind;

namespace ifc {

struct HeightMapPath;
class Cutter;
class MaterialBox;
class RoughingPath;
struct CADModelLoaderResult;

struct Paths{
    std::shared_ptr<Cutter> rough_cutter;
};

class PathGenerator {
public:

    PathGenerator(std::shared_ptr<CADModelLoaderResult> result,
                  std::shared_ptr<MaterialBox> material_box);
    ~PathGenerator();

    Paths GenerateAll();
    std::shared_ptr<Cutter> GenerateRoughingPath();

private:
    std::shared_ptr<HeightMapPath> GenerateRequirements();

    std::vector<glm::vec3> GenerateSamplePoints(
            std::shared_ptr<CADModelLoaderResult> model_loader_result);
    std::vector<glm::vec3> GenerateSamplePoints(
            std::shared_ptr<SurfaceC2Cylind> surface,
            const glm::mat4& model_matrix);
    /**
     * Assumes that height is stored in y component of surface points.
     *
     * Assumes that model is symmetric around
     * (depth - max_depth (e.g. 20mm counting from bottom)) of material box.
     *
     */
    std::shared_ptr<HeightMapPath> GenerateHeightMap(
            const std::vector<glm::vec3>& cad_model_points,
            std::shared_ptr<MaterialBox> material_box_);

    int IndexOfClostestPoint(const glm::vec2& v,
                             const std::vector<glm::vec3>& positions,
                             float error_distance,
                             float init_height);

    std::shared_ptr<CADModelLoaderResult> model_loader_result_;
    std::shared_ptr<MaterialBox> material_box_;

    // Step 1
    std::shared_ptr<RoughingPath> roughing_path_;
};
}

#endif //PROJECT_PATH_GENERATOR_H
