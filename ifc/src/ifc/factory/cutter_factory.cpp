#include "ifc/factory/cutter_factory.h"

#include <ifc/cutter/cutter_loader.h>
#include <factory/mesh_factory.h>
#include <shaders/textures/texture_loader.h>
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

std::shared_ptr<RenderObject> CutterFactory::CreateSphereCutterRenderObject(
        float diameter){
    std::cout << "Creating Sphere Cutter Render Object" << std::endl;

    auto handle_mesh = ifx::MeshFactory::LoadSphere(0.5f);
    auto blade_mesh = ifx::MeshFactory::LoadSphere(0.5f);

    std::vector<Texture> handle_textures = CreateCutterHandleTextures();
    std::vector<Texture> blade_textures = CreateCutterBladeTextures();

    for(int i = 0; i < handle_textures.size(); i++)
        handle_mesh->addTexture(handle_textures[i]);
    for(int i = 0; i < blade_textures.size(); i++)
        blade_mesh->addTexture(blade_textures[i]);

    std::vector<std::unique_ptr<Mesh>> meshes;
    //meshes.push_back(std::move(handle_mesh));
    meshes.push_back(std::move(blade_mesh));

    auto model = Model::MakeModel("SphereCutterModel", std::move(meshes));
    auto render_object
            = std::shared_ptr<RenderObject>(
                    new RenderObject(ObjectID(0), model));
    render_object->scale(MillimetersToGL(diameter));

    render_object->addProgram(ifx::ProgramFactory().LoadMainProgram());
    return render_object;
}

std::shared_ptr<RenderObject> CutterFactory::CreateFlatCutterRenderObject(
        float diameter){
    std::cout << "Creating Flat Cutter Render Object" << std::endl;

    auto handle_mesh = ifx::MeshFactory::LoadCubeUnTextured();
    auto blade_mesh = ifx::MeshFactory::LoadCubeUnTextured();

    std::vector<Texture> handle_textures = CreateCutterHandleTextures();
    std::vector<Texture> blade_textures = CreateCutterBladeTextures();

    for(int i = 0; i < handle_textures.size(); i++)
        handle_mesh->addTexture(handle_textures[i]);
    for(int i = 0; i < blade_textures.size(); i++)
        blade_mesh->addTexture(blade_textures[i]);

    std::vector<std::unique_ptr<Mesh>> meshes;
    //meshes.push_back(std::move(handle_mesh));
    meshes.push_back(std::move(blade_mesh));

    auto model = Model::MakeModel("FlatCutterModel", std::move(meshes));
    auto render_object
            = std::shared_ptr<RenderObject>(
                    new RenderObject(ObjectID(0), model));
    render_object->scale(MillimetersToGL(diameter));

    render_object->addProgram(ifx::ProgramFactory().LoadMainProgram());
    return render_object;
}

std::vector<Texture> CutterFactory::CreateCutterHandleTextures(){
    ifx::Resources &resources = ifx::Resources::GetInstance();
    Texture texture_diffuse
            = TextureLoader().loadTexture(
                    resources.GetResourcePath("cam/cutter_handle.jpg",
                                              ifx::ResourceType::TEXTURE),
                    TextureTypes::DIFFUSE);
    Texture texture_specular
            = TextureLoader().loadTexture(
                    resources.GetResourcePath("cam/cutter_handle.jpg",
                                              ifx::ResourceType::TEXTURE),
                    TextureTypes::SPECULAR);
    return std::vector<Texture>{texture_diffuse,
                                texture_specular};
}

std::vector<Texture> CutterFactory::CreateCutterBladeTextures(){
    ifx::Resources &resources = ifx::Resources::GetInstance();
    Texture texture_diffuse
            = TextureLoader().loadTexture(
                    resources.GetResourcePath("cam/cutter_blade.jpg",
                                              ifx::ResourceType::TEXTURE),
                    TextureTypes::DIFFUSE);
    Texture texture_specular
            = TextureLoader().loadTexture(
                    resources.GetResourcePath("cam/cutter_blade.jpg",
                                              ifx::ResourceType::TEXTURE),
                    TextureTypes::SPECULAR);
    return std::vector<Texture>{texture_diffuse, texture_specular};
}

}