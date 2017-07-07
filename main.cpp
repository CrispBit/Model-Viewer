#define GLEW_STATIC
#include <GL/glew.h>

#include <iostream>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <chrono>

#include "Mesh.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "meemerino", sf::Style::Default, sf::ContextSettings(24));
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    window.setActive(true);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    int success;
    char infoLog[512];

    const GLuint shaderProgram = glCreateProgram();
    GLchar const* vssource =
            "#version 130\n"
                    "in vec3 aPos;"
                    "in vec2 texCoord;"
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

    glAttachShader(shaderProgram, VS);
    glAttachShader(shaderProgram, FS);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    glUseProgram(shaderProgram);
    glDeleteShader(VS);
    glDeleteShader(FS);

    auto t_start = std::chrono::high_resolution_clock::now();

    Mesh object;
    object.loadMesh("assets/tile2.fbx");

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

    bool running = true;
    while (running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                // end the program
                running = false;
            } else if (event.type == sf::Event::Resized) {
                // adjust the viewport when the window is resized
                glViewport(0, 0, event.size.width, event.size.height);
            }
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        trans = glm::rotate(
                trans,
                time * glm::radians(.15f),
                glm::vec3(0.0f, 0.0f, 1.0f)
        );
        glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

        object.draw();
        window.display();
    }

    window.close();
    return 0;
}
