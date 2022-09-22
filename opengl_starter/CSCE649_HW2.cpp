/* Sample code for drawing box and ball.  
   Much of the basics taken directly from learnopengl.com */
/* Need to install glad, GLFW, and glm first */
/* See learnopengl.com for setting up GLFW and glad */
/* You can just copy the headers from glm, or go through the install */
//#include<glad/glad.h>
//#include<GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>

#include <random>
#define _USE_MATH_DEFINES
#include <math.h>

// Copy pasted from opengl_starter project:
#include "Common.h"
// The following are unused parts of the opengl_starter project:
//#include "Bloom.h"
//#include "Camera.h"
//#include "DebugDraw.h"
//#include "Decal.h"
//#include "Font.h"
//#include "Framebuffer.h"
//#include "GltfLoader.h"
//#include "Grass.h"
//#include "ImGuiHandler.h"
//#include "Mesh.h"
//#include "ParticleSystem.h"
//#include "SSAO.h"
//#include "Shader.h"
//#include "Terrain.h"
//#include "TextRenderer.h"
//#include "Texture.h"
//#include "Window.h"

using namespace std;

template <typename T>
int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;
// Hardcoded shaders so we don't need extra files
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec3 vertColor;\n"
"out vec3 normal;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"   vertColor = aColor;\n"
"   normal = aPos.normalize();\n"
"}\0";

static int vtxAttributeIdx_Position = 0;
static int vtxAttributeIdx_Color = 1;

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 vertColor;\n"
"in vec3 normal;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(vertColor, 1.0f);\n"
"}\n\0";

const char* planetFragmentShaderSource = "#version 330 core\n"
"uniform vec3 lightDir;\n"
"out vec4 FragColor;\n"
"in vec3 vertColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(vertColor, 1.0f);\n"
"}\n\0";

// View parameters
float theta = 0.0;
float phi = 0.0;
float camradius = 5.0;
float cameraspeed = 20.0; //degrees per second
float camX = camradius;
float camY = 0.0;
float camZ = 0.0;

// Allow window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Keyboard input: JKIL for camera motion (also escape to close window)
void processInput(GLFWwindow* window, double dt_sec) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    // J key will rotate left
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        theta -= cameraspeed * dt_sec;
        if (theta < 0.0) theta += 360.0;
        camX = camradius * cos(glm::radians(phi)) * cos(glm::radians(theta));
        camY = camradius * cos(glm::radians(phi)) * sin(glm::radians(theta));
        camZ = camradius * sin(glm::radians(phi));
    }
    // L key will rotate right
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        theta += cameraspeed * dt_sec;
        if (theta >= 360.0) theta -= 360.0;
        camX = camradius * cos(glm::radians(phi)) * cos(glm::radians(theta));
        camY = camradius * cos(glm::radians(phi)) * sin(glm::radians(theta));
        camZ = camradius * sin(glm::radians(phi));
    }
    // I key will rotate right
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        if (phi < 90.0-cameraspeed* dt_sec) phi += cameraspeed * dt_sec;
        camX = camradius * cos(glm::radians(phi)) * cos(glm::radians(theta));
        camY = camradius * cos(glm::radians(phi)) * sin(glm::radians(theta));
        camZ = camradius * sin(glm::radians(phi));
    }
    // K key will rotate right
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        if (phi > -90.0+cameraspeed* dt_sec) phi -= cameraspeed * dt_sec;
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
float br = 0.01; // ball radius
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

float pr = 0.3;
float planet[] = {
    // positions         // colors
     pr,  0,  0,   1.0f, 1.0f, 1.0f, // triangle 1
      0, pr,  0,   1.0f, 1.0f, 1.0f,
      0,  0, pr,   1.0f, 1.0f, 1.0f,
      0, pr,  0,   1.0f, 1.0f, 1.0f, // triangle 2
    -pr,  0,  0,   1.0f, 1.0f, 1.0f,
      0,  0, pr,   1.0f, 1.0f, 1.0f,
    -pr,  0,  0,   1.0f, 1.0f, 1.0f, // triangle 3
      0,-pr,  0,   1.0f, 1.0f, 1.0f,
      0,  0, pr,   1.0f, 1.0f, 1.0f,
      0,-pr,  0,   1.0f, 1.0f, 1.0f, // triangle 4
     pr,  0,  0,   1.0f, 1.0f, 1.0f,
      0,  0, pr,   1.0f, 1.0f, 1.0f,
     pr,  0,  0,   1.0f, 1.0f, 1.0f, // triangle 5
      0,-pr,  0,   1.0f, 1.0f, 1.0f,
      0,  0,-pr,   1.0f, 1.0f, 1.0f,
      0,-pr,  0,   1.0f, 1.0f, 1.0f, // triangle 6
    -pr,  0,  0,   1.0f, 1.0f, 1.0f,
      0,  0,-pr,   1.0f, 1.0f, 1.0f,
    -pr,  0,  0,   1.0f, 1.0f, 1.0f, // triangle 7
      0, pr,  0,   1.0f, 1.0f, 1.0f,
      0,  0,-pr,   1.0f, 1.0f, 1.0f,
      0, pr,  0,   1.0f, 1.0f, 1.0f, // triangle 8
     pr,  0,  0,   1.0f, 1.0f, 1.0f,
      0,  0,-pr,   1.0f, 1.0f, 1.0f,
};



