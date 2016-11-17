#ifndef PROJECT_CAD_MODEL_LOADER_H
#define PROJECT_CAD_MODEL_LOADER_H

#include <string>
#include <memory>
#include <vector>

class BicubicBezierPatch;
class SurfaceC2Cylind;
class SurfaceC2Rect;
class Program;

template <class T>
class Matrix;

namespace ifx{
class Model;
class Mesh;
class RenderObject;
}

namespace ifc {

struct CADModel{
    std::vector<std::shared_ptr<SurfaceC2Cylind>> surfaces;
    std::shared_ptr<ifx::RenderObject> render_object;
};

struct CADIntersectionRectangle{
    std::shared_ptr<SurfaceC2Rect> surface;
    std::shared_ptr<ifx::RenderObject> render_object;
};

struct CADModelLoaderResult{
    std::shared_ptr<CADModel> cad_model;
    std::shared_ptr<CADIntersectionRectangle> interection_model;
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
            Matrix<BicubicBezierPatch*>& patches, int id);
    std::unique_ptr<ifx::Mesh> CreateMesh(
            const BicubicBezierPatch* patch, int id_i, int id_j,
            int n, int m);
    std::shared_ptr<Program> LoadProgram();
    void AdjustTransform(std::shared_ptr<ifx::RenderObject> object,
                         std::vector<std::shared_ptr<SurfaceC2Cylind>>& surfaces);

    std::shared_ptr<CADIntersectionRectangle> CreateIntersectionRectangle();
    void AdjustTransformIntersectionRenagle(
            std::shared_ptr<ifx::RenderObject> object,
            std::shared_ptr<SurfaceC2Rect>& surface);

    std::string path_;
};

}

#endif //PROJECT_CAD_MODEL_LOADER_H