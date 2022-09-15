/* Sample code for drawing box and ball.  
   Much of the basics taken directly from learnopengl.com */
/* Need to install glad, GLFW, and glm first */
/* See learnopengl.com for setting up GLFW and glad */
/* You can just copy the headers from glm, or go through the install */
//#include<glad/glad.h>
//#include<GLFW/glfw3.h>
#include<iostream>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

#include <random>

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

using namespace std;
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
// Hardcoded shaders so we don't need extra files
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
// View parameters
float theta = 0.0;
float phi = 0.0;
float camradius = 5.0;
float cameraspeed = 0.02;
float camX = camradius;
float camY = 0.0;
float camZ = 0.0;
// Allow window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
glViewport(0, 0, width, height);
}
// Keyboard input: JKIL for camera motion (also escape to close window)
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    // J key will rotate left
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        theta -= cameraspeed;
        if (theta < 0.0) theta += 360.0;
        camX = camradius * cos(glm::radians(phi)) * cos(glm::radians(theta));
        camY = camradius * cos(glm::radians(phi)) * sin(glm::radians(theta));
        camZ = camradius * sin(glm::radians(phi));
    }
    // L key will rotate right
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        theta += cameraspeed;
        if (theta >= 360.0) theta -= 360.0;
        camX = camradius * cos(glm::radians(phi)) * cos(glm::radians(theta));
        camY = camradius * cos(glm::radians(phi)) * sin(glm::radians(theta));
        camZ = camradius * sin(glm::radians(phi));
    }
    // I key will rotate right
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        if (phi < 90.0-cameraspeed) phi += cameraspeed;
        camX = camradius * cos(glm::radians(phi)) * cos(glm::radians(theta));
        camY = camradius * cos(glm::radians(phi)) * sin(glm::radians(theta));
        camZ = camradius * sin(glm::radians(phi));
    }
    // K key will rotate right
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        if (phi > -90.0+cameraspeed) phi -= cameraspeed;
        camX = camradius * cos(glm::radians(phi)) * cos(glm::radians(theta));
        camY = camradius * cos(glm::radians(phi)) * sin(glm::radians(theta));
        camZ = camradius * sin(glm::radians(phi));
    }
}
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


bool randomBool()
{
    std::random_device r;
    static auto gen = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine(r()));
    return gen();
}





bool hitEveryTick = true;


class RenderBall
{
public:
    double pos[3] = { 0, 0, 0 }; //m
    double vel[3] = { 0, 0, 0 }; //m/s

    double scale = 5; // scale factor from sim units to render units
public:
    double getpos(int idx)
    {
        return pos[idx] / scale;
    }

    double getvel(int idx)
    {
        return vel[idx] / scale;
    }
};

static vector<RenderBall*> hits;

class Ball : public RenderBall
{
public:
    double restitution = 0.8; //Coefficient of restitution

public:
    void random_state()
    {
        std::random_device r;
        // std::seed_seq ssq{r()};
        // and then passing it to the engine does the same
        std::default_random_engine eng{ r() };
        std::uniform_real_distribution<double> distribution_rxyz(-scale/2, scale/2);
        std::uniform_real_distribution<double> distribution_vxy(3.0, 30.0);
        std::uniform_real_distribution<double> distribution_vz(-5.0, 30.0);

        for (int i = 0; i < 3; ++i)
        {
            double number = distribution_rxyz(eng);
            pos[i] = number;
        }
        for (int i = 0; i < 2; ++i)
        {
            double number = distribution_vxy(eng);
            bool sign = randomBool();
            if (sign)
                vel[i] = number;
            else
                vel[i] = -number;
        }
        double number = distribution_vz(eng);
        vel[2] = number;
    }

    void spawnHit()
    {
        hits.push_back(new RenderBall);
        hits.back()->pos[0] = pos[0];
        hits.back()->pos[1] = pos[1];
        hits.back()->pos[2] = pos[2];
    }

