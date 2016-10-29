#ifndef PROJECT_CAM_GUI_H
#define PROJECT_CAM_GUI_H

#include <gui/gui.h>
#include <ifc/material/material_box.h>
#include <ifc/cutter/cutter_simulation.h>

#include <memory>

class RenderObject;
namespace ifx{
    class Scene;
}

namespace ifc {

class CAMGUI : public ifx::GUI{
public:
    CAMGUI(GLFWwindow* window,
           ifx::Scene* scene,
           std::shared_ptr<RenderObject> plane,
           std::shared_ptr<CutterSimulation> simulation);
    ~CAMGUI();

    virtual void Render() override;

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

    ifx::Scene* scene_;

    MaterialBoxCreateParams material_box_create_params_;

    std::shared_ptr<RenderObject> plane_;

    std::shared_ptr<CutterSimulation> simulation_;
};

}

#endif //PROJECT_CAM_GUI_H