bool randomBool()
{
    std::random_device r;
    static auto gen = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine(r()));
    return gen();
}

constexpr double scale = 15000e3; // scale factor from sim units to render units


class RenderBall
{
public:
    double pos[3] = { 0, 0, 0 }; //m
    double vel[3] = { 0, 0, 0 }; //m/s

    
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
        std::uniform_real_distribution<double> distribution_r(6.7e6, 7.5e6);
        std::uniform_real_distribution<double> distribution_theta(0, 2 * M_PI);
        std::uniform_real_distribution<double> distribution_z(-2e6, 2e6);
        //std::uniform_real_distribution<double> distribution_vxy(5000, 9000);
        std::uniform_real_distribution<double> distribution_vtheta(7.6e3, 10e3);
        std::uniform_real_distribution<double> distribution_vr(-1000, 1000);
        std::uniform_real_distribution<double> distribution_vz(-1000, 1000);

        /*
          thetahat-\ /-rhat 
                    X
                |  /
                | /  \
                |/  Theta
                x-----|---------
        */
       
        double radius = distribution_r(eng);
        double theta = distribution_theta(eng);
        double z = distribution_z(eng);

        double rhat[3];
        rhat[0] = std::cos(theta);
        rhat[1] = std::sin(theta);
        rhat[2] = 0;
        double thetahat[3];
        thetahat[0] = -std::sin(theta);
        thetahat[1] = std::cos(theta);
        thetahat[2] = 0;

        double vr = distribution_vr(eng);
        double vtheta = distribution_vtheta(eng);

        pos[0] = radius * rhat[0];
        pos[1] = radius * rhat[1];
        pos[2] = z;

        vel[0] = vr * rhat[0] + vtheta * thetahat[0];
        vel[1] = vr * rhat[1] + vtheta * thetahat[1];
        vel[2] = distribution_vz(eng);
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

        
        double pmag = sqrt(pos[0] * pos[0] + pos[1] * pos[1] + pos[2] * pos[2]);

        double phat[3];
        phat[0] = pos[0] / pmag;
        phat[1] = pos[1] / pmag;
        phat[2] = pos[2] / pmag;

        //double bounds = scale * (1-br);
        //// Collision handling
        //// Box walls are at <scale>m in every direction.
        //if (pos[2] < -bounds)
        //{
        //    vel[2] = -vel[2] * restitution;
        //    pos[2] = -bounds + -(pos[2] + bounds);
        //}
        //else if (pos[2] > bounds)
        //{
        //    vel[2] = -vel[2] * restitution;
        //    pos[2] = bounds + -(pos[2] - bounds);
        //}
        //if (pos[1] < -bounds)
        //{
        //    vel[1] = -vel[1] * restitution;
        //    pos[1] = -bounds + -(pos[1] + bounds);
        //}
        //else if (pos[1] > bounds)
        //{
        //    vel[1] = -vel[1] * restitution;
        //    pos[1] = bounds + -(pos[1] - bounds);
        //}
        //if (pos[0] < -bounds)
        //{
        //    vel[0] = -vel[0] * restitution;
        //    pos[0] = -bounds + -(pos[0] + bounds);
        //}
        //else if (pos[0] > bounds)
        //{
        //    vel[0] = -vel[0] * restitution;
        //    pos[0] = bounds + -(pos[0] - bounds);
        //}

        // Accelerations
       

        //// Air drag
        //double density = 0.1; //kg/m3
        //double Cd = 0.4; //drag coefficient plus some other terms
        //double drag_mag = 0.5 * density * vmag * vmag * Cd;
        //vel[0] -= vhat[0] * drag_mag * dt;
        //vel[1] -= vhat[1] * drag_mag * dt;
        //vel[2] -= vhat[2] * drag_mag * dt;

        //// z- Gravity
        //vel[2] -= dt * 9.81;

        // Newton gravity around center
        // f = G*m*M/r^2
        // mu = G*m
        // a = mu/r^2
        double mu = 3.986004e14; // m^3/s^2
        double ag_x = -(phat[0]) * mu / (pmag * pmag);
        double ag_y = -(phat[1]) * mu / (pmag * pmag);
        double ag_z = -(phat[2]) * mu / (pmag * pmag);
        vel[0] += ag_x * dt;
        vel[1] += ag_y * dt;
        vel[2] += ag_z * dt;
    }

};


using buffer_id_t = unsigned int;
using shader_id_t = unsigned int;
using program_id_t = unsigned int;
using matrix_id_t = unsigned int;


static void Better_glVertexAttribPointer(int attributeToConfigureIdx, int numElementsPerVertex,
    int gl_datatype, bool normalized, size_t strideBytesPerVertex, void* offset)
{
    glVertexAttribPointer(attributeToConfigureIdx, numElementsPerVertex,
        gl_datatype, normalized, strideBytesPerVertex, offset);
}

