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
static int ball_count = 1;
static double timescale = 1.0;
// 1 for Euler, 2 for RK4
static int prop_type = 1;
static double drop_height = 2.0;



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


class RenderSoftCube
{
public:
    vector<Vector3d> verts_pos = {
        { 1, 0, 1 }, //1
        { 1, 1, 1 }, //2
        { 1, 0, 0 }, //3
        { 1, 1, 0 }, //4
        { 0, 0, 1 }, //5
        { 0, 1, 1 }, //6
        { 0, 0, 0 }, //7
        { 0, 1, 0 }  //8
    }; //m
    vector<Vector3d> verts_vel = {
        { 0, 0, 0 }, //1
        { 0, 0, 0 }, //2
        { 0, 0, 0 }, //3
        { 0, 0, 0 }, //4
        { 0, 0, 0 }, //5
        { 0, 0, 0 }, //6
        { 0, 0, 0 }, //7
        { 0, 0, 0 }  //8
    }; //m/s
    
public:
    Vector3d vert_pos(int idx)
    {
        return verts_pos[idx];
    }

    Vector3d vert_vel(int idx)
    {
        return verts_vel[idx];
    }

    void translate_verts(Vector3d offset)
    {
        for (auto& pos : verts_pos)
        {
            pos += offset;
        }
    }

    void velocitate_verts(Vector3d deltaV)
    {
        for (auto& vel : verts_vel)
        {
            vel += deltaV;
        }
    }

};

struct Spring
{
    // 1-indexed start vertex
    int v1;
    // 1-indexed end vertex
    int v2;
    double unstretched_length;

    //// Springs can be deduplicated
    //bool operator ==(const Spring& rhs) const
    //{
    //    return (v1 == rhs.v1 && v2 == rhs.v2 && unstretched_length == rhs.unstretched_length) ||
    //           (v1 == rhs.v2 && v2 == rhs.v1 && unstretched_length == rhs.unstretched_length);
    //}

    // Springs can be deduplicated
    bool operator<(const Spring& rhs) const
    {
        bool vertices_match = (v1 == rhs.v1 && v2 == rhs.v2) ||
                              (v1 == rhs.v2 && v2 == rhs.v1);
        if (vertices_match)
        {
            return unstretched_length < rhs.unstretched_length;
        }

        return std::tie(v1, v2, unstretched_length) < std::tie(rhs.v1, rhs.v2, rhs.unstretched_length);
    }

};

const double hypot_1_1 = sqrt(2.0);

