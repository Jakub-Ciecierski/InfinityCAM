#ifndef PROJECT_SIMULATION_GUI_H
#define PROJECT_SIMULATION_GUI_H


#include <ifc/material/material_box.h>
#include <ifc/cutter/cutter_simulation.h>

#include <memory>

class RenderObject;
namespace ifx{
class Scene;
}

namespace ifc {

class SimulationGUI {
public:
    SimulationGUI(std::shared_ptr<ifx::Scene> scene,
                  std::shared_ptr<ifx::RenderObject> plane,
                  std::shared_ptr<CutterSimulation> simulation);

    ~SimulationGUI();

    void Render();

private:
    void SetDefaultParameters();

    void RenderMainWindow();

    void RenderDebugWindow();

    void RenderMenuBar();

    void RenderSimulationInfoSection();
    void RenderSimulationRequirements();
    void RenderSimulationControl();
    void RenderSimulationError();

    void RenderCutterSection();
    void RenderLoadCutter();
    void RenderShowTrajectoryCutter();

    void RenderMaterialBoxSection();
    void RenderMaterialBoxDimensions();
    void RenderMaterialBoxPrecision();

    void ResetSimulation();

    void RenderDebugSection();

    void RenderPlane();

    void RenderExample();

    void RenderCameraInfo();

    void RenderPolygonMode();

    void RenderErrorWindow();

    std::shared_ptr<ifx::Scene> scene_;

    MaterialBoxCreateParams material_box_create_params_;

    std::shared_ptr<ifx::RenderObject> plane_;
    std::shared_ptr<CutterSimulation> simulation_;
};

}
#endif //PROJECT_SIMULATION_GUI_H
