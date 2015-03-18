#pragma once
#include <iostream>
#include <string>

#include <vector>
#include <map>
#include "glew/glew.h"
#include "VBO.hpp"
#include "VAO.hpp"

#include <assimp/Importer.hpp> // C++ importer interface
#include <assimp/scene.h> // Output data structure
#include <assimp/postprocess.h> // Post processing flags

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class AssimpModel {
	public :
	  AssimpModel() {}
	  AssimpModel(GLuint GLId, const std::string& pFile); // Import du fichier pFile.
	  ~AssimpModel(); // Destructeur.
	  void RenderModel(); // Rendu du modèle 3D.
	  std::vector<struct Vertex> vertices;
	  
	private :
	  bool ImportFromFile(const std::string& pFile);
	  bool BuildAssimpModel(const aiScene* scene);
	  bool LoadTextures(const aiScene* scene);
	  bool GenVerticesObjects();
	  void set_float4(glm::vec4& f, float a, float b, float c, float d);
	  void color4_to_float4(const aiColor4D *c, glm::vec4& f);
	  
	  std::vector<struct Mesh> meshes;
	  VBO vbo;
	  VAO vao;
	  std::map<std::string, GLuint> textureIdMap;
	  GLuint uDiffuseLoc;
	  GLuint uAmbientLoc;
	  GLuint uSpecularLoc;
	  GLuint uEmissiveLoc;
	  GLuint uShininessLoc;
	  GLuint uTexCountLoc;
};

struct Material {
	glm::vec4 diffuse;
	glm::vec4 ambient;
	glm::vec4 specular;
	glm::vec4 emissive;
	float shininess;
	int texCount;
};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

struct Mesh {
	unsigned int nbVertices;
	struct Material material;
	GLuint texId;
};
