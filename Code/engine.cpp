//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "engine.h"
#include "assimp_loading.h"
#include "buffer_management.h"
#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>

GLuint CreateProgramFromSource(String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(vertexShaderDefine),
        (GLint) programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint) strlen(versionString),
        (GLint) strlen(shaderNameDefine),
        (GLint) strlen(fragmentShaderDefine),
        (GLint) programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
    app->programs.push_back(program);

    return app->programs.size() - 1;
}

Image LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint CreateTexture2DFromImage(Image image, GLint texParam)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat     = GL_RGB;
    GLenum dataType       = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
        case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
        case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
        default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    
    if (texParam != GL_LINEAR)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texParam);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texParam);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 LoadTexture2D(App* app, const char* filepath, GLuint texParams)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
        if (app->textures[texIdx].filepath == filepath)
            return texIdx;

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image, texParams);
        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

GLuint FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];

    //Try finding a vao for this submesh/program
    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
    {
        if (submesh.vaos[i].programHandle == program.handle)
            return submesh.vaos[i].handle;
    }

    GLuint vaoHandle = 0;

    //Create a new vao for this submesh/program
    {
        glGenVertexArrays(1, &vaoHandle);
        glBindVertexArray(vaoHandle);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

        //We have to link all vertex input attributes to attributes in the vertex buffer
        for (u32 i = 0; i < program.vertexInputLayout.attributes.size(); ++i)
        {
            bool attributeWasLinked = false;

            for (u32 j = 0; j < submesh.vertexBufferLayout.attributes.size(); ++j)
            {
                if (program.vertexInputLayout.attributes[i].location == submesh.vertexBufferLayout.attributes[j].location)
                {
                    const u32 index = submesh.vertexBufferLayout.attributes[j].location;
                    const u32 ncomp = submesh.vertexBufferLayout.attributes[j].componentCount;
                    const u32 offset = submesh.vertexBufferLayout.attributes[j].offset + submesh.vertexOffset; //attribute offset + vertex offset
                    const u32 stride = submesh.vertexBufferLayout.stride;
                    glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                    glEnableVertexAttribArray(index);

                    attributeWasLinked = true;
                    break;
                }
            }

            assert(attributeWasLinked); //The submesh should provide an attribute for each vertex inputs
        }

        glBindVertexArray(0);
    }

    //Store it in the list of vaos for this submesh
    Vao vao = { vaoHandle, program.handle };
    submesh.vaos.push_back(vao);

    return vaoHandle;
}

//tmp
mat4 TransformPositionScale(const vec3& pos, const vec3& scaleFactors)
{
    mat4 transform = glm::translate(pos);
    transform = glm::scale(transform, scaleFactors);
    return transform;
}

void getOpenGlInfo(App* app)
{
    app->GLInfo = {};
    app->GLInfo.version = (char*)glGetString(GL_VERSION);
    app->GLInfo.renderer = (char*)glGetString(GL_RENDERER);
    app->GLInfo.vendor = (char*)glGetString(GL_VENDOR);
    app->GLInfo.versionGLSL = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    GLint num_extensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);

    app->GLInfo.numExtensions = num_extensions;
    app->GLInfo.extensions = new char* [num_extensions];

    for (int i = 0; i < num_extensions; ++i)
    {
        app->GLInfo.extensions[i] = (char*)glGetStringi(GL_EXTENSIONS, GLuint(i));
    }
}

