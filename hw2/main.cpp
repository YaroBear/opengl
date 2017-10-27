#define GLEW_STATIC
#include <gl/glew.h>
#include <glfw/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

GLFWwindow* initilizeWindowContext() {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	// specify opengl 3.2
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// only new core funcionality
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "yaroslav_berejnoi_HW2", 0, 0); // Windowed
	return window;
}

void logShaderRenderStatus(GLuint id) {
	GLint status;
	glGetShaderiv(id, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE) {
		printf("Shader: %i compiled\n", id);
	}
	else {
		char buffer[512];
		glGetShaderInfoLog(id, 512, NULL, buffer);
		printf(buffer);
	}
};

int main() {
	glfwInit();

	GLFWwindow* window = initilizeWindowContext();
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	// force to check for modern functions
	glewInit();

	// Create Vertex Array Object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a Vertex Buffer Object and copy the vertex data to it
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);


	float squareVert[] = {
		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // Top-left
		0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Top-right
		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom-right

		0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom-right
		-0.5f, -0.5f, 1.0f, 1.0f, 1.0f, // Bottom-left
		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f  // Top-left
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(squareVert), squareVert, GL_STATIC_DRAW);
	//Vertex data is upload once and drawn many times (e.g the world).
	//GL_STATIC_DRAW determines the type of memory to use for efficiency

	char *vertexSource;
	char *fragmentSource;
	vertexSource = "#version 150 \n in vec2 position; in vec3 color; out vec3 Color; void main(){ Color = color; gl_Position = vec4(position, 0.0, 1.0); }"; // vertex shader
	fragmentSource = "#version 150 \n in vec3 Color; out vec4 outColor; void main() {outColor = vec4(Color, 1.0);}"; // fragment shader

	
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	logShaderRenderStatus(vertexShader);
	
	
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	logShaderRenderStatus(fragmentShader);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram); //only one active at a time, just like a vertex buffer

	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
	// input, # of values for the input, type of each component, normalized, stride, offset
	// stride: how many bytes between each position attribute in array. 0 means no data in between
	// offset: how many bytes from the start the attribute occurs
	GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));


	// closed loop event: only handle events when you need to
	while (!glfwWindowShouldClose(window)) {
		glfwSwapBuffers(window);
		//swap back buffer and front buffer after drawing has finished
		glfwPollEvents();
		// retrieve window events
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	glfwTerminate();

}