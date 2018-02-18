#pragma once

#include "OpenGL.h"
#include <vector>
#include "assimp\assimp\cimport.h"
#include "assimp\assimp\postprocess.h"
#include "assimp\assimp\scene.h"
#include "assimp\assimp\Importer.hpp"
#include "assimp\assimp\postprocess.h"
#include "Globals.h"

class Model
{
public:
	Model();
	~Model();

	// const aiscene* scene = 0;
	void Load(const char* filepath);
	void Clear();
	void Draw();

public:

	const aiScene *scene;
	Assimp::Importer importer;

	std::vector<GLfloat> meshvertices;
	std::vector<GLfloat> meshnormals;
	std::vector<GLfloat> meshtexcoords;
	std::vector<GLubyte> meshindices;
	std::vector<GLubyte> meshmaterialsindices;
	std::vector<GLfloat> meshcolors;
};
