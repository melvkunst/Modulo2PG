#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;

// Constantes para largura e altura da janela
const GLint WIDTH = 800, HEIGHT = 600;

// Vetor para armazenar os pontos clicados pelo usuário
vector<glm::vec2> points;

// Vetor para armazenar todos os vértices calculados (triângulos do polígono)
vector<GLfloat> allVertices;

// Coordenadas do centro do polígono
glm::vec2 center(0.0f, 0.0f);

// Indica se o polígono está pronto para ser desenhado
bool polygonReady = false;

// Função para calcular o polígono e criar triângulos
void calculatePolygon() {
    // Verifica se há pontos suficientes para formar um polígono
    if (points.size() < 3) return;

    // Calcula o centro do polígono somando todos os pontos
    center = glm::vec2(0.0f, 0.0f);
    for (const auto& point : points) {
        center += point;
    }
    center /= static_cast<float>(points.size()); // Divide pela quantidade de pontos

    // Cria triângulos conectando o centro e os pontos adjacentes
    for (size_t i = 0; i < points.size(); ++i) {
        size_t next = (i + 1) % points.size(); // Ponto seguinte (em loop)
        allVertices.insert(allVertices.end(), {
            // Coordenadas do centro, convertidas para o sistema de coordenadas normalizadas
            (center.x / WIDTH) * 2 - 1, -(center.y / HEIGHT) * 2 + 1, 0.0f,  1.0f, 0.5f, 0.0f, // Centro (cor laranja)
            // Coordenadas do ponto atual
            (points[i].x / WIDTH) * 2 - 1, -(points[i].y / HEIGHT) * 2 + 1, 0.0f,  1.0f, 0.5f, 0.0f, // Ponto atual (cor laranja)
            // Coordenadas do próximo ponto
            (points[next].x / WIDTH) * 2 - 1, -(points[next].y / HEIGHT) * 2 + 1, 0.0f,  1.0f, 0.5f, 0.0f  // Próximo ponto (cor laranja)
            });
    }

    points.clear(); // Reinicia os pontos para permitir a criação de um novo polígono
    polygonReady = true; // Define o polígono como pronto
}

// Função para capturar cliques do mouse
void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double mx, my;
        glfwGetCursorPos(window, &mx, &my);

        // Armazena o ponto clicado em coordenadas da janela
        points.emplace_back(mx, my);

        // Quando houver 3 ou mais pontos, calcula o polígono
        if (points.size() >= 3) {
            calculatePolygon();
        }
    }
}

int main() {
    // Inicializa a biblioteca GLFW
    if (!glfwInit()) {
        cerr << "Erro ao inicializar GLFW" << endl;
        return -1;
    }

    // Cria uma janela GLFW
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Modulo 2 - Luiz e Melissa", nullptr, nullptr);
    if (!window) {
        cerr << "Erro ao criar janela GLFW" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Inicializa o GLAD para carregar funções OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Erro ao inicializar GLAD" << endl;
        return -1;
    }

    // Registra a função de callback para cliques do mouse
    glfwSetMouseButtonCallback(window, mouseCallback);

    // Shaders para renderização
    const char* vertex_shader = R"(
        #version 410
        layout(location = 0) in vec3 vPosition;
        layout(location = 1) in vec3 vColor;
        out vec3 color;
        void main() {
            color = vColor;
            gl_Position = vec4(vPosition, 1.0);
        }
    )";

    const char* fragment_shader = R"(
        #version 410
        in vec3 color;
        out vec4 frag_color;
        void main() {
            frag_color = vec4(color, 1.0);
        }
    )";

    // Cria e compila os shaders
    int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, nullptr);
    glCompileShader(vs);

    int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, nullptr);
    glCompileShader(fs);

    // Linka os shaders no programa
    int shader_program = glCreateProgram();
    glAttachShader(shader_program, fs);
    glAttachShader(shader_program, vs);
    glLinkProgram(shader_program);

    // Cria buffers para os vértices
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Loop principal da aplicação
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Configura o fundo da janela
        glClearColor(0.4f, 0.65f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Desenha o polígono se houver vértices calculados
        if (!allVertices.empty()) {
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(GLfloat), allVertices.data(), GL_STATIC_DRAW);

            // Configura os atributos dos vértices (posição e cor)
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
            glEnableVertexAttribArray(1);

            // Usa o programa shader e desenha os triângulos
            glUseProgram(shader_program);
            glDrawArrays(GL_TRIANGLES, 0, allVertices.size() / 6);

            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