static void Better_glEnableVertexAttribArray(int attributeToConfigureIdx)
{
    glEnableVertexAttribArray(attributeToConfigureIdx);
}

static void Better_glUniformMatrix4fv(matrix_id_t mat, int howManyMatricesToSend, bool transpose, glm::f32* value)
{
    glUniformMatrix4fv(mat, howManyMatricesToSend, transpose, value);
}


class Shader
{
public:
    // the program ID
    shader_id_t ID = 0;

    Shader(const char* vertexPath, const char* fragmentPath)
    {

        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // 2. compile shaders
        unsigned int vertex, fragment;
        int success;
        char infoLog[512];

        // vertex Shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        // print compile errors if any
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        };

        // similiar for Fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        // print compile errors if any
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        };

        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        // print linking errors if any
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                      << infoLog << std::endl;
        }

        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    ~Shader()
    {
        glDeleteProgram(ID);
    }

    // use/activate the shader
    void use()
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setMat4(const std::string& name, glm::mat4 value) const
    {
        Better_glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, false, glm::value_ptr(value));
    }
};



int main(int argc, char* argv[])
{
    int step_skip_amt = 1; //how many graphics steps per physics step
    int ball_count = 10000;
    float restitution = 0.8;

    if (argc >= 2) {
        string arg = string(argv[1]);
        ball_count = stoi(arg);
    }
    if (argc >= 3)
    {
        string arg1 = string(argv[2]);
        //TODO a different arg
        //step_skip_amt = stoi(arg1);
    }
    if (argc >= 4)
    {
        string arg = string(argv[3]);
        //TODO a different arg
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
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //MacOS specific
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Connor Jakubik HW2", NULL, NULL);
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

    Shader defaultShader("../../../../shaders/default.vs", "../../../../shaders/default.fs");
    Shader planetShader("../../../../shaders/default.vs", "../../../../shaders/default.fs");

    
    defaultShader.use();
    // Set up vertex array object (VAO) and vertex buffers for box and ball
    buffer_id_t boxbuffer, ballbuffer, VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &boxbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, boxbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_STATIC_DRAW);
    glGenBuffers(1, &ballbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, ballbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ball), ball, GL_STATIC_DRAW);
    // position attribute 
    Better_glVertexAttribPointer(vtxAttributeIdx_Position, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)0);
    Better_glEnableVertexAttribArray(vtxAttributeIdx_Position);
    // color attribute
    Better_glVertexAttribPointer(vtxAttributeIdx_Color, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    Better_glEnableVertexAttribArray(vtxAttributeIdx_Color);

    //planetShader.use();
    // Set up vertex array object (VAO) and vertex buffers for planet
    buffer_id_t planetbuffer, VAO_planet;
    glGenVertexArrays(1, &VAO_planet);
    glBindVertexArray(VAO_planet);
    glGenBuffers(1, &planetbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, planetbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planet), planet, GL_STATIC_DRAW);
    // position attribute
    Better_glVertexAttribPointer(vtxAttributeIdx_Position, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)0);
    Better_glEnableVertexAttribArray(vtxAttributeIdx_Position);
    // color attribute
    Better_glVertexAttribPointer(vtxAttributeIdx_Color, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    Better_glEnableVertexAttribArray(vtxAttributeIdx_Color);

    // Declare model/view/projection matrices, which are used as inputs in the vertex shader
    glm::mat4 model = glm::mat4(1.0f);
    //matrix_id_t modelLoc = glGetUniformLocation(shaderProgram, "model");
    glm::mat4 view = glm::mat4(1.0f);
    //matrix_id_t viewLoc = glGetUniformLocation(shaderProgram, "view");
    glm::mat4 projection = glm::mat4(1.0f);
    //matrix_id_t projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    // Set Projection matrix
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    defaultShader.setMat4("projection", projection);
    planetShader.setMat4("projection", projection);


    ///////////////////////////////////////////
    //Options
    double timescale = 1000;



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
    double irl_elapsed = 0;
    double sim_elapsed = 0;

    // Rendering loop
    while (!glfwWindowShouldClose(window)) {
        /*processInput(window);*/
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Set view matrix
        view = glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
        defaultShader.setMat4("view", view);
        planetShader.setMat4("view", view);

        //Wireframe/not
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        //// render planets
        //planetShader.use();
        //glBindVertexArray(VAO_planet);


        // render balls 
        defaultShader.use();
        glBindVertexArray(VAO);

        for (auto ball : balls)
        {
            // Translate ball to its position and draw
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(ball->getpos(0), ball->getpos(1), ball->getpos(2)));
            defaultShader.setMat4("model", model);
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
        //cout << "Framerate: " << 1/dt_sec << endl;

        processInput(window, dt_sec);
        irl_elapsed += dt_sec;

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
            
        /*  if (irl_elapsed > 6)
        {
            irl_elapsed = 0;
            sim_elapsed = 0;
            for (auto ball : balls)
            {
                ball->random_state();
            }
        }*/
                
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &boxbuffer);
    glDeleteBuffers(1, &ballbuffer);
    //glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}