#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

// GPU-safe world: raw array of Sphere values, no vector, no shared_ptr,
// no virtual dispatch. Built on host, copied to device with acc enter data.

#include "hittable.h"
#include "interval.h"
#include "sphere.h"

struct hittable_list {
    Sphere* objects;   // raw pointer — must be copyin'd to device by caller
    int     count;

    hittable_list() : objects(nullptr), count(0) {}
    hittable_list(Sphere* objs, int n) : objects(objs), count(n) {}

    #pragma acc routine seq
    bool hit(const ray& r, interval ray_t, hit_record& rec) const {
        hit_record temp_rec;
        bool hit_anything    = false;
        auto closest_so_far  = ray_t.max;

        for (int i = 0; i < count; i++) {
            if (objects[i].hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything   = true;
                closest_so_far = temp_rec.t;
                rec            = temp_rec;
            }
        }
        return hit_anything;
    }
};

#endif
