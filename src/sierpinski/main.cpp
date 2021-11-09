#include <stdlib.h>
#include <cstdio>
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

// Un arreglo de 3 vectores que representan 3 vértices
static const GLfloat g_vertex_buffer_data[] = {
    -1.0f,
    -1.0f,
    0.0f,
    1.0f,
    -1.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
};

static const GLfloat g_color_buffer_data[]=
{
    1.0, 0.0, 1.0,
    1.0, 1.0, 0.0,
    1.0, 0.0, 1.0
};

GLFWwindow *window;

GLuint programObject;

// Identificar el vertex buffer
GLuint vertexbuffer[2];

static int Init()
{
    char vShaderStr[] =
        "attribute vec4 vPosition;    \n"
        "attribute vec3 color;        \n"
        "varying vec3 vColor;          \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = vPosition;  \n"
        "   vColor=color;"
        "}                            \n";

    char fShaderStr[] =
        "precision mediump float;\n"
        "varying vec3 vColor;          \n"
        "void main()                                  \n"
        "{                                            \n"
        //"  gl_FragColor = vec4 ( 0.0, 0.0, 1.0, 1.0 );\n"
        "  gl_FragColor = vec4 (vColor, 1.0);         \n"
        "}                                            \n";

    GLuint vertexShader;
    GLuint fragmentShader;
    GLint linked;

    vertexShader = gldr::LoadShader(GL_VERTEX_SHADER, vShaderStr);
    fragmentShader = gldr::LoadShader(GL_FRAGMENT_SHADER, fShaderStr);

    programObject = glCreateProgram();
    if (programObject == 0)
        return 0;

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);
    glBindAttribLocation(programObject, 0, "vPosition");
    glBindAttribLocation(programObject, 1, "color");
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

    GLuint VertexArrayID[2];
    glGenVertexArrays(2, VertexArrayID);
    glBindVertexArray(VertexArrayID[0]);

    // Generar un buffer, poner el resultado en el vertexbuffer que acabamos de crear
    glGenBuffers(2, vertexbuffer);
    // Los siguientes comandos le darán características especiales al 'vertexbuffer'
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
    // Darle nuestros vértices a  OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // color 
    glBindVertexArray(VertexArrayID[1]);
    // Los siguientes comandos le darán características especiales al 'vertexbuffer'
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
    // Darle nuestros vértices a  OpenGL.
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    return GL_TRUE;
}

void main_loop()
{
    // 1rst attribute buffer : vértices
    glEnableVertexAttribArray(0);
    glUseProgram(programObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[0]);
    glVertexAttribPointer(
        0,        // atributo 0. No hay razón particular para el 0, pero debe corresponder en el shader.
        3,        // tamaño
        GL_FLOAT, // tipo
        GL_FALSE, // normalizado?
        0,        // Paso
        (void *)0 // desfase del buffer
    );
     // 2rst attribute buffer : color
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Dibujar el triángulo !
    glDrawArrays(GL_TRIANGLES, 0, 3); // Empezar desde el vértice 0S; 3 vértices en total -> 1 triángulo
    glDisableVertexAttribArray(0);    /* Swap front and back buffers */
    glDisableVertexAttribArray(1);    /* Swap front and back buffers */
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

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
#else
    while (!glfwWindowShouldClose(window))
        main_loop();
#endif

    glfwTerminate();

    return EXIT_SUCCESS;
}