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
#include <set>
#include <algorithm>
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

// Options
static int step_skip_amt = 1; //how many graphics steps per physics step
static int ball_count = 20;
static double timescale = 1.0;
// 1 for Euler, 2 for RK4
static int prop_type = 1;
static int shape = 1;
static double drop_height = 2.0;

static double init_delay_s = 1;

static Quaterniond PropagateQuat(Eigen::Quaterniond state, Eigen::Vector3d angVel, double timeElapsedSec)
{
	timeElapsedSec *= -1;

	double q0 = state.w();
	double q1 = state.x();
	double q2 = state.y();
	double q3 = state.z();

	double wx = angVel.x();
	double wy = angVel.y();
	double wz = angVel.z();

	double Quat0 = q0;
	double Quat1 = q1;
	double Quat2 = q2;
	double Quat3 = q3;
	Quat0 += 0.5 * ((-q1 * wx) - (q2 * wy) - (q3 * wz)) * timeElapsedSec;
	Quat1 += 0.5 * ((q0 * wx) - (q3 * wy) + (q2 * wz)) * timeElapsedSec;
	Quat2 += 0.5 * ((q3 * wx) + (q0 * wy) - (q1 * wz)) * timeElapsedSec;
	Quat3 += 0.5 * ((-q2 * wx) + (q1 * wy) + (q0 * wz)) * timeElapsedSec;

	return Eigen::Quaterniond(Quat0, Quat1, Quat2, Quat3).normalized();
}

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
float br2 = 0.05; // ball radius
float ball[] = {
    // positions         // colors
     br2,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 1
      0, br2,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0, br2,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0, br2,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 2
    -br2,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0, br2,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
    -br2,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 3
      0,-br2,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0, br2,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,-br2,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 4
     br2,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0, br2,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
     br2,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 5
      0,-br2,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0,-br2,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,-br2,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 6
    -br2,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0,-br2,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
    -br2,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 7
      0, br2,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0,-br2,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0, br2,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 8
     br2,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0,-br2,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
};


