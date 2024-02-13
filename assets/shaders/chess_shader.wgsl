struct VertexInput {
    @location(0) position: vec3f,
    @location(1) normal: vec3f,
    @location(2) uv: vec2f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) frag_pos: vec4f,
    @location(1) normal: vec3f,
    @location(2) uv: vec2f,
};

// camera group
@group(0) @binding(0) var<uniform> camera_matrix: mat4x4<f32>;
// model group
@group(1) @binding(0) var<storage, read> model_matrix: mat4x4<f32>;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    out.frag_pos = model_matrix * vec4f(in.position, 1.0);
    out.position = camera_matrix * out.frag_pos;
    out.normal = in.normal;
    out.uv = in.uv;
    return out;
}

// material group
@group(2) @binding(0) var texture_sampler: sampler;
@group(2) @binding(1) var albedo_texture: texture_2d<f32>;
@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    // ambient
    let ambient_strength: f32 = 0.1;
    let ambient = ambient_strength * vec3f(1.0);

    let albedo_color = textureSample(albedo_texture, texture_sampler, in.uv);

    // // diffuse 
    let norm = normalize(in.normal);
    let light_dir = normalize(vec3f(0.0,0.45,0.0) - in.frag_pos.xyz);
    let diff = max(0.0, dot(norm, light_dir));
    let diffuse = diff * vec3f(1.0);

    let result = (ambient + diffuse) * albedo_color.rgb;

    return vec4f(result, albedo_color.a);
}