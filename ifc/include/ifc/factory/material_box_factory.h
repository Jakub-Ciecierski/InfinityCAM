#ifndef PROJECT_MATERIAL_BOX_FACTORY_H
#define PROJECT_MATERIAL_BOX_FACTORY_H

#include <ifc/material/material_box.h>
#include <rendering/instanced_render_object.h>
#include <object/render_object.h>

#include <memory>

namespace ifx{
}

namespace ifc {

extern const std::string precision_x_uniform;
extern const std::string precision_z_uniform;

class MaterialBoxFactory {
public:

    MaterialBoxFactory();
    ~MaterialBoxFactory();

    std::shared_ptr<ifx::InstancedRenderObject> CreateTexelledBox(
            MaterialBoxPrecision precision,
            MaterialBoxDimensions dimensions,
            HeightMap* height_map);

    std::shared_ptr<RenderObject> CreatePlane();

private:
};
}

#endif //PROJECT_MATERIAL_BOX_FACTORY_H
