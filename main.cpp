#define GLEW_STATIC
#include <GL/glew.h>

#include <iostream>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include "Mesh.h"

void renderingThread(sf::Window* window) {
    window->setActive(true);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    int success;
    char infoLog[512];

    const GLuint shaderProgram = glCreateProgram();
    GLchar const* vssource =
            "#version 300 es\n"
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
        std::cout << "rip shader comperino 1\n" << infoLog << std::endl;
    }

    GLchar const* fssource =
            "#version 300 es\n"
                    "out mediump vec4 FragColor;"
                    ""
                    "in mediump vec2 texCoordV;"
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
        std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FIAILED\n" << infoLog << std::endl;
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


    while (window->isOpen()) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glEnableVertexAttribArray(0);
        //glEnableVertexAttribArray(1);
        // Calculate transformation
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        trans = glm::rotate(
                trans,
                time * glm::radians(5.0f),
                glm::vec3(0.0f, 0.0f, 1.0f)
        );
        glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));
        //glBindTexture(GL_TEXTURE_2D, texture_handle);
        //glDrawElements(GL_TRIANGLES, numFaces * 3 * 3, GL_UNSIGNED_INT, 0);
        //glDisableVertexAttribArray(0);
        //glDisableVertexAttribArray(1);

        //object.draw();
        /* Mesh x;
        x.LoadMesh("assets/tile2.fbx");
        x.Render(); */

        object.render();
        object.draw();

        window->display();
        sf::sleep(sf::milliseconds(5));
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "meemerino", sf::Style::Default, sf::ContextSettings(24));

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