    void step(double dt)
    {
        // Euler propagation
        pos[0] += vel[0] * dt;
        pos[1] += vel[1] * dt;
        pos[2] += vel[2] * dt;

        // Gather metrics
        double vmag = sqrt(vel[0] * vel[0] + vel[1] * vel[1] + vel[2] * vel[2]);

        double vhat[3];
        vhat[0] = vel[0] / vmag;
        vhat[1] = vel[1] / vmag;
        vhat[2] = vel[2] / vmag;

        double bounds = scale * (1-br);
        // Collision handling
        // Box walls are at <scale>m in every direction.
        if (pos[2] < -bounds)
        {
            vel[2] = -vel[2] * restitution;
            pos[2] = -bounds + -(pos[2] + bounds);
            spawnHit();
        }
        else if (pos[2] > bounds)
        {
            vel[2] = -vel[2] * restitution;
            pos[2] = bounds + -(pos[2] - bounds);
            spawnHit();
        }
        if (pos[1] < -bounds)
        {
            vel[1] = -vel[1] * restitution;
            pos[1] = -bounds + -(pos[1] + bounds);
            spawnHit();
        }
        else if (pos[1] > bounds)
        {
            vel[1] = -vel[1] * restitution;
            pos[1] = bounds + -(pos[1] - bounds);
            spawnHit();
        }
        if (pos[0] < -bounds)
        {
            vel[0] = -vel[0] * restitution;
            pos[0] = -bounds + -(pos[0] + bounds);
            spawnHit();
        }
        else if (pos[0] > bounds)
        {
            vel[0] = -vel[0] * restitution;
            pos[0] = bounds + -(pos[0] - bounds);
            spawnHit();
        }
        if (hitEveryTick)
            spawnHit();


        // Accelerations
       

        // Air drag
        double density = 0.1; //kg/m3
        double Cd = 0.4; //drag coefficient plus some other terms
        double drag_mag = 0.5 * density * vmag * vmag * Cd;
        vel[0] -= vhat[0] * drag_mag * dt;
        vel[1] -= vhat[1] * drag_mag * dt;
        vel[2] -= vhat[2] * drag_mag * dt;

        // Gravity
        vel[2] -= dt * 9.81;
    }

};






int main(int argc, char* argv[])
{
    int step_skip_amt = 1; //how many graphics steps per physics step
    int ball_count = 5;
    float restitution = 0.8;

    if (argc >= 2) {
        string arg = string(argv[1]);
        ball_count = stoi(arg);
    }
    if (argc >= 3)
    {
        string arg1 = string(argv[2]);
        step_skip_amt = stoi(arg1);
    }
    if (argc >= 4)
    {
        string arg = string(argv[3]);
        hitEveryTick = stoi(arg) == 1;
    }
    if (argc >= 5)
    {
        string arg = string(argv[4]);
        restitution = stof(arg);
    }
        
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Physically Based Demo", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // Enable depth buffering, backface culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
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
    glm::mat4 view = glm::mat4(1.0f);
    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    glm::mat4 projection = glm::mat4(1.0f);
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    // Set Projection matrix
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    /* HERE IS INITIAL BALL POSITION, SET AT 0,0,0 FOR DEMO */


    ///////////////////////////////////////////
    //Options
    double timescale = 1.0;



    vector<Ball*> balls;
    for (int i = 0; i<ball_count; i++)
    {
        balls.push_back(new Ball);
        balls.back()->restitution = restitution;
    }

    auto prevTime = std::chrono::high_resolution_clock::now();
    for (auto ball : balls)
    {
        ball->random_state();
    }


    int step_skip_itr = 0;

    double accumulated_dt = 0;
    double sim_elapsed = 0;

    // Rendering loop
    while (!glfwWindowShouldClose(window)) {
    processInput(window);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // Set view matrix
            view = glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
 
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


            float hitcolor[3] = { 0.2, 0.2, 0.5 };
            // color attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, hitcolor);
            glEnableVertexAttribArray(1);
            for (auto ball : hits)
            {
                // Translate ball to its position and draw
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(ball->getpos(0)-0.03, ball->getpos(1), ball->getpos(2)));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 24);
            }


            for (auto ball : balls)
            {
                float hitcolor[3] = { 255,255,255 };
                // color attribute
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
                glEnableVertexAttribArray(1);
                // Translate ball to its position and draw
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(ball->getpos(0)+0.01, ball->getpos(1), ball->getpos(2)));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                glDrawArrays(GL_TRIANGLES, 0, 24);
            }
            



    glfwSwapBuffers(window);
    glfwPollEvents();
            /* HERE YOU CAN ADD YOUR CODE TO COMPUTE THE NEXT SIMULATION UP TO
               THE NEXT DISPLAY FRAME (PROBABLY WANT A FUNCTION CALL) */
            const auto nowTime = std::chrono::high_resolution_clock::now();

            auto dt = nowTime - prevTime;
            typedef std::chrono::duration<double> double_seconds;
            auto dt_sec = std::chrono::duration_cast<double_seconds>(dt).count();
            auto dt_sim = dt_sec * timescale;
            accumulated_dt += dt_sim;
            sim_elapsed += dt_sim;
            prevTime = nowTime;

            if (++step_skip_itr == step_skip_amt)
            {
                step_skip_itr = 0;
                for (auto ball : balls)
                {
                    ball->step(accumulated_dt);
                }
                accumulated_dt = 0;
            }

            if (sim_elapsed > 3)
            {
                sim_elapsed = 0;
                for (auto hit : hits)
                {
                    delete hit;
                }
                hits.clear();
                for (auto ball : balls)
                {
                    ball->random_state();
                }
            }
                
    }
    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &boxbuffer);
    glDeleteBuffers(1, &ballbuffer);
    glDeleteProgram(shaderProgram);
glfwTerminate();
return 0;
}