void OnGlError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    ELOG("OpenGL debug message: %s", message);

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             ELOG(" - source: GL_DEBUG_SOURCE_API"); break; // Calls to the OpenGL API
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   ELOG(" - source: GL_DEBUG_SOURCE_WINDOW_SYSTEM"); break; // Calls to a window-system API
        case GL_DEBUG_SOURCE_SHADER_COMPILER: ELOG(" - source: GL_DEBUG_SOURCE_SHADER_COMPILER"); break; // A compiler for a shading language
        case GL_DEBUG_SOURCE_THIRD_PARTY:     ELOG(" - source: GL_DEBUG_SOURCE_THIRD_PARTY"); break; // An application associated with OpenGL
        case GL_DEBUG_SOURCE_APPLICATION:     ELOG(" - source: GL_DEBUG_SOURCE_APPLICATION"); break; // Generated by the user of this applicat
        case GL_DEBUG_SOURCE_OTHER:           ELOG(" - source: GL_DEBUG_SOURCE_OTHER"); break; // Some source that isn't one of these
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               ELOG(" - type: GL_DEBUG_TYPE_ERROR"); break; // An error, typically from the API
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ELOG(" - type: GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"); break; // Some behavior marked deprecated l
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  ELOG(" - type: GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"); break; // Something has invoked undefined b
        case GL_DEBUG_TYPE_PORTABILITY:         ELOG(" - type: GL_DEBUG_TYPE_PORTABILITY"); break; // Some functionality the user relies upon
        case GL_DEBUG_TYPE_PERFORMANCE:         ELOG(" - type: GL_DEBUG_TYPE_PERFORMANCE"); break; // Code has triggered possible performance
        case GL_DEBUG_TYPE_MARKER:              ELOG(" - type: GL_DEBUG_TYPE_MARKER"); break; // Command stream annotation
        case GL_DEBUG_TYPE_PUSH_GROUP:          ELOG(" - type: GL_DEBUG_TYPE_PUSH_GROUP"); break; // Group pushing
        case GL_DEBUG_TYPE_POP_GROUP:           ELOG(" - type: GL_DEBUG_TYPE_POP_GROUP"); break; // foo
        case GL_DEBUG_TYPE_OTHER:               ELOG(" - type: GL_DEBUG_TYPE_OTHER"); break; // Some type that isn't one of these
    }

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         ELOG(" - severity: GL_DEBUG_SEVERITY_HIGH"); break; // All OpenGL Errors, shader compilation/link
        case GL_DEBUG_SEVERITY_MEDIUM:       ELOG(" - severity: GL_DEBUG_SEVERITY_MEDIUM"); break; // Major performance warnings, shader compil
        case GL_DEBUG_SEVERITY_LOW:          ELOG(" - severity: GL_DEBUG_SEVERITY_LOW"); break; // Redundant state change performance warning,
        case GL_DEBUG_SEVERITY_NOTIFICATION: ELOG(" - severity: GL_DEBUG_SEVERITY_NOTIFICATION"); break; // Anything that isn't an error or per
    }
}

