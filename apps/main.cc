/*
	Marching Cubes Implementation - Christian Ermann
	Version 1.0  May 30, 2020

	A simple implementation of marching cubes to be used as a reference for more complex implementations or
	implementations embedded within other algorithms.

	Based on the implementation at http://paulbourke.net/geometry/polygonise/.
*/

#include "marching_cubes/marching_cubes.h"
#include "marching_cubes/shader.h"

#include "glm/gtc/matrix_transform.hpp"

#include "SDL.h"
#include "GL/glew.h"
#include "SDL_opengl.h"

#include <iostream>
#include <vector>

// OpenGL window size
const GLint WIDTH = 512, HEIGHT = 512;

// Parameters for the marching cubes algorithm.
const glm::vec3 MIN = glm::vec3(-1.0f, -1.0f, -1.0f);
const glm::vec3 MAX = glm::vec3(1.0f, 1.0f, 1.0f);
const int RESOLUTION = 50;

/*
	Returns a vector representing the position of the camera as a function of time.
*/
glm::vec3 viewPos(float t)
{
	return glm::vec3(5.0f * glm::sin(t), 3.0f * glm::sin(t), 5.0f * glm::cos(t));
}

/*
 * Returns the combined model-view-projection matrix as a function of time.
 * A model matrix is not included yet.
 */
glm::mat4 mvp(float t)
{
	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 1.0f, 10.0f);
	glm::mat4 view = glm::lookAt(
		viewPos(t),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	return proj * view;
}

/*
	Takes care of rendering everything using SDL/OpenGL. Mostly boilerplate code.
*/
int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	SDL_Window *window = SDL_CreateWindow("Marching Cubes", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW!" << std::endl;
		return EXIT_FAILURE;
	}

	glViewport(0, 0, WIDTH, HEIGHT);

	glEnable(GL_DEPTH_TEST);

	Shader shader("marching_cubes.vs", "marching_cubes.fs");

    
    // Initialize sphere data.
    SphereSDF unitSphere(glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);
    std::vector<glm::vec3> sphereVertexBufferData;
    marchingCubes((SignedDistanceFunction*)&unitSphere, MIN, MAX, RESOLUTION,
            sphereVertexBufferData);

    // Initialize box data.
    BoxSDF box(glm::vec3(0.8f, 0.8f, 0.8f));
    std::vector<glm::vec3> boxVertexBufferData;
    marchingCubes((SignedDistanceFunction*)&box, MIN, MAX, RESOLUTION,
            boxVertexBufferData);

    // initialize torus data
    TorusSDF torus(glm::vec2(0.8f, 0.2f));
	std::vector<glm::vec3> torusVertexBufferData;
	marchingCubes((SignedDistanceFunction*)&torus, MIN, MAX, RESOLUTION,
            torusVertexBufferData);

    // Render sphere by default.
    std::vector<glm::vec3> vertexBufferData = sphereVertexBufferData;

    // Pass data to OpenGL.
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferData.size() * sizeof(glm::vec3), &vertexBufferData.front(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// Uncomment for wireframe view.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	SDL_Event windowEvent;
	while (true)
	{
		if (SDL_PollEvent(&windowEvent))
		{
			if (windowEvent.type == SDL_QUIT)
			{
				break;
			}
            else if (windowEvent.type == SDL_KEYDOWN)
            {
                auto keypress = windowEvent.key.keysym.sym;
                if (keypress == SDLK_ESCAPE)
                {
                    break;
                }
                else if (keypress == SDLK_1)
                {
                    vertexBufferData = sphereVertexBufferData;
	                glBindBuffer(GL_ARRAY_BUFFER, VBO);
	                glBufferData(GL_ARRAY_BUFFER,
                            vertexBufferData.size() * sizeof(glm::vec3),
                            &vertexBufferData.front(), GL_STATIC_DRAW);
                }
                else if (keypress == SDLK_2)
                {
                    vertexBufferData = boxVertexBufferData;
	                glBindBuffer(GL_ARRAY_BUFFER, VBO);
	                glBufferData(GL_ARRAY_BUFFER,
                            vertexBufferData.size() * sizeof(glm::vec3),
                            &vertexBufferData.front(), GL_STATIC_DRAW);
                }
                else if (keypress == SDLK_3)
                {
                    vertexBufferData = torusVertexBufferData;
	                glBindBuffer(GL_ARRAY_BUFFER, VBO);
	                glBufferData(GL_ARRAY_BUFFER,
                            vertexBufferData.size() * sizeof(glm::vec3),
                            &vertexBufferData.front(), GL_STATIC_DRAW);
                }
            }
		}

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();

		GLuint viewPosLocation = glGetUniformLocation(shader.program, "viewPos");
		glm::vec3 _viewPos = viewPos(SDL_GetTicks() / 1000.0f);
		glUniform3fv(viewPosLocation, 1, &_viewPos[0]);

		GLuint mvpLocation = glGetUniformLocation(shader.program, "mvp");
		glm::mat4 _mvp = mvp(SDL_GetTicks() / 1000.0f);
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &_mvp[0][0]);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertexBufferData.size());
		glBindVertexArray(0);

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
