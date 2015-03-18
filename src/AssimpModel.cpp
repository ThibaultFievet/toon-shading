#include "AssimpModel.hpp"
#include <string>
#include <cstring>
#include <fstream>
#include "stb/stb_image.h"

AssimpModel::AssimpModel(GLuint GLId, const std::string& pFile) {
  // On importe le modèle 3D.
    if(!ImportFromFile(pFile))
		exit(1);
	
	// On génère le VBO et le VAO
	if(!GenVerticesObjects())
		exit(1);
	
	uDiffuseLoc = glGetUniformLocation(GLId, "uDiffuse");
	uAmbientLoc = glGetUniformLocation(GLId, "uAmbient");
	uSpecularLoc = glGetUniformLocation(GLId, "uSpecular");
	uEmissiveLoc = glGetUniformLocation(GLId, "uEmissive");
	uShininessLoc = glGetUniformLocation(GLId, "uShininess");
	uTexCountLoc = glGetUniformLocation(GLId, "uTexCount");
}

AssimpModel::~AssimpModel() {
	// Libération des textures
	std::map<std::string, GLuint>::iterator itr = textureIdMap.begin();
	int i=0;
	for (; itr != textureIdMap.end(); ++i, ++itr)
	{
	  glDeleteTextures(1,(GLuint*) itr->second);
	}
	textureIdMap.clear();
}

void AssimpModel::RenderModel() {
	// Bind du VAO
	vao.bind();
	int j = 0;
	for(int i = 0; i < meshes.size(); ++i)
	{
	  // Bind de la texture
	  struct Material mat = meshes[i].material;
	  glBindTexture(GL_TEXTURE_2D, meshes[i].texId);
	  glUniform4f(uDiffuseLoc, mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], mat.diffuse[3]);
	  glUniform4f(uAmbientLoc, mat.ambient[0], mat.ambient[1], mat.ambient[2], mat.ambient[3]);
	  glUniform4f(uSpecularLoc, mat.specular[0], mat.specular[1], mat.specular[2], mat.specular[3]);
	  glUniform4f(uEmissiveLoc, mat.emissive[0], mat.emissive[1], mat.emissive[2], mat.emissive[3]);
	  glUniform1f(uShininessLoc, mat.shininess);
	  glUniform1i(uTexCountLoc, mat.texCount);
	  // Dessin 
	  glDrawArrays(
		  GL_TRIANGLES, // mode
		  j, // first : indice du premier sommet à dessiner (on n'est pas obligé de tout dessiner)
		  meshes[i].nbVertices // count : combien de sommets à dessiner
	  );

	  glBindTexture(GL_TEXTURE_2D, 0); // debind de la texture
	  j += meshes[i].nbVertices;
	}
	glBindVertexArray(0); // debind du vao
}

bool AssimpModel::ImportFromFile(const std::string& pFile)
{
	// Création d'une instance de la class d'import d'Assimp.
	Assimp::Importer importer;
  
	// On vérifie que le fichier existe.
    std::ifstream fin(pFile.c_str());
    if(!fin.fail()) {
        fin.close();
    } else {
        std::cerr << "Couldn't open file: " << pFile.c_str() << std::endl;
        std::cerr << importer.GetErrorString() << std::endl;
		return false;
    }
 
    const aiScene* scene = importer.ReadFile(pFile, 
      aiProcess_CalcTangentSpace       | 
      aiProcess_Triangulate            |
      aiProcess_JoinIdenticalVertices  |
      aiProcess_SortByPType); 
 
    // Si l'import échoue, on le signale.
    if(!scene)
    {
        std::cerr << importer.GetErrorString() << std::endl;
		return false;
    }
 
    // On peut maintenant accéder au contenu du fichier.
    // On charge les textures.
    if(!LoadTextures(scene)) {
	  std::cerr << "Couldn't load textures: " << pFile.c_str() << std::endl;
	  return false;
	}
	
	// On construit le modèle 3D.
    if(!BuildAssimpModel(scene)) {
	  std::cerr << "Couldn't build model: " << pFile.c_str() << std::endl;
	  return false;
	}
 
    return true;
}

bool AssimpModel::LoadTextures(const aiScene* scene) {

	/* Recherche de textures dans les matériaux de la scène */
	for (unsigned int m=0; m < scene->mNumMaterials; ++m)
	{
		int texIndex = 0;
		aiString path;	// filename

		aiReturn texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
		while (texFound == AI_SUCCESS) {
			//fill map with textures, OpenGL image ids set to 0
			textureIdMap[path.data] = 0; 
			// more textures?
			texIndex++;
			texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
		}
	}
	
	int numTextures = textureIdMap.size();
	
	if(numTextures == 0) return true;
	
	/* Création des images. */
	unsigned char * pTexture;
    
    /* Génération des textures */
    GLuint* textureIds = new GLuint[numTextures];
    glGenTextures(numTextures, textureIds);
    
    int x;
    int y;
    int comp;
  
	/* get iterator */
	std::map<std::string, GLuint>::iterator itr = textureIdMap.begin();
	int i=0;
	for (; itr != textureIdMap.end(); ++i, ++itr)
	{
		std::string filename = "data/" + (*itr).first; // get filename
		(*itr).second = textureIds[i]; // save texture id for filename in map
		
        pTexture = stbi_load(filename.c_str(), &x, &y, &comp, 3);
		
		/* Création et chargement des textures dans OpenGL */
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureIds[i]);
		
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, pTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		
		glBindTexture(GL_TEXTURE_2D, 0);
		
		/* On libère la mémoire. */
		delete pTexture;
		pTexture = NULL;
	}

	delete[] textureIds;

	return true;
}

