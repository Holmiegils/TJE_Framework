#version 330 core

// Input attributes from the vertex array
attribute vec3 a_vertex;
attribute vec2 a_uv;

// Varying variables to pass to the fragment shader
varying vec2 v_uv;

void main()
{
    // Pass through the texture coordinates
    v_uv = a_uv;

    // Calculate the position of the vertex in clip space
    gl_Position = vec4(a_vertex.xy, 0.0, 1.0);
}
