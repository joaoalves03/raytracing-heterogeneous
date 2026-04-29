#ifndef HITTABLE_H
#define HITTABLE_H

// GPU-safe hit_record: no shared_ptr, stores Material by value.
// material.h must be included before this file (via rtweekend.h chain).

#include "vec3.h"
#include "ray.h"
#include "material.h"

struct hit_record {
    point3   p;
    vec3     normal;
    Material mat;      // value, not pointer — GPU can copy this freely
    double   t;
    bool     front_face;

    #pragma acc routine seq
    void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

#endif
