#define GLEW_STATIC
#include <GL/glew.h>

#include <iostream>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include "Mesh.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <chrono>

GLuint shaderProgram;
GLuint texture_handle;

struct Vertex
{
    glm::vec3 m_pos;
    glm::vec2 m_tex;

    Vertex() {}

    Vertex(glm::vec3 pos, glm::vec2 tex)
    {
        m_pos = pos;
        m_tex = tex;
    }
};

int do_thingos(const aiScene* pScene, const std::string& filename) {
    aiMesh *meshy = pScene->mMeshes[0];
    aiString path;
    pScene->mMaterials[meshy->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL);
    sf::Image img_data;
    if (!img_data.loadFromFile(path.C_Str())) {
        std::cout << "Could not load " <<  path.C_Str() << std::endl;
    }
    glGenTextures(1, &texture_handle);
    glBindTexture(GL_TEXTURE_2D, texture_handle);
    glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA,
            img_data.getSize().x, img_data.getSize().y,
            0,
            GL_RGBA, GL_UNSIGNED_BYTE, img_data.getPixelsPtr()
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    std::vector<Vertex> vaortishes;
    std::vector<unsigned int> indexs;
    for (unsigned int i = 0; i < meshy->mNumVertices; ++i) {
        const aiVector3D &potato = meshy->mVertices[i];
        //const aiVector3D &n = meshy->mNormals[i];
        const aiVector3D &tc = meshy->HasTextureCoords(0) ? meshy->mTextureCoords[0][i] : aiVector3D(0,0,0);
        vaortishes.push_back(Vertex(glm::vec3(potato.x, potato.y, potato.z), glm::vec2(tc.x, tc.y)));
    }

    for (unsigned int i = 0; i < meshy->mNumFaces; ++i) {
        const aiFace & fase = meshy->mFaces[i];
        assert(fase.mNumIndices == 3);
        indexs.push_back(fase.mIndices[0]);
        indexs.push_back(fase.mIndices[1]);
        indexs.push_back(fase.mIndices[2]);
    }

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vaortishes.size(), &vaortishes[0], GL_STATIC_DRAW);
    //glBufferData(GL_ARRAY_BUFFER, vaortishes.size() * sizeof(glm::vec3), &vaortishes[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);                 // position
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)12); // texture coordinate
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    GLuint indexbufferthingo;
    glGenBuffers(1, &indexbufferthingo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbufferthingo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexs.size() * sizeof(unsigned int), &indexs[0], GL_STATIC_DRAW);
    return meshy->mNumFaces;
}

int assimpthingo(const std::string& filename) {
    int success;
    char infoLog[512];
    GLchar const* vssource =
            "#version 330 core\n"
            "layout (location = 0) in vec3 aPos;"
            "layout (location = 1) in vec2 texCoord;"
            ""
            "out vec3 ourColor;"
            "out vec2 texCoordV;"
            ""
            "uniform mat4 model;"
            "uniform mat4 view;"
            "uniform mat4 proj;"
            ""
            "void main() {"
            "   gl_Position = proj * view * model * vec4(aPos, 1.0);"
            "   texCoordV = texCoord;"
            "}";
    GLuint VS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VS, 1, &vssource, NULL);
    glCompileShader(VS);

    glGetShaderiv(VS, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(VS, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FIAILED\n" << infoLog << std::endl;
    }

    GLchar const* fssource =
            "#version 330 core\n"
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
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FIAILED\n" << infoLog << std::endl;
    }

    glAttachShader(shaderProgram, VS);
    glAttachShader(shaderProgram, FS);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FIAILED\n" << infoLog << std::endl;
    }
    glUseProgram(shaderProgram);
    glDeleteShader(VS);
    glDeleteShader(FS);


    Assimp::Importer importer;
    const aiScene* pScene = importer.ReadFile(filename.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    if (pScene) {
        return do_thingos(pScene, filename);
    } else {
        std::cout << "SOMETHING JBVADD HAPPENED!@!!@!!!!" << std::endl;
        return -1;
    }
}

void renderingThread(sf::Window* window) {
    window->setActive(true);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    auto t_start = std::chrono::high_resolution_clock::now();

    shaderProgram = glCreateProgram();
    int numFaces = assimpthingo("assets/tile1.fbx");

    /*glm::vec3 vertices[3];
    vertices[0] = glm::vec3(0.0f, 0.0f, 0.0f);
    vertices[1] = glm::vec3(0.5f, 0.0f, 0.0f);
    vertices[2] = glm::vec3(0.0f, 0.5f, 0.0f);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDisableVertexAttribArray(0);*/

    GLint uniTrans = glGetUniformLocation(shaderProgram, "model");
    glm::mat4 trans;
    auto t_now = std::chrono::high_resolution_clock::now();

    glm::mat4 view = glm::lookAt(
            glm::vec3(4.0f, 4.0f, 4.0f),
            glm::vec3(3.0f, 3.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
    );
    GLint uniView = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 proj = glm::perspective(glm::radians(40.0f), 800.0f / 600.0f, 1.0f, 10.0f);
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

    while (window->isOpen()) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        // Calculate transformation
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        trans = glm::rotate(
                trans,
                time * glm::radians(5.0f),
                glm::vec3(0.0f, 0.0f, 1.0f)
        );
        glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));
        glBindTexture(GL_TEXTURE_2D, texture_handle);
        glDrawElements(GL_TRIANGLES, numFaces * 3 * 3, GL_UNSIGNED_INT, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        /* Mesh x;
        x.LoadMesh("assets/tile1.fbx");
        x.Render(); */

        window->display();
        sf::sleep(sf::milliseconds(5));
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "OpenGL", sf::Style::Default, sf::ContextSettings(24));

    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    window.setActive(false);



    sf::Thread thread(&renderingThread, &window);
    thread.launch();

    bool running = true;
    while (running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                running = false;
            }
        }
    }

    window.close();
    return 0;
}
