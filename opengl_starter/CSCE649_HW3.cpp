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

#include <Eigen/Dense>

// Copy pasted from opengl_starter project:
#include "Common.h"
// The following are unused parts of the opengl_starter project:
//#include "Bloom.h"
//#include "Camera.h"
//#include "DebugDraw.h"
//#include "Decal.h"
//#include "Font.h"
//#include "Framebuffer.h"
#include "GltfLoader.h"
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

/*
"Low Poly Jet Mk.2" (https://skfb.ly/6ZrWs) by checkersai is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
*/

using namespace std;
using namespace Eigen;

template <typename T>
int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}


/////////////////////////////////////////
//https://gist.github.com/podgorskiy/04a3cb36a27159e296599183215a71b0
#include <glm/matrix.hpp>
template <typename T, int m, int n>
inline glm::mat<m, n, float, glm::precision::highp> E2GLM(const Eigen::Matrix<T, m, n>& em)
{
    glm::mat<m, n, float, glm::precision::highp> mat;
    for (int i = 0; i < m; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            mat[j][i] = em(i, j);
        }
    }
    return mat;
}

template <typename T, int m>
inline glm::vec<m, float, glm::precision::highp> E2GLM(const Eigen::Matrix<T, m, 1>& em)
{
    glm::vec<m, float, glm::precision::highp> v;
    for (int i = 0; i < m; ++i)
    {
        v[i] = em(i);
    }
    return v;
}
/////////////////////////////////////////

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;
static int vtxAttributeIdx_Position = 0;
static int vtxAttributeIdx_Color = 1;

// View parameters
static float theta = 0.0;
static float phi = 0.0;
static float camradius = 6.0;
static float cameraspeed = 40.0; //degrees per second
static float camX = camradius;
static float camY = 0.0;
static float camZ = 0.0;

void updateCamCoords()
{
    camX = camradius * cos(glm::radians(phi)) * cos(glm::radians(theta));
    camY = camradius * cos(glm::radians(phi)) * sin(glm::radians(theta));
    camZ = camradius * sin(glm::radians(phi));
}

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
        updateCamCoords();
    }
    // L key will rotate right
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        theta += cameraspeed * dt_sec;
        if (theta >= 360.0) theta -= 360.0;
        updateCamCoords();
    }
    // I key will rotate right
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        if (phi < 90.0-cameraspeed* dt_sec) phi += cameraspeed * dt_sec;
        updateCamCoords();
    }
    // K key will rotate right
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        if (phi > -90.0+cameraspeed* dt_sec) phi -= cameraspeed * dt_sec;
        updateCamCoords();
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

float poly[] = {
    // positions         // colors
    1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f, // left
    -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 1.0f
};

// This is a really bad "ball" - just an octahedron
float br = 0.01; // ball radius
float ball[] = {
    // positions         // colors
     4*br, 0, 0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 1
      0,3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0, br,   255.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 2
    -br,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0, br,   255.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
    -br,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 3
      0,-3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0, br,   255.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,-3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 4
     4*br, 0, 0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0, br,   255.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
     4*br, 0, 0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 5
      0,-3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0,-br,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,-3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 6
    -br,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0,-br,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
    -br,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 7
      0,3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0,-br,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 8
     4*br, 0, 0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0,-br,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
};

float pr = 0.3;
float planet[] = {
    // positions         // colors
     pr,  0,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f, // triangle 1
      0, pr,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
      0,  0, pr,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
      0, pr,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f, // triangle 2
    -pr,  0,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
      0,  0, pr,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
    -pr,  0,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f, // triangle 3
      0,-pr,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
      0,  0, pr,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
      0,-pr,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f, // triangle 4
     pr,  0,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
      0,  0, pr,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
     pr,  0,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f, // triangle 5
      0,-pr,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
      0,  0,-pr,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
      0,-pr,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f, // triangle 6
    -pr,  0,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
      0,  0,-pr,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
    -pr,  0,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f, // triangle 7
      0, pr,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
      0,  0,-pr,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
      0, pr,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f, // triangle 8
     pr,  0,  0,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
      0,  0,-pr,   67.0f/255.0f, 222.0f/255.0f, 32.0f/255.0f,
};


using buffer_id_t = unsigned int;
using shader_id_t = unsigned int;
using program_id_t = unsigned int;
using matrix_id_t = unsigned int;

