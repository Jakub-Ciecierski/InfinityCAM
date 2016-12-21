#ifndef PROJECT_MEASURES_H
#define PROJECT_MEASURES_H

#include <math/math_ifx.h>

namespace ifc{

extern float MillimeterToGLScale;

/**
 * Converts Millimeters to/from OpenGL distances.
 */
glm::vec2 MillimetersToGL(const glm::vec2& vec_mm);
glm::vec3 MillimetersToGL(const glm::vec3& vec);
float MillimetersToGL(float mm);

glm::vec2 GLToMillimeters(const glm::vec2& vec_gl);
glm::vec3 GLToMillimeters(const glm::vec3& vec_gl);
float GLToMillimeters(float gl);

}


#endif //PROJECT_MEASURES_H
