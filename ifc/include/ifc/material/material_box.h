#ifndef PROJECT_MATERIAL_BOX_H
#define PROJECT_MATERIAL_BOX_H

#include <memory>
#include <object/render_object.h>

namespace ifx{
    class InstancedRenderObject;
}

namespace ifc {

class HeightMap;

/*
 * Diemensions in millimeters.
 */
struct MaterialBoxDimensions{
    float x;
    float z;
    float depth;
    float max_depth;
};

/*
 * Density of little rectangles.
 */
struct MaterialBoxPrecision{
    int x;
    int z;
};

struct MaterialBoxCreateParams{
    MaterialBoxDimensions dimensions;
    MaterialBoxPrecision precision;
};

class MaterialBox {
public:

    MaterialBox(MaterialBoxCreateParams params);
    ~MaterialBox();

    std::shared_ptr<RenderObject>
    box_render_object(){return box_render_object_;}

    HeightMap* height_map(){return height_map_.get();}
    MaterialBoxDimensions dimensions(){return dimensions_;}
    MaterialBoxPrecision precision(){return precision_;}

    void Update();

private:
    std::shared_ptr<RenderObject> box_render_object_;
    std::unique_ptr<HeightMap> height_map_;

    MaterialBoxDimensions dimensions_;
    MaterialBoxPrecision precision_;

};
}

#endif //PROJECT_MATERIAL_BOX_H