void CreateSphere(buffer_id_t spherePositionVbo, buffer_id_t sphereIndexVbo)
{
    int stacks = 20;
    int slices = 20;

    std::vector<float> positions;
    std::vector<GLuint> indices;

    // loop through stacks.
    for (int i = 0; i <= stacks; ++i)
    {

        float V = (float)i / (float)stacks;
        float phi = V * M_PI;

        // loop through the slices.
        for (int j = 0; j <= slices; ++j)
        {

            float U = (float)j / (float)slices;
            float theta = U * (M_PI * 2);

            // use spherical coordinates to calculate the positions.
            float x = cos(theta) * sin(phi);
            float y = cos(phi);
            float z = sin(theta) * sin(phi);

            positions.push_back(x);
            positions.push_back(y);
            positions.push_back(z);
        }
    }

    // Calc The Index Positions
    for (int i = 0; i < slices * stacks + slices; ++i)
    {
        indices.push_back(GLuint(i));
        indices.push_back(GLuint(i + slices + 1));
        indices.push_back(GLuint(i + slices));

        indices.push_back(GLuint(i + slices + 1));
        indices.push_back(GLuint(i));
        indices.push_back(GLuint(i + 1));
    }

    // upload geometry to GPU.
    glGenBuffers(1, &spherePositionVbo);
    glBindBuffer(GL_ARRAY_BUFFER, spherePositionVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positions.size(), positions.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &sphereIndexVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndexVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);

    //sphereIndexCount = indices.size();
}


bool randomBool()
{
    std::random_device r;
    static auto gen = std::bind(std::uniform_int_distribution<>(0, 1), std::default_random_engine(r()));
    return gen();
}

constexpr double scale = 15000e3; // scale factor from sim units to render units

/*
 1 x-----------x 3
    \  Front  /
     \  Face / 
      \     / 
       \   /
        \ /
         x 2
*/
//
bool BehindTriangle(Vector3d v1, Vector3d v2, Vector3d v3, Vector3d pt)
{
    Vector3d v_1_2 = v2 - v1;
    Vector3d v_2_3 = v3 - v2;
    Vector3d v_3_1 = v1 - v3;
    Vector3d v_1_pt = pt - v1;
    Vector3d v_2_pt = pt - v2;
    Vector3d v_3_pt = pt - v3;

    //faces out of the front side of the triangle
    auto normal = v_1_2.cross(v_2_3).normalized();

    // If dot product >= 0, pt is in front of triangle
    if (v_1_pt.dot(normal) >= 0)
        return false;

    // Testing whether inside triangle bounds in plane of triangle

    auto test1 = v_1_2.cross(v_1_pt);
    if (test1.dot(normal) < 0)
        return false;

    auto test2 = v_2_3.cross(v_2_pt);
    if (test2.dot(normal) < 0)
        return false;

    auto test3 = v_3_1.cross(v_3_pt);
    if (test3.dot(normal) < 0)
        return false;


    return true;
}


static vector<Vector3d> planet_verts = 
{ { pr, 0, 0 },
    { 0, pr, 0 },
    { 0, 0, pr },
    { 0, pr, 0 },
    { -pr, 0, 0 },
    { 0, 0, pr },
    { -pr, 0, 0 },
    { 0, -pr, 0 },
    { 0, 0, pr },
    { 0, -pr, 0 },
    { pr, 0, 0 },
    { 0, 0, pr },
    { pr, 0, 0 },
    { 0, -pr, 0 },
    { 0, 0, -pr },
    { 0, -pr, 0 },
    { -pr, 0, 0 },
    { 0, 0, -pr },
    { -pr, 0, 0 },
    { 0, pr, 0 },
    { 0, 0, -pr },
    { 0, pr, 0 },
    { pr, 0, 0 },
    { 0, 0, -pr }};

class RenderBall
{
public:
    Vector3d pos = { 0, 0, 0 }; //m
    Vector3d vel = { 0, 0, 0 }; //m/s
    Matrix4d T_world_body = Matrix4d::Identity();
    
public:
    double getpos(int idx)
    {
        return pos[idx] / scale;
    }

    double getvel(int idx)
    {
        return vel[idx] / scale;
    }

    Matrix4d getrot()
    {
        return T_world_body;    
    }
};

