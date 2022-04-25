//
// Created by henrique on 04/11/2021.
//

#ifndef ITU_GRAPHICS_PROGRAMMING_RT_TYPES_H
#define ITU_GRAPHICS_PROGRAMMING_RT_TYPES_H

#include "glm/glm.hpp"

namespace rt{
    namespace Colors {
        // color is a vec4
        typedef glm::vec4 color;

        // hardcoded some colors for convenience
        const color white = glm::vec4(1, 1, 1, 1);
        const color grey = glm::vec4(.5, .5, .5, 1);;
        const color dark = glm::vec4(.15, .15, .15, 1);
        const color black = glm::vec4(0, 0, 0, 1);
        const color blue = glm::vec4(0, 0, 1, 1);
        const color green = glm::vec4(0, 1, 0, 1);
        const color red = glm::vec4(1, 0, 0, 1);

        inline std::uint32_t toRGBA32(color c) {
            // convert color to four 8 bits uint, packed in a 32 bits uint.
            // that is the proper format for the color buffer that renders to the screen
            color c_clamp = glm::clamp(c, .0f, 1.0f);
            return (uint32_t(255 *  c_clamp.r)) + (uint32_t(255 * c_clamp.g) << 8) +
                   (uint32_t(255 * c_clamp.b) << 16) + (uint32_t(255 * c_clamp.a) << 24);
        }
    }

    struct Ray{
        Ray(glm::vec3 orig, glm::vec3 dir): origin(orig), direction(dir){};
        glm::vec3 origin;
        glm::vec3 direction;
    };

    struct Hit{
        int hit_ID = -1; // negative values for no hit, other values for the index of the first vertex in a triangle
        glm::vec3 barycentric; // the barycentric coordinates of the triangle that was hit (if any)
        float dist = FLT_MAX;  // used to store the intersection distance
    };

    struct vertex {
        glm::vec4 pos;
        glm::vec4 norm;
        Colors::color col;
        glm::vec2 uv;

        friend vertex operator/(vertex v, float sc) {
            return vertex{v.pos / sc, v.norm / sc, v.col / sc, v.uv / sc};
        }

        friend vertex operator*(vertex v, float sc) {
            return vertex{v.pos * sc, v.norm * sc, v.col * sc, v.uv * sc};
        }

        friend vertex operator-(vertex v1, const vertex &v2) {
            return vertex{v1.pos - v2.pos, v1.norm - v2.norm, v1.col - v2.col, v1.uv - v2.uv};
        }

        friend vertex operator+(vertex v1, const vertex &v2) {
            return vertex{v1.pos + v2.pos, v1.norm + v2.norm, v1.col + v2.col, v1.uv + v2.uv};
        }
    };
}

#endif //ITU_GRAPHICS_PROGRAMMING_RT_TYPES_H
