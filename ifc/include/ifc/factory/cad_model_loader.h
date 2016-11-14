#ifndef PROJECT_CAD_MODEL_LOADER_H
#define PROJECT_CAD_MODEL_LOADER_H

#include <string>
#include <memory>
#include <vector>

class BicubicBezierPatch;
class SurfaceC2Cylind;
class Program;

namespace ifx{
class Model;
class Mesh;
class RenderObject;
}

namespace ifc {

struct CADModel{
    std::vector<std::shared_ptr<SurfaceC2Cylind>> surfaces;
};

struct CADModelLoaderResult{
    std::shared_ptr<CADModel> cad_model;
    std::shared_ptr<ifx::RenderObject> render_object;
};

class CADModelLoader {
public:

    CADModelLoader();
    ~CADModelLoader();

    std::shared_ptr<CADModelLoaderResult> Load(std::string path);

private:
    std::shared_ptr<ifx::RenderObject> CreateRenderObject(
            std::vector<std::shared_ptr<SurfaceC2Cylind>> surfaces);

    std::shared_ptr<ifx::Model> CreateModel(
            std::shared_ptr<SurfaceC2Cylind> surface, int id);
    std::unique_ptr<ifx::Mesh> CreateMesh(
            const BicubicBezierPatch* patch, int id_i, int id_j,
            int n, int m);
    std::shared_ptr<Program> LoadProgram();
    void AdjustTransform(std::shared_ptr<ifx::RenderObject> object);

    std::string path_;
};

}

#endif //PROJECT_CAD_MODEL_LOADER_H