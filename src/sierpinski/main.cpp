#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include <stack>
#ifdef _MSC_VER
#define _USE_MATH_DEFINES
#endif // _MSC_VER
#include <math.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#else
//#include <glad/glad.h>
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include "utils.h"

typedef GLfloat NodePoints[3][2];
typedef GLfloat (*NodePointsPtr)[2];

class Node
{
public:
    NodePoints point;
    int degree;
    int nlado;
};

GLuint positionAttributeLocation = 0;
GLuint VERTEX_ATTR_COLOR = 1;

static std::stack<Node *> nodes;
static int degree_current = 0;
static int triangle_numbers = 0;
static int line_idx0, line_idx, line_num;
static GLfloat line_len, line_rad, line_ang;
static GLboolean triangle_draw = GL_TRUE, toward = GL_TRUE;

// Un arreglo de 3 vectores que representan 3 vértices
//static const NodePoints g_vertex_buffer_data = {
// static const float g_vertex_buffer_data[] = {
//     -1.0f,
//     -1.0f,
//     1.0f,
//     -1.0f,
//     0.0f,
//     1.0f,
//     -1.0f,
//     -1.0f};

static const float g_vertex_buffer_data[] = {
    -0.5, 0.5, -0, 0.5, 0.0, 0.0, 0.5, 0.5, -0.5, 0.5};

static const GLfloat g_color_buffer_data[] =
    {
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        0.0,
};

GLFWwindow *window;

GLuint programObject;

// Identificar el vertex buffer, color draw triangle
// vertex, colores triangles
GLuint vertexbuffer[4];

GLuint VertexArrayID[4];

static void SetLine()
{
    NodePointsPtr points = nodes.top()->point;
    GLfloat x = points[line_idx][0] - points[line_idx0][0];
    GLfloat y = points[line_idx][1] - points[line_idx0][1];

    line_len = sqrt(points[line_idx][0]);
    line_ang = ((y != 0.0 || x >= 0.0)) ? asin(y / line_len) : M_PI;
    line_rad = 0;
    line_num = line_idx + 1;
}

static void SetDrawTriangle(const NodePoints points)
{
    Node *node = new Node();

    memcpy(node->point, points, sizeof(node->point));
    node->degree = degree_current;
    node->nlado = 0;
    nodes.push(node);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(node->point), points);
    line_idx = 1;
    line_idx0 = 0;
    SetLine();
}

static int Init()
{

    return GL_TRUE;
}

static int InitLevel()
{
    //GLMake(degree_current*3*sizeof(float))
    degree_current++;

    return GL_TRUE;
}

static int glInit()
{
    char vShaderStr[] =
        "#version 300 es                \n"
        "in vec4 vPosition;            \n"
        //"attribute vec3 color;        \n"
        //"out vec3 vColor;          \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = vPosition;  \n"
        //"   gl_PointSize = 5.0;                     \n"
        //"   vColor=color;"
        "}                            \n";

    char fShaderStr[] =
        "#version 300 es                              \n"
        "precision highp float;\n"
        //"varying vec3 vColor;          \n"
        "out vec4 outColor;           \n"
        "void main()                                  \n"
        "{                                            \n"
        //"  gl_FragColor = vec4 ( 0.0, 0.0, 1.0, 1.0 );\n"
        //"  gl_FragColor = vec4 (vColor, 1.0);         \n"
        "  outColor = vec4 ( 0.0, 0.0, 1.0, 1.0 );    \n"
        "}                                            \n";

    char vShaderStr2[] =
        "#version 300 es\n"
        "uniform int numVerts;\n"
        "uniform vec2 resolution;\n"
        "#define PI radians(180.0)\n"
        "void main() {\n"
        "float u = float(gl_VertexID) / float(numVerts);  // goes from 0 to 1\n"
        "float angle = u * PI * 2.0;                      // goes from 0 to 2PI\n"
        "float radius = 0.8;\n"
        "vec2 pos = vec2(cos(angle), sin(angle)) * radius;\n"
        "float aspect = resolution.y / resolution.x;\n"
        "vec2 scale = vec2(aspect, 1);\n"
        "gl_Position = vec4(pos * scale, 0, 1);\n"
        "gl_PointSize = 5.0;\n"
        "}";

    GLuint vertexShader;
    GLuint fragmentShader;
    GLint linked;

    vertexShader = gldr::LoadShader(GL_VERTEX_SHADER, vShaderStr);
    //vertexShader = gldr::LoadShader(GL_VERTEX_SHADER, vShaderStr2);
    fragmentShader = gldr::LoadShader(GL_FRAGMENT_SHADER, fShaderStr);

    programObject = glCreateProgram();
    if (programObject == 0)
        return 0;

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);
    glLinkProgram(programObject);
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1)
        {
            char *infoLog = static_cast<char *>(malloc(sizeof(char) * infoLen));
            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            printf("Error linking program:\n%s\n", infoLog);
            free(infoLog);
        }
        glDeleteProgram(programObject);

        return GL_FALSE;
    }

    positionAttributeLocation = glGetAttribLocation(programObject, "vPosition");
    //VERTEX_ATTR_COLOR=glGetAttribLocation(programObject, VERTEX_ATTR_COLOR, "color");

    // Generar un buffer, poner el resultado en el vertexbuffer que acabamos de crear
    glGenBuffers(1, vertexbuffer);

    // Los siguientes comandos le darán características especiales al 'vertexbuffer'
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);

    // Darle nuestros vértices a  OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glGenVertexArrays(1, VertexArrayID);
    glBindVertexArray(VertexArrayID[0]);

    glEnableVertexAttribArray(positionAttributeLocation);

    glVertexAttribPointer(
        positionAttributeLocation, // atributo 0. No hay razón particular para el 0, pero debe corresponder en el shader.
        2,                         // tamaño
        GL_FLOAT,                  // tipo
        GL_FALSE,                  // normalizado?
        0,                         // Paso
        (void *)0                  // desfase del buffer
    );

    // target flag is GL_ARRAY_BUFFER, and usage flag is GL_STREAM_DRAW because we will update vertices every frame.
    //glBufferData(GL_ARRAY_BUFFER, sizeof(NodePoints), 0, GL_STREAM_DRAW);
    //glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(g_vertex_buffer_data), g_vertex_buffer_data);

    // color
    /*
    glBindVertexArray(VertexArrayID[1]);
    // Los siguientes comandos le darán características especiales al 'vertexbuffer'
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
    // Darle nuestros vértices a  OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);
    */

    /*
    if (!InitLevel())
        return GL_FALSE;

    SetDrawTriangle(g_vertex_buffer_data);
    */

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    return GL_TRUE;
}

