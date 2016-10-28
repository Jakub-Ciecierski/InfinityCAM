#ifndef PROJECT_CUTTER_SIMULATION_H
#define PROJECT_CUTTER_SIMULATION_H

#include <vr/simulation.h>
#include <ifc/cutter/cutter.h>
#include <ifc/material/material_box.h>
#include <rendering/scene/scene.h>

#include <memory>

namespace ifc {

class CutterSimulation : public ifx::Simulation {
public:

    CutterSimulation(ifx::Scene* scene);
    ~CutterSimulation();

    std::shared_ptr<MaterialBox> material_box(){return material_box_;}
    std::shared_ptr<Cutter> cutter(){return cutter_;}

    float* time_delta_ptr(){return &time_delta_;}
    float* line_delta_ptr(){return &line_delta_;}

    float time_delta() {return time_delta_;}
    float time_delta(float t) {time_delta_ = t;}

    float line_delta(){return line_delta_;}
    void line_delta(float d){line_delta_ = d;}

    float total_time_s(){return total_time_s_;}

    void SetMaterialBox(std::shared_ptr<MaterialBox> material_box);
    void SetCutter(std::shared_ptr<Cutter> cutter);

    void Update() override;
    bool CanUpdate();

    void UpdateAllAtOnce();
private:
    bool SatisfiesTimeDelta();

    std::shared_ptr<MaterialBox> material_box_;
    std::shared_ptr<Cutter> cutter_;

    ifx::Scene* scene_;

    float time_delta_;
    float current_update_time_;
    float last_update_time_;
    float total_time_s_;

    float line_delta_;

    CutterStatus status;
};
}

#endif //PROJECT_CUTTER_SIMULATION_H
