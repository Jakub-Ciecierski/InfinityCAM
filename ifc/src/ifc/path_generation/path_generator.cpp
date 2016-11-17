#include "ifc/path_generation/path_generator.h"

#include <ifc/path_generation/paths/roughing_path.h>
#include <ifc/path_generation/paths/flat_around_hm_path.h>
#include <ifc/path_generation/paths/flat_around_intersection_path.h>
#include <ifc/path_generation/paths/parametrization_path.h>

#include <ifc/path_generation/height_map_paths.h>
#include <ifc/material/material_box.h>
#include <ifc/material/height_map.h>
#include <ifc/measures.h>
#include <ifc/factory/cad_model_loader.h>
#include <ifc/cutter/instruction.h>
#include <object/render_object.h>
#include <infinity_cad/rendering/render_objects/surfaces/surface_c2_cylind.h>

#include <iostream>

namespace ifc{

PathGenerator::PathGenerator(
        std::shared_ptr<CADModelLoaderResult> model_loader_result,
        std::shared_ptr<MaterialBox> material_box,
        std::shared_ptr<ifx::Scene> scene) :
        model_loader_result_(model_loader_result),
        material_box_(material_box){
    roughing_path_.reset(new RoughingPath(model_loader_result_,
                                          material_box_));
    flat_around_hm_path_.reset(new FlatAroundHMPath(model_loader_result_,
                                                    material_box_));
    flat_around_intersection_path_.reset(
            new FlatAroundIntersectionPath(model_loader_result_,
                                           material_box_, scene));
    parametrization_path_.reset(
            new ParametrizationPath(model_loader_result_,
                                           material_box_, scene));
}

PathGenerator::~PathGenerator(){}

Paths PathGenerator::GenerateAll(){
    auto height_map_path = GenerateRequirements();

    Paths paths;

    paths.rough_cutter = roughing_path_->Generate(height_map_path);
    paths.flat_heighmap_cutter
            = flat_around_hm_path_->Generate(height_map_path);
    paths.flat_intersection_cutter = flat_around_intersection_path_->Generate();
    paths.parametrization_cutter = parametrization_path_->Generate();

    return paths;
}

std::shared_ptr<Cutter> PathGenerator::GenerateRoughingPath(){
    auto height_map_path = GenerateRequirements();

    return roughing_path_->Generate(height_map_path);
}

std::shared_ptr<Cutter> PathGenerator::GenerateFlatHeightmapPath(){
    auto height_map_path = GenerateRequirements();

    return flat_around_hm_path_->Generate(height_map_path);
}

std::shared_ptr<Cutter> PathGenerator::GenerateFlatIntersectionPath(){
    return flat_around_intersection_path_->Generate();
}

std::shared_ptr<Cutter> PathGenerator::GenerateParametrizationPath(){
    return parametrization_path_->Generate();
}

std::shared_ptr<HeightMapPath> PathGenerator::GenerateRequirements(){
    std::cout << std::endl;
    std::cout << "0.1) Generating Sample Points" << std::endl;
    auto points = GenerateSamplePoints(model_loader_result_);

    std::cout
    << "Generated: " << points.size() << " sample points"
    << std::endl;

    std::cout << std::endl;
    std::cout << "0.2) Generating Heigtmap" << std::endl;
    return GenerateHeightMap(points, material_box_);
}

std::vector<glm::vec3> PathGenerator::GenerateSamplePoints(
        std::shared_ptr<CADModelLoaderResult> model_loader_result){
    std::vector<glm::vec3> points;
    for(unsigned int i = 0;
        i < model_loader_result->cad_model->surfaces.size(); i++){
        std::cout << "Sampling Surface[" << i << "]" << std::endl;
        std::vector<glm::vec3> surface_points
                = GenerateSamplePoints(
                        model_loader_result->cad_model->surfaces[i],
                        model_loader_result->cad_model->render_object
                                ->GetModelMatrix());
        points.insert(points.end(),
                      surface_points.begin(),
                      surface_points.end());
    }
    return points;
}

std::vector<glm::vec3> PathGenerator::GenerateSamplePoints(
        std::shared_ptr<SurfaceC2Cylind> surface,
        const glm::mat4& model_matrix){
    std::vector<glm::vec3> points;

    // should be based on material_box.height_map precision
    // and number of patches in respective direction.
    int n = surface->GetBicubicBezierPatches().rowCount();
    int m = surface->GetBicubicBezierPatches().columnCount();
    int count = n*m;
    float scale = 1.0f;
    const float du = 0.06 * 1.0f/(float)m * scale;
    const float dv = 0.06 * 1.0f/(float)n * scale;
    int points_count = 1.0f / du * 1.0f / dv;

    std::cout << "[n,m]: " << n << ", " << m << std::endl;
    std::cout << "[du,dv]: " << du << ", " << dv << std::endl;
    std::cout << "Estimated Point count: " << points_count << std::endl;
    points.resize(points_count);
    int i = 0;
    for(float u = 0.0f; u < 1.0f; u+=du){
        for(float v = 0.0f; v < 1.0f; v+=dv){
            glm::vec3 surface_point = surface->compute(u,v);
            if(i >= points.size())
                points.push_back(surface_point);
            else
                points[i] = surface_point;
            i++;
        }
    }
    return points;
}

std::shared_ptr<HeightMapPath> PathGenerator::GenerateHeightMap(
        const std::vector<glm::vec3>& cad_model_points,
        std::shared_ptr<MaterialBox> material_box){
    int width = material_box->height_map()->texture_data()->width;
    int height = material_box->height_map()->texture_data()->height;
    int size = width * height;
    std::vector<glm::vec2>& material_box_positions
            = material_box->height_map()->positions();

    std::vector<float> heights;
    heights.resize(size);
    float init_height = MillimetersToGL(material_box->dimensions().depth -
                                        material_box->dimensions().max_depth);
    for(unsigned int i = 0; i < heights.size(); i++){
        heights[i] = init_height;
    }

    auto height_map_path = std::shared_ptr<HeightMapPath>(
            new HeightMapPath(heights, material_box_positions,
                              width, height,
                              material_box->dimensions().x,
                              material_box->dimensions().z,
                              init_height)
    );

    // should be based on material_box dimensions/precision
    float error_distance = 0.01f;
    for(int i = 0; i < size; i++){
        int min_index = IndexOfClostestPoint(
                height_map_path->positions[i], cad_model_points,
                error_distance, init_height);
        if(min_index != -1)
            height_map_path->heights[i] = cad_model_points[min_index].y;
    }

    return height_map_path;
}

int PathGenerator::IndexOfClostestPoint(const glm::vec2& v,
                                       const std::vector<glm::vec3>& positions,
                                       float error_distance,
                                       float init_height){
    float min = ifx::EuclideanDistance(v,
                                       glm::vec2(positions[0].x,
                                                 positions[0].z));
    int min_index = 0;
    for(unsigned int i = 1; i < positions.size(); i++){
        if (positions[i].y < init_height)
            continue;
        float distance = ifx::EuclideanDistance(v,
                                                glm::vec2(positions[i].x,
                                                          positions[i].z));
        if(distance < min){
            min = distance;
            min_index = i;
        }
    }
    if(min > error_distance)
        return -1;
    return min_index;
}

}