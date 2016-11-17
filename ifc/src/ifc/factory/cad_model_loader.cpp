#include "ifc/factory/cad_model_loader.h"

#include <object/render_object.h>
#include <infinity_cad/serialization/deserialization_scene.h>
#include <model/patch/patch.h>
#include <factory/texture_factory.h>
#include <factory/program_factory.h>
#include <shaders/loaders/program_loader.h>

namespace ifc {

CADModelLoader::CADModelLoader(){

}

CADModelLoader::~CADModelLoader(){

}

std::shared_ptr<CADModelLoaderResult> CADModelLoader::Load(std::string path){
    path_ = path;
    DeserializationScene deserialization;
    deserialization.load(path);

    auto result = std::make_shared<CADModelLoaderResult>();
    result->cad_model = std::shared_ptr<CADModel>(new CADModel());
    result->cad_model->surfaces = deserialization.surfaces();
    result->cad_model->render_object
            = CreateRenderObject(deserialization.surfaces());

    result->interection_model = CreateIntersectionRectangle();

    return result;
}

std::shared_ptr<ifx::RenderObject> CADModelLoader::CreateRenderObject(
        std::vector<std::shared_ptr<SurfaceC2Cylind>> surfaces){
    std::vector<std::shared_ptr<ifx::Model>> models;
    for(unsigned int i = 0;i < surfaces.size();i++){
        models.push_back(CreateModel(surfaces[i]->GetBicubicBezierPatches(),
                                     i));
    }

    auto object = std::shared_ptr<ifx::RenderObject>(new ifx::RenderObject(
            ObjectID(1, "CAD Model"), models
    ));
    object->do_render(false);
    object->addProgram(LoadProgram());

    AdjustTransform(object,
                    surfaces);

    return object;
}

std::shared_ptr<ifx::Model> CADModelLoader::CreateModel(
        Matrix<BicubicBezierPatch*>& patches, int id){
    int n = patches.rowCount();
    int m = patches.columnCount();
    std::vector<std::unique_ptr<ifx::Mesh>> meshes;
    meshes.resize(n*m);
    int l = 0;
    for(int i = 0; i < n;i++){
        for(int j = 0; j < m; j++){
            BicubicBezierPatch* patch = patches[i][j];
            meshes[l++] = std::move(CreateMesh(patch, i, j, n, m));
        }
    }
    std::string model_name = path_;
    model_name += std::to_string(id);
    auto model = ifx::Model::MakeModel(model_name, std::move(meshes));
    return model;
}

std::unique_ptr<ifx::Mesh> CADModelLoader::CreateMesh(
        const BicubicBezierPatch* patch, int id_i, int id_j, int n, int m){
    const int M = 4;
    std::vector<Vertex> vertices;
    vertices.resize(M*M);

    const glm::mat4& X = patch->getX();
    const glm::mat4& Y = patch->getY();
    const glm::mat4& Z = patch->getZ();

    int l = 0;
    for(int i = 0; i < M; i++){
        for(int j = 0; j < M; j++){
            vertices[l++]
                    = Vertex{glm::vec3(X[i][j], Y[i][j], Z[i][j]),
                             glm::vec3(0.0f, 0.0f, -1.0f),
                             glm::vec2(0.0f, 0.0f)};
        }
    }

    // Indices for Patch
    std::vector <GLuint> indices = {
            0, 1, 2, 3,
            4, 5, 6, 7,
            8, 9, 10, 11,
            12, 13, 14, 15
    };

    auto mesh = std::unique_ptr<ifx::Mesh>(
            new ifx::Patch(vertices, indices,
                           ifx::TesselationParams{
                           2.0f, 2.0f, vertices.size(),
                           id_i, id_j, n, m}));
    auto material = std::make_shared<ifx::Material>();
    material->AddTexture(
            ifx::Texture2D::MakeTexture2DFromFile(
                    ifx::Resources::GetInstance().GetResourcePath(
                            "cam/box1_diff.png",
                            ifx::ResourceType::TEXTURE),
                    ifx::TextureTypes::DIFFUSE));
    material->AddTexture(
            ifx::Texture2D::MakeTexture2DFromFile(
                    ifx::Resources::GetInstance().GetResourcePath(
                            "cam/box1_spec.png",
                            ifx::ResourceType::TEXTURE),
                    ifx::TextureTypes::SPECULAR));
    mesh->material(material);

    return mesh;
}

std::shared_ptr<Program> CADModelLoader::LoadProgram(){
    ifx::Resources& resources = ifx::Resources::GetInstance();
    std::string vertex_path =
            resources.GetResourcePath("cam/tessellation/tess.vs",
                                      ifx::ResourceType::SHADER);
    std::string fragment_path =
            resources.GetResourcePath("cam/tessellation/tess.fs",
                                      ifx::ResourceType::SHADER);
    std::string tcs_path =
            resources.GetResourcePath("cam/tessellation/tess.tcs",
                                      ifx::ResourceType::SHADER);
    std::string tes_path =
            resources.GetResourcePath("cam/tessellation/tess.tes",
                                      ifx::ResourceType::SHADER);

    return ProgramLoader().CreateProgram(vertex_path, fragment_path,
                                         tcs_path, tes_path);
}

void CADModelLoader::AdjustTransform(
        std::shared_ptr<ifx::RenderObject> object,
        std::vector<std::shared_ptr<SurfaceC2Cylind>>& surfaces){
    float move_x = 0.32;
    float move_y = 0.182;
    float move_z = 0.102;
    float scale = 0.35;
    float rotate_x = 0;
    float rotate_y = 0;
    float rotate_z = 90;

    object->moveTo(glm::vec3(0.32, 0.182, 0.102));
    object->scale(glm::vec3(0.35, 0.35, 0.35));
    object->rotateTo(glm::vec3(0, 0, 90));
    const glm::mat4& model_matrix = object->GetModelMatrix();

    for(auto& surface : surfaces){
        const Matrix<ifc::Point*>& points = surface->getMatrixPoints();
        int n = points.rowCount();
        int m = points.columnCount();
        for(int i = 0; i < n; i++){
            for(int j = 0; j < m ; j++){
                const glm::vec3& pos = points[i][j]->getPosition();
                glm::vec4 pos4 = glm::vec4(pos.x, pos.y, pos.z, 1.0f);
                pos4 = model_matrix * pos4;
                points[i][j]->moveTo(pos4.x, pos4.y, pos4.z);
            }
        }
    }
}

std::shared_ptr<CADIntersectionRectangle>
        CADModelLoader::CreateIntersectionRectangle(){
    auto cad_intersection_rectangle =
            std::make_shared<CADIntersectionRectangle>();

    cad_intersection_rectangle->surface
            = std::shared_ptr<SurfaceC2Rect>(new SurfaceC2Rect(SceneID(), "",
                                                               1, 1,
                                                               1.5f, 1.5f));
    auto model = CreateModel(
            cad_intersection_rectangle->surface->GetBicubicBezierPatches(),
            10);
    auto object = std::shared_ptr<ifx::RenderObject>(new ifx::RenderObject(
            ObjectID(10, "Inters Surface"), model));
    object->do_render(false);
    object->addProgram(LoadProgram());
    AdjustTransformIntersectionRenagle(object,
                                       cad_intersection_rectangle->surface);

    cad_intersection_rectangle->render_object = object;

    return cad_intersection_rectangle;
}

void CADModelLoader::AdjustTransformIntersectionRenagle(
        std::shared_ptr<ifx::RenderObject> object,
        std::shared_ptr<SurfaceC2Rect>& surface){
    object->moveTo(glm::vec3(-2.300, 0.180, -2.200));
    object->scale(glm::vec3(4.0, 4.0, 1.0));
    object->rotateTo(glm::vec3(90, 0, 0));

    const glm::mat4& model_matrix = object->GetModelMatrix();

    const Matrix<ifc::Point*>& points = surface->getMatrixPoints();
    int n = points.rowCount();
    int m = points.columnCount();
    for(int i = 0; i < n; i++){
        for(int j = 0; j < m ; j++){
            const glm::vec3& pos = points[i][j]->getPosition();
            glm::vec4 pos4 = glm::vec4(pos.x, pos.y, pos.z, 1.0f);
            pos4 = model_matrix * pos4;
            points[i][j]->moveTo(pos4.x, pos4.y, pos4.z);
        }
    }
}


}