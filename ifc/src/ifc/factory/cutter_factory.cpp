#include "ifc/factory/cutter_factory.h"

#include <ifc/cutter/cutter_loader.h>
#include <factory/mesh_factory.h>
#include <shaders/textures/texture_loader.h>
#include <shaders/textures/texture.h>
#include <ifc/measures.h>
#include <factory/program_factory.h>

namespace ifc {

CutterFactory::CutterFactory(){}

CutterFactory::~CutterFactory(){}

std::shared_ptr<Cutter> CutterFactory::CreateCutter(std::string filepath){
    CutterLoader loader(filepath);
    auto cutter = loader.Load();
    if(!cutter)
        return cutter;

    if(cutter->type() == CutterType::Sphere){
        cutter->render_object(CreateSphereCutterRenderObject(
                cutter->diameter()));
    }else if(cutter->type() == CutterType::Flat){
        cutter->render_object(CreateFlatCutterRenderObject(
                cutter->diameter()));
    }
    return cutter;
}

std::shared_ptr<ifx::RenderObject> CutterFactory::CreateSphereCutterRenderObject(
        float diameter){
    std::cout << "Creating Sphere Cutter Render Object" << std::endl;

    auto handle_mesh = ifx::MeshFactory::LoadSphere(0.5f);
    auto blade_mesh = ifx::MeshFactory::LoadSphere(0.5f);

    std::vector<std::shared_ptr<ifx::Texture2D>> blade_textures
            =  CreateCutterBladeTextures();
    for(int i = 0; i < blade_textures.size(); i++)
        blade_mesh->AddTexture(blade_textures[i]);

    std::vector<std::unique_ptr<ifx::Mesh>> meshes;
    //meshes.push_back(std::move(handle_mesh));
    meshes.push_back(std::move(blade_mesh));

    auto model = ifx::Model::MakeModel("SphereCutterModel", std::move(meshes));
    auto render_object
            = std::shared_ptr<ifx::RenderObject>(
                    new ifx::RenderObject(ObjectID(0, "Cutter"), model));
    render_object->scale(MillimetersToGL(diameter));

    render_object->addProgram(ifx::ProgramFactory().LoadMainProgram());
    return render_object;
}

std::shared_ptr<ifx::RenderObject> CutterFactory::CreateFlatCutterRenderObject(
        float diameter){
    std::cout << "Creating Flat Cutter Render Object" << std::endl;

    auto handle_mesh = ifx::MeshFactory::LoadCubeUnTextured();
    auto blade_mesh = ifx::MeshFactory::LoadCubeUnTextured();

    std::vector<std::shared_ptr<ifx::Texture2D>> blade_textures
            =  CreateCutterBladeTextures();
    for(int i = 0; i < blade_textures.size(); i++)
        blade_mesh->AddTexture(blade_textures[i]);

    std::vector<std::unique_ptr<ifx::Mesh>> meshes;
    //meshes.push_back(std::move(handle_mesh));
    meshes.push_back(std::move(blade_mesh));

    auto model = ifx::Model::MakeModel("FlatCutterModel", std::move(meshes));
    auto render_object
            = std::shared_ptr<ifx::RenderObject>(
                    new ifx::RenderObject(ObjectID(0, "Cutter"), model));
    render_object->scale(MillimetersToGL(diameter));

    render_object->addProgram(ifx::ProgramFactory().LoadMainProgram());
    return render_object;
}

std::vector<std::shared_ptr<ifx::Texture2D>>
        CutterFactory::CreateCutterBladeTextures(){
    ifx::Resources &resources = ifx::Resources::GetInstance();
    auto texture_diffuse = ifx::Texture2D::MakeTexture2DFromFile(
            ifx::Resources::GetInstance().GetResourcePath(
                    "cam/cutter_blade_diff.jpg",
                    ifx::ResourceType::TEXTURE),
            ifx::TextureTypes::DIFFUSE
    );
    auto texture_specular = ifx::Texture2D::MakeTexture2DFromFile(
            ifx::Resources::GetInstance().GetResourcePath(
                    "cam/cutter_blade_spec.jpg",
                    ifx::ResourceType::TEXTURE),
            ifx::TextureTypes::SPECULAR
    );
    return std::vector<std::shared_ptr<ifx::Texture2D>>{
            texture_diffuse, texture_specular};
}

}