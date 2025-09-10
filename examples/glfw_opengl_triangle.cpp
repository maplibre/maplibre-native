// GLFW + OpenGL Triangle Demo
// This shows what the WebGPU example would render if we had Dawn/wgpu integrated

#include <iostream>
#include <cmath>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 vertexColor;
void main() {
    gl_Position = vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vertexColor, 1.0);
}
)";

void errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int main() {
    std::cout << "GLFW + OpenGL Triangle Demo\n";
    std::cout << "============================\n";
    std::cout << "This demonstrates what WebGPU would render\n\n";
    
    glfwSetErrorCallback(errorCallback);
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    
    // OpenGL 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Triangle (WebGPU would look similar)", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync
    
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n\n";
    
    // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    // Create shader program
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Triangle vertices with colors
    float vertices[] = {
        // positions        // colors
         0.0f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // top (red)
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left (green)
         0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // bottom right (blue)
    };
    
    // Create vertex array and buffer
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    std::cout << "Rendering a colorful triangle...\n";
    std::cout << "This is what the WebGPU version would show with Dawn/wgpu.\n";
    std::cout << "Press ESC to exit.\n\n";
    
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        
        // Clear screen with dark blue
        glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Draw triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    std::cout << "Demo complete!\n";
    std::cout << "\nTo make WebGPU work like this, we need:\n";
    std::cout << "1. Link against Dawn or wgpu-native libraries\n";
    std::cout << "2. Replace OpenGL calls with WebGPU calls\n";
    std::cout << "3. Use WGSL shaders instead of GLSL\n";
    std::cout << "4. The infrastructure we built is ready for this!\n";
    
    return 0;
}