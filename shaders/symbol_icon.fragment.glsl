uniform sampler2D u_texture;

in vec2 v_tex;
in float v_fade_opacity;

#pragma mapbox: define lowp float opacity

void main() {
    #pragma mapbox: initialize lowp float opacity

    lowp float alpha = opacity * v_fade_opacity;
    gl_FragColor = texture2D(u_texture, v_tex) * alpha;

#ifdef OVERDRAW_INSPECTOR
    gl_FragColor = vec4(1.0);
#endif
}
