#version 330

// Matrices de transformations reçues en uniform
uniform mat4 uMVPMatrix;
uniform mat4 uMVMatrix;
uniform mat4 uNormalMatrix;
 
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

// Sorties du shader
out vec3 vPosition_vs; // Position du sommet transformé dans l'espace View
out vec3 vNormal_vs; // Normale du sommet transformé dans l'espace View
out vec2 vTexCoords; // Coordonnées de texture du sommet
out mat4 world;

void main(void){
    vTexCoords = texCoord;
    vPosition_vs = vec3(uMVMatrix * vec4(position, 1.0));
	vNormal_vs = vec3(uMVMatrix * vec4(normal, 0.0));
	world = uMVMatrix;
	
    gl_Position = uMVPMatrix * vec4(position, 1.0);
}
