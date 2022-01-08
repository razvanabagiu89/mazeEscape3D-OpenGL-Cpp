#version 330

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Uniforms for light properties
uniform vec3 light_position;
uniform vec3 eye_position;
uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

uniform vec3 object_color;

// Output value to fragment shader
out vec3 color;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{

//    // DONE(student): Compute world space vectors
//    vec3 world_pos = (Model * vec4(v_position, 1)).xyz;
//    vec3 world_normal = normalize(mat3(Model) * normalize(v_normal));
//
//    vec3 N = normalize(world_normal);
//	vec3 L = normalize(light_position - world_pos);
//	vec3 V = normalize(eye_position - world_pos);
//	vec3 H = normalize(L + V);
//
//    // DONE(student): Define ambient light component
//    float ambient_light = material_kd * 0.25;
//
//    // DONE(student): Compute diffuse light component
//    float intensitateLumina = 1;
//    float diffuse_light = material_kd * intensitateLumina * max (dot(N, L), 0);
//
//    // DONE(student): Compute specular light component
//    float specular_light = 0;
//
//    // It's important to distinguish between "reflection model" and
//    // "shading method". In this shader, we are experimenting with the Phong
//    // (1975) and Blinn-Phong (1977) reflection models, and we are using the
//    // Gouraud (1971) shading method. There is also the Phong (1975) shading
//    // method, which we'll use in the future. Don't mix them up!
//    if (diffuse_light > 0)
//    {
//        // vec3 R = reflect (-L, N);
//        // specular_light = material_ks * pow(max(dot(V, R), 0), material_shininess);  // -> Phong
//        specular_light = material_ks * pow(max(dot(normalize(N), H), 0), material_shininess); // -> Blinn
//    }
//
//    // DONE(student): Compute light
//    // am adaugat +1 ptc asa am vazut in video ul de pe ocw ca ar fi mai bine
//    float factor_atenuare =  1/(pow(distance(light_position, world_pos), 2) + 1);
//
//    float light = ambient_light + factor_atenuare * (specular_light + diffuse_light);
//
//    // DONE(student): Send color light output to fragment shader
//    color = object_color * light;
    vec3 newpos = v_position;
	newpos = v_position + v_normal * (rand(vec2(v_position)) * 60);
	gl_Position = Projection * View * Model * vec4(newpos, 1.0);
    color = vec3(1, 0, 0);


    // gl_Position = Projection * View * Model * vec4(v_position, 1.0) * vec4(sin(0.35f), cos(0.23f), sin(0.29f), 1.0f);
    // gl_Position = pozitie * M * V * P + vec3(sin(uniform_random), cos(uniform_random, sin(uniform_random)))

}
