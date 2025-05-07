#pragma once

#include "engine.h"

struct VertexV3V2
{
	glm::vec3 pos;
	glm::vec2 uv;
};

GLuint CreateProgramFromSource(String programSource, const char* shaderName);

u32 LoadProgram(App* app, const char* filepath, const char* programName);

Image LoadImage(const char* filename);

void FreeImage(Image image);

GLuint CreateTexture2DFromImage(Image image, GLint texParam);

u32 LoadTexture2D(App* app, const char* filepath, GLuint texParams = GL_LINEAR);

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program);