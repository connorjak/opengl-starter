﻿#include "Common.h"
#include "Font.h"
#include "Framebuffer.h"
#include "Mesh.h"
#include "Shader.h"
#include "TextRenderer.h"
#include "Texture.h"
#include "Window.h"

void InitOpenGL()
{
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageCallback([](GLenum /*source*/, GLenum type, GLuint /*id*/, GLenum severity, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/) {
        auto level = type == GL_DEBUG_TYPE_ERROR ? spdlog::level::err : spdlog::level::warn;
        spdlog::log(level, "[opengl]: Severity = {}, Message = {}", severity, message);
    },
        nullptr);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
}

int main()
{
    const int frameWidth = 1280;
    const int frameHeight = 720;
    opengl_starter::Window wnd{ frameWidth, frameHeight };

    InitOpenGL();

    opengl_starter::Mesh meshCube("assets/cube.glb");
    opengl_starter::Texture texOpengl("assets/opengl.png");
    opengl_starter::Texture texFont{ "assets/robotoregular.png" };
    opengl_starter::Texture texFontMono{ "assets/robotomono.png" };

    opengl_starter::Shader shaderCube("assets/cube.vert", "assets/cube.frag");
    opengl_starter::Shader shaderPost("assets/post.vert", "assets/post.frag");
    opengl_starter::Shader shaderFont("assets/sdf_font.vert", "assets/sdf_font.frag");

    opengl_starter::Font fontRobotoRegular{ "assets/robotoregular.fnt" };
    opengl_starter::Font fontRobotoMono{ "assets/robotomono.fnt" };

    opengl_starter::Texture texColor{ frameWidth, frameHeight, GL_RGBA8 };
    opengl_starter::Texture texDepth{ frameWidth, frameHeight, GL_DEPTH32F_STENCIL8 };
    opengl_starter::Framebuffer framebuffer{
        { { GL_COLOR_ATTACHMENT0, texColor.textureName },
            { GL_DEPTH_STENCIL_ATTACHMENT, texDepth.textureName } }
    };

    opengl_starter::TextRenderer textRenderer{ &shaderFont, &texFont, &fontRobotoRegular, frameWidth, frameHeight };
    opengl_starter::TextRenderer textRendererMono{ &shaderFont, &texFontMono, &fontRobotoMono, frameWidth, frameHeight };

    wnd.onResize = [&](int width, int height) {
        textRenderer.ResizeWindow(width, height);
        textRendererMono.ResizeWindow(width, height);
    };

    // DummyVao for post process step. glDraw cannot draw without bound vao. (todo - or can it somehow?)
    GLuint dummyVao = 0;
    glCreateVertexArrays(1, &dummyVao);

    auto prevTime = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(wnd.window))
    {
        glfwPollEvents();

        if (glfwGetKey(wnd.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(wnd.window, true);

        // Update
        const auto nowTime = std::chrono::high_resolution_clock::now();
        const auto delta = static_cast<float>(std::chrono::duration_cast<std::chrono::duration<double>>(nowTime - prevTime).count());
        prevTime = nowTime;

        static float r = 0.0f;
        r += delta * 1.0f;

        const glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(frameWidth) / static_cast<float>(frameHeight), 0.1f, 100.0f);
        const glm::mat4 view = glm::lookAt(glm::vec3{ 7.5f, 3.0f, 0.0f }, { 0.0f, 0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f });
        const glm::mat4 model = glm::rotate(glm::mat4{ 1.0f }, glm::sin(r) * glm::pi<float>(), { 0.0f, 1.0f, 0.0f }) *
                                glm::rotate(glm::mat4{ 1.0f }, glm::sin(-r) * glm::pi<float>(), { 1.0f, 0.0f, 0.0f });

        const float mixValue = glm::sin(r);

        // Render
        const float clearColor[4] = { 112.0f / 255.0f, 94.0f / 255.0f, 120.0f / 255.0f, 1.0f };
        const float clearDepth = 1.0f;

        textRenderer.Reset();
        textRendererMono.Reset();

        // Render cube to texture
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);

        glClearNamedFramebufferfv(framebuffer.fbo, GL_COLOR, 0, clearColor);
        glClearNamedFramebufferfv(framebuffer.fbo, GL_DEPTH, 0, &clearDepth);

        glViewport(0, 0, frameWidth, frameHeight);

        glBindProgramPipeline(shaderCube.pipeline);
        glProgramUniformMatrix4fv(shaderCube.vertProg, glGetUniformLocation(shaderCube.vertProg, "vp"), 1, GL_FALSE, glm::value_ptr(projection * view));
        glProgramUniformMatrix4fv(shaderCube.vertProg, glGetUniformLocation(shaderCube.vertProg, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glProgramUniform1i(shaderCube.fragProg, glGetUniformLocation(shaderCube.fragProg, "texSampler"), 0);
        glBindTextureUnit(0, texOpengl.textureName);

        glBindVertexArray(meshCube.vao);
        glDrawElements(GL_TRIANGLES, meshCube.indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);

        // Render to screen with post process
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glClearNamedFramebufferfv(0, GL_COLOR, 0, clearColor);
        glClearNamedFramebufferfv(0, GL_DEPTH, 0, &clearDepth);

        glViewport(0, 0, wnd.width, wnd.height);

        glBindProgramPipeline(shaderPost.pipeline);
        glProgramUniform1f(shaderPost.fragProg, glGetUniformLocation(shaderPost.fragProg, "mixValue"), mixValue);
        glProgramUniform1i(shaderPost.fragProg, glGetUniformLocation(shaderPost.fragProg, "texSampler"), 0);
        glBindTextureUnit(0, texColor.textureName);

        glBindVertexArray(dummyVao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // Render some text
        const auto ipsum = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.";

        auto text1transform = glm::translate(glm::mat4{ 1.0f }, { 5.0f, 5.0f, 0.0f });
        textRenderer.RenderString(fmt::format("Single line - {}", ipsum), text1transform);

        auto text2transform = glm::translate(glm::mat4{ 1.0f }, { 5.0f, 55.0f, 0.0f });
        textRenderer.RenderString(fmt::format("Wrapped - {}", ipsum), text2transform, 500.0f);

        auto text3transform = glm::translate(glm::mat4{ 1.0f }, { wnd.width / 2.0f, wnd.height - 60.0f, 0.0f }) * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ 0.6f });
        textRendererMono.RenderString(fmt::format("Center - {}", ipsum), text3transform, 1500.0f, true);

        auto text4transform = glm::translate(glm::mat4{ 1.0f }, { 5.0f, 250.0f, 0.0f });
        textRenderer.RenderString(fmt::format("Progress - {}", ipsum), text4transform, 700.0f, false, glm::sin(r / 2.0f));

        glfwSwapBuffers(wnd.window);
    }

    glDeleteVertexArrays(1, &dummyVao);
}
