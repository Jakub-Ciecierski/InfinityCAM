#include "ifc/measures.h"

namespace ifc{

float MillimeterToGLScale = 0.009f;

glm::vec2 MillimetersToGL(const glm::vec2& vec_mm){
    glm::vec2 vec_gl;
    vec_gl.x = MillimetersToGL(vec_mm.x);
    vec_gl.y = MillimetersToGL(vec_mm.y);

    return vec_gl;
}

glm::vec3 MillimetersToGL(const glm::vec3& vec_mm){
    glm::vec3 vec_gl;
    vec_gl.x = MillimetersToGL(vec_mm.x);
    vec_gl.y = MillimetersToGL(vec_mm.y);
    vec_gl.z = MillimetersToGL(vec_mm.z);

    return vec_gl;
}

float MillimetersToGL(float mm){
    return mm * MillimeterToGLScale;
}


glm::vec2 GLToMillimeters(const glm::vec2& vec_gl){
    glm::vec2 vec_mm;
    vec_mm.x = GLToMillimeters(vec_gl.x);
    vec_mm.y = GLToMillimeters(vec_gl.y);

    return vec_mm;
}

glm::vec3 GLToMillimeters(const glm::vec3& vec_gl){
    glm::vec3 vec_mm;
    vec_mm.x = GLToMillimeters(vec_gl.x);
    vec_mm.y = GLToMillimeters(vec_gl.y);
    vec_mm.z = GLToMillimeters(vec_gl.z);

    return vec_mm;
}

float GLToMillimeters(float gl){
    return gl / MillimeterToGLScale;
}

}