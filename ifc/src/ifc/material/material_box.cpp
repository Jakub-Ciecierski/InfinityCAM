#include "ifc/material/material_box.h"

#include <ifc/factory/material_box_factory.h>
#include <ifc/material/height_map.h>

namespace ifc{

MaterialBox::MaterialBox(MaterialBoxCreateParams params) :
        dimensions_(params.dimensions),
        precision_(params.precision){
    height_map_
            = std::unique_ptr<HeightMap>(
            new HeightMap(params.precision.x,
                          params.precision.z,
                          params.dimensions.depth));

    box_render_object_
            = MaterialBoxFactory().
            CreateMaterialBoxRenderObject(params.precision,
                                          params.dimensions,
                                          height_map_.get());
}

MaterialBox::~MaterialBox(){}


void MaterialBox::Update(){
    height_map_->Update();
}
}