#version 460

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec3 in_world_position;

layout(set = 0, binding = 0) uniform MyUniform {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 position;
} ubo;

const float GRID_SIZE = 100.0;
const float GRID_CELL_SIZE = 0.025;
const float MIN_PIXELS_BETWEEN_CELLS = 2.0;
const vec4 GRID_COLOR_THIN = vec4(0.5, 0.5, 0.5, 0.0);
const vec4 GRID_COLOR_THICK = vec4(0.0, 0.0, 0.0, 1.0);

float max2(vec2 v)
{
    float f = max(v.x, v.y);
    return f;
}

vec2 satv(vec2 x)
{
    vec2 v = clamp(x, vec2(0.0), vec2(1.0));
    return v;
}

float log10(float x)
{
    float f = log(x) / log(10.0);
    return f;
}

float satf(float x)
{
    float f = clamp(x, 0.0, 1.0);
    return f;
}

void main() {
    vec2 dvx = vec2(dFdx(in_world_position.x), dFdy(in_world_position.x));
    vec2 dvy = vec2(dFdx(in_world_position.z), dFdy(in_world_position.z));

    float lx = length(dvx);
    float ly = length(dvy);

    vec2 dudv = vec2(lx, ly);

    float l = length(dudv);

    float LOD = max(0.0, log10(l * MIN_PIXELS_BETWEEN_CELLS / GRID_CELL_SIZE) + 1.0);

    float grid_cell_size_lod0 = GRID_CELL_SIZE * pow(10.0, floor(LOD));
    float grid_cell_size_lod1 = grid_cell_size_lod0 * 10.0;
    float grid_cell_size_lod2 = grid_cell_size_lod1 * 10.0;

    dudv *= 4.0;

    vec2 mod_div_dudv = mod(in_world_position.xz, grid_cell_size_lod0) / dudv;
    float lod0a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)));

    mod_div_dudv = mod(in_world_position.xz, grid_cell_size_lod1) / dudv;
    float lod1a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)));

    mod_div_dudv = mod(in_world_position.xz, grid_cell_size_lod2) / dudv;
    float lod2a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)));

    float lod_fade = fract(LOD);
    vec4 color;

    if (lod2a > 0.0) {
        color = GRID_COLOR_THICK;
        color.a *= lod2a;
    } else {
        if (lod1a > 0.0) {
            color = mix(GRID_COLOR_THICK, GRID_COLOR_THIN, lod_fade);
            color.a *= lod1a;
        } else {
            color = GRID_COLOR_THIN;
            color.a *= (lod0a * (1.0 - lod_fade));
        }
    }

    float opacity_falloff = (1.0 - satf(length(in_world_position.xz - ubo.position.xz) / GRID_SIZE));
    color.a *= opacity_falloff;

    out_color = color;
}