float br = 0.015; // ball radius
float ship[] = {
    // positions         // colors
     4*br, 0, 0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 1
      0,3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0, br,   0,0,0,
      0,3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 2
    -br,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0, br,   0,0,0,
    -br,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 3
      0,-3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0, br,   0,0,0,
      0,-3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 4
     4*br, 0, 0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0, br,   0,0,0,
     4*br, 0, 0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 5
      0,-3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0,-br,   0,0,0,
      0,-3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 6
    -br,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0,-br,   0,0,0,
    -br,  0,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 7
      0,3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0,-br,   0,0,0,
      0,3*br,  0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f, // triangle 8
     4*br, 0, 0,   107.0f/255.0f, 78.0f/255.0f, 45.0f/255.0f,
      0,  0,-br,   0,0,0,
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

double randomDouble(double min, double max)
{
    std::random_device r;
    static auto gen = std::bind(std::uniform_real_distribution<>(min, max), std::default_random_engine(r()));
    return gen();
}

constexpr double scale = 5; // scale factor from sim units to render units

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
bool BehindTriangle(Vector3d v1, Vector3d v2, Vector3d v3, Vector3d pt, double& out_depth, Vector3d& out_normal)
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

    // depth (positive behind triangle) is the negative projection of v 1->pt onto normal
    out_depth = -v_1_pt.dot(normal);
    out_normal = normal;

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


/*
   5+----------+6
   /          /|
 1+----------+ |
  |         2| |
  |          | |
  | o7       | +8
  |          |/
 3+----------+4

     z
     ^
     |
   7 o--->y
    /
    x
*/

static double cube_radius = 0.5;

class RenderRigidCube
{
public:
    vector<Vector3d> bodyframe_verts_pos; //m
    Vector3d pos = { 0, 0, 0 }; //m, world coords
    Vector3d vel = { 0, 0, 0 }; //m/s, world coords
    Quaterniond rot = Quaterniond(1, 0, 0, 0); //Passive Hamilton quat, world coords
    Vector3d angvel = { 0, 0, 0 }; //rad/s, world coords
    Matrix3d Inertia; // body coords
    double mass = 1000.0; //kg
    double restitution = 0.2; 
    
public:
    void init(int shape_choice)
    {
        switch (shape_choice)
        {
        case 1: 
        {
        
            bodyframe_verts_pos = {
                { 2*cube_radius, -cube_radius, cube_radius },   //1
                { cube_radius, cube_radius, cube_radius },    //2
                { cube_radius, -cube_radius, -2 * cube_radius }, //3
                { cube_radius, cube_radius, -cube_radius },   //4
                { -cube_radius, -cube_radius, cube_radius },  //5
                { -cube_radius, 2*cube_radius, cube_radius },   //6
                { -cube_radius, -cube_radius, -cube_radius }, //7
                { -cube_radius, cube_radius, -cube_radius }   //8
            };                                                //m

            double i11 = mass * pow(cube_radius * 2, 2) / 6.0;

            Inertia << i11, 0, 0,
                0, i11, 0,
                0, 0, i11;
        }
            break;

        case 2:
        {
            bodyframe_verts_pos = {
                { 3*cube_radius, -cube_radius, cube_radius },   //1
                { 3*cube_radius, cube_radius, cube_radius },    //2
                { 3*cube_radius, -cube_radius, -cube_radius },  //3
                { 3 * cube_radius, cube_radius, -cube_radius },  //4
                { -3*cube_radius, -cube_radius, cube_radius },  //5
                { -3*cube_radius, cube_radius, cube_radius },   //6
                { -3*cube_radius, -cube_radius, -cube_radius }, //7
                { -3*cube_radius, cube_radius, -cube_radius }   //8
            };                                                //m

            double i11 = mass * pow(cube_radius * 2, 2) / 6.0;

            Inertia << i11, 0, 0,
                0, 3*i11, 0,
                0, 0, 3*i11;
        }
            break;
        }
    }

    Vector3d vert_world_pos(int idx)
    {
        auto T_world_body = rot.toRotationMatrix();
        return T_world_body.transpose() * bodyframe_verts_pos[idx] + pos;
    }

    void translate(Vector3d offset)
    {
        pos += offset;
    }

    void rotate(Quaterniond offset)
    {
        rot *= offset;
    }

    void velocitate(Vector3d deltaV)
    {
        vel += deltaV;
    }

    void angvelocitate(Vector3d deltaAngV)
    {
        angvel += deltaAngV;
    }

    Vector3d pt_world_to_body(Vector3d r_world)
    {
        auto T_world_body = rot.toRotationMatrix();
        return T_world_body * (r_world - pos);
    }

    Vector3d pt_world_to_bodycentered(Vector3d r_world)
    {
        return r_world - pos;
    }

    Vector3d pt_body_to_world(Vector3d r_body)
    {
        auto T_world_body = rot.toRotationMatrix();
        return T_world_body.transpose() * r_body + pos;
    }

    Vector3d world_vel_at_body_pt(Vector3d r_body)
    {
        auto T_world_body = rot.toRotationMatrix();
        Vector3d r_pt_world = T_world_body.transpose() * r_body;
        return vel + angvel.cross(r_pt_world);
    }

    //https://www.cs.utah.edu/~ladislav/kavan03rigid/kavan03rigid.pdf
    void collide(RenderRigidCube* other, Vector3d contact_pt_world, Vector3d other_face_normal)
    {
        // Position of contact point in body coords
        Vector3d self_contact_pt_body = pt_world_to_body(contact_pt_world);
        Vector3d other_contact_pt_body = other->pt_world_to_body(contact_pt_world);

        // Velocity of contact point in world coords
        Vector3d self_contact_vel_world = world_vel_at_body_pt(self_contact_pt_body);
        Vector3d other_contact_vel_world = other->world_vel_at_body_pt(other_contact_pt_body);

        // Relative velocity between contact points
        Vector3d vel_self_rel_to_other_at_contact = self_contact_vel_world - other_contact_vel_world;
        
        // Should be negative
        double diverging_vel = vel_self_rel_to_other_at_contact.dot(other_face_normal);
        //// How fast self is converging with other's face
        //double converging_vel = -diverging_vel;

        //// In frame centered on other's contact pt,
        //double momentum_self = converging_vel * mass;
        //// other's momentum is zero
        //double total_momentum = momentum_self;

        //// To zero-momentum frame
        //double vC = total_momentum / (mass + other->mass);
        //double ZM_vel_self = converging_vel - vC;
        //double ZM_vel_other = 0 - vC;

        auto T_world_body = rot.toRotationMatrix();
        auto T_body_world = T_world_body.transpose();
        auto T_world_body_B = other->rot.toRotationMatrix();
        auto T_body_world_B = T_world_body_B.transpose();
        
        
        //https://www.cs.utah.edu/~ladislav/kavan03rigid/kavan03rigid.pdf eq 28
        Vector3d rA = T_body_world * self_contact_pt_body;
        Vector3d rB = T_body_world_B * other_contact_pt_body;
        Vector3d nB = other_face_normal;
        auto IA = T_body_world * Inertia;
        auto IB = T_body_world * other->Inertia;

        double interim1 = (1.0 / mass) + (1.0 / other->mass) + 
            nB.dot((IA.inverse() * (rA.cross(nB))).cross(rA)) +
            nB.dot((IB.inverse() * (rB.cross(nB))).cross(rB));

        double collision_impulse = (-1 - restitution) * diverging_vel / interim1;

        if (collision_impulse > 10)
            collision_impulse = std::min(collision_impulse, vel.norm() * mass * 5);

        impulse(self_contact_pt_body, collision_impulse * other_face_normal);
        other->impulse(other_contact_pt_body, collision_impulse * (-other_face_normal));
    }

    void collide_wall(Vector3d contact_pt_world, Vector3d other_face_normal, double dt)
    {
        // Position of contact point in body coords
        Vector3d self_contact_pt_body = pt_world_to_body(contact_pt_world);
        //Vector3d other_contact_pt_body = other->pt_world_to_body(contact_pt_world);

        // Velocity of contact point in world coords
        Vector3d self_contact_vel_world = world_vel_at_body_pt(self_contact_pt_body);
        Vector3d other_contact_vel_world = /*other->world_vel_at_body_pt(other_contact_pt_body)*/ {0,0,0};

        // Relative velocity between contact points
        Vector3d vel_self_rel_to_other_at_contact = self_contact_vel_world - other_contact_vel_world;

        // Should be negative
        double diverging_vel = vel_self_rel_to_other_at_contact.dot(other_face_normal);
        //// How fast self is converging with other's face
        //double converging_vel = -diverging_vel;

        //// In frame centered on other's contact pt,
        //double momentum_self = converging_vel * mass;
        //// other's momentum is zero
        //double total_momentum = momentum_self;

        //// To zero-momentum frame
        //double vC = total_momentum / (mass + 9999999999);
        //double ZM_vel_self = converging_vel - vC;
        //double ZM_vel_other = 0 - vC;

        auto T_world_body = rot.toRotationMatrix();
        auto T_body_world = T_world_body.transpose();
        //auto T_world_body_B = other->rot.toRotationMatrix();
        //auto T_body_world_B = T_world_body_B.transpose();

        //https://www.cs.utah.edu/~ladislav/kavan03rigid/kavan03rigid.pdf eq 28
        Vector3d rA = T_body_world * self_contact_pt_body;
        //Vector3d rB = T_body_world_B * other_contact_pt_body;
        Vector3d nB = other_face_normal;
        auto IA = T_body_world * Inertia;
        //auto IB = T_body_world * other->Inertia;

        double interim1 = (1.0 / mass) + (0.0) +
                          nB.dot((IA.inverse() * (rA.cross(nB))).cross(rA)) +
                          0.0;

        double collision_impulse = (-1 - restitution) * diverging_vel / interim1;

        collision_impulse = std::min(collision_impulse, vel.norm() * mass * 5);

        impulse(self_contact_pt_body, collision_impulse * other_face_normal);
        //other->impulse(other_contact_pt_body, (1 + restitution) * collision_impulse * (-other_face_normal));



    }
    
    void impulse(Vector3d offset_body, Vector3d add_momentum_world)
    {
        vel += add_momentum_world / mass;

        auto T_world_body = rot.toRotationMatrix();
        auto T_body_world = T_world_body.transpose();
        Vector3d offset_world = T_body_world * offset_body;


        auto I_world = T_body_world * Inertia;

        // torque = I a
        // r x f = I a
        // r x i = I w
        angvel += I_world.inverse() * offset_world.cross(add_momentum_world);

        if (angvel.norm() > 10)
        {
            angvel *= 10.0 / angvel.norm();
        }
    }

};

static const vector<tuple<int, int, int>> cube_triangles = {
    // Front
    { 1, 3, 2 },
    { 2, 3, 4 },
    // Left
    { 5, 7, 1 },
    { 1, 7, 3 },
    // Back
    { 6, 8, 5 },
    { 5, 8, 7 },
    // Right
    { 2, 4, 6 },
    { 6, 4, 8 },
    // Top
    { 5, 1, 6 },
    { 6, 1, 2 },
    // Bottom
    { 3, 7, 4 },
    { 4, 7, 8 },
};

class RigidCube : public RenderRigidCube
{
public:
    float age = 0.0f;
    vector<RigidCube*>* others;
    double vert_mass = 0.4; //kg
    bool dead = false;
    
public:

    Vector3d get_accel_for_state(int vert_idx, VectorXd X)
    {
        //auto& pos = verts_pos[i];
        //auto& vel = verts_vel[vert_idx];
        auto pos = X.head(3);
        auto vel = X.tail(3);
        Vector3d acc = { 0, 0, 0 };

        // Gather metrics
        double vmag = vel.norm();

        Vector3d vhat = vel.normalized();

        // Distance and direction to center
        double pmag = pos.norm();
        Vector3d phat = pos.normalized();

        //// Collision handling
        //if(pmag < 6.2e6)
        //{
        //    for (size_t i = 0; i < planet_verts.size(); i += 3)
        //    {
        //        Vector3d v1 = planet_verts[i];
        //        Vector3d v2 = planet_verts[i + 1];
        //        Vector3d v3 = planet_verts[i + 2];

        //        Vector3d v_1_2 = v2 - v1;
        //        Vector3d v_2_3 = v3 - v2;

        //        auto normal = v_1_2.cross(v_2_3).normalized();

        //        // Avoid checking tris not on the same side as the particle
        //        if (pos.dot(normal) <= 0)
        //            continue;

        //        if (BehindTriangle(v1,v2,v3,pos/scale))
        //            dead = true;
        //    }
        //}

        // Accelerations

        //// Air drag
        //double density = 0.1; //kg/m3
        //double Cd = 0.4; //drag coefficient plus some other terms
        //double drag_mag = 0.5 * density * vmag * vmag * Cd;
        //vel[0] -= vhat[0] * drag_mag * dt;
        //vel[1] -= vhat[1] * drag_mag * dt;
        //vel[2] -= vhat[2] * drag_mag * dt;

        // Vertical Gravity
        acc += Vector3d{ 0, 0, -9.8 };


        return acc;
    }

    void step(double dt)
    {   
        if (dead)
            return;

        VectorXd X_0(6);
        X_0.head(3) = pos;
        X_0.tail(3) = vel;
        // Accelerations
        auto acc = get_accel_for_state(0, X_0);


        if (pos.norm() > 1000)
            dead = true;

        //// 
        //for (int i = 0; i < bodyframe_verts_pos.size(); i++)
        //{
        //    auto& pos = vert_world_pos(i);
        //    //auto& vel = verts_vel[i];
        //    //auto& acc = verts_accel[i];

        //    // Gather metrics
        //    double vmag = vel.norm();

        //    Vector3d vhat = vel.normalized();

        //    // Distance and direction to center
        //    double pmag = pos.norm();
        //    Vector3d phat = pos.normalized();

        //    //// Collision handling
        //    //if(pmag < 6.2e6)
        //    //{
        //    //    for (size_t i = 0; i < planet_verts.size(); i += 3)
        //    //    {
        //    //        Vector3d v1 = planet_verts[i];
        //    //        Vector3d v2 = planet_verts[i + 1];
        //    //        Vector3d v3 = planet_verts[i + 2];

        //    //        Vector3d v_1_2 = v2 - v1;
        //    //        Vector3d v_2_3 = v3 - v2;

        //    //        auto normal = v_1_2.cross(v_2_3).normalized();

        //    //        // Avoid checking tris not on the same side as the particle
        //    //        if (pos.dot(normal) <= 0)
        //    //            continue;

        //    //        if (BehindTriangle(v1,v2,v3,pos/scale))
        //    //            dead = true;
        //    //    }
        //    //}

        //    VectorXd X(6);
        //    X.head(3) = pos;
        //    X.tail(3) = vel;

        //    // Accelerations
        //    acc = get_accel_for_state(i, X);
        //}


        // Vertex-Face Collisions
        for (int i = 0; i < bodyframe_verts_pos.size(); i++)
        {
            auto& vpos = vert_world_pos(i);
            //auto& vel = verts_vel[i];
            
            //Collision with others
            for (auto other : *others)
            {
                if (other == this)
                    continue;

                if (other->dead)
                    continue;

                double bounds_radius = sqrt(3.0 * 2 * cube_radius * 2 * cube_radius);

                Vector3d offset = pos - other->pos;

                if (offset.norm() > bounds_radius * 2.2)
                    continue;

                for (const auto& tri : cube_triangles)
                {
                    auto v1 = get<0>(tri);
                    auto v2 = get<1>(tri);
                    auto v3 = get<2>(tri);
                    const auto& pos1 = other->vert_world_pos(v1 - 1);
                    const auto& pos2 = other->vert_world_pos(v2 - 1);
                    const auto& pos3 = other->vert_world_pos(v3 - 1);

                    double depth = 0.0;
                    Vector3d tri_normal;

                    if (BehindTriangle(pos1, pos2, pos3, vpos, depth, tri_normal))
                    {
                        // Not too far behind that triangle
                        if (depth < 0.5)
                        {
                            // Collide with that triangle
                            // Velocity into collision is projection of this vert's velocity onto normal

                            collide(other, vpos, tri_normal);

                            //auto collision_vel = vel.dot(tri_normal) * tri_normal;

                            ////depenetrate
                            //vpos += depth * tri_normal * 0.5;

                            //other->verts_pos[v1 - 1] -= depth * tri_normal * 0.5 / 3;
                            //other->verts_pos[v2 - 1] -= depth * tri_normal * 0.5 / 3;
                            //other->verts_pos[v3 - 1] -= depth * tri_normal * 0.5 / 3;

                            //// neutralize vel
                            //vel -= collision_vel;

                            //// impart momentum on triangle
                            //other->verts_vel[v1 - 1] += collision_vel / 3;
                            //other->verts_vel[v2 - 1] += collision_vel / 3;
                            //other->verts_vel[v3 - 1] += collision_vel / 3;
                        }
                    }
                }
            }

            // "Floor" collision
            // TODO can apply surface friction based on time during tick spent on surface
            double floor_height = -1;
            if (vpos.z() < floor_height)
            {
                collide_wall(vpos, { 0, 0, 1 }, dt);
                double distance_lower = -(vpos.z() - floor_height);
                translate({ 0, 0, distance_lower });

                //// Fake friction //TODO add to collide_wall
                //vel.x() *= 0.6;
                //vel.y() *= 0.6;
            }
            if (vpos.x() < -3.1)
            {
                collide_wall(vpos, { 1, 0, 0 }, dt);
                double distance_further_nX = -(vpos.x() + 3.1);
                translate({ distance_further_nX, 0, 0 });
            }
            if (vpos.x() > 3.1)
            {
                collide_wall(vpos, { -1, 0, 0 }, dt);
                double distance_further_pX = (vpos.x() - 3.1);
                translate({ -distance_further_pX, 0, 0 });
            }
            if (vpos.y() < -3.1)
            {
                collide_wall(vpos, { 0, 1, 0 }, dt);
                double distance_further_nY = -(vpos.y() + 3.1);
                translate({ 0, distance_further_nY, 0  });
            }
            if (vpos.y() > 3.1)
            {
                collide_wall(vpos, { 0, -1, 0 }, dt);
                double distance_further_pY = (vpos.y() - 3.1);
                translate({ 0, -distance_further_pY, 0 });
            }
        }

        // Propagation
        if (prop_type == 1)
        {
            // Euler propagation
            pos += vel * dt;

            // Euler propagation
            vel += acc * dt;

            rot = PropagateQuat(rot, angvel, dt);
        }
        else //RK4
        {
            //// RK4 propagation
            ////http://spiff.rit.edu/richmond/nbody/OrbitRungeKutta4.pdf

            ////rdot = v
            ////vdot = a
            //auto k1v = get_accel_for_pos(i, pos);
            //auto k2v = get_accel_for_pos(i, pos + k1v * dt/2);
            //auto k3v = get_accel_for_pos(i, pos + k2v * dt/2);
            //auto k4v = get_accel_for_pos(i, pos + k3v * dt);

            //auto k1r = vel;
            //auto k2r = vel.norm() * k1v * dt / 2;
            //auto k3r = vel.norm() * k2v * dt / 2;
            //auto k4r = vel.norm() * k3v * dt;

            //vel = vel + (dt / 6) * (k1v + 2 * k2v + 2 * k3v + k4v);
            //pos = pos + (dt / 6) * (k1r + 2 * k2r + 2 * k3r + k4r);

            // RK4 propagation
            //https://scicomp.stackexchange.com/a/19022

            //rdot = v
            //vdot = a

            VectorXd X(6);
            X.head(3) = pos;
            X.tail(3) = vel;

            VectorXd k1(6);
            VectorXd k2(6);
            VectorXd k3(6);
            VectorXd k4(6);
            k1 << vel, get_accel_for_state(0, X);
            k2 << vel, get_accel_for_state(0, X + k1 * dt / 2);
            k3 << vel, get_accel_for_state(0, X + k2 * dt / 2);
            k4 << vel, get_accel_for_state(0, X + k3 * dt);

            X = X + (dt / 6) * (k1 + 2 * k2 + 2 * k3 + k4);

            pos = X.head(3);
            vel = X.tail(3);
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

static void Better_glDisableVertexAttribArray(int attributeToConfigureIdx)
{
    glDisableVertexAttribArray(attributeToConfigureIdx);
}

static void Better_glUniformMatrix4fv(matrix_id_t mat, int howManyMatricesToSend, bool transpose, glm::f32* value)
{
    glUniformMatrix4fv(mat, howManyMatricesToSend, transpose, value);
}

static void updateBuffer(buffer_id_t id, size_t offset, void* data, size_t size, int type)
{
    glBindBuffer(type, id);
    glBufferSubData(type, offset, size, data);
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
    if (argc >= 2) {
        string arg = string(argv[1]);
        ball_count = stoi(arg);
    }
    if (argc >= 3)
    {
        string arg = string(argv[2]);
        timescale = stod(arg);
    }
    if (argc >= 4)
    {
        string arg = string(argv[3]);
        prop_type = stoi(arg);
    }
    if (argc >= 5)
    {
        string arg = string(argv[4]);
        shape = stoi(arg);
    }
        
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //MacOS specific
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Connor Jakubik HW5", NULL, NULL);
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
    Shader shotShader("../../../../shaders/default.vs", "../../../../shaders/soft_body.fs");
    Shader planetShader("../../../../shaders/default.vs", "../../../../shaders/planet.fs");

    
    // Set up vertex array object (VAO) and vertex buffers for box and ball
    buffer_id_t /*boxbuffer, */shipbuffer, VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    //glGenBuffers(1, &boxbuffer);
    //glBindBuffer(GL_ARRAY_BUFFER, boxbuffer);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_STATIC_DRAW);
    glGenBuffers(1, &shipbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, shipbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ship), ship, GL_STATIC_DRAW);
    // position attribute 
    Better_glVertexAttribPointer(vtxAttributeIdx_Position, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)0);
    Better_glEnableVertexAttribArray(vtxAttributeIdx_Position);
    // color attribute
    Better_glVertexAttribPointer(vtxAttributeIdx_Color, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    Better_glEnableVertexAttribArray(vtxAttributeIdx_Color);

    buffer_id_t ballbuffer, VAO_shot;
    glGenVertexArrays(1, &VAO_shot);
    glBindVertexArray(VAO_shot);
    glGenBuffers(1, &ballbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, ballbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ball), ball, GL_STATIC_DRAW);
    // position attribute
    Better_glVertexAttribPointer(vtxAttributeIdx_Position, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)0);
    Better_glEnableVertexAttribArray(vtxAttributeIdx_Position);
    // color attribute
    Better_glVertexAttribPointer(vtxAttributeIdx_Color, 3, GL_FLOAT, false, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    Better_glEnableVertexAttribArray(vtxAttributeIdx_Color);

    // STUFF FOR SOFT CUBE TRIANGLE-DRAWING
    // http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
    buffer_id_t VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    // An array of 3 vectors which represents 3 vertices
    static const GLfloat g_vertex_buffer_data[] = {
       -1.0f, -1.0f, 0.0f,
       1.0f, -1.0f, 0.0f,
       0.0f,  1.0f, 0.0f,
    };
    // This will identify our vertex buffer
    buffer_id_t vertexbuffer;
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexbuffer);
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

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

    vector<RigidCube*> bodies;

    //for (int i = -(ball_count / 2); i < ball_count - (ball_count / 2); i++)
    //{
    //    bodies.push_back(new SoftCube);
    //    bodies.back()->translate_verts(Vector3d{ i * 3.0, 0.0, drop_height });
    //    bodies.back()->others = &bodies;
    //}

    //Spawning
    for (int i = 0; i < ball_count; i++)
    {
        int shape_ = 1;
        if (randomBool())
        {
            shape_ = 2;
        }
        bodies.push_back(new RigidCube);
        bodies.back()->init(shape_);
        bodies.back()->translate(Vector3d{ randomDouble(-1, 1), randomDouble(-1, 1), drop_height + i * 2.0 });
        bodies.back()->velocitate(Vector3d{ randomDouble(-3000, 3000), randomDouble(-3000, 3000), 0 });
        AngleAxisd rot_shift;
        rot_shift = AngleAxisd(randomDouble(-1, 1), Vector3d{ randomDouble(-1, 1), randomDouble(-1, 1), randomDouble(-1, 1) });
        bodies.back()->rotate(Quaterniond(rot_shift));
        bodies.back()->others = &bodies;
    }

    auto prevTime = std::chrono::high_resolution_clock::now();


    int step_skip_itr = 0;

    double accumulated_dt = 0;
    double irl_elapsed = 0;
    double sim_elapsed = 0;

    int ballsSoFar = 0;

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

        // Planet "floor"
        for (int i = -4; i <= 4; i++)
        {
            for (int j = -4; j <= 4; j++)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(i*0.35, j*0.35, -0.5));
                planetShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 24);
            }
        }

        //planetShader.setFloat("scale", 0.3f);

        //// Translate planet to its position and draw
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, glm::vec3(moon.getpos(0), moon.getpos(1), moon.getpos(2)));
        //planetShader.setMat4("model", model);
        //glDrawArrays(GL_TRIANGLES, 0, 24);
        /////////////////////////////////////////




        ///////////////////////////////////////////
        //// Render ships 
        //defaultShader.use();
        //defaultShader.setMat4("projection", projection);
        //defaultShader.setMat4("view", view);
        //defaultShader.setVec3("lightDir", lightDir);
        //defaultShader.setFloat("scale", 1.0f);
        //glBindVertexArray(VAO);
        ////glBindVertexArray(meshes[0]->vao);

        //
        //for (auto ball : red_team)
        //{
        //    if (ball->dead)
        //        continue;
        //    // Translate ball to its position and draw
        //    model = glm::mat4(1.0f);
        //    model = glm::translate(model, glm::vec3(ball->getpos(0), ball->getpos(1), ball->getpos(2)));
        //    auto transf_rot = ball->getrot();
        //    model = model * E2GLM(transf_rot);
        //    defaultShader.setMat4("model", model);
        //    defaultShader.setFloat("age", 100);
        //    //defaultShader.setFloat("age", ball->age / timescale);
        //    glDrawArrays(GL_TRIANGLES, 0, 24);
        //}
        //
        //for (auto ball : blue_team)
        //{
        //    if (ball->dead)
        //        continue;
        //    // Translate ball to its position and draw
        //    model = glm::mat4(1.0f);
        //    model = glm::translate(model, glm::vec3(ball->getpos(0), ball->getpos(1), ball->getpos(2)));
        //    auto transf_rot = ball->getrot();
        //    model = model * E2GLM(transf_rot);
        //    defaultShader.setMat4("model", model);
        //    defaultShader.setFloat("age", 0);
        //    glDrawArrays(GL_TRIANGLES, 0, 24);
        //}
        ///////////////////////////////////////////
        //// Render shots
        //shotShader.use();
        //shotShader.setMat4("projection", projection);
        //shotShader.setMat4("view", view);
        //shotShader.setVec3("lightDir", lightDir);
        //shotShader.setFloat("scale", 0.3f);
        //glBindVertexArray(VAO_shot);
        ////glBindVertexArray(meshes[0]->vao);

        //for (auto body : bodies)
        //{
        //    Vector3d model_center = {0,0,0};
        //    for (const auto& vert_pos : body->verts_pos)
        //    {
        //        model_center += vert_pos / body->verts_pos.size();
        //    }

        //    for (const auto& vert_pos : body->verts_pos)
        //    {
        //        auto scaled_pos = vert_pos / scale;
        //        // Translate ball to its position and draw
        //        model = glm::mat4(1.0f);
        //        model = glm::translate(model, glm::vec3(scaled_pos.x(), scaled_pos.y(), scaled_pos.z()));
        //        shotShader.setMat4("model", model);
        //        shotShader.setVec3("model_center", glm::vec3(model_center.x(), model_center.y(), model_center.z()));
        //        shotShader.setFloat("age", 100);
        //        //defaultShader.setFloat("age", ball->age / timescale);
        //        glDrawArrays(GL_TRIANGLES, 0, 24);
        //    }
        //}
        /////////////////////////////////////////
        // Render soft cubes
        shotShader.use();
        shotShader.setMat4("projection", projection);
        shotShader.setMat4("view", view);
        shotShader.setVec3("lightDir", lightDir);
        shotShader.setFloat("scale", 1.0f);
        glBindVertexArray(VAO_shot);
        
        //// 1st attribute buffer : vertices
        //glEnableVertexAttribArray(0);
        //glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        //glVertexAttribPointer(
        //    0,        // attribute 0. No particular reason for 0, but must match the layout in the shader.
        //    3,        // size
        //    GL_FLOAT, // type
        //    GL_FALSE, // normalized?
        //    0,        // stride
        //    (void*)0  // array buffer offset
        //);

        for (auto body : bodies)
        {
            Vector3d model_center = { 0, 0, 0 };
            /*for (const auto& vert_pos : body->verts_pos)
            {
                model_center += vert_pos / scale / body->verts_pos.size();
            }*/
            model_center = body->pos;

            for (const auto& tri : cube_triangles)
            {
                auto v1 = get<0>(tri);
                auto v2 = get<1>(tri);
                auto v3 = get<2>(tri);
                auto scaled_pos1 = body->vert_world_pos(v1-1) / scale;
                auto scaled_pos2 = body->vert_world_pos(v2-1) / scale;
                auto scaled_pos3 = body->vert_world_pos(v3-1) / scale;
                // Translate ball to its position and draw
                model = glm::mat4(1.0f);
                //model = glm::translate(model, glm::vec3(scaled_pos.x(), scaled_pos.y(), scaled_pos.z()));
                shotShader.setMat4("model", model);
                //auto r_1_2 = scaled_pos2 - scaled_pos1;
                //auto r_2_3 = scaled_pos3 - scaled_pos2;
                //auto norm = r_1_2.cross(r_2_3);
                shotShader.setVec3("model_center", glm::vec3(model_center.x(), model_center.y(), model_center.z()));


                // update verts

                // An array of 3 vectors which represents 3 vertices
                GLfloat g_vertex_buffer_data_new[] = {
                    scaled_pos1.x(),
                    scaled_pos1.y(),
                    scaled_pos1.z(),
                    scaled_pos2.x(),
                    scaled_pos2.y(),
                    scaled_pos2.z(),
                    scaled_pos3.x(),
                    scaled_pos3.y(),
                    scaled_pos3.z(),
                };


                // 1st attribute buffer : vertices
                Better_glEnableVertexAttribArray(0);
                //glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
                updateBuffer(vertexbuffer, 0, g_vertex_buffer_data_new, sizeof(g_vertex_buffer_data_new), GL_ARRAY_BUFFER);
                Better_glVertexAttribPointer(
                    0,        // attribute 0. No particular reason for 0, but must match the layout in the shader.
                    3,        // size
                    GL_FLOAT, // type
                    false,    // normalized?
                    0,        // stride
                    (void*)0  // array buffer offset
                );
                // Draw the triangle !
                glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
                Better_glDisableVertexAttribArray(0);
            }
        }

        ////// Draw the triangle !
        //glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
        //glDisableVertexAttribArray(0);
        ///////////////////////////////////////////

        

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
        if (irl_elapsed > init_delay_s)
        {
            accumulated_dt += dt_sim;
            sim_elapsed += dt_sim;
            if (++step_skip_itr == step_skip_amt)
            {
                step_skip_itr = 0;

                for (auto body : bodies)
                {
                    body->step(min(0.1, accumulated_dt));
                    //body->step(0.002);
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

        
        prevTime = nowTime;
                
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &VAO_shot);
    glDeleteVertexArrays(1, &VAO_planet);
    //glDeleteBuffers(1, &boxbuffer);
    glDeleteBuffers(1, &ballbuffer);
    glDeleteBuffers(1, &shipbuffer);
    //glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}