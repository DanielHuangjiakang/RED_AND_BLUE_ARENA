#version 330 core

in vec2 frag_texcoord;
out vec4 color;

uniform float time; // Pass the current time from the application

void main() {
    // Interpolate between yellow and red
    float transition = sin(time * 0.1) * 0.5 + 0.5; // Oscillates between 0.0 and 1.0 over time
    vec3 yellow = vec3(1.0, 1.0, 0.0);
    vec3 red = vec3(1.0, 0.0, 0.0);

    // Compute the interpolated color (reverse the order for yellow to red)
    vec3 interpolated_color = mix(red, yellow, transition);

    // Add a gradient effect using the y-coordinate
    float gradient = frag_texcoord.y;

    // Combine the interpolated color with the gradient
    vec3 beam_color = interpolated_color * gradient;

    // Output the final color with full opacity
    color = vec4(beam_color, 1.0);
}
