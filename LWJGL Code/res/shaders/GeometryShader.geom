#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in DATA {
    vec3 normal;
    vec3 color;
    vec2 textureCoordinates;
    mat4 projection;
} dataIn[];

out vec3 fragmentPosition;
out vec3 fragmentNormal;
out vec3 fragmentColor;
out vec2 textureCoordinates;

void main() {
	gl_Position = dataIn[0].projection * gl_in[0].gl_Position;
    fragmentNormal = dataIn[0].normal;
    fragmentColor = dataIn[0].color;
    textureCoordinates = dataIn[0].textureCoordinates;
    fragmentPosition = gl_in[0].gl_Position.xyz;
    EmitVertex();

    gl_Position = dataIn[1].projection * gl_in[1].gl_Position;
    fragmentNormal = dataIn[1].normal;
    fragmentColor = dataIn[1].color;
    textureCoordinates = dataIn[1].textureCoordinates;
    fragmentPosition = gl_in[1].gl_Position.xyz;
    EmitVertex();

    gl_Position = dataIn[2].projection * gl_in[2].gl_Position;
    fragmentNormal = dataIn[2].normal;
    fragmentColor = dataIn[2].color;
    textureCoordinates = dataIn[2].textureCoordinates;
    fragmentPosition = gl_in[2].gl_Position.xyz;
    EmitVertex();

    EndPrimitive();
}

// to show normals: (Also change 'triangle_strip' to 'line_strip' in the layout)

// vec3 P0 = gl_in[0].gl_Position.xyz;
// vec3 P1 = gl_in[1].gl_Position.xyz;
// vec3 P2 = gl_in[2].gl_Position.xyz;

// vec3 V0 = P1 - P0;
// vec3 V1 = P2 - P0;

// vec3 N = normalize(cross(V0, V1));

// vec3 P = (P0 + P1 + P2) / 3.0;

// gl_Position = dataIn[0].projection * vec4(P, 1.0);
// fragmentNormal = N;
// fragmentColor = dataIn[0].color;
// textureCoordinates = dataIn[0].textureCoordinates;
// fragmentPosition = P;
// EmitVertex();

// gl_Position = dataIn[0].projection * vec4(P + N * 1, 1.0);
// fragmentNormal = N;
// fragmentColor = dataIn[0].color;
// textureCoordinates = dataIn[0].textureCoordinates;
// fragmentPosition = P;
// EmitVertex();

// EndPrimitive();