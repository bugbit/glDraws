#pragma once

namespace gldr
{
    inline GLuint LoadShader(GLenum type, const char *shaderSrc)
    {
        GLuint shader;
        GLint compiled;

        shader = glCreateShader(type);
        if (shader == 0)
            return 0;

        glShaderSource(shader, 1, &shaderSrc, NULL);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1)
            {
                char *infoLog = static_cast<char *>(malloc(sizeof(char) * infoLen));
                glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
                printf("Error compiling shader:\n%s\n", infoLog);
                free(infoLog);
            }
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }
}