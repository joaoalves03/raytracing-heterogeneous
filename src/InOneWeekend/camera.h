#ifndef CAMERA_H
#define CAMERA_H

#include "hittable_list.h"

class camera {
  public:
    double aspect_ratio      = 1.0;
    int    image_width       = 100;
    int    samples_per_pixel = 10;
    int    max_depth         = 10;

    double vfov     = 90;
    point3 lookfrom = point3(0, 0, 0);
    point3 lookat   = point3(0, 0, -1);
    vec3   vup      = vec3(0, 1, 0);

    double defocus_angle = 0;
    double focus_dist    = 10;

    void render(const hittable_list& world) {
        initialize();

        int     n_spheres    = world.count;
        Sphere* spheres      = world.objects;
        int     total_pixels = image_width * image_height;
        color*  framebuffer  = new color[total_pixels]();

        int    iw  = image_width;
        int    ih  = image_height;
        int    spp = samples_per_pixel;
        double pss = pixel_samples_scale;
        int    md  = max_depth;
        point3 p00 = pixel00_loc;
        vec3   pdu = pixel_delta_u;
        vec3   pdv = pixel_delta_v;
        point3 ctr = center;
        double da  = defocus_angle;
        vec3   ddu = defocus_disk_u;
        vec3   ddv = defocus_disk_v;

        std::clog << "Rendering " << iw << "x" << ih
                  << " @ " << spp << " spp...\n" << std::flush;

#pragma acc parallel loop gang vector \
    copyin(spheres[0:n_spheres]) \
    copyout(framebuffer[0:total_pixels]) \
    firstprivate(iw,ih,spp,pss,md,p00,pdu,pdv,ctr,da,ddu,ddv,n_spheres)
        for (int idx = 0; idx < total_pixels; idx++) {
            int j = idx / iw;
            int i = idx % iw;
            //std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            // unique per-pixel seed, advanced through all random calls by ref
            unsigned int seed = (unsigned int)(idx * 1973u + 9277u);

            color pixel_color(0, 0, 0);
            for (int s = 0; s < spp; s++) {
                ray r = get_ray_d(i, j, p00, pdu, pdv, ctr, da, ddu, ddv, seed);
                hittable_list w(spheres, n_spheres);
                pixel_color += ray_color_d(r, md, w, seed);
            }
            framebuffer[idx] = pss * pixel_color;
        }

        std::cout << "P3\n" << iw << ' ' << ih << "\n255\n";
        for (int idx = 0; idx < total_pixels; idx++)
            write_color(std::cout, framebuffer[idx]);
        delete[] framebuffer;
        std::clog << "Done.\n";
    }

  private:
    int    image_height;
    double pixel_samples_scale;
    point3 center;
    point3 pixel00_loc;
    vec3   pixel_delta_u;
    vec3   pixel_delta_v;
    vec3   u, v, w;
    vec3   defocus_disk_u;
    vec3   defocus_disk_v;

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        if (image_height < 1) image_height = 1;
        pixel_samples_scale = 1.0 / samples_per_pixel;
        center = lookfrom;

        auto theta = degrees_to_radians(vfov);
        auto h     = std::tan(theta / 2);
        auto vp_h  = 2 * h * focus_dist;
        auto vp_w  = vp_h * (double(image_width) / image_height);

        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        vec3 vp_u = vp_w * u;
        vec3 vp_v = vp_h * -v;

        pixel_delta_u = vp_u / image_width;
        pixel_delta_v = vp_v / image_height;

        auto vp_upper_left = center - (focus_dist*w) - vp_u/2 - vp_v/2;
        pixel00_loc = vp_upper_left + 0.5*(pixel_delta_u + pixel_delta_v);

        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle/2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    // seed is unsigned int& — advanced by every random call
    #pragma acc routine seq
    static ray get_ray_d(int i, int j,
                         const point3& p00, const vec3& pdu, const vec3& pdv,
                         const point3& ctr, double da,
                         const vec3& ddu, const vec3& ddv,
                         unsigned int& seed) {
        vec3 offset = vec3(random_double_gpu(seed) - 0.5,
                           random_double_gpu(seed) - 0.5, 0);
        auto pixel_sample = p00 + ((i + offset.x()) * pdu)
                                + ((j + offset.y()) * pdv);
        point3 ray_origin;
        if (da <= 0) {
            ray_origin = ctr;
        } else {
            auto p = random_in_unit_disk_gpu(seed);
            ray_origin = ctr + (p[0]*ddu) + (p[1]*ddv);
        }
        return ray(ray_origin, pixel_sample - ray_origin);
    }

    #pragma acc routine seq
    static color ray_color_d(const ray& r, int depth,
                              const hittable_list& world,
                              unsigned int& seed) {
        ray   cur_ray = r;
        color attn(1.0, 1.0, 1.0);

        for (int i = 0; i < depth; i++) {
            hit_record rec;
            if (world.hit(cur_ray, interval(0.001, infinity), rec)) {
                ray   scattered;
                color mat_attn;
                if (rec.mat.scatter(cur_ray, rec.p, rec.normal,
                                    rec.front_face, mat_attn, scattered, seed)) {
                    attn    = attn * mat_attn;
                    cur_ray = scattered;
                } else {
                    return color(0, 0, 0);
                }
            } else {
                vec3 ud = unit_vector(cur_ray.direction());
                auto a  = 0.5 * (ud.y() + 1.0);
                return attn * ((1.0-a)*color(1,1,1) + a*color(0.5,0.7,1.0));
            }
        }
        return color(0, 0, 0);
    }
};

#endif
