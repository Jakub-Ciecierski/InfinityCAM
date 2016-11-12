#ifndef PROJECT_CUTTER_FACTORY_H
#define PROJECT_CUTTER_FACTORY_H

#include <ifc/cutter/cutter.h>
#include <object/render_object.h>

#include <memory>

namespace ifx{
class Texture2D;
}

namespace ifc {

class CutterFactory {
public:

    CutterFactory();
    ~CutterFactory();

    std::shared_ptr<Cutter> CreateCutter(std::string filepath);

private:
    std::shared_ptr<ifx::RenderObject> CreateSphereCutterRenderObject(
            float diameter);
    std::shared_ptr<ifx::RenderObject> CreateFlatCutterRenderObject(
            float diameter);

    std::vector<std::shared_ptr<ifx::Texture2D>> CreateCutterBladeTextures();
};

}

#endif //PROJECT_CUTTER_FACTORY_H
