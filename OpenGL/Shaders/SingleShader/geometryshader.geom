#version 330 core
layout(points) in; // Input primitive type (points)
layout(triangle_strip, max_vertices = 3) out; // Output primitive type

void main()
{
    for (int i = 0; i < gl_in.length(); ++i) {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}