class Ball : public RenderBall
{
public:
    double restitution = 0.8; //Coefficient of restitution
    Ball* moon = nullptr;
    float age = 0.0f;
    bool dead = false;

public:
    void random_state()
    {
        std::random_device r;
        // std::seed_seq ssq{r()};
        // and then passing it to the engine does the same
        std::default_random_engine eng{ r() };
        std::uniform_real_distribution<double> distribution_r(8e6, 11e6);
        std::uniform_real_distribution<double> distribution_theta(0, 2 * M_PI);
        std::uniform_real_distribution<double> distribution_z(-2e6, 2e6);
        //std::uniform_real_distribution<double> distribution_vxy(5000, 9000);
        std::uniform_real_distribution<double> distribution_vtheta(7.6e3, 8.5e3);
        std::uniform_real_distribution<double> distribution_vr(-300, 300);
        std::uniform_real_distribution<double> distribution_vz(-2000, 2000);

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

    void random_bump()
    {
        std::random_device r;
        // std::seed_seq ssq{r()};
        // and then passing it to the engine does the same
        std::default_random_engine eng{ r() };
        std::uniform_real_distribution<double> distribution_r(6.7e6 * 0.3, 7.5e6 * 0.3);
        std::uniform_real_distribution<double> distribution_theta(0, 2 * M_PI);
        std::uniform_real_distribution<double> distribution_phi(M_PI / 4, 3*M_PI / 4);
        std::uniform_real_distribution<double> distribution_v(1000, 5000);
        std::uniform_real_distribution<double> distribution_vz(-500, 500);

        double radius = distribution_r(eng);
        double theta = distribution_theta(eng);
        double phi = distribution_phi(eng);

        double rhat[3];
        rhat[0] = std::cos(theta) * std::sin(phi);
        rhat[1] = std::sin(theta) * std::sin(phi);
        rhat[2] = std::cos(phi);

        pos[0] += radius * rhat[0];
        pos[1] += radius * rhat[1];
        pos[2] += radius * rhat[2];

        for (int i = 0; i < 2; ++i)
        {
            double number = distribution_v(eng);
            bool sign = randomBool();
            if (sign)
                vel[i] += number;
            else
                vel[i] -= number;
        }

        double number = distribution_vz(eng);
        vel[2] += number;
    }

    void step(double dt)
    {
        // Euler propagation
        pos += vel * dt;

        // Gather metrics
        double vmag = vel.norm();

        Vector3d vhat = vel.normalized();

        // Distance and direction to center
        double pmag = pos.norm();
        Vector3d phat = pos.normalized();

        // interesting rotation
        T_world_body(seq(0, 3),0) = vhat;
        T_world_body(seq(0, 3), 1) = vhat.cross(phat);
        T_world_body(seq(0, 3), 2) = phat;
        
        // Distance and direction for moon
        Vector3d rpos;
        double mpmag;
        Vector3d mphat;
        if (moon != nullptr)
        {
            rpos = pos - moon->pos;
            mpmag = rpos.norm();
            mphat = rpos.normalized();
        }


        // Collision handling
        if(pmag < 6.2e6)
        {
            for (size_t i = 0; i < planet_verts.size(); i += 3)
            {
                Vector3d v1 = planet_verts[i];
                Vector3d v2 = planet_verts[i + 1];
                Vector3d v3 = planet_verts[i + 2];

                Vector3d v_1_2 = v2 - v1;
                Vector3d v_2_3 = v3 - v2;

                auto normal = v_1_2.cross(v_2_3).normalized();

                // Avoid checking tris not on the same side as the particle
                if (pos.dot(normal) <= 0)
                    continue;

                if (BehindTriangle(v1,v2,v3,pos/scale))
                    dead = true;
            }
        }
        if (moon != nullptr && mpmag < 6.2e6 * 0.3 * 0.6)
        {
            dead = true;
        }

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
        Vector3d ag_ = -(phat) * mu / (pmag * pmag);
        vel += ag_ * dt;

        if (moon != nullptr)
        {
            // Newton gravity around moon
            mu = 3.986004e14 * 0.3 * 0.3 * 0.3; // m^3/s^2
            ag_ = -(mphat) * mu / (mpmag * mpmag);
            vel += ag_ * dt;
        }

        age += dt;
    }

};




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
    void setVec3(const std::string& name, glm::vec3 value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
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


    
    static glm::mat4 projection = glm::mat4(1.0f);
    // Set Projection matrix
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetFramebufferSizeCallback(window, 
        [](GLFWwindow* window, int width, int height) 
        {
            glViewport(0, 0, width, height); 
            projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
        });
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) 
        {
            camradius -= yoffset/3;
            updateCamCoords();
        });
