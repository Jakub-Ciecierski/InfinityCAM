#include "ifc/factory/cad_model_loader.h"

#include <object/render_object.h>
#include <infinity_cad/serialization/deserialization_scene.h>
#include <model/patch/patch.h>
#include <factory/texture_factory.h>
#include <factory/program_factory.h>
#include <shaders/textures/texture_loader.h>
#include <shaders/loaders/program_loader.h>

namespace ifc {

CADModelLoader::CADModelLoader(){

}

CADModelLoader::~CADModelLoader(){

}

std::shared_ptr<ifx::RenderObject> CADModelLoader::Load(std::string path){
    path_ = path;
    DeserializationScene deserialization;
    deserialization.load(path);

    return CreateRenderObject(deserialization.surfaces());
}

std::shared_ptr<ifx::RenderObject> CADModelLoader::CreateRenderObject(
        std::vector<std::shared_ptr<SurfaceC2Cylind>> surfaces){
    std::vector<std::shared_ptr<ifx::Model>> models;
    for(unsigned int i = 0;i < surfaces.size();i++){
        models.push_back(CreateModel(surfaces[i], i));
    }

    auto object = std::shared_ptr<ifx::RenderObject>(new ifx::RenderObject(
            ObjectID(1, "CAD Model"), models
    ));
    object->addProgram(LoadProgram());

    AdjustTransform(object);

    return object;
}

std::shared_ptr<ifx::Model> CADModelLoader::CreateModel(
std::shared_ptr<SurfaceC2Cylind> surface, int id){
    Matrix<BicubicBezierPatch*>& patches =
            surface->GetBicubicBezierPatches();
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
                           2.0f, 2.0f, vertices.size(),
                           id_i, id_j, n, m));
    mesh->AddTexture(
            ifx::Texture2D::MakeTexture2DFromFile(
                    ifx::Resources::GetInstance().GetResourcePath(
                            "cam/box1_diff.png",
                            ifx::ResourceType::TEXTURE),
                    ifx::TextureTypes::DIFFUSE));
    mesh->AddTexture(
            ifx::Texture2D::MakeTexture2DFromFile(
                    ifx::Resources::GetInstance().GetResourcePath(
                            "cam/box1_spec.png",
                            ifx::ResourceType::TEXTURE),
                    ifx::TextureTypes::SPECULAR));
    mesh->setPolygonMode(GL_FILL);
    mesh->setPrimitiveMode(GL_PATCHES);

    Material material;
    material.shininess = 32.0f;
    mesh->setMaterial(material);

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
        std::shared_ptr<ifx::RenderObject> object){
    object->moveTo(glm::vec3(0.32, 0.182, 0.102));
    object->scale(glm::vec3(0.35, 0.35, 0.35));
    object->rotateTo(glm::vec3(0, 0, 90));
}

}