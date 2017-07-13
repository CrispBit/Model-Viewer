#include "MeshShaders.h"
#include <iostream>

GLuint MeshShaders::staticMeshShaderProgram;
GLuint MeshShaders::bonedMeshShaderProgram; 
GLuint* MeshShaders::currentProgram = nullptr;

void MeshShaders::createStaticShader() {
    staticMeshShaderProgram = glCreateProgram();

    int success;
    char infoLog[512];

    GLchar const* vssource =
            "#version 130\n"
            "in vec3 aPos;"
            "in vec2 texCoord;"
            "in vec3 aNormal;"
            ""
            "out vec2 texCoordV;"
            ""
            "uniform mat4 model;"
            "uniform mat4 view;"
            "uniform mat4 proj;"
            ""
            "void main() {"
            "   vec4 posL = vec4(aPos, 1.0);"
            "   gl_Position = proj * view * model * posL;"
            "   texCoordV = texCoord;"
            "}";
    GLuint VS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VS, 1, &vssource, NULL);
    glCompileShader(VS);

    glGetShaderiv(VS, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(VS, 512, NULL, infoLog);
        std::cout << "rip shader comperino 1\n" << infoLog << std::endl;
    }

    GLchar const* fssource =
            "#version 130\n"
                    "out vec4 FragColor;"
                    ""
                    "in vec2 texCoordV;"
                    ""
                    "uniform sampler2D ourTexture;"
                    ""
                    "void main() {"
                    "   FragColor = texture(ourTexture, texCoordV);"
                    "}";
    GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FS, 1, &fssource, NULL);
    glCompileShader(FS);

    glGetShaderiv(FS, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(FS, 512, NULL, infoLog);
        std::cout << "rip shader comperino 2\n" << infoLog << std::endl;
    }

    glAttachShader(staticMeshShaderProgram, VS);
    glAttachShader(staticMeshShaderProgram, FS);

    glBindAttribLocation(staticMeshShaderProgram, 0, "aPos");
    glBindAttribLocation(staticMeshShaderProgram, 1, "texCoord");
    glBindAttribLocation(staticMeshShaderProgram, 2, "aNormal");

    glLinkProgram(staticMeshShaderProgram);

    glGetProgramiv(staticMeshShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(staticMeshShaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(VS);
    glDeleteShader(FS);
}

void MeshShaders::createBonedShader() {
    bonedMeshShaderProgram = glCreateProgram();

    int success;
    char infoLog[512];

    GLchar const* vssource =
            "#version 130\n"
            "in vec3 aPos;"
            "in vec2 texCoord;"
            "in vec3 aNormal;"
            "in ivec4 boneIDs;"
            "in ivec4 eBoneIDs;"
            "in vec4 weights;"
            "in vec4 eWeights;"
            ""
            "out vec2 texCoordV;"
            ""
            "const int MAX_BONES = 8;"
            "const int MAX_MESHES = 5;"
            ""
            "uniform mat4 model;"
            "uniform mat4 view;"
            "uniform mat4 proj;"
            "uniform mat4 gBones[MAX_MESHES * MAX_BONES];"
            ""
            "void main() {"
            "   mat4 boneTransform = gBones[boneIDs[0]] * weights[0];"
            "   boneTransform += gBones[boneIDs[1]] * weights[1];"
            "   boneTransform += gBones[boneIDs[2]] * weights[2];"
            "   boneTransform += gBones[boneIDs[3]] * weights[3];"
            "   boneTransform += gBones[eBoneIDs[0]] * eWeights[0];"
            "   boneTransform += gBones[eBoneIDs[1]] * eWeights[1];"
            "   boneTransform += gBones[eBoneIDs[2]] * eWeights[2];"
            "   boneTransform += gBones[eBoneIDs[3]] * eWeights[3];"
            "   vec4 posL = boneTransform * vec4(aPos, 1.0);"
            "   gl_Position = proj * view * model * posL;"
            "   texCoordV = texCoord;"
            "}";
    GLuint VS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VS, 1, &vssource, NULL);
    glCompileShader(VS);

    glGetShaderiv(VS, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(VS, 512, NULL, infoLog);
        std::cout << "rip shader comperino 1\n" << infoLog << std::endl;
    }

    GLchar const* fssource =
            "#version 130\n"
                    "out vec4 FragColor;"
                    ""
                    "in vec2 texCoordV;"
                    ""
                    "uniform sampler2D ourTexture;"
                    ""
                    "void main() {"
                    "   FragColor = texture(ourTexture, texCoordV);"
                    "}";
    GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FS, 1, &fssource, NULL);
    glCompileShader(FS);

    glGetShaderiv(FS, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(FS, 512, NULL, infoLog);
        std::cout << "rip shader comperino 2\n" << infoLog << std::endl;
    }

    glAttachShader(bonedMeshShaderProgram, VS);
    glAttachShader(bonedMeshShaderProgram, FS);

    glBindAttribLocation(bonedMeshShaderProgram, 0, "aPos");
    glBindAttribLocation(bonedMeshShaderProgram, 1, "texCoord");
    glBindAttribLocation(bonedMeshShaderProgram, 2, "aNormal");
    glBindAttribLocation(bonedMeshShaderProgram, 3, "boneIDs");
    glBindAttribLocation(bonedMeshShaderProgram, 4, "eBoneIDs");
    glBindAttribLocation(bonedMeshShaderProgram, 5, "weights");
    glBindAttribLocation(bonedMeshShaderProgram, 6, "eWeights");

    glLinkProgram(bonedMeshShaderProgram);

    glGetProgramiv(bonedMeshShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(staticMeshShaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(VS);
    glDeleteShader(FS);
}

void MeshShaders::init() {
    createStaticShader();
    createBonedShader();
    staticMeshShaderProgram = glCreateProgram();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
