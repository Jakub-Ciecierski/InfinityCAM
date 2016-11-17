#ifndef PROJECT_FLAT_AROUND_HM_PATH_H
#define PROJECT_FLAT_AROUND_HM_PATH_H

#include <math/math_ifx.h>

#include <memory>
#include <vector>

namespace ifc {

struct CADModelLoaderResult;
struct HeightMapPath;
class MaterialBox;
class Cutter;
class Instruction;

/**
 * Flat around height map path.
 */
class FlatAroundHMPath {
public:

    FlatAroundHMPath(std::shared_ptr<CADModelLoaderResult> model_loader_result,
                     std::shared_ptr<MaterialBox> material_box);
    ~FlatAroundHMPath();

    std::shared_ptr<Cutter> Generate(std::shared_ptr<HeightMapPath> height_map);
private:
    std::shared_ptr<Cutter> CreatePath(
            std::shared_ptr<HeightMapPath> height_map_path,
            std::shared_ptr<MaterialBox> material_box);

    std::vector<Instruction> CreatePathFirstHalf(
            std::shared_ptr<HeightMapPath> height_map_path,
            float save_height, float start_height, float radius,
            int n, int m,
            int skip_rows, int skip_columns,
            int look_ahead_radius_row, int look_ahead_radius_column);
    std::vector<Instruction> CreatePathSecondHalf(
            std::shared_ptr<HeightMapPath> height_map_path,
            float save_height, float start_height, float radius,
            int n, int m,
            int skip_rows, int skip_columns,
            int look_ahead_radius_row, int look_ahead_radius_column);

    bool ShouldGoBack(int i, int j, int n, int m,
                      int look_ahead_radius_row,
                      int look_ahead_radius_column,
                      std::shared_ptr<HeightMapPath> height_map_path);


    Instruction CreateInstruction(int id, const glm::vec2& v, float height);

    std::shared_ptr<CADModelLoaderResult> model_loader_result_;
    std::shared_ptr<MaterialBox> material_box_;

    const float diameter_ = 10.0f;
    const float radius_ = diameter_ / 2.0f;
};
}

#endif //PROJECT_FLAT_AROUND_HM_PATH_H
