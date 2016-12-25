// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <stack>   
#include <sstream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
using namespace std;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/tga.c>
#include <common/ray_casting.h>

const int window_width = 800, window_height = 800;

typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float *coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
};

typedef struct Vertexuv {
	float Position[4];
	float Color[4];
	float Normal[3];
	float UV[2];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float *coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
	void SetUV(float *coords) {
		UV[0] = coords[0];
		UV[1] = coords[1];
	}
};

// function prototypes
int initWindow(void);
void initOpenGL(void);
void loadObject(char*, glm::vec4, Vertex * &, GLuint* &, int);
void createVAOs(Vertex[], GLuint[], int);
void createuvVAOs(Vertexuv[], GLuint[], int);
void createObjects(void);
void createMesh(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
void subdiv(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);
fstream myStream;

// GLOBAL VARIABLES
GLFWwindow* window;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint cd = 1;
GLuint anime = 0;
GLuint programID;
GLuint pickingProgramID;
GLuint textureID;
GLuint textureMatrixID;
GLuint textureSamID;
GLuint textureModelMatrixID;
GLuint textureViewMatrixID;
GLuint textureProjMatrixID;

const GLuint NumObjects = 6;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects] = { 0,1,2,3,4,5 };
GLuint VertexBufferId[NumObjects] = { 0,1,2,3,4,5 };
GLuint IndexBufferId[NumObjects] = { 0,1,2,3,4,5 };

size_t NumIndices[NumObjects] = { 0,1,2,3,4.5 };
size_t VertexBufferSize[NumObjects] = { 0,1,2,3,4,5 };
size_t IndexBufferSize[NumObjects] = { 0,1,2,3,4,5 };

long *width = NULL;
long *height = NULL;
GLuint Texture;
GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorID;
GLuint LightID;
GLuint Light2ID;
GLuint ambientID;
glm::vec3 lightPos;
glm::vec3 light2;
GLuint PickingModelMatrixID;
GLuint PickingViewMatrixID;
GLuint PickingProjMatrixID;
GLuint pickingColorArrayRID;


Vertex* Verts1;// , *Verts2, *Verts3, *Verts4, *Verts5, *Verts6, *Verts7, *Verts8, *Verts9;
GLuint* Idcs1;// , *Idcs2, *Idcs3, *Idcs4, *Idcs5, *Idcs6, *Idcs7, *Idcs8;
float pickingColorR[441];
GLuint meshline[2400];
Vertexuv mesh[441];
Vertexuv subVert[3721];
GLuint subIdcs[21600];
GLuint p = 0;

GLuint m = 0,r=0;
GLint gX = 0.0;
GLint gZ = 0.0;
float px = 0.785f;
float py = 0.785f;
int stick = 0,fu = 0;
float x, y, z, lx1, ly1, lz1, lx2, ly2, lz2, theta, beta;
glm::vec3 camMatrix;
glm::vec3 upMatrix = glm::vec3(0.0, 1.0, 0.0);
int sd = 0;
GLuint chicki,micki,smile;
ofstream Outfile;
ifstream Infile;

// animation control
bool animation = false;
GLfloat phi = 0.0;

void loadObject(char* file, glm::vec4 color, Vertex * &out_Vertices, GLuint* &out_Indices, int ObjectId)
{
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, normals);

	std::vector<GLuint> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
	}
	out_Indices = new GLuint[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
	}

	// set global variables!!
	NumIndices[ObjectId] = idxCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLuint) * idxCount;
}


