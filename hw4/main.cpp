#define GLEW_STATIC
#include <gl/glew.h>
#include <glfw/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>

GLFWwindow* initilizeWindowContext() {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	// specify opengl 3.2
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// only new core funcionality
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "yaroslav_berejnoi_HW4", 0, 0); // Windowed
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

void phongOne(GLuint shaderProgram) {
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));

	GLint uniTrans = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(model));

	glm::mat4 view = glm::lookAt(
		glm::vec3(10.0f, 10.0f, 10.0f), //camera position
		glm::vec3(0.0f, 0.0f, 0.0f), // point centered on screen
		glm::vec3(0.0f, 1.0f, 0.0f) // view up
	);
	GLint uniView = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 proj = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 1.0f, 100.0f);
	GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
}

void phongTwo(GLuint shaderProgram) {
	glm::mat4 model;
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));

	GLint uniTrans = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(model));

	glm::mat4 view = glm::lookAt(
		glm::vec3(3.0f, 3.0f, 3.0f), //camera position
		glm::vec3(0.0f, 0.0f, 0.0f), // point centered on screen
		glm::vec3(0.0f, 1.0f, 0.0f) // view up
	);
	GLint uniView = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 proj = glm::perspective(glm::radians(70.0f), 800.0f / 600.0f, 1.0f, 100.0f);
	GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
}


int main() {
	glfwInit();

	GLFWwindow* window = initilizeWindowContext();
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	// force to check for modern functions
	glewInit();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Create Vertex Array Object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create a Vertex Buffer Object and copy the vertex data to it
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	std::ifstream infile("Tris1.txt");
	int tris;

	infile >> tris;
	int numberVertices = tris * 10 * 3;

	GLfloat* vertices = new GLfloat[numberVertices];

	GLfloat number;

	int i = 0;
	while (infile >> number) {
		vertices[i] = number;
		i++;
	}

	glBufferData(GL_ARRAY_BUFFER, numberVertices * 4, vertices, GL_STATIC_DRAW);
	//Vertex data is upload once and drawn many times (e.g the world).
	//GL_STATIC_DRAW determines the type of memory to use for efficiency

	char *vertexSource;
	char *fragmentSource;
	vertexSource =
		"#version 330 core \n"
		"in vec3 position;"
		"in vec3 normal;"
		"in vec4 objectColor;"
		"out vec3 ObjectColor;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 proj;"

		//make a new light
		"void makeNewLight(in vec3 color, in vec3 direction, in float diffuseIntensity, in float ambientIntensity, in float specularIntensity, in float specularPower, out vec3 calculatedColor){"
		"float cosTheta = dot(normal, direction);"
		"vec3 materialDiffuseColor = color;" //diffuse, this should be the original color of the teapot (white) but this looks close to the expected output
		"vec3 materialAmbientColor = vec3(ambientIntensity,ambientIntensity,ambientIntensity) * materialDiffuseColor;" //ambient

		//specular
		"vec3 eye = normalize(normal);"
		"vec3 reflection = reflect(-direction, normal);"
		"float cosAlpha = clamp(dot(eye, reflection), 0, 1);"
		"vec3 specularCalculation = materialDiffuseColor * color * specularIntensity * pow(cosAlpha,specularPower*2);"

		"calculatedColor =  materialAmbientColor + materialDiffuseColor * color * diffuseIntensity  * cosTheta  + specularCalculation;}"

		//main
		"void main(){"

		"vec3 redColor = vec3(1, 0.1, 0.1);"
		"vec3 redDirection = vec3(0, 1, 0);"

		"vec3 greenColor = vec3(0.1, 1.0, 0.1);"
		"vec3 greenDirection = vec3(1, 0, 0);"

		"vec3 blueColor = vec3(0.1, 0.1, 1.0);"
		"vec3 blueDirection = vec3(0, 0, 1);"

		"vec3 firstLight;"
		"makeNewLight(redColor, redDirection, 0.3, 0.1, 0.6, 3.0, firstLight);"

		"vec3 secondLight;"
		"makeNewLight(greenColor, greenDirection, 0.3, 0.1, 0.6, 3.0, secondLight);"

		"vec3 thirdLight;"
		"makeNewLight(blueColor, blueDirection, 0.3, 0.1, 0.6, 3.0, thirdLight);"

		"ObjectColor = firstLight +secondLight + thirdLight;"
	
		"gl_Position = proj * view * model * vec4(position, 1.0); }"; // vertex shader

	fragmentSource =
		"#version 330 core \n"
		"in vec3 ObjectColor;"
		"out vec3 outColor;"
		"void main() {outColor = vec3(ObjectColor);}"; // fragment shader

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
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), 0);
	// input, # of values for the input, type of each component, normalized, stride, offset
	// stride: how many bytes between each position attribute in array. 0 means no data in between
	// offset: how many bytes from the start the attribute occurs

	GLint normAttrib = glGetAttribLocation(shaderProgram, "normal");
	glEnableVertexAttribArray(normAttrib);
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	GLint colAttrib = glGetAttribLocation(shaderProgram, "objectColor");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

	phongOne(shaderProgram);

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// closed loop event: only handle events when you need to
	while (!glfwWindowShouldClose(window)) {

		glfwSwapBuffers(window);
		//swap back buffer and front buffer after drawing has finished
		glfwPollEvents();
		// retrieve window events

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, numberVertices);

		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
			phongOne(shaderProgram);
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			phongTwo(shaderProgram);
		}
	}
	glfwTerminate();
}