layout (location = 0) in vec2 a_pos;
out vec3 v_data;

layout (std140) uniform GlobalPaintParamsUBO {
    highp vec2 u_pattern_atlas_texsize;
    highp vec2 u_units_to_pixels;
    highp vec2 u_world_size;
    highp float u_camera_to_center_distance;
    highp float u_symbol_fade_change;
    highp float u_aspect_ratio;
    highp float u_pixel_ratio;
    highp float u_map_zoom;
    lowp float global_pad1;
};

layout (std140) uniform CircleDrawableUBO {
    highp mat4 u_matrix;
    highp vec2 u_extrude_scale;
    // Interpolations
    lowp float u_color_t;
    lowp float u_radius_t;
    lowp float u_blur_t;
    lowp float u_opacity_t;
    lowp float u_stroke_color_t;
    lowp float u_stroke_width_t;
    lowp float u_stroke_opacity_t;
    highp float u_globe_extrude_scale;
    lowp float drawable_pad2;
    lowp float drawable_pad3;
};

layout (std140) uniform CircleEvaluatedPropsUBO {
    highp vec4 u_color;
    highp vec4 u_stroke_color;
    mediump float u_radius;
    lowp float u_blur;
    lowp float u_opacity;
    mediump float u_stroke_width;
    lowp float u_stroke_opacity;
    bool u_scale_with_map;
    bool u_pitch_with_map;
    lowp float props_pad1;
};

#pragma mapbox: define highp vec4 color
#pragma mapbox: define mediump float radius
#pragma mapbox: define lowp float blur
#pragma mapbox: define lowp float opacity
#pragma mapbox: define highp vec4 stroke_color
#pragma mapbox: define mediump float stroke_width
#pragma mapbox: define lowp float stroke_opacity

void main(void) {
    #pragma mapbox: initialize highp vec4 color
    #pragma mapbox: initialize mediump float radius
    #pragma mapbox: initialize lowp float blur
    #pragma mapbox: initialize lowp float opacity
    #pragma mapbox: initialize highp vec4 stroke_color
    #pragma mapbox: initialize mediump float stroke_width
    #pragma mapbox: initialize lowp float stroke_opacity

    // unencode the extrusion vector that we snuck into the a_pos vector
    vec2 extrude = vec2(mod(a_pos, 2.0) * 2.0 - 1.0);

    // multiply a_pos by 0.5, since we had it * 2 in order to sneak
    // in extrusion data
    vec2 circle_center = floor(a_pos * 0.5);
    if (u_pitch_with_map) {
#ifdef PROJECTION_GLOBE
        vec3 center_vector = projectToSphere(circle_center, vec2(0.0, 0.0));
        float angle_scale = u_globe_extrude_scale;
#endif
        vec2 corner_position = circle_center;
        if (u_scale_with_map) {
            corner_position += extrude * (radius + stroke_width) * u_extrude_scale;
#ifdef PROJECTION_GLOBE
            angle_scale *= (radius + stroke_width);
#endif
        } else {
            // Pitching the circle with the map effectively scales it with the map
            // To counteract the effect for pitch-scale: viewport, we rescale the
            // whole circle based on the pitch scaling effect at its central point
#ifdef PROJECTION_GLOBE
            vec4 projected_center = interpolateProjection(circle_center, center_vector, 0.0);
            angle_scale *= (radius + stroke_width) * (projected_center.w / u_camera_to_center_distance);
#else
            vec4 projected_center = u_matrix * vec4(circle_center, 0, 1);
#endif
            corner_position += extrude * (radius + stroke_width) * u_extrude_scale * (projected_center.w / u_camera_to_center_distance);
        }

#ifdef PROJECTION_GLOBE
        vec3 corner_vector = globeRotateVector(center_vector, extrude * angle_scale);
        gl_Position = interpolateProjection(corner_position, corner_vector, 0.0);
#else
        gl_Position = u_matrix * vec4(corner_position, 0, 1);
#endif
    } else {
#ifdef PROJECTION_GLOBE
        gl_Position = projectTile(circle_center);
        if (gl_Position.z / gl_Position.w > 1.0) {
            gl_Position.xy = vec2(10000.0);
        }
#else
        gl_Position = u_matrix * vec4(circle_center, 0, 1);
#endif

        if (u_scale_with_map) {
            gl_Position.xy += extrude * (radius + stroke_width) * u_extrude_scale * u_camera_to_center_distance;
        } else {
            gl_Position.xy += extrude * (radius + stroke_width) * u_extrude_scale * gl_Position.w;
        }
    }

    // This is a minimum blur distance that serves as a faux-antialiasing for
    // the circle. since blur is a ratio of the circle's size and the intent is
    // to keep the blur at roughly 1px, the two are inversely related.
    lowp float antialiasblur = 1.0 / DEVICE_PIXEL_RATIO / (radius + stroke_width);

    v_data = vec3(extrude.x, extrude.y, antialiasblur);
}
