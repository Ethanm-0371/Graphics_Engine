//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include <glad/glad.h>

typedef glm::vec2  vec2;
typedef glm::vec3  vec3;
typedef glm::vec4  vec4;
typedef glm::ivec2 ivec2;
typedef glm::ivec3 ivec3;
typedef glm::ivec4 ivec4;
typedef glm::mat4  mat4;

#pragma region Resource Management

struct Image
{
	void* pixels;
	ivec2 size;
	i32   nchannels;
	i32   stride;
};

struct Texture
{
	GLuint      handle;
	std::string filepath;
};

struct Cubemap
{
	GLuint      handle;
	std::vector<std::string> filepaths;
};

//VBO, EBO, shader, VAO stuff
struct VertexBufferAttribute
{
	u8 location;
	u8 componentCount;
	u8 offset;
};

struct VertexBufferLayout
{
	std::vector<VertexBufferAttribute> attributes;
	u8 stride;
};

struct VertexShaderAttribute
{
	u8 location;
	u8 componentCount;
};

struct VertexShaderLayout
{
	std::vector<VertexShaderAttribute> attributes;
};

struct Vao
{
	GLuint handle;
	GLuint programHandle;
};

//3D Model
struct Model
{
	u32 meshIdx;
	std::vector<u32> materialIdx;
};

struct Submesh
{
	VertexBufferLayout vertexBufferLayout;
	std::vector<float> vertices;
	std::vector<u32> indices;
	u32 vertexOffset;
	u32 indexOffset;

	std::vector<Vao> vaos;
};

struct Mesh
{
	std::vector<Submesh> submeshes;
	GLuint vertexBufferHandle;
	GLuint indexBufferHandle;
};

struct Material
{
	std::string name;
	vec3 albedo;
	vec3 emissive;
	f32 smoothness;
	u32 albedoTextureIdx;
	u32 emissiveTextureIdx;
	u32 specularTextureIdx;
	u32 normalsTextureIdx;
	u32 bumpTextureIdx;
};

struct Program
{
	GLuint             handle;
	std::string        filepath;
	std::string        programName;
	u64                lastWriteTimestamp;
	VertexShaderLayout vertexInputLayout;
};

#pragma endregion

enum Mode
{
	Mode_TexturedQuad,
	Mode_Meshes,
	Mode_FrameBuffer,
	Mode_DeferredRenderTextures,
	Mode_Count
};

enum RenderTextureMode
{
	RendTexMode_Albedo,
	RendTexMode_Normals,
	RendTexMode_Position,
	RendTexMode_Depth,
	RendTexMode_Deferred,
	RendTexMode_Count
};

//Buffer
struct Buffer
{
	GLuint handle;
	GLenum type;
	u32 size;
	u32 head;
	void* data; //mapped data
};

//Camera
struct Camera
{
	mat4 transformation;

	vec3 pivot;
	float pivotDistance;

	float aspectRatio;
	float znear;
	float zfar;
	float fov;
};

//Entity
struct Entity
{
	//vec3 position;
	//vec3 scale;
	mat4 transformationMatrix;
	u32 model;

	u32 head;
	u32 size;
};

//Lights
enum LightType
{
	LightType_Directional,
	LightType_Point,
};

struct Light
{
	unsigned int type;
	unsigned int strength;
	vec3 color;
	vec3 direction;
	vec3 position;
};

//App
struct OpenGLInfo
{
	char* version;
	char* renderer;
	char* vendor;
	char* versionGLSL;
	int numExtensions;
	char** extensions;
};

struct App
{
	// Loop
	f32  deltaTime;
	bool isRunning;

	// Input
	Input input;

	// Graphics
	char gpuName[64];
	char openGlVersion[64];
	OpenGLInfo GLInfo;

	ivec2 displaySize;

	std::vector<Texture> textures;
	std::vector<Cubemap> cubemaps;
	std::vector<Material> materials;
	std::vector<Mesh> meshes;
	std::vector<Model> models;
	std::vector<Program> programs;

	// program indices
	u32 texturedGeometryProgramIdx;
	u32 texturedMeshProgramIdx;
	u32 renderTexturesProgramIdx;
	u32 deferredLightingProgramIdx;
	u32 lightVisualizationProgramIdx;
	u32 skyboxProgramIdx;
	u32 skyboxReflectionProgramIdx;

	// texture indices
	u32 diceTexIdx;
	u32 whiteTexIdx;
	u32 blackTexIdx;
	u32 normalTexIdx;
	u32 magentaTexIdx;

	// cubemap indices
	u32 meadowSkyboxTexIdx;
	u32 langholmenSkyboxTexIdx;
	u32 SFParkSkyboxTexIdx;
	u32 bikiniBottomSkyboxTexIdx;
	u32 hornstullsStrandSkyboxTexIdx;
	u32 pondSkyboxTexIdx;
	u32 powerLinesSkyboxTexIdx;
	u32 swedishRoyalCastleSkyboxTexIdx;
	u32 yokohamaSkyboxTexIdx;

	int currentSkybox;

	//model indices
	u32 patrickModel;
	u32 planeModel;

	//Scene entities
	std::vector<Entity> entityList;

	//Scene lights
	std::vector<Light> lightList;

	// Mode
	Mode mode;
	RenderTextureMode renderTexMode;
	Camera camera;

	// Location of the texture uniform in the textured quad shader
	GLuint programUniformTexture;

	// Location of the texture uniform in the mesh shader???
	GLuint texturedMeshProgram_uTexture;

	// Location of the texture uniform in the render texture shader???
	GLuint renderTexturesProgram_uTexture;
	GLuint renderTexturesProgram_cubeTexture;

	// Location of the texture uniforms in the lighting pass shader???
	GLuint deferredLightingPass_posTexture;
	GLuint deferredLightingPass_normalTexture;
	GLuint deferredLightingPass_albedoTexture;

	// Location of the texture uniforms in the skybox shader???
	GLuint skybox_uMatrix;
	GLuint skybox_uTexture;

	// VAOs
	GLuint targetQuad_vao;
	GLuint cube_vao;
	GLuint sphere_vao;

	GLuint skybox_vao;
	mat4 skyboxViewProjection;

	//Uniforms buffer
	Buffer uniformsBuffer;
	GLint maxUniformBufferSize;
	GLint uniformBlockAlignment;

	GLint globalParamsSize;

	//Light matrices buffer
	Buffer lightMatricesBuffer;

	GLuint albedoAttachmentHandle;
	GLuint normalsAttachmentHandle;
	GLuint positionAttachmentHandle;
	GLuint depthAttachmentHandle;
	GLuint deferredAttachmentHandle;

	GLuint frameBufferAttachmentHandle;

	GLuint directFrameBufferHandle;
	GLuint deferredFrameBufferHandle;
};

void GenFrameBuffers(App* app);

void Init(App* app);

void Gui(App* app);

void PushSceneToBuffer(App* app, mat4 projection, mat4 view);

void Update(App* app);

void Render(App* app);