bool AssimpModel::BuildAssimpModel(const aiScene* scene) {
	// Pour chaque mesh
	for (int n = 0; n < scene->mNumMeshes; ++n)
	{
		const aiMesh* mesh = scene->mMeshes[n];
		struct Mesh myMesh;
		
		for (int t = 0; t < mesh->mNumFaces; ++t) {
			const aiFace& face = mesh->mFaces[t];
			
			for(int k=0; k<3; ++k) { 
         		// On récupère les informations de vertices.
         		struct Vertex vertex;
				
				vertex.position.x = mesh->mVertices[face.mIndices[k]].x;
				vertex.position.y = mesh->mVertices[face.mIndices[k]].y;
				vertex.position.z = mesh->mVertices[face.mIndices[k]].z;
				
				// On récupère les informations de textures.
				if (mesh->HasTextureCoords(0)) {
					vertex.texCoords.x = mesh->mTextureCoords[0][face.mIndices[k]].x;
					vertex.texCoords.y = 1 - mesh->mTextureCoords[0][face.mIndices[k]].y;
				} else {
					vertex.texCoords.x = mesh->mVertices[face.mIndices[k]].x;
					vertex.texCoords.y = mesh->mVertices[face.mIndices[k]].y;
				}
				
				// On récupère les informations de normales.
				if (mesh->HasNormals()) {
				  vertex.normal.x = mesh->mNormals[face.mIndices[k]].x;
				  vertex.normal.y = mesh->mNormals[face.mIndices[k]].y;
				  vertex.normal.z = mesh->mNormals[face.mIndices[k]].z;
				} else {
				  vertex.normal.x = 1.0f;
				  vertex.normal.y = 1.0f;
				  vertex.normal.z = 1.0f;
				}
				
				vertices.push_back(vertex);
			}
		}
		
		myMesh.nbVertices = mesh->mNumFaces * 3;
			
		aiMaterial* mtl = scene->mMaterials[mesh->mMaterialIndex];
		
		aiString texPath;	//contains filename of texture
		if(AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath)){
				//bind texture
				myMesh.texId = textureIdMap[texPath.data];
				myMesh.material.texCount = 1;
			}
		else
			myMesh.material.texCount = 0;

		set_float4(myMesh.material.diffuse, 0.8f, 0.8f, 0.8f, 1.0f);
		aiColor4D diffuse;
		if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
			color4_to_float4(&diffuse, myMesh.material.diffuse);

		set_float4(myMesh.material.ambient, 0.2f, 0.2f, 0.2f, 1.0f);
		aiColor4D ambient;
		if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
			color4_to_float4(&ambient, myMesh.material.ambient);

		set_float4(myMesh.material.specular, 0.0f, 0.0f, 0.0f, 1.0f);
		aiColor4D specular;
		if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
			color4_to_float4(&specular, myMesh.material.specular);

		set_float4(myMesh.material.emissive, 0.0f, 0.0f, 0.0f, 1.0f);
		aiColor4D emission;
		if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
			color4_to_float4(&emission, myMesh.material.emissive);

		float shininess = 0.0;
		unsigned int max;
		aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
		myMesh.material.shininess = shininess;
			
		meshes.push_back(myMesh);
	}
	
	return true;
}

bool AssimpModel::GenVerticesObjects() {
	// VBO
	vbo.bind(); // Binding du vbo
	  glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(struct Vertex), &vertices[0], GL_STATIC_DRAW); //envoi des données à la carte graphique
	glBindBuffer(GL_ARRAY_BUFFER, 0); // Débind du VBO

	//VAO
	vao.bind(); // Binding du vao
	
	// Activation des attributs de vertex
	glEnableVertexAttribArray(0); // Position des vertices.
	glEnableVertexAttribArray(1); // Normales des vertices.
	glEnableVertexAttribArray(2); // Coordonnées des textures.
	
	// Spécification du format des attributs de vertex
	glBindBuffer(GL_ARRAY_BUFFER, vbo.getGLId()); // on rebind le vbo
	
	// Coordonnées
	glVertexAttribPointer(
		0, // index: indice
		3, // size : nombre de variables composant notre attribut
		GL_FLOAT, // type: type de nos variables
		GL_FALSE, // normalized : attributs normalisés (ramené entre 0 et 1) ou non
		sizeof(struct Vertex), // stride : nb d'octets pour aller à la prochaine occurence de l'attribut
		(const GLvoid*) (offsetof(struct Vertex,position)) // pointer : offset (décalage) en octets de la première occurence de l'attribut dans le VBO
	);
	
	// Normales
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vertex),
		(const GLvoid*) (offsetof(struct Vertex,normal))
	);
	
	// Coordonnées de textures
	glVertexAttribPointer(
		2,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vertex),
		(const GLvoid*) (offsetof(struct Vertex,texCoords))
	);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0); // Débind du VBO
	glBindVertexArray(0); // debind du vao
	
	return true;
}

void AssimpModel::set_float4(glm::vec4& f, float a, float b, float c, float d)
{
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}

void AssimpModel::color4_to_float4(const aiColor4D *c, glm::vec4& f)
{
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}