void main_loop()
{
    glViewport(0, 0, 640, 480);

    /*
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const int vertexIdLoc = glGetAttribLocation(programObject, "vertexId");
    const int numVertsLoc = glGetUniformLocation(programObject, "numVerts");
    const int resolutionLoc = glGetUniformLocation(programObject, "resolution");

    const int numVerts = 20;

    // draw
    //gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);

    glUseProgram(programObject);

    // tell the shader the number of verts
    glUniform1i(numVertsLoc, numVerts);
    // tell the shader the resolution
    glUniform2f(resolutionLoc, 2, 2);

    const int offset = 0;
    glDrawArrays(GL_LINES, offset, numVerts);
    */
    glClear(GL_COLOR_BUFFER_BIT);
    // 1rst attribute buffer : vértices
    //glEnableVertexAttribArray(positionAttributeLocation);
    glUseProgram(programObject);
    glBindVertexArray(VertexArrayID[0]);
    /*
    // 2rst attribute buffer : color
    glEnableVertexAttribArray(VERTEX_ATTR_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
    glVertexAttribPointer(VERTEX_ATTR_COLOR, 3, GL_FLOAT, GL_FALSE, 0, 0);
    */

    // Dibujar el triángulo !
    //glDrawArrays(GL_TRIANGLES, 0, 3); // Empezar desde el vértice 0S; 3 vértices en total -> 1 triángulo
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
    glDrawArrays(GL_LINES, 0, 4);
    //glDisableVertexAttribArray(VERTEX_ATTR_COORDS); // Swap front and back buffers
    //glDisableVertexAttribArray(VERTEX_ATTR_COLOR);  // Swap front and back buffers

    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
}

int main()
{
#ifndef __EMSCRIPTEN__
    const char *description;
#endif

    /* Initialize the library */
    if (!glfwInit())
    {
#ifndef __EMSCRIPTEN__
        glfwGetError(&description);
        printf("glfwInit error : %s,", description);
#else
        printf("glfwInit error,");
#endif

        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
#ifndef __EMSCRIPTEN__
        glfwGetError(&description);
        printf("glfwInit error : %s,", description);
#else
        printf("glfwInit error,");
#endif

        glfwTerminate();

        return EXIT_FAILURE;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

#ifndef __EMSCRIPTEN__
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

        glfwTerminate();

        return EXIT_FAILURE;
    }
#endif

    if (!Init())
        return EXIT_FAILURE;

    if (!glInit())
        return EXIT_FAILURE;

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
#else
    while (!glfwWindowShouldClose(window))
        main_loop();
#endif

    glfwTerminate();

    return EXIT_SUCCESS;
}