void createObjects(void)
{
	//-- COORDINATE AXES --//
	Vertex CoordVerts[] =
	{
		{ { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 5.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
	};

	VertexBufferSize[0] = sizeof(CoordVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(CoordVerts, NULL, 0);

	//-- GRID --//
	float coords1[3], coords2[3];
	float color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float norm[] = { 0.0f, 0.0f, 0.0f };
	Vertex Grid[84];
	int i = 0;
	float xx = 0.0f;
	for (int yy = 0; yy <= 20; yy++, xx += 0.5f) {
		coords1[0] = -5.0f + xx;
		coords1[1] = 0.0f;
		coords1[2] = -5.0f;
		coords2[0] = -5.0f + xx;
		coords2[1] = 0.0f;
		coords2[2] = 5.0f;
		Grid[i].SetPosition(coords1);
		Grid[i].SetColor(color);
		Grid[i].SetNormal(norm);
		i++;
		Grid[i].SetPosition(coords2);
		Grid[i].SetColor(color);
		Grid[i].SetNormal(norm);
		i++;
	}
	xx = 0.0f;
	for (int yy = 0; yy <= 20; yy++, xx += 0.5f) {
		coords1[0] = -5.0f;
		coords1[1] = 0.0f;
		coords1[2] = -5.0f + xx;
		coords2[0] = 5.0f;
		coords2[1] = 0.0f;
		coords2[2] = -5.0f + xx;
		Grid[i].SetPosition(coords1);
		Grid[i].SetColor(color);
		Grid[i].SetNormal(norm);
		i++;
		Grid[i].SetPosition(coords2);
		Grid[i].SetColor(color);
		Grid[i].SetNormal(norm);
		i++;
	}
	VertexBufferSize[1] = sizeof(Grid);
	createVAOs(Grid, NULL, 1);

	// ATTN: create your grid vertices here!

	//-- .OBJs --//


	// ATTN: load your models here
	loadObject("untitled.obj", glm::vec4(0.5, 0.5, 0.5, 1.0), Verts1, Idcs1, 2);
	createVAOs(Verts1, Idcs1, 2);
	createMesh();
	Texture = load_texture_TGA("maity_brajgopal.tga", width, height, GL_CLAMP, GL_CLAMP);
}
void createMesh()
{
	float uv[2];
	float coords1[3], coords2[3];
	float colorr[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float normm[] = { 0.0f, 0.0f, 1.0f };
	int i = 0;
	float xx;
	float jj = 0.0f;
	for (int j = 0; j <= 20; j++, jj += 0.45f) {
		xx = 0.0f;
		for (int yy = 0; yy <= 20; yy++, xx += 0.45f) {
			coords1[0] = -4.5f + xx;
			coords1[1] = 0.0f + jj;
			coords1[2] = 5.0f;
			uv[0] = yy / 21.0f;
			uv[1] = j / 22.7f;
			mesh[i].SetPosition(coords1);
			mesh[i].SetColor(colorr);
			mesh[i].SetNormal(normm);
			mesh[i].SetUV(uv);
			i++;
		}
	}
	VertexBufferSize[3] = sizeof(mesh);
		
	i = 0;
	int k;
	for (int j = 0; j < 20; j++) {
		k = j * 21;
		for (int m = 0; m < 20; m++) {
			meshline[i++] = k++;
			meshline[i++] = k;
			meshline[i++] = k + 21;
			meshline[i++] = k + 21;
			meshline[i++] = k + 20;
			meshline[i++] = k - 1;
		}
	}
	IndexBufferSize[3] = sizeof(meshline);
	createuvVAOs(mesh, meshline, 3);
}

void cylinder()
{
	float x = 0;
	float z = 0;
	float a = 0;
	int index = 0;
	for (int i = 0;i<21;i++)
	{
		for (int j = 0; j < 21; j++)
		{
			a = (2.0f * 3.14f*j / 20.0f) - 1.31f;
			x = 5 * cos(a);
			z = 5 * sin(a);
			mesh[index].Position[0] = x;
			mesh[index].Position[2] = z;
			index++;
		}
	}
}

void subdiv() {
int rty = 0, qwe = 0;
int ert = 61, wer = 21;
float a1[4] = { 1.0f };
float a2[4] = { 1.0f };
float a3[4] = { 1.0f };
float a4[4] = { 1.0f };
float a5[4] = { 1.0f };
float a6[4] = { 1.0f };
float a7[4] = { 1.0f };
float a8[4] = { 1.0f };
float a9[4] = { 1.0f };
for (int i = 0; i < 20; i++)
{
	rty = 3 * i*ert;
	qwe = i*wer;
	for (int j = 0; j < 20; j++)
	{
		if (i == 0)
		{
			//
		}
		else
		{
			a1[0] = (16.0*mesh[qwe + j].Position[0] +
				4 * (mesh[qwe + j + 1].Position[0] +
					mesh[qwe + j + 21].Position[0] +
					mesh[qwe + j - 1].Position[0] +
					mesh[qwe + j - 21].Position[0]) +
					(mesh[qwe + j + 21 + 1].Position[0] +
						mesh[qwe + j + 21 - 1].Position[0] +
						mesh[qwe + j - 21 + 1].Position[0] +
						mesh[qwe + j - 21 - 1].Position[0])) / 36;

			a1[1] = (16.0*mesh[qwe + j].Position[1] +
				4 * (mesh[qwe + j + 1].Position[1] +
					mesh[qwe + j + 21].Position[1] +
					mesh[qwe + j - 1].Position[1] +
					mesh[qwe + j - 21].Position[1]) +
					(mesh[qwe + j + 21 + 1].Position[1] +
						mesh[qwe + j + 21 - 1].Position[1] +
						mesh[qwe + j - 21 + 1].Position[1] +
						mesh[qwe + j - 21 - 1].Position[1])) / 36;

			a1[2] = (16.0*mesh[qwe + j].Position[2] +
				4 * (mesh[qwe + j + 1].Position[2] +
					mesh[qwe + j + 21].Position[2] +
					mesh[qwe + j - 1].Position[2] +
					mesh[qwe + j - 21].Position[2]) +
					(mesh[qwe + j + 21 + 1].Position[2] +
						mesh[qwe + j + 21 - 1].Position[2] +
						mesh[qwe + j - 21 + 1].Position[2] +
						mesh[qwe + j - 21 - 1].Position[2])) / 36;
			subVert[rty].SetPosition(a1);

			a2[0] = (8 * mesh[qwe + j].Position[0] +
				2 * (mesh[qwe + j + 21].Position[0] + mesh[qwe + j - 21].Position[0]) +
				4 * (mesh[qwe + j + 1].Position[0]) +
				(mesh[qwe + j + 21 + 1].Position[0] + mesh[qwe + j - 21 + 1].Position[0])) / 18;

			a2[1] = (8 * mesh[qwe + j].Position[1] +
				2 * (mesh[qwe + j + 21].Position[1] + mesh[qwe + j - 21].Position[1]) +
				4 * (mesh[qwe + j + 1].Position[1]) +
				(mesh[qwe + j + 21 + 1].Position[1] + mesh[qwe + j - 21 + 1].Position[1])) / 18;

			a2[2] = (8 * mesh[qwe + j].Position[2] +
				2 * (mesh[qwe + j + 21].Position[2] + mesh[qwe + j - 21].Position[2]) +
				4 * (mesh[qwe + j + 1].Position[2]) +
				(mesh[qwe + j + 21 + 1].Position[2] + mesh[qwe + j - 21 + 1].Position[2])) / 18;
			subVert[rty + 1].SetPosition(a2);

			a3[0] = (8 * mesh[qwe + j + 1].Position[0] +
				2 * (mesh[qwe + j + 21 + 1].Position[0] + mesh[qwe + j - 21 + 1].Position[0]) +
				4 * (mesh[qwe + j].Position[0]) +
				(mesh[qwe + j + 21].Position[0] + mesh[qwe + j - 21].Position[0])) / 18;

			a3[1] = (8 * mesh[qwe + j + 1].Position[1] +
				2 * (mesh[qwe + j + 21 + 1].Position[1] + mesh[qwe + j - 21 + 1].Position[1]) +
				4 * (mesh[qwe + j].Position[1]) +
				(mesh[qwe + j + 21].Position[1] + mesh[qwe + j - 21].Position[1])) / 18;

			a3[2] = (8 * mesh[qwe + j + 1].Position[2] +
				2 * (mesh[qwe + j + 21 + 1].Position[2] + mesh[qwe + j - 21 + 1].Position[2]) +
				4 * (mesh[qwe + j].Position[2]) +
				(mesh[qwe + j + 21].Position[2] + mesh[qwe + j - 21].Position[2])) / 18;
			subVert[rty + 2].SetPosition(a3);
		}
		if (j == 0)
		{
			//
		}
		else
		{
			a4[0] = (8 * mesh[qwe + j].Position[0] +
				2 * (mesh[qwe + j - 1].Position[0] + mesh[qwe + j + 1].Position[0]) +
				4 * (mesh[qwe + j + 21].Position[0]) + (mesh[qwe + j + 21 - 1].Position[0] +
					mesh[qwe + j + 21 + 1].Position[0])) / 18;

			a4[1] = (8 * mesh[qwe + j].Position[1] +
				2 * (mesh[qwe + j - 1].Position[1] + mesh[qwe + j + 1].Position[1]) +
				4 * (mesh[qwe + j + 21].Position[1]) + (mesh[qwe + j + 21 - 1].Position[1] +
					mesh[qwe + j + 21 + 1].Position[1])) / 18;

			a4[2] = (8 * mesh[qwe + j].Position[2] +
				2 * (mesh[qwe + j - 1].Position[2] + mesh[qwe + j + 1].Position[2]) +
				4 * (mesh[qwe + j + 21].Position[2]) + (mesh[qwe + j + 21 - 1].Position[2] +
					mesh[qwe + j + 21 + 1].Position[2])) / 18;
			subVert[rty + 61].SetPosition(a4);

			a7[0] = (8 * mesh[qwe + j + 21].Position[0] +
				2 * (mesh[qwe + j + 21 - 1].Position[0] + mesh[qwe + j + 21 + 1].Position[0]) +
				4 * (mesh[qwe + j].Position[0]) +
				(mesh[qwe + j + 1].Position[0] + mesh[qwe + j - 1].Position[0])) / 18;

			a7[1] = (8 * mesh[qwe + j + 21].Position[1] +
				2 * (mesh[qwe + j + 21 - 1].Position[1] + mesh[qwe + j + 21 + 1].Position[1]) +
				4 * (mesh[qwe + j].Position[1]) +
				(mesh[qwe + j + 1].Position[1] + mesh[qwe + j - 1].Position[1])) / 18;

			a7[2] = (8 * mesh[qwe + j + 21].Position[2] +
				2 * (mesh[qwe + j + 21 - 1].Position[2] + mesh[qwe + j + 21 + 1].Position[2]) +
				4 * (mesh[qwe + j].Position[2]) +
				(mesh[qwe + j + 1].Position[2] + mesh[qwe + j - 1].Position[2])) / 18;
			subVert[rty + 122].SetPosition(a7);
		}
		a5[0] = (4 * mesh[qwe + j].Position[0] +
			2 * (mesh[qwe + j + 1].Position[0] + mesh[qwe + j + 21].Position[0]) +
			mesh[qwe + j + 21 + 1].Position[0]) / 9;

		a5[1] = (4 * mesh[qwe + j].Position[1] +
			2 * (mesh[qwe + j + 1].Position[1] + mesh[qwe + j + 21].Position[1]) +
			mesh[qwe + j + 21 + 1].Position[1]) / 9;

		a5[2] = (4 * mesh[qwe + j].Position[2] +
			2 * (mesh[qwe + j + 1].Position[2] + mesh[qwe + j + 21].Position[2]) +
			mesh[qwe + j + 21 + 1].Position[2]) / 9;
		subVert[rty + 61 + 1].SetPosition(a5);

		a6[0] = (4 * mesh[qwe + j + 1].Position[0] +
			2 * (mesh[qwe + j].Position[0] + mesh[qwe + j + 21 + 1].Position[0]) +
			mesh[qwe + j + 21].Position[0]) / 9;

		a6[1] = (4 * mesh[qwe + j + 1].Position[1] +
			2 * (mesh[qwe + j].Position[1] + mesh[qwe + j + 21 + 1].Position[1]) +
			mesh[qwe + j + 21].Position[1]) / 9;

		a6[2] = (4 * mesh[qwe + j + 1].Position[2] +
			2 * (mesh[qwe + j].Position[2] + mesh[qwe + j + 21 + 1].Position[2]) +
			mesh[qwe + j + 21].Position[2]) / 9;
		subVert[rty + 61 + 2].SetPosition(a6);

		a8[0] = (4 * mesh[qwe + j + 21].Position[0] +
			2 * (mesh[qwe + j].Position[0] + mesh[qwe + j + 21 + 1].Position[0]) +
			(mesh[qwe + j + 1].Position[0])) / 9;

		a8[1] = (4 * mesh[qwe + j + 21].Position[1] +
			2 * (mesh[qwe + j].Position[1] + mesh[qwe + j + 21 + 1].Position[1]) +
			(mesh[qwe + j + 1].Position[1])) / 9;

		a8[2] = (4 * mesh[qwe + j + 21].Position[2] +
			2 * (mesh[qwe + j].Position[2] + mesh[qwe + j + 21 + 1].Position[2]) +
			(mesh[qwe + j + 1].Position[2])) / 9;
		subVert[rty + 122 + 1].SetPosition(a8);

		a9[0] = (4 * mesh[qwe + j + 21 + 1].Position[0] +
			2 * (mesh[qwe + j + 1].Position[0] + mesh[qwe + j + 21].Position[0]) +
			(mesh[qwe + j].Position[0])) / 9;

		a9[1] = (4 * mesh[qwe + j + 21 + 1].Position[1] +
			2 * (mesh[qwe + j + 1].Position[1] + mesh[qwe + j + 21].Position[1]) +
			(mesh[qwe + j].Position[1])) / 9;

		a9[2] = (4 * mesh[qwe + j + 21 + 1].Position[2] +
			2 * (mesh[qwe + j + 1].Position[2] + mesh[qwe + j + 21].Position[2]) +
			(mesh[qwe + j].Position[2])) / 9;
		subVert[rty + 122 + 2].SetPosition(a9);
		rty = rty + 3;
		//qwe++;
	}
}
/////////////boundary///////////////////////
rty = 0;
qwe = 0;
for (int i = 0; i < 20; i++)
{
	subVert[rty].SetPosition(mesh[qwe].Position);
	a1[0] = (2 * mesh[qwe].Position[0] + mesh[qwe + 1].Position[0]) / 3;
	a1[1] = (2 * mesh[qwe].Position[1] + mesh[qwe + 1].Position[1]) / 3;
	a1[2] = (2 * mesh[qwe].Position[2] + mesh[qwe + 1].Position[2]) / 3;
	subVert[rty + 1].SetPosition(a1);

	a2[0] = (2 * mesh[qwe + 1].Position[0] + mesh[qwe].Position[0]) / 3;
	a2[1] = (2 * mesh[qwe + 1].Position[1] + mesh[qwe].Position[1]) / 3;
	a2[2] = (2 * mesh[qwe + 1].Position[2] + mesh[qwe].Position[2]) / 3;
	subVert[rty + 2].SetPosition(a2);
	rty = rty + 3;
	qwe++;
}

rty = 60;
qwe = 20;
for (int i = 0; i < 20; i++)
{
	subVert[rty].SetPosition(mesh[qwe].Position);
	a1[0] = (2 * mesh[qwe].Position[0] + mesh[qwe + 21].Position[0]) / 3;
	a1[1] = (2 * mesh[qwe].Position[1] + mesh[qwe + 21].Position[1]) / 3;
	a1[2] = (2 * mesh[qwe].Position[2] + mesh[qwe + 21].Position[2]) / 3;
	subVert[rty + 61].SetPosition(a1);

	a2[0] = (2 * mesh[qwe + 21].Position[0] + mesh[qwe].Position[0]) / 3;
	a2[1] = (2 * mesh[qwe + 21].Position[1] + mesh[qwe].Position[1]) / 3;
	a2[2] = (2 * mesh[qwe + 21].Position[2] + mesh[qwe].Position[2]) / 3;
	subVert[rty + 122].SetPosition(a2);
	rty = rty + 183;
	qwe = qwe + 21;
}

rty = 0;
qwe = 0;
for (int i = 0; i < 20; i++)
{
	subVert[rty].SetPosition(mesh[qwe].Position);
	a1[0] = (2 * mesh[qwe].Position[0] + mesh[qwe + 21].Position[0]) / 3;
	a1[1] = (2 * mesh[qwe].Position[1] + mesh[qwe + 21].Position[1]) / 3;
	a1[2] = (2 * mesh[qwe].Position[2] + mesh[qwe + 21].Position[2]) / 3;
	subVert[rty + 61].SetPosition(a1);

	a2[0] = (2 * mesh[qwe + 21].Position[0] + mesh[qwe].Position[0]) / 3;
	a2[1] = (2 * mesh[qwe + 21].Position[1] + mesh[qwe].Position[1]) / 3;
	a2[2] = (2 * mesh[qwe + 21].Position[2] + mesh[qwe].Position[2]) / 3;
	subVert[rty + 122].SetPosition(a2);
	rty = rty + 183;
	qwe = qwe + 21;
}

rty = 3660;
qwe = 420;
for (int i = 0; i < 20; i++)
{
	subVert[rty].SetPosition(mesh[qwe].Position);
	a1[0] = (2 * mesh[qwe].Position[0] + mesh[qwe + 1].Position[0]) / 3;
	a1[1] = (2 * mesh[qwe].Position[1] + mesh[qwe + 1].Position[1]) / 3;
	a1[2] = (2 * mesh[qwe].Position[2] + mesh[qwe + 1].Position[2]) / 3;
	subVert[rty + 1].SetPosition(a1);

	a2[0] = (2 * mesh[qwe + 1].Position[0] + mesh[qwe].Position[0]) / 3;
	a2[1] = (2 * mesh[qwe + 1].Position[1] + mesh[qwe].Position[1]) / 3;
	a2[2] = (2 * mesh[qwe + 1].Position[2] + mesh[qwe].Position[2]) / 3;
	subVert[rty + 2].SetPosition(a2);
	rty = rty + 3;
	qwe++;
}

subVert[3720].SetPosition(mesh[440].Position);
/////////////////////////
float c[] = { 1.0f, 0.0f, 0.0f, 1.0f };
float n[] = { 0.0, 1.0, 1.0 };
float u[] = { 0.0f,0.0f };
int ik = 0;
for (int i = 0; i < 61; i++)
{
	for (int j = 0; j < 61; j++)
	{
		u[0] = j / 63.0f;
		u[1] = i / 67.1f;
		subVert[ik].SetColor(c);
		subVert[ik].SetNormal(n);
		subVert[ik].SetUV(u);
		ik++;
	}
 }
/////////////////////////////////////
rty = 0;
ert = 61;
int idc = 0;
for (int i = 0; i < 60; i++)
{
	//Idcs[i] = i;
	rty = i*ert;
	for (int j = 0; j < 60; j++)
	{
		subIdcs[idc] = rty++;
		subIdcs[idc + 1] = rty;
		//Idcs[idc + 2] = rty;
		subIdcs[idc + 2] = rty + ert;
		subIdcs[idc + 3] = rty + ert;
		//	Idcs[idc + 4] = rty - 1;
		subIdcs[idc + 4] = rty + ert - 1;
		subIdcs[idc + 5] = rty - 1;
		idc += 6;
	}
}
//////////////////////////////////////////
}

int rayCast(vec3 unpro, vec3 unpro1,Vertex* Verts1,GLuint* Idcs1) {
	int ind = -1, k = 0;
	for (int i = 0; i < NumIndices[2]; i = i + 3)
	{
		float cg[3] = { 0.0 };
		int j = 0;
		float v1[3] = { Verts1[Idcs1[i]].Position[0],Verts1[Idcs1[i]].Position[1],Verts1[Idcs1[i]].Position[2] };
		float v2[3] = { Verts1[Idcs1[i + 1]].Position[0],Verts1[Idcs1[i + 1]].Position[1],Verts1[Idcs1[i + 1]].Position[2] };
		float v3[3] = { Verts1[Idcs1[i + 2]].Position[0],Verts1[Idcs1[i + 2]].Position[1],Verts1[Idcs1[i + 2]].Position[2] };
		float l[3] = { unpro[0],unpro[1],unpro[2] };
		float r[3] = { unpro1[0],unpro1[1],unpro1[2] };
		ray_cast(v1, v2, v3, l, r, cg);
		while (cg[j] > 0)
		{
			j++;
		}
		if (j == 3 && k == 0)
		{
			ind = Idcs1[i + 1];
			k = 1;
		}
		else if (j == 3 && k == 1)
		{
			float dis = distance(unpro, vec3(v2[0], v2[1], v2[2]));
			float dis2 = distance(unpro, vec3(Verts1[ind].Position[0], Verts1[ind].Position[1], Verts1[ind].Position[2]));
			if (dis<dis2)
				ind = Idcs1[i + 1];
		}
	}
	return ind;
}
void rayCast2() {
	int tr = -1;
	for (int i = 0; i < 441; i++)
	{
		vec3 f = { mesh[i].Position[0],mesh[i].Position[1],mesh[i].Position[2] };
		vec3 center = { 0,mesh[i].Position[1],0 };
		tr = rayCast(f, center - f, Verts1,Idcs1);
		if (tr >= 0)
		{
			mesh[i].Position[0] = Verts1[tr].Position[0];
			mesh[i].Position[1] = Verts1[tr].Position[1];
			mesh[i].Position[2] = Verts1[tr].Position[2];
		}
	}
}

void animate() {
	if (anime == 1)
	{
		if (chicki <= 60 && smile == 1) {	
			mesh[157].Position[1] -= 0.01f;
			mesh[136].Position[1] -= 0.01f;
			mesh[155].Position[1] -= 0.01f;
			mesh[134].Position[1] -= 0.01f;		
			chicki++;
			if (chicki == 60) {
				smile = 0;
				micki = 0;
			}
		}
		else if (micki <=60 && smile == 0){
			mesh[157].Position[1] += 0.01f;
			mesh[136].Position[1] += 0.01f;
			mesh[155].Position[1] += 0.01f;
			mesh[134].Position[1] += 0.01f;
			micki++;
			if (micki == 60) {
				smile = 1;
				chicki = 0;
			}
		}
		subdiv();
	}
}
void reset(void) {
	if (r == 1) {
		r = 0;
		px = 0.785f;
		py = 0.785f;
		y = 22.0f*sin(py);
		z = 22.0f*cos(py)*cos(px);
		x = 22.0f*cos(py)*sin(px);
		lz1 = 10.0f*cos(py)*cos(px - 0.2f);
		lz2 = 10.0f*cos(py)*cos(px + 0.2f);
		lx1 = 10.0f*cos(py)*sin(px - 0.2f);
		lx2 = 10.0f*cos(py)*sin(px + 0.2f);
		ly1 = 10.0f*sin(py);
		ly2 = 10.0f*sin(py);
		lightPos = glm::vec3(lx1, ly1, lz1);
		light2 = glm::vec3(lx2, ly2, lz2);
		camMatrix = glm::vec3(x, y, z);
		upMatrix = glm::vec3(0.0, 1.0, 0.0);
		gViewMatrix = glm::lookAt(camMatrix,	// eye
			glm::vec3(0.0, 0.0, 0.0),	// center
			upMatrix);	// up
		createMesh();
		subdiv();
	}

}
void rotateCam(void) {
	
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			py += 0.01f;
			if (py > 6.28f)
				py = 0.00f;
			y = 22.0f*sin(py);
			z = 22.0f*cos(py)*cos(px);
			x = 22.0f*cos(py)*sin(px);
			camMatrix = glm::vec3(x, y, z);
			lz1 = 10.0f*cos(py)*cos(px - 0.2f);
			lz2 = 10.0f*cos(py)*cos(px + 0.2f);
			lx1 = 10.0f*cos(py)*sin(px - 0.2f);
			lx2 = 10.0f*cos(py)*sin(px + 0.2f);
			ly1 = 10.0f*sin(py);
			ly2 = 10.0f*sin(py);
			lightPos = glm::vec3(lx1, ly1, lz1);
			light2 = glm::vec3(lx2, ly2, lz2);
			if (py > 1.57  && py < 4.71)
				upMatrix = glm::vec3(0.0, -1.0, 0.0);
			else
				upMatrix = glm::vec3(0.0, 1.0, 0.0);
			gViewMatrix = glm::lookAt(camMatrix,	// eye
				glm::vec3(0.0, 0.0, 0.0),	// center
				upMatrix);	// up
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			py -= 0.01f;
			if (py < 0.00f)
				py = 6.28f;
			y = 22.0f*sin(py);
			z = 22.0f*cos(py)*cos(px);
			x = 22.0f*cos(py)*sin(px);
			camMatrix = glm::vec3(x, y, z);
			lz1 = 10.0f*cos(py)*cos(px - 0.2f);
			lz2 = 10.0f*cos(py)*cos(px + 0.2f);
			lx1 = 10.0f*cos(py)*sin(px - 0.2f);
			lx2 = 10.0f*cos(py)*sin(px + 0.2f);
			ly1 = 10.0f*sin(py);
			ly2 = 10.0f*sin(py);
			lightPos = glm::vec3(lx1, ly1, lz1);
			light2 = glm::vec3(lx2, ly2, lz2);  
			if (py > 1.57  && py < 4.71)
				upMatrix = glm::vec3(0.0, -1.0, 0.0);
			else
				upMatrix = glm::vec3(0.0, 1.0, 0.0);
			gViewMatrix = glm::lookAt(camMatrix,	// eye
				glm::vec3(0.0, 0.0, 0.0),	// center
				upMatrix);	// up
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			px -= 0.01f;
			if (px < 0.00f)
				px = 6.28f;
			x = 22.0f*cos(py)*sin(px);
			z = 22.0f*cos(py)*cos(px);
			camMatrix = glm::vec3(x, y, z);
			lz1 = 10.0f*cos(py)*cos(px - 0.2f);
			lz2 = 10.0f*cos(py)*cos(px + 0.2f);
			lx1 = 10.0f*cos(py)*sin(px - 0.2f);
			lx2 = 10.0f*cos(py)*sin(px + 0.2f);
			ly1 = 10.0f*sin(py);
			ly2 = 10.0f*sin(py);
			lightPos = glm::vec3(lx1, ly1, lz1);
			light2 = glm::vec3(lx2, ly2, lz2);
			if (py > 1.57  && py < 4.71)
				upMatrix = glm::vec3(0.0, -1.0, 0.0);
			else
				upMatrix = glm::vec3(0.0, 1.0, 0.0);
			gViewMatrix = glm::lookAt(camMatrix,	// eye
				glm::vec3(0.0, 0.0, 0.0),	// center
				upMatrix);	// up
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			px += 0.01f;
			if (px > 6.28f)
				px = 0.00f;
			x = 22.0f*cos(py)*sin(px);
			z = 22.0f*cos(py)*cos(px);
			camMatrix = glm::vec3(x, y, z);
			lz1 = 10.0f*cos(py)*cos(px - 0.2f);
			lz2 = 10.0f*cos(py)*cos(px + 0.2f);
			lx1 = 10.0f*cos(py)*sin(px - 0.2f);
			lx2 = 10.0f*cos(py)*sin(px + 0.2f);
			ly1 = 10.0f*sin(py);
			ly2 = 10.0f*sin(py);
			lightPos = glm::vec3(lx1, ly1, lz1);
			light2 = glm::vec3(lx2, ly2, lz2);
			if (py > 1.57  && py < 4.71)
				upMatrix = glm::vec3(0.0, -1.0, 0.0);
			else
				upMatrix = glm::vec3(0.0, 1.0, 0.0);
			gViewMatrix = glm::lookAt(camMatrix,	// eye
				glm::vec3(0.0, 0.0, 0.0),	// center
				upMatrix);	// up
		}
	
}

void renderScene(void)
{
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!


	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		//glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glm::mat4x4 ModelMatrix = glm::mat4(1.0);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(Light2ID, light2.x, light2.y, light2.z);
		glUniform3f(ambientID, 1, 1, 1);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);


		glBindVertexArray(VertexArrayId[0]);	// draw CoordAxes
		glDrawArrays(GL_LINES, 0, 6);
		glBindVertexArray(0);

		glBindVertexArray(VertexArrayId[1]);
		glDrawArrays(GL_LINES, 0, 84);
		glBindVertexArray(0);
		if (fu == 1) {
			glUniform3f(ambientID, 0.2f, 0.2f, 0.2f);
			glBindVertexArray(VertexArrayId[2]);
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[2]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Verts1), Verts1);
			glDrawElements(GL_TRIANGLES, NumIndices[2], GL_UNSIGNED_INT, (void *)0);
			glBindVertexArray(0);
		}
		if (p == 0 && m == 1) {
			glBindVertexArray(VertexArrayId[3]);
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mesh), mesh);
			glDrawElements(GL_LINES, 2400, GL_UNSIGNED_INT, (void *)0);
			glBindVertexArray(0);						
		}
		if (sd == 1 && cd == 1) {
			glPointSize(5.0);
			glBindVertexArray(VertexArrayId[5]);
			glDrawArrays(GL_POINTS, 0, 3721);
			glBindVertexArray(0);
		}
	}
	glUseProgram(0);
	glUseProgram(textureID);
	{
		if (m == 1) {
			glm::mat4x4 ModelMatrix = glm::mat4(1.0);
			glUniformMatrix4fv(textureViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
			glUniformMatrix4fv(textureProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
			glUniformMatrix4fv(textureModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			//Texture = load_texture_TGA("maity_brajgopal.tga", width, height, GL_CLAMP, GL_CLAMP);
			//glUniform1i(textureSamID, Texture);

			//glUniform3f(ambientID, 1, 1, 1);
			

			

			if (sd == 1) {
				glBindVertexArray(VertexArrayId[5]);
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[5]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subVert), subVert);
				glDrawElements(GL_TRIANGLES, 21600, GL_UNSIGNED_INT, (void *)0);
				glBindVertexArray(0);
			}
			else {
				//glUniform3f(ambientID, 1, 1, 1);
				glBindVertexArray(VertexArrayId[3]);
				glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(mesh), mesh);
				glDrawElements(GL_TRIANGLES, 2400, GL_UNSIGNED_INT, (void *)0);
				glBindVertexArray(0);
			}

			if (p == 0) {
				glPointSize(10.0);
				glBindVertexArray(VertexArrayId[3]);
				glDrawArrays(GL_POINTS, 0, 441);
				glBindVertexArray(0);
			}
		}
	}
	glUseProgram(0);
	// Draw GUI
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void pickObject(void)
{
	// Clear the screen in white
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(pickingProgramID);
	{
		glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
		glUniform3fv(pickingColorArrayRID, 441, pickingColorR);
		glUniformMatrix4fv(PickingViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(PickingProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(PickingModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glEnable(GL_PROGRAM_POINT_SIZE);
		glPointSize(10.0);
	
		glBindVertexArray(VertexArrayId[3]);									// draw MESH GRID
		glDrawArrays(GL_POINTS, 0, 441);
		glBindVertexArray(0);
		// Send our transformation to the currently bound shader, in the "MVP" uniform
		//glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
		
		// ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
		glBindVertexArray(0);

	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow ! 
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel, 
	// because the framebuffer is on the GPU.
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	// Convert the color back to an integer ID
	gPickedIndex = int(data[0]) + int(data[1] * 256) + int(data[2] * 256 * 256);
	//printf("%d \n", gPickedIndex);
	if (gPickedIndex == 1644825){ // Full white, must be the background !
		gMessage = "background";
	}
	else if (gPickedIndex < 441) {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
	}

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

void moveVertex(int gPickedIndex)
{
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glm::vec4 vp = glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	if (gPickedIndex < 441) {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
		int i = gPickedIndex;

		glm::vec3 pos = glm::vec3(xpos, (window_height - ypos), 0.0);
		glm::vec3 pos1 = glm::vec3(xpos, (window_height - ypos), 1.0);
		glm::vec3 unpro = glm::unProject(pos, gViewMatrix, gProjectionMatrix, vp);
		glm::vec3 unpro1 = glm::unProject(pos1, gViewMatrix, gProjectionMatrix, vp);

		glm::vec3 v1 = glm::normalize(unpro1 - unpro);
		glm::vec3 po = glm::vec3(mesh[gPickedIndex].Position[0], mesh[gPickedIndex].Position[1], mesh[gPickedIndex].Position[2]);
		glm::vec3 v2 = po - unpro;
		float firstDot = glm::dot(v1, v1);
		float secDot = glm::dot(v2, v1);
		float t = secDot / firstDot;
		glm::vec3 fin = unpro + v1*t;
		if (stick == 1)
		{
			int j = 0;
			j = rayCast(fin, unpro1 - fin, Verts1,Idcs1);
			if (j >= 0)
			{
				mesh[gPickedIndex].Position[0] = Verts1[j].Position[0];
				mesh[gPickedIndex].Position[1] = Verts1[j].Position[1];
				mesh[gPickedIndex].Position[2] = Verts1[j].Position[2];// -Grid_Vertex_three[gPickedIndex].Position[0];
			}
		}
		else
		{
			mesh[gPickedIndex].Position[0] = fin[0];
			mesh[gPickedIndex].Position[1] = fin[1];
			mesh[gPickedIndex].Position[2] = fin[2];
		}
	}

}

int initWindow(void)
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Maity,BrajaGopal(81490468)", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void)
{

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 1.0f / 1.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	// Camera matrix
	y = 22.0f*sin(py);
	z = 22.0f*cos(py)*cos(px);
	x = 22.0f*cos(py)*sin(px);
	camMatrix = glm::vec3(x, y, z);
	lz1 = 10.0f*cos(py)*cos(px - 0.2f);
	lz2 = 10.0f*cos(py)*cos(px + 0.2f);
	lx1 = 10.0f*cos(py)*sin(px - 0.2f);
	lx2 = 10.0f*cos(py)*sin(px + 0.2f);
	ly1 = 10.0f*sin(py);
	ly2 = 10.0f*sin(py);
	lightPos = glm::vec3(lx1, ly1, lz1);
	light2 = glm::vec3(lx2, ly2, lz2);

	gViewMatrix = glm::lookAt(camMatrix,	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		upMatrix);	// up


	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	textureID = LoadShaders("Texture.vertexshader", "Texture.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");
	
	//textureMatrixID = glGetUniformLocation(textureID, "MVP");
	textureSamID = glGetUniformLocation(textureID, "myTextureSampler");
	textureModelMatrixID = glGetUniformLocation(textureID, "M");
	textureViewMatrixID = glGetUniformLocation(textureID, "V");
	textureProjMatrixID = glGetUniformLocation(textureID, "P");

	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	PickingModelMatrixID = glGetUniformLocation(pickingProgramID, "M");
	PickingViewMatrixID = glGetUniformLocation(pickingProgramID, "V");
	PickingProjMatrixID = glGetUniformLocation(pickingProgramID, "P");
	// Get a handle for our "pickingColorID" uniform
	pickingColorArrayRID = glGetUniformLocation(pickingProgramID, "PickingColorArrayR");
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	Light2ID = glGetUniformLocation(programID, "LightP2");
	ambientID = glGetUniformLocation(programID, "lit");

	createObjects();
	
}


void createVAOs(Vertex Vertices[], unsigned int Indices[], int ObjectId) {

	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
	glBindVertexArray(VertexArrayId[ObjectId]);		//

	// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset); 
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal

	// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
			);
	}
}

void createuvVAOs(Vertexuv Vertices[], unsigned int Indices[], int ObjectId) {

	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;
	const size_t UVoffset = sizeof(Vertices[0].Normal) + Normaloffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
	glBindVertexArray(VertexArrayId[ObjectId]);		//

													// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	//if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	//}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)UVoffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal
	glEnableVertexAttribArray(3);	// uvs

									// Disable our Vertex Buffer Object 
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
		);
	}
}

void cleanup(void)
{
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// ATTN: MODIFY AS APPROPRIATE
	int i;
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_A:
			cylinder();
			break;
		case GLFW_KEY_T:
			rayCast2();
			break;
		case GLFW_KEY_W:
			stick = (stick + 1) % 2;
			break;
		case GLFW_KEY_F:
			fu = (fu + 1) % 2;
			break;
		case GLFW_KEY_S:
			Outfile.open("cp.p3");
			for (int i = 0; i < 441; i++)
			{
				Outfile << mesh[i].Position[0] << " ";
				Outfile << mesh[i].Position[1] << " ";
				Outfile << mesh[i].Position[2] << " ";
				Outfile << mesh[i].Position[3] << " ";
			}
			Outfile.close();
			break;
		case GLFW_KEY_L:
			Infile.open("cp.p3");
			if (!Infile)
				printf("error");
			if (!Infile)
				printf("error");
			for (int i = 0; i < 441; i++)
			{
				Infile >> mesh[i].Position[0];
				Infile >> mesh[i].Position[1];
				Infile >> mesh[i].Position[2];
				Infile >> mesh[i].Position[3];
			}
			Infile.close();
			break;
		case GLFW_KEY_SPACE:
			break;
		case GLFW_KEY_C:
			m = (m + 1) % 2;
			break;
		case GLFW_KEY_R:
			r = 1;
			break;
		case GLFW_KEY_P:
			p = (p + 1) % 2;
			cd = (cd + 1) % 2;
			break;
		case GLFW_KEY_N:
			anime = (anime + 1) % 2;
			mesh[157].Position[1] -= 0.3f;
			mesh[136].Position[1] -= 0.3f;
			mesh[155].Position[1] -= 0.3f;
			mesh[134].Position[1] -= 0.3f;
			chicki = micki = smile = 0;
			break;
		default:
			break;
		}
	}
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
		moveVertex(gPickedIndex);
	}
}