void GenFrameBuffers(App* app)
{
    // Framebuffer
    glGenTextures(1, &app->colorAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->colorAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, app->displaySize.x, app->displaySize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &app->depthAttachmentHandle);
    glBindTexture(GL_TEXTURE_2D, app->depthAttachmentHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, app->displaySize.x, app->displaySize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);


    glGenFramebuffers(1, &app->frameBufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, app->frameBufferHandle);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, app->colorAttachmentHandle, 0);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, app->depthAttachmentHandle, 0);

    GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (framebufferStatus)
        {
        case GL_FRAMEBUFFER_UNDEFINED:ELOG("GL_FRAMEBUFFER_UNDEFINED"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"); break;
        case GL_FRAMEBUFFER_UNSUPPORTED:ELOG("GL_FRAMEBUFFER_UNSUPPORTED"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:ELOG("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"); break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"); break;
        default:ELOG("Unknown framebuffer status error");
        }
    }

    glDrawBuffers(1, &app->colorAttachmentHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Init(App* app)
{
    if (GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 3))
    {
        glDebugMessageCallback(OnGlError, app);
    }

    getOpenGlInfo(app);
    //We want OpenGL to handle z-buffer shenanigans for us
    glEnable(GL_DEPTH_TEST);

    // TODO: Initialize your resources here!
    // - vertex buffers
    // - element/index buffers
    // - vaos
    // - programs (and retrieve uniform indices)
    // - textures
    struct VertexV3V2
    {
        glm::vec3 pos;
        glm::vec2 uv;
    };

    //This is for loading the dice image manually
    const VertexV3V2 vertices[] =
    {
        { glm::vec3(-0.8, -0.8, 0.0),   glm::vec2(0.0, 0.0) }, //bottom-left
        { glm::vec3(0.8, -0.8, 0.0),    glm::vec2(1.0, 0.0) }, //bottom-right
        { glm::vec3(0.8, 0.8, 0.0),     glm::vec2(1.0, 1.0) }, //top-right
        { glm::vec3(-0.8, 0.8, 0.0),    glm::vec2(0.0, 1.0) } //top-left
    };

    const u16 indices[] =
    {
        0,1,2,
        0,2,3
    };

    //Prepare geometry manually
    //VBO
    glGenBuffers(1, &app->embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //EBO
    glGenBuffers(1, &app->embeddedElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //VAO, we do this in render, in FindVAOs
    glGenVertexArrays(1, &app->vao);
    glBindVertexArray(app->vao);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)0);  //The first parameter is 0 because this is
    glEnableVertexAttribArray(0);                                                   //the "location" we declare in the shader
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)12); //The first parameter is 1 because this is
    glEnableVertexAttribArray(1);                                                   //the "location" we declare in the shader

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBindVertexArray(0);

    //Load the dice image program
    app->texturedGeometryProgramIdx = LoadProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY"); //This is used to render a plane
    Program& texturedGeometryProgram = app->programs[app->texturedGeometryProgramIdx];
    app->programUniformTexture = glGetUniformLocation(texturedGeometryProgram.handle, "uTexture");

    app->texturedMeshProgramIdx = LoadProgram(app, "shaders.glsl", "SHOW_TEXTURED_MESH"); //This is used to render a mesh
    Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
    //Manually passing the attributes
    //texturedMeshProgram.vertexInputLayout.attributes.push_back({0,3}); //position
    //texturedMeshProgram.vertexInputLayout.attributes.push_back({2,2}); //texcoord

    //All of this reads the attributes from the mesh, and stores them to send them later
    GLint attributeCount = 0;
    glGetProgramiv(texturedMeshProgram.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);

    //This gets the length of the longest attribute name, so that the buffer
    //of said length can be allocated to then fill with the actual name
    GLint maxAttributeNameLength = 0;
    glGetProgramiv(texturedMeshProgram.handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttributeNameLength);

    for (u64 i = 0; i < attributeCount; ++i)
    {
        std::string attributeName(maxAttributeNameLength, '\0'); //+null terminator
        GLsizei attributeNameLength;
        GLint attributeSize;
        GLenum attributeType;

        glGetActiveAttrib(texturedMeshProgram.handle, i, 
                          maxAttributeNameLength, 
                          &attributeNameLength, 
                          &attributeSize, 
                          &attributeType, 
                          &attributeName[0]);

        attributeName.resize(attributeNameLength);

        u8 attributeLocation = glGetAttribLocation(texturedMeshProgram.handle, attributeName.c_str());
        u8 componentCount = (u8)(attributeType == GL_FLOAT_VEC3 ? 3 : (attributeType == GL_FLOAT_VEC2 ? 2 : 1));

        texturedMeshProgram.vertexInputLayout.attributes.push_back({ attributeLocation, componentCount });
    }
    
    texturedMeshProgram.camera = Camera{ vec3(2.0f, 2.0f, 6.0f), 
                                         vec3(0, 1.8, 0),
                                         (float)app->displaySize.x / (float)app->displaySize.y,
                                         0.1f,
                                         1000.0f,
                                         60.0f
                                        };

    //I think this has to be done at some point to set a variable so that it doesn't explode.
    app->texturedMeshProgram_uTexture = glGetUniformLocation(texturedMeshProgram.handle, "uTexture");

    //Initialize textures
    app->diceTexIdx = LoadTexture2D(app, "dice.png");
    app->whiteTexIdx = LoadTexture2D(app, "color_white.png");
    app->blackTexIdx = LoadTexture2D(app, "color_black.png");
    app->normalTexIdx = LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx = LoadTexture2D(app, "color_magenta.png");

    //Initialize models
    app->patrickModel = LoadModel(app, "Patrick/Patrick.obj");
    app->planeModel = LoadModel(app, "Plane/Plane.obj", GL_NEAREST);

    //Place entities in scene
    app->entityList.push_back({ TransformPositionScale(vec3(0, 1.8, 0), vec3(0.5)), app->patrickModel });
    app->entityList.push_back({ TransformPositionScale(vec3(2.5, 1.8, 0), vec3(0.3)), app->patrickModel });
    app->entityList.push_back({ TransformPositionScale(vec3(0, 1.8, -2.5), vec3(0.2)), app->patrickModel });

    app->entityList.push_back({ TransformPositionScale(vec3(0, 0, 0), vec3(5.0)), app->planeModel });

    //Place lights in scene
    //app->lightList.push_back({ LightType_Directional, 1, vec3(1,0,0), vec3(1), vec3(0) });
    app->lightList.push_back({ LightType_Point, 1, vec3(1,0,0), vec3(0), vec3(0,0.1,0) });
    app->lightList.push_back({ LightType_Point, 1, vec3(0,1,0), vec3(0), vec3(1,3,1.5) });
    app->lightList.push_back({ LightType_Point, 1, vec3(0,0,1), vec3(0), vec3(-1,3,1.5) });

    //Get info to create and use uniforms buffer
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &app->maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBlockAlignment);

    //Create the buffer to pass the transforms to the shader
    app->uniformsBuffer = CreateConstantBuffer(app->maxUniformBufferSize);
    
    GenFrameBuffers(app);

    //app->mode = Mode_TexturedQuad;
    //app->mode = Mode_Meshes;
    app->mode = Mode_FrameBuffers;
}

