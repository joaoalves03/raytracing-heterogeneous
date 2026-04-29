#ifndef SPHERE_H
#define SPHERE_H

// GPU-safe sphere: no virtual base, stores Material by value.

struct Sphere {
    point3   center;
    double   radius;
    Material mat;

    Sphere() : center(point3(0,0,0)), radius(0), mat(Material::make_lambertian(color(0,0,0))) {}
    Sphere(const point3& c, double r, const Material& m)
        : center(c), radius(std::fmax(0, r)), mat(m) {}

    #pragma acc routine seq
    bool hit(const ray& r, interval ray_t, hit_record& rec) const {
        vec3 oc = center - r.origin();
        auto a  = r.direction().length_squared();
        auto h  = dot(r.direction(), oc);
        auto c  = oc.length_squared() - radius * radius;
        auto discriminant = h * h - a * c;
        if (discriminant < 0) return false;

        auto sqrtd = std::sqrt(discriminant);
        auto root  = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root)) return false;
        }

        rec.t  = root;
        rec.p  = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;
        return true;
    }
};

#endif
