
#version 450

//const array of positions for the triangle
	const vec3 positions[3] = vec3[3](
		vec3(1.f,1.f, 0.0f),
		vec3(-1.f,1.f, 0.0f),
		vec3(0.f,-1.f, 0.0f)
	);



void main() {
    gl_Position = vec4(positions[gl_VertexIndex]/2.f, 1.0);
   
}