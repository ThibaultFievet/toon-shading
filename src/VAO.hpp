#pragma once
#include "glew/glew.h"

class VAO{
	private:
		GLuint m_nGLId;

		// les deux méthodes suivantes sont privées pour qu'on ne puisse pas les utiliser : pas de copie, pas d'affectation 
		VAO (const VAO& vao); // Constructeur par copie
		VAO& operator =(const VAO& vao); // Opérateur de recopie
	
	public:
		// Getter de m_nGLId
		GLuint getGLId() const {
			return m_nGLId;
		}

		// Constructeur par défaut
		VAO(){
			glGenVertexArrays(1, &m_nGLId);
		}

		// Destructeur
		~VAO(){
			glDeleteVertexArrays(1, &m_nGLId);
		}

		// Binder un VAO
		void bind(void) const{
			glBindVertexArray(m_nGLId);
		}
};

struct Vertex2DRGB {
	GLfloat x, y;
	GLfloat r, g, b;

	Vertex2DRGB(GLfloat x, GLfloat y, GLfloat r, GLfloat g, GLfloat b):
		x(x), y(y), r(r), g(g), b(b) {
	}
};