C:
    //std::vector<opengl_starter::Mesh*> meshes;
    //opengl_starter::Node root{ "root" };
    //opengl_starter::GltfLoader::Load("assets/lowpoly_jet/scene.gltf", &root, meshes);
    //opengl_starter::GltfLoader::Load("assets/lowpoly_jet/scene.gltf", nullptr, meshes);

    

    // Enable depth buffering, backface culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    Shader defaultShader("../../../../shaders/default.vs", "../../../../shaders/default.fs");
    Shader planetShader("../../../../shaders/default.vs", "../../../../shaders/planet.fs");

    
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

    // Set up vertex array object (VAO) and vertex buffers for planet
    buffer_id_t VAO_planet;
    glGenVertexArrays(1, &VAO_planet);
    glBindVertexArray(VAO_planet);
    buffer_id_t planetbuffer;
    glGenBuffers(1, &planetbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, planetbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planet), planet, GL_STATIC_DRAW);
    // position attribute
    Better_glVertexAttribPointer(vtxAttributeIdx_Position, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)0);
    Better_glEnableVertexAttribArray(vtxAttributeIdx_Position);
    // color attribute
    Better_glVertexAttribPointer(vtxAttributeIdx_Color, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    Better_glEnableVertexAttribArray(vtxAttributeIdx_Color);
    /* buffer_id_t planetPosBuffer;
    buffer_id_t planetIdxBuffer;
    glGenBuffers(1, &planetPosBuffer);
    glGenBuffers(1, &planetIdxBuffer);
    CreateSphere(planetPosBuffer, planetIdxBuffer);*/

    // Very frequently reused matrices as shader inputs
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);


    ///////////////////////////////////////////
    //Options
    double timescale = 1000;


    
    Ball moon;
    vector<Ball*> balls;
    /*for (int i = 0; i<ball_count; i++)
    {
        balls.push_back(new Ball);
        balls.back()->restitution = restitution;
    }*/


    auto prevTime = std::chrono::high_resolution_clock::now();
    for (auto ball : balls)
    {
        ball->random_state();
    }

    moon.random_state();

    int step_skip_itr = 0;

    double accumulated_dt = 0;
    double irl_elapsed = 0;
    double sim_elapsed = 0;

    glm::vec3 lightDir = { 1, 3, 1 }; // position of lightsource
    lightDir = -glm::normalize(lightDir);

    // Rendering loop
    while (!glfwWindowShouldClose(window)) {
        /*processInput(window);*/
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Set view matrix
        view = glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
        
        

        // Wireframe/not
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);



        /////////////////////////////////////////
        // Render planets
        planetShader.use();
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        planetShader.setVec3("lightDir", lightDir);
        planetShader.setFloat("scale", 1.0f);
        glBindVertexArray(VAO_planet);

        // Translate planet to its position and draw
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
        planetShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 24);

        planetShader.setFloat("scale", 0.3f);

        // Translate planet to its position and draw
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(moon.getpos(0), moon.getpos(1), moon.getpos(2)));
        planetShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 24);
        /////////////////////////////////////////




        /////////////////////////////////////////
        // Render balls 
        defaultShader.use();
        defaultShader.setMat4("projection", projection);
        defaultShader.setMat4("view", view);
        defaultShader.setVec3("lightDir", lightDir);
        defaultShader.setFloat("scale", 1.0f);
        glBindVertexArray(VAO);
        //glBindVertexArray(meshes[0]->vao);

        for (auto ball : balls)
        {
            if (ball->dead)
                continue;
            // Translate ball to its position and draw
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(ball->getpos(0), ball->getpos(1), ball->getpos(2)));
            auto transf_rot = ball->getrot();
            model = model * E2GLM(transf_rot);
            defaultShader.setMat4("model", model);
            defaultShader.setFloat("age", ball->age / timescale);
            glDrawArrays(GL_TRIANGLES, 0, 24);
        }
        /////////////////////////////////////////
            



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

            if (irl_elapsed < 1)
            {
                for (int i = 0; i < 10; i++)
                {
                    balls.push_back(new Ball);
                    balls.back()->restitution = restitution;
                    balls.back()->moon = &moon;
                    balls.back()->pos[0] = moon.pos[0];
                    balls.back()->pos[1] = moon.pos[1];
                    balls.back()->pos[2] = moon.pos[2];
                    balls.back()->vel[0] = moon.vel[0];
                    balls.back()->vel[1] = moon.vel[1];
                    balls.back()->vel[2] = moon.vel[2];
                    balls.back()->random_bump();
                }
            }
            for (auto ball : balls)
            {
                if (!ball->dead)
                    ball->step(accumulated_dt);
            }
            moon.step(accumulated_dt);
            
            
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