void Gui(App* app)
{
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);
    ImGui::Separator();

    ImGui::Dummy(ImVec2(0.0f, 20.0f)); //Spacing

    ImGui::Text("Rendering info");
    ImGui::Separator();
    ImGui::Text("Current render mode: %d", app->mode);
    ImGui::Spacing();
    ImGui::Text("Render modes:");
    ImGui::Text("0: Textured quad");
    ImGui::Text("1: Direct mesh rendering");
    ImGui::Text("2: Framebuffers rendering");

    ImGui::Dummy(ImVec2(0.0f, 20.0f)); //Spacing

    ImGui::Text("OpenGL info");
    ImGui::Separator();
    ImGui::Text("ImGui version: %s", ImGui::GetVersion());
    ImGui::Text("GL Version: %s", app->GLInfo.version);
    ImGui::Text("Renderer Version: %s", app->GLInfo.renderer);
    ImGui::Text("Vendor: %s", app->GLInfo.vendor);
    ImGui::Text("GLSL Version: %s", app->GLInfo.versionGLSL);

    std::string dropdownTitle = "Extensions [" + std::to_string(app->GLInfo.numExtensions) + "]";
    if (ImGui::CollapsingHeader(dropdownTitle.c_str(), ImGuiTreeNodeFlags_None))
    {
        for (int i = 0; i < app->GLInfo.numExtensions; i++)
        {
            std::string name = "[" + std::to_string(i) + "] " + app->GLInfo.extensions[i];
            ImGui::Text("%s", name.c_str());
        }
    }
    
    ImGui::End();
}

