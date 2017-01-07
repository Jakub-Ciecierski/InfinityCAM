#include <factory/render_object_factory.h>
#include <game/game_loop.h>
#include <rendering/renderer.h>
#include <ifc/gui/cam_gui.h>
#include <ifc/cutter/cutter_simulation.h>
#include <ifc/factory/material_box_factory.h>

#include <memory>
#include <game/factory/game_loop_factory.h>

void InitGUI(std::shared_ptr<ifx::GameLoop> game_loop);
void InitCamera(std::shared_ptr<ifx::GameLoop> game_loop);

void InitGUI(std::shared_ptr<ifx::GameLoop> game_loop){
    auto plane = ifc::MaterialBoxFactory().CreatePlane();
    plane->models()[0]->getMesh(0)->polygon_mode(ifx::PolygonMode::LINE);

    game_loop->renderer()->scene()->AddRenderObject(plane);

    auto simulation = std::shared_ptr<ifc::CutterSimulation>(
            new ifc::CutterSimulation(game_loop->renderer()->scene()));
    game_loop->AddSimulation(simulation);

    auto gui = std::unique_ptr<ifc::CAMGUI>(new ifc::CAMGUI(
            game_loop->renderer()->window()->getHandle(),
            game_loop->renderer(),
            game_loop->renderer()->scene(),
            plane,
            simulation
    ));
    game_loop->renderer()->SetGUI(std::move(gui));
}

void InitCamera(std::shared_ptr<ifx::GameLoop> game_loop){
    game_loop->renderer()->scene()->camera()->moveTo(
            glm::vec3(2.0f, 2.0f, 2.0f));
    game_loop->renderer()->scene()->camera()->rotateTo(
            glm::vec3(220.0f, -45.0f, 0.0f));
}

int main() {
    auto game_loop = ifx::GameLoopFactory().Create();

    InitGUI(game_loop);
    InitCamera(game_loop);

    game_loop->Start();
}

