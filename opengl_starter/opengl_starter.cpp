#include "Common.h"

#include "Bloom.h"
#include "Camera.h"
#include "DebugDraw.h"
#include "Decal.h"
#include "Font.h"
#include "Framebuffer.h"
#include "GltfLoader.h"
#include "Grass.h"
#include "ImGuiHandler.h"
#include "Mesh.h"
#include "ParticleSystem.h"
#include "SSAO.h"
#include "Shader.h"
#include "Terrain.h"
#include "TextRenderer.h"
#include "Texture.h"
#include "Window.h"

extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

class DebugGroupScope
{
public:
    DebugGroupScope(const std::string& name)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, (GLsizei)name.size(), name.c_str());
    }

    ~DebugGroupScope()
    {
        glPopDebugGroup();
    }
};

float health = 1.0f;
float animationPos = 0.0f;
bool animationManual = false;
glm::vec3 lightAnglesDeg = { 340.0f, 317.0f, 127.0f };

std::vector<opengl_starter::ParticleSystem*> particleSystems; // todo - not cleaned up
std::vector<std::string> particleSystemFiles;
std::vector<std::string> particleBitmapFiles;
int selectedParticleSystemFile = -1;
bool particleSystemEdit = false;
int particleSystemEditIndex = 0;








///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec3 ourColor;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"   ourColor = aColor;\n"
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(ourColor, 1.0f);\n"
"}\n\0";


// Box with different colors for each face
float box[] = {
    // positions         // colors
     1.0f, -1.0f, -1.0f,  0.0f, 0.0f, 1.0f, // bottom
     1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
    -1.0f,  1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
     1.0f, -1.0f, -1.0f,  0.0f, 0.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top
     1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
     1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 1.0f, // left
    -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
     1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 1.0f,
     1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f, // right
    -1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
     1.0f,  1.0f,  1.0f,  0.0f, 1.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,  0.0f, 1.0f, 0.0f,
    -1.0f,  1.0f,  1.0f,  1.0f, 0.0f, 0.0f, // back
    -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
    -1.0f, -1.0f,  1.0f,  1.0f, 0.0f, 0.0f,
    -1.0f, -1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
    -1.0f,  1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
     1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 0.0f, // front
     1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 0.0f,
     1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
     1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
     1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
     1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 0.0f
};
// This is a really bad "ball" - just an octahedron
float br = 0.05; // ball radius
float ball[] = {
    // positions         // colors
     br,  0,  0,   1.0f, 1.0f, 1.0f, // triangle 1
      0, br,  0,   1.0f, 1.0f, 1.0f,
      0,  0, br,   1.0f, 1.0f, 1.0f,
      0, br,  0,   1.0f, 1.0f, 1.0f, // triangle 2
    -br,  0,  0,   1.0f, 1.0f, 1.0f,
      0,  0, br,   1.0f, 1.0f, 1.0f,
    -br,  0,  0,   1.0f, 1.0f, 1.0f, // triangle 3
      0,-br,  0,   1.0f, 1.0f, 1.0f,
      0,  0, br,   1.0f, 1.0f, 1.0f,
      0,-br,  0,   1.0f, 1.0f, 1.0f, // triangle 4
     br,  0,  0,   1.0f, 1.0f, 1.0f,
      0,  0, br,   1.0f, 1.0f, 1.0f,
     br,  0,  0,   1.0f, 1.0f, 1.0f, // triangle 5
      0,-br,  0,   1.0f, 1.0f, 1.0f,
      0,  0,-br,   1.0f, 1.0f, 1.0f,
      0,-br,  0,   1.0f, 1.0f, 1.0f, // triangle 6
    -br,  0,  0,   1.0f, 1.0f, 1.0f,
      0,  0,-br,   1.0f, 1.0f, 1.0f,
    -br,  0,  0,   1.0f, 1.0f, 1.0f, // triangle 7
      0, br,  0,   1.0f, 1.0f, 1.0f,
      0,  0,-br,   1.0f, 1.0f, 1.0f,
      0, br,  0,   1.0f, 1.0f, 1.0f, // triangle 8
     br,  0,  0,   1.0f, 1.0f, 1.0f,
      0,  0,-br,   1.0f, 1.0f, 1.0f,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////











void DrawSceneUI(opengl_starter::Node* node, opengl_starter::DebugDraw* debugdraw);

void InitOpenGL()
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/) {
        if (source == GL_DEBUG_SOURCE_APPLICATION && id == 0)
            return;

        auto level = type == GL_DEBUG_TYPE_ERROR ? spdlog::level::err : spdlog::level::warn;
        spdlog::log(level, "[opengl]: Severity = {}, Message = {}", severity, message);
    },
        nullptr);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