int main(void)
{
	// initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;
	
	// initialize OpenGL pipeline
	initOpenGL();


	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	int pg = 0;
	int r, g, b;
	for (int k = 0; k < 441; k++)
	{
		r = (k & 0x000000FF) >> 0;
		g = (k & 0x0000FF00) >> 8;
		b = (k & 0x00FF0000) >> 16;
		pickingColorR[pg] = r / 255.0f;
		pickingColorR[pg + 1] = g / 255.0f;
		pickingColorR[pg + 2] = b / 255.0f;
		pg += 3;
	
	}
	do {
		//// Measure speed
		//double currentTime = glfwGetTime();
		//nbFrames++;
		//if (currentTime - lastTime >= 1.0){ // If last prinf() was more than 1sec ago
		//	// printf and reset
		//	printf("%f ms/frame\n", 1000.0 / double(nbFrames));
		//	nbFrames = 0;
		//	lastTime += 1.0;
		//}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
			moveVertex(gPickedIndex);
		rotateCam();
		reset();
		animate();
		if (animation){
			phi += 0.01;
			if (phi > 360)
				phi -= 360;
		}
		if (glfwGetKey(window, GLFW_KEY_D) && sd == 0)
		{
			subdiv();
			VertexBufferSize[5] = sizeof(subVert);
			IndexBufferSize[5] = sizeof(subIdcs);
			createuvVAOs(subVert, subIdcs, 5);
			sd = 1;
		}
		// DRAWING POINTS
		renderScene();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}