void Update(App* app)
{
    // You can handle app->input keyboard/mouse here

    if (app->input.keys[K_0] == BUTTON_PRESS) { app->mode = Mode_TexturedQuad; }
    if (app->input.keys[K_1] == BUTTON_PRESS) { app->mode = Mode_Meshes; }
    if (app->input.keys[K_2] == BUTTON_PRESS) { app->mode = Mode_FrameBuffers; }

    //Shader hot reload
    for (u64 i = 0; i < app->programs.size(); ++i)
    {
        Program& program = app->programs[i];
        u64 currentTimestamp = GetFileLastWriteTimestamp(program.filepath.c_str());
        if (currentTimestamp > program.lastWriteTimestamp)
        {
            glDeleteProgram(program.handle);

            String programSource = ReadTextFile(program.filepath.c_str());
            const char* programName = program.programName.c_str();
            program.handle = CreateProgramFromSource(programSource, programName);
            program.lastWriteTimestamp = currentTimestamp;
        }
    }

    //Shader Transform update
    Camera& cam = app->programs[app->texturedMeshProgramIdx].camera;

    if (app->input.mouseButtons[0] == BUTTON_PRESSED)
    {
        cam.position.x -= app->input.mouseDelta.x / 50.0f;
        cam.position.y += app->input.mouseDelta.y / 50.0f;
    }

    //Play with Patricks transforms
    app->entityList.at(0).transformationMatrix = glm::rotate(app->entityList.at(0).transformationMatrix, glm::radians(1.0f), vec3(0, 1, 0));
    app->entityList.at(1).transformationMatrix = TransformPositionScale(vec3(app->entityList.at(0).transformationMatrix[2][0] * 3.0f,
                                                                             app->entityList.at(0).transformationMatrix[2][1] * 3.0f,
                                                                             app->entityList.at(0).transformationMatrix[2][2] * 3.0f) + 
                                                                        vec3(0, 1.8, 0),
                                                                        vec3(0.3f));
    app->entityList.at(2).transformationMatrix = TransformPositionScale(vec3(0, 1.8, -2.5), vec3(app->entityList.at(0).transformationMatrix[2][0]));

    app->lightList.at(0).position = vec3(app->entityList.at(0).transformationMatrix[2][0] * 5.0f,
                                         app->entityList.at(0).transformationMatrix[2][1] * 5.0f,
                                         app->entityList.at(0).transformationMatrix[2][2] * 5.0f) +
                                    vec3(0, 0.1, 0);
    //------------------------------

    mat4 projection = glm::perspective(glm::radians(cam.fov), cam.aspectRatio, cam.znear, cam.zfar);
    mat4 view = glm::lookAt(cam.position, cam.lookAt, vec3(0, 1, 0));

    MapBuffer(app->uniformsBuffer, GL_WRITE_ONLY);

    //Push lights to the buffer
    //app->globalParamsOffset = app->uniformsBuffer.head; //Might need to track where it starts in the future. Not now though.

    PushVec3(app->uniformsBuffer, cam.position);

    PushUInt(app->uniformsBuffer, app->lightList.size());

    for (Light& light : app->lightList)
    {
        AlignHead(app->uniformsBuffer, sizeof(vec4));

        PushUInt(app->uniformsBuffer, light.type);
        PushUInt(app->uniformsBuffer, light.strength);
        PushVec3(app->uniformsBuffer, light.color);
        PushVec3(app->uniformsBuffer, light.direction);
        PushVec3(app->uniformsBuffer, light.position);
    }

    //app->globalParamsSize = app->uniformsBuffer.head - app->globalParamsOffset; //Same as up
    app->globalParamsSize = app->uniformsBuffer.head;

    //Push entities to the buffer
    for (Entity& entity : app->entityList)
    {
        //mat4 world = TransformPositionScale(entity.position, entity.scale);
        mat4 worldViewProjection = projection * view * entity.transformationMatrix;

        AlignHead(app->uniformsBuffer, app->uniformBlockAlignment);

        entity.head = app->uniformsBuffer.head;

        PushMat4(app->uniformsBuffer, entity.transformationMatrix);
        PushMat4(app->uniformsBuffer, worldViewProjection);

        entity.size = app->uniformsBuffer.head - entity.head;
    }
    
    UnmapBuffer(app->uniformsBuffer);
}

