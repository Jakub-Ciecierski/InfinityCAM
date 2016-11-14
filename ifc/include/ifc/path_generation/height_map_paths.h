#ifndef PROJECT_HEIGHTMAPPATHS_H
#define PROJECT_HEIGHTMAPPATHS_H

#include <math/math_ifx.h>

#include <vector>
#include <iostream>

namespace ifc {

/**
 * Stored in GL coordinates.
 */
struct HeightMapPath {
    HeightMapPath(std::vector<float> &heights,
                  std::vector <glm::vec2> &positions,
                  int row_count, int column_count,
                  float width_mm, float height_mm,
                  float init_height) :
            heights(heights), positions(positions),
            row_count(row_count), column_count(column_count),
            width_mm(width_mm), height_mm(height_mm),
            init_height(init_height) {
        row_width = row_count / width_mm;
        column_width = column_count / height_mm;

        std::cout << "HeightMapPath: " << std::endl;
        std::cout << "row_width: " << row_width << std::endl;
        std::cout << "column_width: " << column_width << std::endl;
    }

    std::vector<float> heights;
    std::vector <glm::vec2> positions;

    int row_count;
    int column_count;

    float width_mm;
    float height_mm;

    float row_width;
    float column_width;

    float init_height;

    float GetHeight(int i, int j) {
        return heights[index(i, j)];
    }

    const glm::vec2 &Position(int i, int j) {
        return positions[index(i, j)];
    }

    int index(int i, int j) {
        return i * row_count + j;
    }
};
}


#endif //PROJECT_HEIGHTMAPPATHS_H
