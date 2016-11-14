#ifndef PROJECT_ROUGHING_PATH_H
#define PROJECT_ROUGHING_PATH_H

#include <math/math_ifx.h>
#include <ifc/factory/cad_model_loader.h>

#include <memory>
#include <iostream>

class SurfaceC2Cylind;

namespace ifx{
class Texture2D;
}

namespace ifc {

struct CADModel;
struct HeightMapPath;
class MaterialBox;
class Cutter;
class Instruction;

class RoughingPath {
public:

    RoughingPath(std::shared_ptr<CADModelLoaderResult> model_loader_result,
                 std::shared_ptr<MaterialBox> material_box);
    ~RoughingPath();

    std::shared_ptr<Cutter> Generate(std::shared_ptr<HeightMapPath>);

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

    std::vector<Instruction> CreatePathFromAbove(
            std::shared_ptr<HeightMapPath> height_map_path,
            float save_height, float start_height, float radius,
            int n, int m,
            int skip_rows, int skip_columns,
            int look_ahead_radius_row, int look_ahead_radius_column);

    bool ShouldGoBack(int i, int j, int n, int m,
                      int look_ahead_radius_row,
                      int look_ahead_radius_column,
                      std::shared_ptr<HeightMapPath> height_map_path);

    float MaxHeightInVicinity(int i, int j, int n, int m,
                              int look_ahead_radius_row,
                              int look_ahead_radius_column,
                              std::shared_ptr<HeightMapPath> height_map_path);

    Instruction CreateInstruction(int id, const glm::vec2& v, float height);

    void DEBUG_AddHeightMapTexture(
            std::shared_ptr<HeightMapPath>,
            std::shared_ptr<MaterialBox> material_box_);

    std::shared_ptr<CADModelLoaderResult> model_loader_result_;
    std::shared_ptr<MaterialBox> material_box_;
    std::vector<glm::vec3> model_sample_points_;

    std::shared_ptr<ifx::Texture2D> debug_texture_;
};
}

#endif //PROJECT_ROUGHING_PATH_H