void Render(App* app)
{
    switch (app->mode)
    {
        case Mode_TexturedQuad:
            {
                // TODO: Draw your textured quad here!
                // - clear the framebuffer
                // - set the viewport
                // - set the blending state
                // - bind the texture into unit 0
                // - bind the program 
                //   (...and make its texture sample from unit 0)
                // - bind the vao
                // - glDrawElements() !!!
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glViewport(0, 0, app->displaySize.x, app->displaySize.y);

                Program& programTexturedGeometry = app->programs[app->texturedGeometryProgramIdx];
                glUseProgram(programTexturedGeometry.handle);
                glBindVertexArray(app->vao);

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                glUniform1i(app->programUniformTexture, 0);
                glActiveTexture(GL_TEXTURE0);
                GLuint textureHandle = app->textures[app->diceTexIdx].handle;
                glBindTexture(GL_TEXTURE_2D, textureHandle);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

                glBindVertexArray(0);
                glUseProgram(0);
            }
            break;
        case Mode_Meshes:
            {
                glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glEnable(GL_DEPTH_TEST);

                glViewport(0, 0, app->displaySize.x, app->displaySize.y);


                Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
                glUseProgram(texturedMeshProgram.handle);

                
                //Bind buffer for global params
                glBindBufferRange(GL_UNIFORM_BUFFER, 0, app->uniformsBuffer.handle, 0, app->globalParamsSize); //Harcoded at 0 bc it is at the beginning

                for (Entity& entity : app->entityList)
                {
                    //Model& model = app->models[app->patrickModel];
                    Model& model = app->models[entity.model];
                    Mesh& mesh = app->meshes[model.meshIdx];

                    //Bind buffer per entity
                    glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->uniformsBuffer.handle, entity.head, entity.size);

                    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                    {
                        GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
                        glBindVertexArray(vao);

                        u32 submeshMaterialIdx = model.materialIdx[i];
                        Material& submeshMaterial = app->materials[submeshMaterialIdx];

                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
                        glUniform1i(app->texturedMeshProgram_uTexture, 0);

                        Submesh& submesh = mesh.submeshes[i];
                        glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                    }
                }

                glDisable(GL_DEPTH_TEST);

                glBindVertexArray(0);
                glUseProgram(0);
            }
            break;
        case Mode_FrameBuffers:
        {
            //Render on this frame buffer render targets
            glBindFramebuffer(GL_FRAMEBUFFER, app->frameBufferHandle);

            //Select on which render targets to draw
            GLuint drawBuffers[] = { app->colorAttachmentHandle };
            glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);

            //Clear color and depth (only if required)
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //Render code loops
            // - Bind programs
            // - Bind buffers
            // - Set states
            // - Draw calls

            glEnable(GL_DEPTH_TEST);
            Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
            glUseProgram(texturedMeshProgram.handle);


            //Bind buffer for global params
            glBindBufferRange(GL_UNIFORM_BUFFER, 0, app->uniformsBuffer.handle, 0, app->globalParamsSize); //Harcoded at 0 bc it is at the beginning

            for (Entity& entity : app->entityList)
            {
                //Model& model = app->models[app->patrickModel];
                Model& model = app->models[entity.model];
                Mesh& mesh = app->meshes[model.meshIdx];

                //Bind buffer per entity
                glBindBufferRange(GL_UNIFORM_BUFFER, 1, app->uniformsBuffer.handle, entity.head, entity.size);

                for (u32 i = 0; i < mesh.submeshes.size(); ++i)
                {
                    GLuint vao = FindVAO(mesh, i, texturedMeshProgram);
                    glBindVertexArray(vao);

                    u32 submeshMaterialIdx = model.materialIdx[i];
                    Material& submeshMaterial = app->materials[submeshMaterialIdx];

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTextureIdx].handle);
                    glUniform1i(app->texturedMeshProgram_uTexture, 0);

                    Submesh& submesh = mesh.submeshes[i];
                    glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
                }
            }

            glBindVertexArray(0);
            glUseProgram(0);

            //glBindFramebuffer(GL_FRAMEBUFFER, app->frameBufferHandle);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDisable(GL_DEPTH_TEST);

            //End of code loops----------------

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, app->displaySize.x, app->displaySize.y);

            Program& programTexturedGeometry = app->programs[app->texturedGeometryProgramIdx];
            glUseProgram(programTexturedGeometry.handle);
            glBindVertexArray(app->vao);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glUniform1i(app->programUniformTexture, 0);
            glActiveTexture(GL_TEXTURE0);
            GLuint textureHandle = app->colorAttachmentHandle;
            glBindTexture(GL_TEXTURE_2D, textureHandle);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

            glBindVertexArray(0);
            glUseProgram(0);
        }
        break;

        default:;
    }
}
