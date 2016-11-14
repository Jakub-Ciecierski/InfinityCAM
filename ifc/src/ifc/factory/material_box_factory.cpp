#include "ifc/factory/material_box_factory.h"

#include <ifc/measures.h>
#include <factory/model_factory.h>
#include <factory/program_factory.h>
#include <ifc/material/height_map.h>

namespace ifc {

const std::string precision_x_uniform = "precision_x";
const std::string precision_z_uniform = "precision_z";

MaterialBoxFactory::MaterialBoxFactory(){

}

MaterialBoxFactory::~MaterialBoxFactory(){

}

std::shared_ptr<ifx::InstancedRenderObject>
MaterialBoxFactory::CreateTexelledBox(MaterialBoxPrecision precision,
                                      MaterialBoxDimensions dimensions,
                                      HeightMap* height_map){
    auto model = ifx::ModelFactory::LoadCAMMaterial();

    InstancedData data;
    data.data_count = precision.x * precision.z;
    data.model_matrices.resize(data.data_count);
    const float default_scale = 1.0f;

    float single_box_scale_x
            = MillimetersToGL(dimensions.x) / (float)precision.x;
    float single_box_scale_z
            = MillimetersToGL(dimensions.z) / (float)precision.z;
    float single_box_scale_y =
            MillimetersToGL(dimensions.depth);
    single_box_scale_x *= default_scale;
    single_box_scale_z *= default_scale;

    float dx = 1.0f * single_box_scale_x;
    float dz = 1.0f * single_box_scale_z;
    float x_translate = 0.0f;
    float z_translate = 0.0f;

    int i = 0;
    std::vector<glm::vec2> positions;
    positions.resize(data.data_count);
    for(int x = 0; x < precision.x; x++){
        z_translate = 0.0f;
        for(int z = 0; z < precision.z; z++){
            ifx::MovableObject model_object(ObjectID(0));
            model_object.move(glm::vec3(x_translate, 0.0f, z_translate));
            model_object.scale(glm::vec3(single_box_scale_x,
                                         single_box_scale_y,
                                         single_box_scale_z));

            model_object.move(
                    glm::vec3(-MillimetersToGL(dimensions.x / 2.0f),
                              0.0f,
                              -MillimetersToGL(dimensions.z / 2.0f)));

            positions[i] = glm::vec2(model_object.getPosition().x,
                                     model_object.getPosition().z);

            data.model_matrices[i] = model_object.GetModelMatrix();

            i++;
            z_translate += dz;
        }
        x_translate += dx;

    }
    std::vector<glm::vec2> reordered;
    int k = 0;
    for(int l = 0; l < precision.x; l++){
        for(int j = 0; j < precision.z; j++){
            int kk = j*precision.z + l;
            glm::vec2 p = positions[j*precision.z + l];
            reordered.push_back(p);
            k++;
        }

    }

    height_map->positions(reordered);
    auto instanced_render_object
            = std::shared_ptr<ifx::InstancedRenderObject>(
                    new ifx::InstancedRenderObject(ObjectID(0),
                                                   model, data));
    instanced_render_object->move(glm::vec3(2, 0, 2));
    // so that cutter moves properly
    //instanced_render_object->rotateTo(glm::vec3(0, -90, 0));
    /*
    instanced_render_object->move(
            glm::vec3(-MillimetersToGL(2*dimensions.x / 2.0f),
                      -MillimetersToGL(2*dimensions.depth/2.0f),
                      -MillimetersToGL(2*dimensions.z / 2.0f)));*/

    instanced_render_object->SetBeforeRender([precision, height_map](
            const Program* program){
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        glUniform1i(glGetUniformLocation(program->getID(),
                                         precision_x_uniform.c_str()),
                    precision.x);
        glUniform1i(glGetUniformLocation(program->getID(),
                                         precision_z_uniform.c_str()),
                    precision.z);

        glActiveTexture(GL_TEXTURE0);
        height_map->texture_data()->texture->Bind();
        glUniform1i(glGetUniformLocation(program->getID(),
                                         "height_map"),
                    0);
    });
    instanced_render_object->SetAfterRender([](const Program* program){
        glDisable(GL_CULL_FACE);
    });

    auto program = ifx::ProgramFactory().LoadCAMMaterialBoxProgram();
    instanced_render_object->addProgram(program);

    return instanced_render_object;
}

std::shared_ptr<ifx::RenderObject>
MaterialBoxFactory::CreateMaterialBoxRenderObject(
        MaterialBoxPrecision precision,
        MaterialBoxDimensions dimensions,
        HeightMap* height_map){
    auto renderObject
            = std::shared_ptr<ifx::RenderObject>(
                    new ifx::RenderObject(ObjectID(0, "Material Box"),
                                     ifx::ModelFactory::CreateQuad(
                                             precision.x,
                                             precision.z)));
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
    renderObject->models()[0]->getMesh(0)->material(material);

    float scaleFactorX = MillimetersToGL(dimensions.x);
    float scaleFactorY = MillimetersToGL((dimensions.x + dimensions.z) / 2.0f);
    float scaleFactorZ = MillimetersToGL(dimensions.z);
    renderObject->scale(glm::vec3(scaleFactorX,
                                  scaleFactorY,
                                  scaleFactorZ));
    renderObject->rotateTo(glm::vec3(90.0, 0.0f, 0));
    renderObject->moveTo(glm::vec3(-MillimetersToGL(dimensions.x/2.0f),
                                   0,
                                   -MillimetersToGL(dimensions.z/2.0f)));

    float single_box_scale_x
            = MillimetersToGL(dimensions.x) / (float)precision.x;
    float single_box_scale_z
            = MillimetersToGL(dimensions.z) / (float)precision.z;
    float single_box_scale_y =
            MillimetersToGL(dimensions.depth);

    float dx = 1.0f * single_box_scale_x;
    float dz = 1.0f * single_box_scale_z;
    float x_translate = 0.0f;
    float z_translate = 0.0f;
    const float const_single_box_scale_x
            = -MillimetersToGL(dimensions.x / 2.0f);
    const float const_single_box_scale_z
            = -MillimetersToGL(dimensions.z / 2.0f);
    PositionInfo position_info{single_box_scale_x, single_box_scale_z,
                               const_single_box_scale_x,
                               const_single_box_scale_z};
    height_map->position_info(position_info);

    int i = 0;
    std::vector<glm::vec2> positions;
    positions.resize(precision.x * precision.z);
    for(int x = 0; x < precision.x; x++){
        z_translate = 0.0f;
        for(int z = 0; z < precision.z; z++){
            ifx::MovableObject model_object(ObjectID(0));
            model_object.move(glm::vec3(x_translate, 0.0f, z_translate));
            model_object.move(
                    glm::vec3(const_single_box_scale_x,
                              0.0f,
                              const_single_box_scale_z));
            /*
            model_object.scale(glm::vec3(single_box_scale_x,
                                         single_box_scale_y,
                                         single_box_scale_z));*/



            positions[i] = glm::vec2(model_object.getPosition().x,
                                     model_object.getPosition().z);

            i++;
            z_translate += dz;
        }
        x_translate += dx;

    }
    std::vector<glm::vec2> reordered;
    int k = 0;
    for(int l = 0; l < precision.x; l++){
        for(int j = 0; j < precision.z; j++){
            int kk = j*precision.z + l;
            glm::vec2 p = positions[j*precision.z + l];
            reordered.push_back(p);
            k++;
        }

    }

    height_map->positions(reordered);
    renderObject->addProgram(
            ifx::ProgramFactory().LoadHeightmapProgram());

    renderObject->SetBeforeRender([height_map](
            const Program* program){
        glActiveTexture(GL_TEXTURE1);
        height_map->texture_data()->texture->Bind();
        glUniform1i(glGetUniformLocation(program->getID(),
                                         "height_map"), 1);
    });
    renderObject->SetAfterRender([height_map](const Program* program){
        height_map->texture_data()->texture->Unbind();
    });

    return renderObject;
}

std::shared_ptr<ifx::RenderObject> MaterialBoxFactory::CreatePlane(){
    auto renderObject
            = std::shared_ptr<ifx::RenderObject>(
                    new ifx::RenderObject(ObjectID(0, "Plane"),
                                     ifx::ModelFactory::LoadSquareModel()));
    float scaleFactor = 4.0f;

    renderObject->moveTo(glm::vec3(-2.0f, 0.0f, -2.0f));
    renderObject->scale(glm::vec3(scaleFactor, scaleFactor, scaleFactor));
    renderObject->rotateTo(glm::vec3(90.0, 0.0f, 0));
    renderObject->addProgram(
            ifx::ProgramFactory().LoadCAMPlane());
    return renderObject;
}

}