// Index of start vert, index of end vert, unstretched length.
// Auto-deduplicates.
static const set<Spring> springs = {
    // Front face
    { 1, 2, 1.0 },
    { 2, 4, 1.0 },
    { 4, 3, 1.0 },
    { 3, 1, 1.0 },
    { 1, 4, hypot_1_1 },
    // Back face
    { 1 + 4, 2 + 4, 1.0 },
    { 2 + 4, 4 + 4, 1.0 },
    { 4 + 4, 3 + 4, 1.0 },
    { 3 + 4, 1 + 4, 1.0 },
    { 1 + 4, 4 + 4, hypot_1_1 },
    // Left face
    { 1, 5, 1.0 },
    { 5, 7, 1.0 },
    { 7, 3, 1.0 },
    { 3, 1, 1.0 },
    { 1, 7, hypot_1_1 },
    // Right face
    { 1 + 1, 5 + 1, 1.0 },
    { 5 + 1, 7 + 1, 1.0 },
    { 7 + 1, 3 + 1, 1.0 },
    { 3 + 1, 1 + 1, 1.0 },
    { 1 + 1, 7 + 1, hypot_1_1 },
    // Top face
    { 1, 2, 1.0 },
    { 2, 6, 1.0 },
    { 6, 5, 1.0 },
    { 5, 1, 1.0 },
    { 1, 6, hypot_1_1 },
    // Bottom face
    { 1 + 2, 2 + 2, 1.0 },
    { 2 + 2, 6 + 2, 1.0 },
    { 6 + 2, 5 + 2, 1.0 },
    { 5 + 2, 1 + 2, 1.0 },
    { 1 + 2, 6 + 2, hypot_1_1 },
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

class SoftCube : public RenderSoftCube
{
public:
    double springconst = 100.0; // N / m
    double damperconst = 5.0; // N / (m/s)
    float age = 0.0f;

    double vert_mass = 0.4; //kg
    
public:


    void step(double dt)
    {
        // For deferring actually moving until all verts' accels have been determined
        vector<Vector3d> verts_accel = {
            { 0, 0, 0 }, //1
            { 0, 0, 0 }, //2
            { 0, 0, 0 }, //3
            { 0, 0, 0 }, //4
            { 0, 0, 0 }, //5
            { 0, 0, 0 }, //6
            { 0, 0, 0 }, //7
            { 0, 0, 0 }  //8
        };               //m/s2


        for (int i = 0; i < verts_pos.size(); i++)
        {
            auto& pos = verts_pos[i];
            auto& vel = verts_vel[i];
            auto& acc = verts_accel[i];

            // Gather metrics
            double vmag = vel.norm();

            Vector3d vhat = vel.normalized();

            // Distance and direction to center
            double pmag = pos.norm();
            Vector3d phat = pos.normalized();

            //if (pmag > 4e7)
            //    dead = true;

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
            //if (moon != nullptr && mpmag < 6.2e6 * 0.3 * 0.6)
            //{
            //    dead = true;
            //}

            // Accelerations
            acc = { 0, 0, 0 };


            //// Air drag
            //double density = 0.1; //kg/m3
            //double Cd = 0.4; //drag coefficient plus some other terms
            //double drag_mag = 0.5 * density * vmag * vmag * Cd;
            //vel[0] -= vhat[0] * drag_mag * dt;
            //vel[1] -= vhat[1] * drag_mag * dt;
            //vel[2] -= vhat[2] * drag_mag * dt;


            // Vertical Gravity
            acc += Vector3d{0, 0, -9.8};
            
            // Spring/Damper forces
            for (const auto& spring : springs)
            {
                // Index of spring start
                auto v1 = spring.v1 - 1;
                auto v2 = spring.v2 - 1;
                if (v1 != i && v2 != i)
                    continue;

                Vector3d other_vert_pos;
                Vector3d other_vert_vel;
                if (v1 == i)
                {
                    other_vert_pos = verts_pos[v2];
                    other_vert_vel = verts_vel[v2];
                }
                else
                {
                    other_vert_pos = verts_pos[v1];
                    other_vert_vel = verts_vel[v1];
                }

                // Spring Forces
                auto r_me_to_other = other_vert_pos - pos;
                auto current_length = r_me_to_other.norm();
                auto deflection = current_length - spring.unstretched_length;
                auto rhat_me_to_other = r_me_to_other.normalized();
                // Positive for a stretched spring, negative for compressed
                auto spring_force_mag = deflection * springconst;
                // Force on this vert due to spring
                auto spring_force_on_me = spring_force_mag * rhat_me_to_other;
                acc += spring_force_on_me / vert_mass;

                // Damper Forces
                auto v_me_rel_to_other = other_vert_vel - vel;
                auto radial_vel_me_towards_other = v_me_rel_to_other.dot(rhat_me_to_other);
                // Positive for a vel towards, negative for vel away
                auto damper_force_mag = radial_vel_me_towards_other * damperconst;
                // Force on this vert due to damper
                auto damper_force_on_me = damper_force_mag * (rhat_me_to_other);
                acc += damper_force_on_me / vert_mass;
            }



        }


        // Propagation
        for (int i = 0; i < verts_pos.size(); i++)
        {
            auto& pos = verts_pos[i];
            auto& vel = verts_vel[i];
            
            if (prop_type == 1)
            {
                // Euler propagation
                pos += vel * dt;

                const auto& acc = verts_accel[i];

                // Euler propagation
                vel += acc * dt;
            }
            else //RK4
            {
                // RK4 propagation
            }

            

            // "Floor" collision
            // TODO can apply surface friction based on time during tick spent on surface
            double floor_height = -1;
            if (pos.z() < floor_height)
            {
                pos.z() = floor_height;
                if (vel.z() < 0)
                    vel.z() = 0;
            }
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
        //TODO a different arg
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

    vector<SoftCube*> bodies;

    for (int i = -(ball_count / 2); i < ball_count - (ball_count / 2); i++)
    {
        bodies.push_back(new SoftCube);
        bodies.back()->translate_verts(Vector3d{ i * 3.0, 0.0, drop_height });
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
            for (const auto& vert_pos : body->verts_pos)
            {
                model_center += vert_pos / scale / body->verts_pos.size();
            }

            for (const auto& tri : cube_triangles)
            {
                auto v1 = get<0>(tri);
                auto v2 = get<1>(tri);
                auto v3 = get<2>(tri);
                auto scaled_pos1 = body->verts_pos[v1-1] / scale;
                auto scaled_pos2 = body->verts_pos[v2-1] / scale;
                auto scaled_pos3 = body->verts_pos[v3-1] / scale;
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
        accumulated_dt += dt_sim;
        sim_elapsed += dt_sim;
        prevTime = nowTime;

        if (++step_skip_itr == step_skip_amt)
        {
            step_skip_itr = 0;

            for (auto body : bodies)
            {
                body->step(accumulated_dt);
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
    glDeleteVertexArrays(1, &VAO_shot);
    glDeleteVertexArrays(1, &VAO_planet);
    //glDeleteBuffers(1, &boxbuffer);
    glDeleteBuffers(1, &ballbuffer);
    glDeleteBuffers(1, &shipbuffer);
    //glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}