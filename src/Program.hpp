#pragma once

#include <GL/glew.h>
#include "Shader.hpp"

class Program {
	public:
		Program(): m_nGLId(glCreateProgram()) {
		}

		~Program() {
			glDeleteProgram(m_nGLId);
		}
		
		

		GLuint getGLId() const {
			return m_nGLId;
		}

		void attachShader(const Shader& shader) {
			glAttachShader(m_nGLId, shader.getGLId());
		}

		bool link();

		const std::string getInfoLog() const;

		void use() const {
			glUseProgram(m_nGLId);
		}
		
		void stop() const {
			glUseProgram(0);
		}

	private:
		
		GLuint m_nGLId;
};

// Build a GLSL program from source code
Program buildProgram(const GLchar* vsSrc, const GLchar* fsSrc);

// Load source code from files and build a GLSL program
Program loadProgram(const char* vsFile, const char* fsFile);
