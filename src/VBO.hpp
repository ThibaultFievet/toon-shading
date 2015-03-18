#pragma once
#include "glew/glew.h"

class VBO{
	private:
		GLuint m_nGLId;

		// les deux méthodes suivantes sont privées pour qu'on ne puisse pas les utiliser : pas de copie, pas d'affectation 
		VBO (const VBO& vbo); // Constructeur par copie
		VBO& operator =(const VBO& vbo); // Opérateur de recopie
	
	public:
		// Getter de m_nGLId
		GLuint getGLId() const {
			return m_nGLId;
		}

		// Constructeur par défaut
		VBO(){
			glGenBuffers(1, &m_nGLId);
		}

		// Destructeur
		~VBO(){
			glDeleteBuffers(1, &m_nGLId);
		}

		// Binder un VBO
		void bind(GLenum target = GL_ARRAY_BUFFER) const{
			glBindBuffer(target, m_nGLId);
		}

};
