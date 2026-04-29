#ifndef MATERIAL_H
#define MATERIAL_H

struct Material {
    enum Type { LAMBERTIAN, METAL, DIELECTRIC } type;
    vec3   albedo;
    double fuzz;
    double refraction_index;

    static Material make_lambertian(const color& a) {
        Material m; m.type=LAMBERTIAN; m.albedo=a; m.fuzz=0; m.refraction_index=0; return m;
    }
    static Material make_metal(const color& a, double f) {
        Material m; m.type=METAL; m.albedo=a; m.fuzz=(f<1?f:1); m.refraction_index=0; return m;
    }
    static Material make_dielectric(double ri) {
        Material m; m.type=DIELECTRIC; m.refraction_index=ri; m.fuzz=0; return m;
    }

    #pragma acc routine seq
    static double reflectance(double cosine, double ri) {
        auto r0 = (1-ri)/(1+ri); r0=r0*r0;
        return r0 + (1-r0)*std::pow((1-cosine),5);
    }

    // seed passed by reference — all random calls use GPU LCG
    #pragma acc routine seq
    bool scatter(const ray& r_in, const vec3& p, const vec3& normal,
                 bool front_face, color& attenuation, ray& scattered,
                 unsigned int& seed) const {
        if (type == LAMBERTIAN) {
            auto dir = normal + random_unit_vector_gpu(seed);
            if (dir.near_zero()) dir = normal;
            scattered   = ray(p, dir);
            attenuation = albedo;
            return true;
        }
        if (type == METAL) {
            vec3 refl = reflect(unit_vector(r_in.direction()), normal);
            refl = unit_vector(refl) + (fuzz * random_unit_vector_gpu(seed));
            scattered   = ray(p, refl);
            attenuation = albedo;
            return dot(scattered.direction(), normal) > 0;
        }
        // DIELECTRIC
        attenuation = color(1.0, 1.0, 1.0);
        double ri = front_face ? (1.0/refraction_index) : refraction_index;
        vec3 ud = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-ud, normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);
        vec3 direction;
        if (ri*sin_theta > 1.0 || reflectance(cos_theta,ri) > random_double_gpu(seed))
            direction = reflect(ud, normal);
        else
            direction = refract(ud, normal, ri);
        scattered = ray(p, direction);
        return true;
    }
};

#endif
