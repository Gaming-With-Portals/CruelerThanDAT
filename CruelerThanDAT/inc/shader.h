#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <filesystem>

class ShaderManager {
public:
    // Access the one and only instance
    static ShaderManager& Instance() {
        static ShaderManager instance;
        return instance;
    }

    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;
    ShaderManager(ShaderManager&&) = delete;
    ShaderManager& operator=(ShaderManager&&) = delete;

    unsigned int defaultShader = 0;
    unsigned int decalShader = 0;
    unsigned int skinShader = 0;
    unsigned int stageShader = 0;
    unsigned int outlineShader = 0;
private:

    std::string loadShaderSource(const char* filePath) {
        std::ifstream shaderFile(filePath);
        std::stringstream buffer;
        buffer << shaderFile.rdbuf();
        return buffer.str();
    }

    unsigned int compileShader(GLenum type, const std::string& source) {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, NULL);
        glCompileShader(shader);

        // Check for errors
        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            CTDLog::Log::getInstance().LogError("Failed to compile shader:");
            CTDLog::Log::getInstance().LogError(infoLog);
            return 0;
        }

        return shader;
    }

    unsigned int createShaderProgram(const char* vertexPath, const char* fragmentPath) {
        std::string vertexCode = loadShaderSource(vertexPath);
        std::string fragmentCode = loadShaderSource(fragmentPath);

        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode);

        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        GLint success;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            CTDLog::Log::getInstance().LogError("Failed to link shader program:");
            CTDLog::Log::getInstance().LogError(infoLog);
            return 0;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return shaderProgram;
    }

    ShaderManager() {
        defaultShader = createShaderProgram("Assets/System/OGL/basic.vert", "Assets/System/OGL/basic.frag");
        decalShader = createShaderProgram("Assets/System/OGL/decal.vert", "Assets/System/OGL/decal.frag");
        skinShader = createShaderProgram("Assets/System/OGL/skin.vert", "Assets/System/OGL/skin.frag");
        stageShader = createShaderProgram("Assets/System/OGL/stage.vert", "Assets/System/OGL/stage.frag");
        outlineShader = createShaderProgram("Assets/System/OGL/highlight.vert", "Assets/System/OGL/highlight.frag");
        return;
    }
 
    ~ShaderManager() = default;
};