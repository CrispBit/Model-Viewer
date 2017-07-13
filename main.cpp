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
#include <chrono>

#include "Mesh/MeshContainer.h"
#include "Mesh/MeshShaders.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "meemerino", sf::Style::Default, sf::ContextSettings(24));
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    window.setActive(true);

    MeshShaders::init();
    MeshShaders::currentProgram = &MeshShaders::bonedMeshShaderProgram;
    glUseProgram(*MeshShaders::currentProgram);

    auto t_start = std::chrono::high_resolution_clock::now();

    MeshContainer meshes;
    BonedMesh tempObject;
    tempObject.loadMesh("assets/boblampclean.md5mesh");
    BonedMesh& object = meshes.put("bob", std::move(tempObject));

    GLint uniTrans = glGetUniformLocation(*MeshShaders::currentProgram, "model");
    glm::mat4 trans;
    auto t_now = std::chrono::high_resolution_clock::now();
    sf::Clock clock;

    glm::mat4 view = glm::lookAt(
            glm::vec3(4.0f, 4.0f, 4.0f),
            glm::vec3(3.0f, 3.0f, 3.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
    );
    view *= glm::scale(glm::vec3(.06f, .06f, .06f));
    GLint uniView = glGetUniformLocation(*MeshShaders::currentProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    float lookDeg = 100;

    glm::mat4 proj = glm::perspective(glm::radians(lookDeg), 800.0f / 600.0f, 1.0f, 10.0f);
    GLint uniProj = glGetUniformLocation(*MeshShaders::currentProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

    glClearColor(0.3f, 0.5f, 0.8f, 1.0f);

    bool running = true;
    bool active = true;
    while (running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            do {
                if (event.type == sf::Event::Closed) {
                    // end the program
                    running = false;
                } else if (event.type == sf::Event::Resized) {
                    // adjust the viewport when the window is resized
                    glViewport(0, 0, event.size.width, event.size.height);
                    proj = glm::perspective(glm::radians(lookDeg), (float) event.size.width / (float) event.size.height, 1.0f, 10.0f);
                    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
                } else if (event.type == sf::Event::LostFocus) {
                        active = false;
                } else if (event.type == sf::Event::GainedFocus) {
                    active = true;
                }
            } while (!active && window.waitEvent(event));
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        trans = glm::rotate(
                trans,
                time * glm::radians(2.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
        );
        glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

        std::vector<glm::mat4> Transforms;
        object.boneTransform(clock.getElapsedTime().asSeconds(), Transforms);
        for (unsigned int i = 0; i < Transforms.size(); ++i) {
            const std::string name = "gBones[" + std::to_string(i) + "]"; // every transform is for a different bone
            GLint boneTransform = glGetUniformLocation(MeshShaders::bonedMeshShaderProgram, name.c_str());
            Transforms[i] = glm::transpose(Transforms[i]);
            glUniformMatrix4fv(boneTransform, 1, GL_TRUE, glm::value_ptr(Transforms[i]));
        }

        object.draw();
        window.display();
    }

    window.setActive(false);
    window.close();
    return 0;
}
