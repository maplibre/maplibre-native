layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_texture_pos;

layout (std140) uniform HillshadePrepareDrawableUBO {
    highp mat4 u_matrix;
};

layout (std140) uniform HillshadePrepareTilePropsUBO {
    highp vec4 u_unpack;
    highp vec2 u_dimension;
    highp float u_zoom;
    highp float u_maxzoom;
};

out vec2 v_pos;

void main() {
    gl_Position = u_matrix * vec4(a_pos, 0, 1);

    // u_dimension is the DEM buffer stride (interior + 2 * border).
    // The buffer has `border = 3` so stride = dim + 6.
    //
    // Render target is (dim+3) × (dim+3). Output pixel i (i ∈ [0, dim+2])
    // computes the Sobel-derived slope centred on buffer texel index i+2
    // (the first interior cell at i=0 through the second-east-border
    // cell at i=dim). The east-border cells hold the eastern neighbour's
    // first two interior columns, so the output pixel at i=dim of tile A
    // and the output pixel at i=0 of tile B reference identical source
    // pixels and produce identical slope values — which is what makes
    // the rendered hillshade seamless at the tile boundary.
    //
    // Quad corners have a_texture_pos ∈ {0, EXTENT}². At the rendered
    // framebuffer corner (0, 0) we want v_pos at the texel-2 centre,
    // i.e. 2.5 / stride. At the opposite corner (EXTENT, EXTENT) we
    // want v_pos at the texel-(dim+2) centre, i.e. (dim + 2.5) / stride.
    // Given output pixel i sits at framebuffer-coord (i + 0.5)/(dim+1),
    // the linear interpolation that lands those endpoints is:
    //
    //   v_pos = (a_texture_pos / EXTENT) * (dim + 1)/stride + 2/stride
    //
    // Or, in terms of u_dimension = stride = dim + 4:
    float scale = (u_dimension.x - 3.0) / u_dimension.x;
    v_pos = (a_texture_pos / 8192.0) * scale + 2.0 / u_dimension;
}