int main()
{
    spdlog::set_level(spdlog::level::debug);

    // (In VS2022, change from 'Debug' -> 'Debug and Launch...' Add "currentDir": "...")
    spdlog::info("Using working dir: {}", std::filesystem::current_path().string());

    const int frameWidth = 2560;
    const int frameHeight = 1440;
    opengl_starter::Window wnd{ frameWidth, frameHeight };

    InitOpenGL();

    std::vector<opengl_starter::Mesh*> meshes;
    std::vector<opengl_starter::Mesh*> meshesCube;

    particleBitmapFiles = Utils::File::GetFiles("assets/particles");
    auto particleBitmaps2 = Utils::File::GetFiles("assets/particles2");
    particleBitmapFiles.insert(particleBitmapFiles.end(), particleBitmaps2.begin(), particleBitmaps2.end());

    particleSystemFiles = Utils::File::GetFiles("assets", ".ps");

    // todo - child nodes are currently allocated and deleted by no one.
    opengl_starter::Node root{ "root" };

    //opengl_starter::GltfLoader::Load("assets/atelier.glb", &root, meshes);
    opengl_starter::GltfLoader::Load("assets/cube.glb", nullptr, meshes);
    opengl_starter::GltfLoader::Load("assets/unit_cube.glb", nullptr, meshesCube);

    //auto animClockRoot = opengl_starter::GltfLoader::Load("assets/anim_test_clock.glb", &root, meshes, true);
    //auto animBallRoot = opengl_starter::GltfLoader::Load("assets/anim_test_ball.glb", &root, meshes);
    //animClockRoot->pos = glm::vec3{ 11.0f, 0.0f, -2.5f };
    //animBallRoot->pos = glm::vec3{ 8.0f, 0.0f, -4.5f };
    




    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderProgram);
    // Set up vertex array object (VAO) and vertex buffers for box and ball
    unsigned int boxbuffer, ballbuffer, VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &boxbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, boxbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_STATIC_DRAW);
    glGenBuffers(1, &ballbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, ballbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ball), ball, GL_STATIC_DRAW);
    // Declare model/view/projection matrices
    glm::mat4 model = glm::mat4(1.0f);
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    /* HERE IS INITIAL BALL POSITION, SET AT 0,0,0 FOR DEMO */
    float ballposition[] = { 0.0, 0.0, 0.0 };
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////







    opengl_starter::Mesh* meshUnitCube = meshesCube[0];

    opengl_starter::Texture texPalette("assets/lospec500-32x.png");
    opengl_starter::Texture texGear("assets/gear.png");
    opengl_starter::Texture texHeight("assets/gradient.png");
    opengl_starter::Texture texGreen("assets/green.png");
    opengl_starter::Texture texBrown("assets/brown.png");
    opengl_starter::Texture texFont{ "assets/robotoregular.png" };
    opengl_starter::Texture texFontMono{ "assets/robotomono.png" };
    opengl_starter::Texture texHealthbar{ "assets/healthbar-gradient.png", opengl_starter::Texture::Wrap::ClampToEdge, opengl_starter::Texture::Wrap::ClampToEdge };
    opengl_starter::Texture texHealthbarMask{ "assets/healthbar-mask.png" };
    opengl_starter::Texture texParticle1{ "assets/particles/flame_01.png" };
    auto texNoise = opengl_starter::Texture::CreateNoiseTexture(4, 4);
    opengl_starter::Shader shaderCube("assets/cube.vert", "assets/cube.frag");
    opengl_starter::Shader shaderCubeDepth("assets/cube.vert", "assets/cubeDepth.frag");
    opengl_starter::Shader shaderDecal("assets/decal.vert", "assets/decal.frag");
    opengl_starter::Shader shaderTerrain("assets/terrain.vert", "assets/terrain.frag");
    opengl_starter::Shader shaderTerrainTess("assets/terrain_tess.vert", "assets/terrain_tess.frag", "assets/terrain_tess.tesc", "assets/terrain_tess.tese");
    opengl_starter::Shader shaderBillboardImage("assets/billboard-image.vert", "assets/billboard-image.frag");
    opengl_starter::Shader shaderBillboardHealth("assets/billboard-healthbar.vert", "assets/billboard-healthbar.frag");
    opengl_starter::Shader shaderSSAO("assets/ssao.vert", "assets/ssao.frag");
    opengl_starter::Shader shaderSSAOBlur("assets/blur.vert", "assets/blur.frag");
    opengl_starter::Shader shaderBloomExtract("assets/bloom-extract-brights.vert", "assets/bloom-extract-brights.frag");
    opengl_starter::Shader shaderPost("assets/post.vert", "assets/post.frag");
    opengl_starter::Shader shaderFont("assets/sdf_font.vert", "assets/sdf_font.frag");
    opengl_starter::Shader shaderGaussianBlur("assets/gaussian-blur.vert", "assets/gaussian-blur.frag");

    opengl_starter::Font fontRobotoRegular{ "assets/robotoregular.fnt" };
    opengl_starter::Font fontRobotoMono{ "assets/robotomono.fnt" };

    opengl_starter::Texture texColor{ frameWidth, frameHeight, GL_RGBA8 }; // GL_R11F_G11F_B10F
    opengl_starter::Texture texDepth{ frameWidth, frameHeight, GL_DEPTH32F_STENCIL8 };
    opengl_starter::Texture texNormals{ frameWidth, frameHeight, GL_RGBA32F };
    opengl_starter::Texture texSSAO{ frameWidth, frameHeight, GL_RGBA32F };
    opengl_starter::Texture texSSAOBlur{ frameWidth, frameHeight, GL_RGBA32F };
    opengl_starter::Texture texBloom{ frameWidth / 4, frameHeight / 4, GL_RGBA8 };
    opengl_starter::Texture texBloomBlur{ frameWidth / 4, frameHeight / 4, GL_RGBA8 };

    const int ShadowMapSize = 1024;
    opengl_starter::Texture texShadowMap{ ShadowMapSize, ShadowMapSize, GL_DEPTH32F_STENCIL8,
        opengl_starter::Texture::Wrap::ClampToBorder, opengl_starter::Texture::Wrap::ClampToBorder, opengl_starter::Texture::Filter::Nearest };

    opengl_starter::Framebuffer framebuffer{
        { { GL_COLOR_ATTACHMENT0, texColor.textureName },
            { GL_COLOR_ATTACHMENT1, texNormals.textureName },
            { GL_DEPTH_STENCIL_ATTACHMENT, texDepth.textureName } }
    };

    opengl_starter::Framebuffer framebufferSSAO{
        {
            { GL_COLOR_ATTACHMENT0, texSSAO.textureName },
        }
    };

    opengl_starter::Framebuffer framebufferSSAOBlur{
        {
            { GL_COLOR_ATTACHMENT0, texSSAOBlur.textureName },
        }
    };

    opengl_starter::Framebuffer framebufferBloom{
        {
            { GL_COLOR_ATTACHMENT0, texBloom.textureName },
        }
    };

    opengl_starter::Framebuffer framebufferBloomBlur{
        {
            { GL_COLOR_ATTACHMENT0, texBloomBlur.textureName },
        }
    };

    opengl_starter::Framebuffer framebufferShadowMap{
        {
            { GL_DEPTH_ATTACHMENT, texShadowMap.textureName },
        }
    };

    //opengl_starter::TextRenderer textRenderer{ &shaderFont, &texFont, &fontRobotoRegular, frameWidth, frameHeight };
    //opengl_starter::TextRenderer textRendererMono{ &shaderFont, &texFontMono, &fontRobotoMono, frameWidth, frameHeight };

    opengl_starter::Terrain terrain{ true, &shaderTerrainTess, &texHeight, &texGreen, &texBrown };

    opengl_starter::Camera camera{ wnd.window };

    opengl_starter::ImGuiHandler imgui{ wnd.window };

    opengl_starter::DebugDraw debugDraw;

    opengl_starter::Decals decal;

    opengl_starter::SSAO ssao;
    opengl_starter::Bloom bloom;
    //opengl_starter::Grass grass{ root.FindNode("grass")->mesh };

    //root.RecurseNodes([&debugDraw](opengl_starter::Node* node) {
    //    if (node->name.find(".particles") != std::string::npos)
    //    {
    //        auto ps = new opengl_starter::ParticleSystem{ &debugDraw };
    //        ps->Load("assets/ps_torch.ps", node);
    //        ps->Start();
    //        particleSystems.push_back(ps);
    //    }
    //});

    //auto notesPs = new opengl_starter::ParticleSystem{ &debugDraw };
    //notesPs->Load("assets/ps_notes.ps", root.FindNode("keyboard"));
    //notesPs->Start();
    //particleSystems.push_back(notesPs);

    wnd.onResize = [&](int width, int height) {
        //textRenderer.ResizeWindow(width, height);
        //textRendererMono.ResizeWindow(width, height);
    };

    wnd.onCursorPos = [&](double x, double y) {
        if (imgui.OnMouse(static_cast<float>(x), static_cast<float>(y)))
            return;

        camera.UpdateMouse(static_cast<float>(x), static_cast<float>(y));
    };

    wnd.onScroll = [&](double x, double y) {
        if (imgui.OnScroll(static_cast<float>(x), static_cast<float>(y)))
            return;

        camera.UpdateScroll(static_cast<float>(x), static_cast<float>(y));
    };

    wnd.onKey = [&](int key, int scancode, int action, int mods) { imgui.OnKey(key, scancode, action, mods); };
    wnd.onChar = [&](unsigned int chr) { imgui.OnChar(chr); };

    // DummyVao for post process step. glDraw cannot draw without bound vao. (todo - or can it somehow?)
    GLuint dummyVao = 0;
    glCreateVertexArrays(1, &dummyVao);

    auto prevTime = std::chrono::high_resolution_clock::now();
    float totalTime = 0.0f;

    float t = 0.0f;

    while (!glfwWindowShouldClose(wnd.window))
    {
        glfwPollEvents();

        if (glfwGetKey(wnd.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(wnd.window, true);

        imgui.NewFrame();
        debugDraw.NewFrame();

        // Update
        const auto nowTime = std::chrono::high_resolution_clock::now();
        const auto delta = static_cast<float>(std::chrono::duration_cast<std::chrono::duration<double>>(nowTime - prevTime).count());
        prevTime = nowTime;

        totalTime += delta;

        camera.Update(delta);
        for (auto ps : particleSystems)
            ps->Update(delta);

        imgui.Update(delta);
        //grass.Update(delta, totalTime);

        const glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), static_cast<float>(frameWidth) / static_cast<float>(frameHeight), 0.1f, 100.0f);
        const glm::mat4 view = camera.GetViewMatrix();

        if (animationManual)
            t = animationPos;
        else
            t += delta;

        auto transform = [](opengl_starter::Node* n, const glm::mat4& parentTransform, auto& transformRef, auto& t) -> void {
            if (n->animations.size() > 0)
            {
                for (auto& anim : n->animations)
                    anim.Animate(t, n);

                glm::mat4 local = glm::translate(glm::mat4{ 1.0f }, n->pos) *
                                  glm::mat4_cast(n->rotq) *
                                  glm::scale(glm::mat4{ 1.0f }, n->scale);

                n->model = parentTransform * local;
            }
            else
            {
                n->model = parentTransform * glm::translate(glm::mat4{ 1.0f }, n->pos) *
                           glm::mat4_cast(n->rotq) *
                           glm::scale(glm::mat4{ 1.0f }, n->scale);
            }

            for (auto c : n->children)
                transformRef(c, n->model, transformRef, t);
        };
        transform(&root, glm::mat4{ 1.0f }, transform, t);

        //decal.OnDecalUI();
        //ssao.OnUI();
        //bloom.OnUI();
        //grass.OnUI();
        DrawSceneUI(&root, &debugDraw);

        // Render
        const float clearColor[4] = { 112.0f / 255.0f, 94.0f / 255.0f, 120.0f / 255.0f, 1.0f };
        const float clearColor2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        const float clearDepth = 1.0f;

        //textRenderer.Reset();
        //textRendererMono.Reset();

        const glm::vec3 lightPos{ 15.0f, 15.0f, 15.0f };
        debugDraw.DrawCross({ 10.0f, 0.0f, 10.0f }, 2.0f);

        glm::mat4 lightDebugMat = glm::translate(glm::mat4{ 1.0f }, lightPos) *
                                  glm::rotate(glm::mat4{ 1.0f }, glm::radians(lightAnglesDeg.y), { 0.0f, 1.0f, 0.0f }) *
                                  glm::rotate(glm::mat4{ 1.0f }, glm::radians(lightAnglesDeg.z), { 0.0f, 0.0f, 1.0f }) *
                                  glm::rotate(glm::mat4{ 1.0f }, glm::radians(lightAnglesDeg.x), { 1.0f, 0.0f, 0.0f });

        glm::vec3 lightDir = lightDebugMat * glm::vec4{ 0.0f, 1.0f, 0.0f, 0.0f };

        debugDraw.DrawArrow({}, 1.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, lightDebugMat);

        // Shadowmap
        glm::mat4 lightSpaceMatrix;
        {
            glm::mat4 lightProjection, lightView;
            float near_plane = 1.0f, far_plane = 70.5f;
            lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
            lightView = glm::lookAt(lightPos, lightPos + lightDir, glm::vec3(0.0, 1.0, 0.0));
            lightSpaceMatrix = lightProjection * lightView;

            DebugGroupScope debugScope{ "Shadowmap" };

            glBindFramebuffer(GL_FRAMEBUFFER, framebufferShadowMap.fbo);

            glClearNamedFramebufferfv(framebufferShadowMap.fbo, GL_DEPTH, 0, &clearDepth);

            glViewport(0, 0, ShadowMapSize, ShadowMapSize);

            glBindProgramPipeline(shaderCubeDepth.pipeline);
            shaderCubeDepth.SetMat4("vp", lightSpaceMatrix);

            auto render = [&shaderCubeDepth, &lightView](opengl_starter::Node* node, auto& renderRef) -> void {
                if (node->mesh != nullptr)
                {
                    shaderCubeDepth.SetMat4("model", node->model);
                    shaderCubeDepth.SetMat4("view", lightView);
                    glBindVertexArray(node->mesh->vao);
                    glDrawElements(GL_TRIANGLES, node->mesh->indexCount, GL_UNSIGNED_INT, nullptr);
                    glBindVertexArray(0);
                }
                for (auto c : node->children)
                    renderRef(c, renderRef);
            };
            render(&root, render);
        }

        // Scene
        {
            DebugGroupScope debugScope{ "Scene" };

            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

            unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

            glNamedFramebufferDrawBuffers(framebuffer.fbo, 2, attachments);

            glClearNamedFramebufferfv(framebuffer.fbo, GL_COLOR, 0, clearColor);
            glClearNamedFramebufferfv(framebuffer.fbo, GL_COLOR, 1, clearColor2);
            glClearNamedFramebufferfv(framebuffer.fbo, GL_DEPTH, 0, &clearDepth);

            glViewport(0, 0, frameWidth, frameHeight);

            // terrain.Render(projection, view, camera.Position);

            glBindProgramPipeline(shaderCube.pipeline);
            shaderCube.SetMat4("vp", projection * view);

            auto render = [&shaderCube, &texPalette, &texShadowMap, &view, &lightDir, &lightSpaceMatrix](opengl_starter::Node* node, auto& renderRef) -> void {
                if (node->mesh != nullptr)
                {
                    shaderCube.SetMat4("model", node->model);
                    shaderCube.SetMat4("view", view);
                    shaderCube.SetVec3("lightDir", -lightDir);
                    shaderCube.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
                    glBindTextureUnit(0, texPalette.textureName);
                    glBindTextureUnit(1, texShadowMap.textureName);
                    glBindVertexArray(node->mesh->vao);
                    glDrawElements(GL_TRIANGLES, node->mesh->indexCount, GL_UNSIGNED_INT, nullptr);
                    glBindVertexArray(0);
                }
                for (auto c : node->children)
                    renderRef(c, renderRef);
            };
            render(&root, render);


            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            
            // render the box
            glBindBuffer(GL_ARRAY_BUFFER, boxbuffer);
            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            // color attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            // draw the box (no model transform needed)
            model = glm::mat4(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
            // render the ball
            glBindBuffer(GL_ARRAY_BUFFER, ballbuffer);
            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            // color attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            // Translate ball to its position and draw
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(ballposition[0], ballposition[1], ballposition[2]));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 24);
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        }

        //grass.Render(projection, view);

        //// PS
        //{
        //    DebugGroupScope debugScope{ "PS" };
        //    for (auto ps : particleSystems)
        //        ps->Render(projection, view);
        //}

        //// Decals
        //{
        //    DebugGroupScope debugScope{ "Decals" };
        //    for (const auto& decal : decal.decals)
        //    {
        //        const glm::mat4 modelDecal = glm::translate(glm::mat4{ 1.0f }, decal.pos) *
        //                                     glm::rotate(glm::mat4{ 1.0f }, glm::radians(decal.rot.y), { 0.0f, 1.0f, 0.0f }) *
        //                                     glm::rotate(glm::mat4{ 1.0f }, glm::radians(decal.rot.z), { 0.0f, 0.0f, 1.0f }) *
        //                                     glm::rotate(glm::mat4{ 1.0f }, glm::radians(decal.rot.x), { 1.0f, 0.0f, 0.0f }) *
        //                                     glm::scale(glm::mat4{ 1.0f }, decal.scale);

        //        debugDraw.DrawBox({}, decal.scale.x, glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, modelDecal);

        //        glDisable(GL_CULL_FACE);
        //        glDepthMask(GL_FALSE);
        //        glEnable(GL_BLEND);
        //        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //        glBindProgramPipeline(shaderDecal.pipeline);

        //        shaderDecal.SetMat4("model", modelDecal);
        //        shaderDecal.SetMat4("vp", projection * view);
        //        /*shaderDecal.SetMat4("invView", glm::inverse(view));
        //        shaderDecal.SetMat4("invProj", glm::inverse(projection));
        //        shaderDecal.SetMat4("invModel", glm::inverse(modelDecal));*/
        //        shaderDecal.SetVec4("decalColor", decal.color);
        //        glProgramUniformMatrix4fv(shaderDecal.fragProg, glGetUniformLocation(shaderDecal.fragProg, "invView"), 1, GL_FALSE, glm::value_ptr(glm::inverse(view)));
        //        glProgramUniformMatrix4fv(shaderDecal.fragProg, glGetUniformLocation(shaderDecal.fragProg, "invProj"), 1, GL_FALSE, glm::value_ptr(glm::inverse(projection)));
        //        glProgramUniformMatrix4fv(shaderDecal.fragProg, glGetUniformLocation(shaderDecal.fragProg, "invModel"), 1, GL_FALSE, glm::value_ptr(glm::inverse(modelDecal)));
        //        glBindTextureUnit(0, texGear.textureName);
        //        glBindTextureUnit(1, texDepth.textureName);
        //        glBindVertexArray(meshUnitCube->vao);
        //        glDrawElements(GL_TRIANGLES, meshUnitCube->indexCount, GL_UNSIGNED_INT, nullptr);
        //        glBindVertexArray(0);
        //        glDepthMask(GL_TRUE);
        //        glDisable(GL_BLEND);
        //        glEnable(GL_CULL_FACE);
        //    }
        //}

        {
            DebugGroupScope debugScope{ "billboard test" };
            shaderBillboardImage.Bind();
            shaderBillboardImage.SetMat4("model", glm::translate(glm::mat4{ 1.0f }, { 9.0f, 2.4f, -17.2f }));
            shaderBillboardImage.SetMat4("view", view);
            shaderBillboardImage.SetMat4("projection", projection);
            shaderBillboardImage.SetFloat("time", totalTime);
            glEnable(GL_BLEND);
            glDepthMask(GL_FALSE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBindTextureUnit(0, texParticle1.textureName);
            glBindVertexArray(dummyVao);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            glDisable(GL_BLEND);
            glDepthMask(GL_TRUE);

            shaderBillboardHealth.Bind();
            shaderBillboardHealth.SetMat4("model", glm::translate(glm::mat4{ 1.0f }, { 9.0f, 4.4f, -17.2f }));
            shaderBillboardHealth.SetMat4("view", view);
            shaderBillboardHealth.SetMat4("projection", projection);
            shaderBillboardHealth.SetFloat("time", totalTime);
            shaderBillboardHealth.SetFloat("health", health);
            shaderBillboardHealth.SetFloat("distanceToCamera", glm::distance({ 9.0f, 4.4f, -17.2f }, camera.Position));
            glEnable(GL_BLEND);
            glDepthMask(GL_FALSE);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBindTextureUnit(0, texHealthbar.textureName);
            glBindTextureUnit(1, texHealthbarMask.textureName);
            glBindVertexArray(dummyVao);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            glDisable(GL_BLEND);
            glDepthMask(GL_TRUE);
        }

        debugDraw.Render(camera.GetViewMatrix(), projection);

        // bloom
        if (bloom.enable)
        {
            DebugGroupScope debugScope{ "bloom" };

            glBindFramebuffer(GL_FRAMEBUFFER, framebufferBloom.fbo);
            unsigned int attachments3[1] = { GL_COLOR_ATTACHMENT0 };
            glNamedFramebufferDrawBuffers(framebufferBloom.fbo, 1, attachments3);

            glClearNamedFramebufferfv(framebufferBloom.fbo, GL_COLOR, 0, clearColor2);
            glViewport(0, 0, frameWidth / 4, frameHeight / 4);

            glBindProgramPipeline(shaderBloomExtract.pipeline);
            shaderBloomExtract.SetFloat("threshold", bloom.threshold);

            glBindTextureUnit(0, texColor.textureName);

            glBindVertexArray(dummyVao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);

            // bloomblur x
            glBindFramebuffer(GL_FRAMEBUFFER, framebufferBloomBlur.fbo);
            unsigned int bloomblurattach[1] = { GL_COLOR_ATTACHMENT0 };
            glNamedFramebufferDrawBuffers(framebufferBloomBlur.fbo, 1, bloomblurattach);

            glClearNamedFramebufferfv(framebufferBloomBlur.fbo, GL_COLOR, 0, clearColor2);

            glViewport(0, 0, frameWidth / 4, frameHeight / 4);

            glBindProgramPipeline(shaderGaussianBlur.pipeline);
            shaderGaussianBlur.SetVec2("winSize", glm::vec2{ wnd.width / 4, wnd.height / 4 });
            shaderGaussianBlur.SetVec2("direction", { 1.0f, 0.0f });

            glBindTextureUnit(0, texBloom.textureName);

            glBindVertexArray(dummyVao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);

            // bloomblur y
            glBindFramebuffer(GL_FRAMEBUFFER, framebufferBloom.fbo);
            unsigned int bloomblurattach2[1] = { GL_COLOR_ATTACHMENT0 };
            glNamedFramebufferDrawBuffers(framebufferBloom.fbo, 1, bloomblurattach2);

            glClearNamedFramebufferfv(framebufferBloom.fbo, GL_COLOR, 0, clearColor2);

            glViewport(0, 0, frameWidth / 4, frameHeight / 4);

            glBindProgramPipeline(shaderGaussianBlur.pipeline);
            shaderGaussianBlur.SetVec2("winSize", glm::vec2{ wnd.width / 4, wnd.height / 4 });
            shaderGaussianBlur.SetVec2("direction", { 0.0f, 1.0f });

            glBindTextureUnit(0, texBloomBlur.textureName);

            glBindVertexArray(dummyVao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }

        // ssao
        if (ssao.enableAo)
        {
            DebugGroupScope debugScope{ "ssao" };

            glBindFramebuffer(GL_FRAMEBUFFER, framebufferSSAO.fbo);

            unsigned int attachments2[1] = { GL_COLOR_ATTACHMENT0 };
            glNamedFramebufferDrawBuffers(framebufferSSAO.fbo, 1, attachments2);

            glClearNamedFramebufferfv(framebufferSSAO.fbo, GL_COLOR, 0, clearColor2);

            glViewport(0, 0, frameWidth, frameHeight);

            glBindProgramPipeline(shaderSSAO.pipeline);
            glProgramUniformMatrix4fv(shaderSSAO.fragProg, glGetUniformLocation(shaderSSAO.fragProg, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            shaderSSAO.SetFloat("radius", ssao.radius);
            shaderSSAO.SetFloat("strength", ssao.strength);
            shaderSSAO.SetFloat("bias", ssao.bias);
            shaderSSAO.SetVec2("winSize", { wnd.width, wnd.height });
            shaderSSAO.SetInt("kernelSize", ssao.kernelSize);

            for (int i = 0; i < ssao.kernelSize; ++i)
                glProgramUniform3fv(shaderSSAO.fragProg, glGetUniformLocation(shaderSSAO.fragProg, fmt::format("sampleSphere[{}]", i).c_str()), 1, glm::value_ptr(ssao.ssaoKernel[i]));

            glBindTextureUnit(0, texNormals.textureName);
            glBindTextureUnit(1, texDepth.textureName);
            glBindTextureUnit(2, texNoise->textureName);

            glBindVertexArray(dummyVao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);

            // ssaoblur
            glBindFramebuffer(GL_FRAMEBUFFER, framebufferSSAOBlur.fbo);

            glNamedFramebufferDrawBuffers(framebufferSSAOBlur.fbo, 1, attachments2);

            glClearNamedFramebufferfv(framebufferSSAOBlur.fbo, GL_COLOR, 0, clearColor2);

            glViewport(0, 0, frameWidth, frameHeight);

            glBindProgramPipeline(shaderSSAOBlur.pipeline);
            shaderSSAOBlur.SetVec2("winSize", { wnd.width, wnd.height });
            glBindTextureUnit(0, texSSAO.textureName);

            glBindVertexArray(dummyVao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }

        // Render to screen with post process
        {
            DebugGroupScope debugScope{ "post" };

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glClearNamedFramebufferfv(0, GL_COLOR, 0, clearColor);
            glClearNamedFramebufferfv(0, GL_DEPTH, 0, &clearDepth);

            glViewport(0, 0, wnd.width, wnd.height);

            glBindProgramPipeline(shaderPost.pipeline);
            shaderPost.SetInt("enableAo", (int)ssao.enableAo);
            shaderPost.SetInt("visualizeAo", (int)ssao.visualizeAo);
            shaderPost.SetInt("enableBloom", (int)bloom.enable);
            shaderPost.SetInt("visualizeBloom", (int)bloom.visualize);
            glBindTextureUnit(0, texColor.textureName);
            glBindTextureUnit(1, texSSAOBlur.textureName);
            glBindTextureUnit(2, texBloom.textureName);

            glBindVertexArray(dummyVao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }

        //// Render some text
        //{
        //    DebugGroupScope debugScope{ "text" };
        //    const auto ipsum = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";

        //    auto text1transform = glm::translate(glm::mat4{ 1.0f }, { 5.0f, 5.0f, 0.0f });
        //    textRenderer.RenderString(fmt::format("Single line - {}", ipsum), text1transform);

        //    auto text2transform = glm::translate(glm::mat4{ 1.0f }, { 5.0f, 55.0f, 0.0f });
        //    textRenderer.RenderString(fmt::format("Wrapped - {}", ipsum), text2transform, 500.0f);

        //    auto text3transform = glm::translate(glm::mat4{ 1.0f }, { wnd.width / 2.0f, wnd.height - 60.0f, 0.0f }) * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ 0.6f });
        //    textRendererMono.RenderString(fmt::format("Center - {}", ipsum), text3transform, 1500.0f, true);

        //    auto text4transform = glm::translate(glm::mat4{ 1.0f }, { 5.0f, 250.0f, 0.0f });
        //    textRenderer.RenderString(fmt::format("Progress - {}", ipsum), text4transform, 700.0f, false, glm::sin(totalTime / 2.0f));
        //}

        // ImGui
        {
            DebugGroupScope debugScope{ "ImGui" };
            imgui.Render();
        }

        glfwSwapBuffers(wnd.window);
    }

    for (auto mesh : meshes)
    {
        delete mesh;
    }
    meshes.clear();

    glDeleteVertexArrays(1, &dummyVao);
}

void DrawSceneUI(opengl_starter::Node* rootNode, opengl_starter::DebugDraw* debugDraw)
{
    static opengl_starter::Node* node_clicked = nullptr;

    /*ImGui::Begin("Scene");

    if (ImGui::Button("Add"))
    {
        auto parent = node_clicked ? node_clicked : rootNode;

        auto n = new opengl_starter::Node{ fmt::format("{}.{}", parent->name, parent->children.size() + 1).c_str() };
        parent->children.push_back(n);
    }

    {
        ImGui::BeginChild("scene_tree");

        auto createNode = [](opengl_starter::Node* n, bool defaultOpen, auto& createNodeRef) -> void {
            ImGuiTreeNodeFlags flags = {};
            if (defaultOpen)
                flags |= ImGuiTreeNodeFlags_DefaultOpen;
            if (node_clicked == n)
                flags |= ImGuiTreeNodeFlags_Selected;
            if (n->children.size() == 0)
                flags |= ImGuiTreeNodeFlags_Leaf;

            if (ImGui::TreeNodeEx((void*)(intptr_t)n, flags, fmt::format("{}", n->name).c_str()))
            {
                if (ImGui::IsItemClicked())
                    node_clicked = n;

                for (const auto child : n->children)
                {
                    createNodeRef(child, false, createNodeRef);
                }

                ImGui::TreePop();
            }
        };

        createNode(rootNode, true, createNode);

        ImGui::EndChild();
    }

    ImGui::End();*/

   /* ImGui::Begin("Object");
    if (node_clicked)
    {
        ImGui::Spacing();

        ImGui::Text(fmt::format("{} properties", node_clicked->name).c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::DragFloat3("Pos", glm::value_ptr(node_clicked->pos), 0.1f, -1000.0f, 1000.0f);
        ImGui::DragFloat4("Rot", glm::value_ptr(node_clicked->rotq), 0.1f, -360.0f, 360.0f);
        ImGui::DragFloat3("Scl", glm::value_ptr(node_clicked->scale), 0.1f, 0.1f, 10.0f);
    }

    ImGui::End();*/

    ImGui::Begin("Hax");
    ImGui::DragFloat("Health", &health, 0.01f, 0.0f, 1.0f);
    ImGui::DragFloat3("Light", glm::value_ptr(lightAnglesDeg), 1.0f, 0.0f, 360.0f);
    /*ImGui::Checkbox("AnimManual", &animationManual);
    ImGui::DragFloat("animationPos", &animationPos, 0.01f, 0.0f, 100.0f);
    if (ImGui::Button("note burst"))
        particleSystems[4]->Burst();*/
    ImGui::End();

    // Particles
   /* ImGui::Begin("Particles");
    ImGui::SetWindowSize({ 400.0f, 700.0f }, ImGuiCond_FirstUseEver);

    auto stringGetter = [](void* data, int index, const char** out) -> bool {
        const auto v = (std::vector<std::string>*)data;
        *out = v->at(index).c_str();
        return true;
    };
    ImGui::Combo("Systems2", &selectedParticleSystemFile, stringGetter, (void*)&particleSystemFiles, particleSystemFiles.size());

    if (ImGui::Button("New"))
    {
        particleSystemEdit = true;
        auto psNode = new opengl_starter::Node{ "new ps" };
        rootNode->children.push_back(psNode);
        auto ps = new opengl_starter::ParticleSystem{ debugDraw };
        ps->Load("assets/ps_default.ps", psNode);
        ps->Start();
        particleSystems.push_back(ps);
        particleSystemEditIndex = particleSystems.size() - 1;
    }
    ImGui::SameLine();

    if (ImGui::Button("Open") && selectedParticleSystemFile != -1)
    {
        particleSystemEdit = true;
        auto psNode = new opengl_starter::Node{ particleSystemFiles[selectedParticleSystemFile] };
        rootNode->children.push_back(psNode);
        auto ps = new opengl_starter::ParticleSystem{ debugDraw };
        ps->Load(particleSystemFiles[selectedParticleSystemFile], psNode);
        ps->Start();
        particleSystems.push_back(ps);
        particleSystemEditIndex = particleSystems.size() - 1;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (particleSystemEdit)
    {
        particleSystems[particleSystemEditIndex]->OnUI(particleBitmapFiles);
    }

    ImGui::End();*/
}
