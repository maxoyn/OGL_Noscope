#include "stdafx.h"

#include <GL\glew.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <gl\GLU.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <iostream>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include "GeometryNode.h"
#include "GroupNode.h"
#include "TransformNode.h"

#include <math.h>

bool init();
bool initGL();
void render();
void close();

GLuint CreateCrosshair(GLuint&);
void DrawCross(GLuint id);
bool crosshair = true;
GLuint CreateTri(GLuint&, GLuint&);
void DrawTri(GLuint id);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

Shader gShader;
Shader gShader2;
Shader gShader3;

GLuint gVAO, gVBO, gEBO, gVAO2;
GLuint texID;

Mix_Chunk *firesound;

glm::vec3 cPos(0.0f, 3.5f, 8.0f);
Camera camera(cPos);

float lastX = -1;
float lastY = -1;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
double prevrect = 0;
double bestTime = 10;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

//statics
unsigned int Node::genID;
glm::mat4 TransformNode::transformMatrix = glm::mat4(1.0f);

void CreateScene();
GroupNode* gRoot;
TransformNode* trSuit;
Model mAwp;

void SpawnBehind(TransformNode* tr, glm::vec3 camerapos);
//int xPos = 10;       // used to swap the model place
float nAngle = -3.14; // used to angle the model 
int xmult = 1;		//change between -1 and 1 to swap translate model around player

bool shoot = false;
int tries = 0;

//event handlers
void HandleKeyDown(const SDL_KeyboardEvent& key);
void HandleMouseMotion(const SDL_MouseMotionEvent& motion);
void HandleMouseWheel(const SDL_MouseWheelEvent& wheel);
void HandleMouseButtonUp(const SDL_MouseButtonEvent& button, float f1, float f2);
void HandleMouseButtonDown(const SDL_MouseButtonEvent& button, float f1);

//raycasting
void ScreenPosToWorldRay(
	int mouseX, int mouseY,             // Mouse position, in pixels, in window coordinates
	int viewportWidth, int viewportHeight,  // Viewport size, in pixels
	glm::mat4 ViewMatrix,               // Camera position and orientation
	glm::mat4 ProjectionMatrix,         // Camera parameters (ratio, field of view, near and far planes)
	glm::vec3& out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
);

int main(int argc, char* args[])
{
	init();

	CreateScene();

	SDL_Event e;
	//While application is running
	bool quit = false;
	bool running = true;
	while (!quit)
	{
		// per-frame time logic
		// --------------------
		float currentFrame = SDL_GetTicks() / 1000.0f;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		int cticks = SDL_GetTicks();
		shoot = false;
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			switch (e.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q)
				{
					bestTime = round(bestTime * 100) / 100;
					string endresult;
					if (bestTime < 10 && tries > 1) {
						endresult = std::to_string(bestTime);
						endresult.append(" Seconds");
					}
					else {
						endresult = "none.";
					}
					SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Your best reaction:", endresult.c_str(), gWindow);
					quit = true;
				}
				else
				{
					HandleKeyDown(e.key);
				}
				break;
			case SDL_MOUSEMOTION:
				HandleMouseMotion(e.motion);
				break;
			case SDL_MOUSEBUTTONDOWN:
				HandleMouseButtonDown(e.button, currentFrame);
				break;
			case SDL_MOUSEBUTTONUP:
				HandleMouseButtonUp(e.button, currentFrame, prevrect);
				break;
			}
		}
		//Render
		render();
		
		//Update screen
		SDL_GL_SwapWindow(gWindow);
	}

	close();
	return 0;
}

void HandleKeyDown(const SDL_KeyboardEvent& key)
{
	switch (key.keysym.sym)
	{
	case SDLK_w:
		camera.ProcessKeyboard(FORWARD, deltaTime);
		break;
	case SDLK_s:
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		break;
	case SDLK_a:
		camera.ProcessKeyboard(LEFT, deltaTime);
		break;
	case SDLK_d:
		camera.ProcessKeyboard(RIGHT, deltaTime);
		break;
	case SDLK_SPACE:
		camera.ProcessKeyboard(JUMP, deltaTime);
		camera.jumping = true;
		break;
	case SDLK_h:
		if (crosshair) {
			crosshair = false;
		}
		else
			crosshair = true;
		break;
	case SDLK_g: //test
		glm::vec3 asd = trSuit->getTranslation();
		//camera.Front = asd;
		std::cout << "CAMERA: " << camera.Position.x << " " << camera.Position.y << " " << camera.Position.z << std::endl;
		std::cout << "Model: " << asd.x << " " << asd.y << " " << asd.z << std::endl;
		break;
	}
}

void HandleMouseMotion(const SDL_MouseMotionEvent& motion)
{
	if (firstMouse)
	{
		//lastX = motion.x;
		//lastY = motion.y;
		lastX = 1366 / 2;
		lastY = 768 / 2;
		SDL_WarpMouseInWindow(gWindow, 1366 / 2, 768 / 2);
		firstMouse = false;	}
	else
	{
		camera.ProcessMouseMovement(motion.x - lastX, lastY - motion.y);
		lastX = motion.x;
		lastY = motion.y;
	}
}

void HandleMouseWheel(const SDL_MouseWheelEvent& wheel)
{
	camera.ProcessMouseScroll(wheel.y);
}

void HandleMouseButtonDown(const SDL_MouseButtonEvent& button, float f1) {
	if (button.button == SDL_BUTTON_LEFT) {
		shoot = true;
		Mix_PlayChannel(0, firesound, 0);
	}
}
void HandleMouseButtonUp(const SDL_MouseButtonEvent& button, float f1, float f2) //f1 = current frame, f2 = frame from previous click
{
	if (button.clicks == 1 && button.button == SDL_BUTTON_LEFT)
	{
		int width, height;
		SDL_GetWindowSize(gWindow, &width, &height);

		glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), 4.0f / 3.0f, 0.1f, 100.0f);
		glm::vec3 rayOrigin, rayDirection;

		//ScreenPosToWorldRay(button.x, button.y, width, height, camera.GetViewMatrix(), proj, rayDirection);
		ScreenPosToWorldRay(1366/2, 768/2, width, height, camera.GetViewMatrix(), proj, rayDirection);
		//printf("\nRay origin: %f %f %f\nRay direction: %f %f %f", rayOrigin.x, rayOrigin.y, rayOrigin.z, rayDirection.x, rayDirection.y, rayDirection.z);

		std::vector<Intersection*> hits;
		gRoot->TraverseIntersection(camera.Position, rayDirection, hits);
		
		//read center screen pixel:
		glFlush();
		glFinish();
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		for (unsigned int i = 0; i < hits.size(); i++)
		{
			/*printf("\nIntersected: %s", hits[i]->intersectedNode->GetName().c_str());
			for (unsigned int k = 0; k < hits[i]->Route.size(); k++) {
				printf(hits[i]->Route[k]->GetName().c_str());
			}*/

			unsigned char data[4];
			//glReadPixels(button.x, button.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glReadPixels(1366 / 2, 768 / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
			int pickedID =
				data[0] +
				data[1] * 256 +
				data[2] * 256 * 256;

			//std::cout << std::endl;
			//std::cout << "picked: " << pickedID << std::endl;


			if (pickedID != 1280) { //3349504 blue
				if (f1 - f2 < bestTime) {
					bestTime = f1 - f2;
				}
				std::cout << std::endl;
				std::cout << "reaction: " << f1 - f2 << std::endl;
				std::cout << "BEST: " << bestTime << std::endl;

				SpawnBehind(trSuit, camera.Position);

				prevrect = f1; // between
			}
			delete hits[i];
		}
		hits.clear();
	}
}

void SpawnBehind(TransformNode* tr, glm::vec3 camerapos) {
	tries++;
	//xPos *= -1;
	nAngle *= -1;
	xmult *= -1;
	//trSuit->SetTranslation(glm::vec3(xPos, 2.0f, 8.0f));
	camera.negate();
	camera.Position = cPos;
	//trSuit->SetTranslation(glm::vec3(camera.Position.x + (xmult * 10.0f), 2.0f, camera.Position.z));
	//trSuit->SetRotation(nAngle / 2, glm::vec3(0, 1, 0));
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//mixer and load sound
		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
			std::cout << "ERROR: " << Mix_GetError() << std::endl;
		}
		firesound = Mix_LoadWAV("awp.wav");
		firesound->volume = 1;

		//Use OpenGL 3.3
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

		//Create window
		gWindow = SDL_CreateWindow("360 Noscopes", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1366, 768,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN*/);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create context
			gContext = SDL_GL_CreateContext(gWindow);
			if (gContext == NULL)
			{
				printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Use Vsync
				if (SDL_GL_SetSwapInterval(1) < 0)
				{
					printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
				}

				//Initialize OpenGL
				if (!initGL())
				{
					printf("Unable to initialize OpenGL!\n");
					success = false;
				}
			}
		}
		//"simulate" mouse
		SDL_SetWindowGrab(gWindow, SDL_TRUE);
		SDL_ShowCursor(SDL_DISABLE);
	}
	return success;
}

bool initGL()
{
	bool success = true;
	GLenum error = GL_NO_ERROR;
	
	glewInit();

	error = glGetError();
	if (error != GL_NO_ERROR)
	{
		success = false;
		printf("Error initializing OpenGL! %s\n", gluErrorString(error));
	}
	
	//glClearColor(0.00f, 0.11f, 0.2f, 1.0f);
	glClearColor(0.00f, 0.02f, 0.00f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gShader.Load("./shaders/vertex.vert", "./shaders/fragment.frag");

	gShader2.Load("./shaders/vertex2.vert", "./shaders/frag2.frag");

	gShader3.Load("./shaders/vertex2.vert", "./shaders/frag1.frag");

	mAwp.LoadModel("models/dragon lore/AWP_Dragon_Lore.obj");

	gVAO = CreateCrosshair(gVBO);

	if (!LoadTexture("./images/flash2.png", texID))
	{
		printf("Could not load texture \"flash2.png\"");
	}
	gShader3.setInt("Texture1", 0);
	gVAO2 = CreateTri(gVBO, gEBO);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //other modes GL_FILL, GL_POINT
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	return success;
}

void CreateScene()
{
	gRoot = new GroupNode("root");
	trSuit = new TransformNode("Nanosuit Transform");
	trSuit->SetTranslation(glm::vec3(camera.Position.x + (xmult * 10.0f), 0.0f, 8.0f));
	trSuit->SetRotation(nAngle / 2, glm::vec3(0, 1, 0));
	trSuit->SetScale(glm::vec3(0.21f, 0.21f, 0.21f));
	GeometryNode *nanosuit = new GeometryNode("nanosuit");
	nanosuit->LoadFromFile("models/nanosuit/nanosuit.obj");
	nanosuit->SetShader(&gShader);
	trSuit->AddChild(nanosuit);
	gRoot->AddChild(trSuit);
}

void close()
{
	//delete GL programs, buffers and objects
	glDeleteProgram(gShader.ID);
	glDeleteProgram(gShader2.ID);
	glDeleteProgram(gShader3.ID);
	glDeleteVertexArrays(1, &gVAO);
	glDeleteVertexArrays(1, &gVAO2);
	glDeleteBuffers(1, &gVBO);
	glDeleteBuffers(1, &gEBO);

	texID = NULL;
	mAwp = NULL;
	gRoot = NULL;
	//Delete OGL context
	SDL_GL_DeleteContext(gContext);
	//Destroy window	
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	Mix_FreeChunk(firesound);
	firesound = NULL;
	//Quit SDL subsystems
	Mix_Quit();
	SDL_Quit();
}


void render()
{
	//Clear color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom), 16.0f / 9.0f, 0.1f, 100.0f);

	glUseProgram(gShader.ID);
	gShader.setMat4("view", view);
	gShader.setMat4("proj", proj);

	//lighting
	//gShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
	//gShader.setVec3("light.position", lightPos);
	//gShader.setVec3("viewPos", camera.Position);
	gShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
	gShader.setVec3("light.position", camera.Position);
	gShader.setVec3("light.direction", camera.Front);
	gShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
	gShader.setVec3("viewPos", camera.Position);

	gRoot->Traverse();

	glm::mat4 model = glm::mat4(1.0f);
	glUseProgram(gShader2.ID);
	//gShader2.setMat4("view", view);
	//gShader2.setMat4("proj", proj);
	//gShader2.setVec3("viewPos", camera.Position);

	if (crosshair) {
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		gShader2.setMat4("model", model);
		DrawCross(gVAO);
	}

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.6f, -1.2f, 0.0f));
	model = glm::rotate(model, -0.6f, glm::vec3(0, 1, 0));
	model = glm::rotate(model, -0.25f, glm::vec3(1, 0, 0));
	model = glm::rotate(model, 0.1f, glm::vec3(0, 0, 1));
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.01f));
	gShader2.setMat4("model", model);
	
	mAwp.Draw(gShader2);

	if (shoot) {
		glUseProgram(gShader3.ID);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.1f, -0.18f, 0.5f));
		model = glm::rotate(model, 3.14f, glm::vec3(0, 1, 0));
		model = glm::rotate(model, 0.60f, glm::vec3(0, 0, 1));
		gShader3.setMat4("model", model);
		DrawTri(gVAO2);
	}
	
	if (camera.Position.y > 3.5f && camera.jumping) {
		camera.Position.y -= 5 * deltaTime;
	}
	else if (camera.Position.y != 3.5) {
		camera.Position.y = 3.5;
		camera.jumping = false;
	}
}

GLuint CreateCrosshair(GLuint& VBO) {
	float vertices[] = {
		//bottom
		-0.02f, -0.05f, 0.0f, //left
		0.02f, -0.05f, 0.0f, //right
		0.0f, -0.02f, 0.0f, //center
		//top
		-0.02f, 0.05f, 0.0f,  //left
		0.02f, 0.05f, 0.0f, //right
		0.0f, 0.02f, 0.0f, //center
		//left
		-0.02f, 0.0f, 0.0f, //center
		-0.05f, -0.02f, 0.0f, //bottom
		-0.05f, 0.02f, 0.0f, //top
		//right
		0.05f, -0.02f, 0.0f, //bottom
		0.02f, 0.0f, 0.0f, //center
		0.05f, 0.02f, 0.0f, //top
	};

	GLuint VAO;

	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //the data comes from the currently bound GL_ARRAY_BUFFER
	glEnableVertexAttribArray(0);

	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);
	
	return VAO;
}

void DrawCross(GLuint vaoID)
{
	glBindVertexArray(vaoID);
	//glDrawElements uses the indices in the EBO to get to the vertices in the VBO
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glBindVertexArray(0);
}

GLuint CreateTri(GLuint& VBO, GLuint& EBO) {
	float vertices[] = {
		//pos               //colors             //tex
		0.05f, 0.05f, 0.0f, 1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
		0.05f, -0.05f, 0.0f, 0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
		-0.05f, -0.05f, 0.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
		-0.05f, 0.05f, 0.0f, 1.0f, 1.0f, 0.0f,   0.0f, 1.0f
	};
	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	GLuint VAO;
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//we have to change the stride to 6 floats, as each vertex now has 6 attribute values
	//the last value (pointer) is still 0, as the position values start from the beginning
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); //the data comes from the currently bound GL_ARRAY_BUFFER
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);
	//the elements buffer must be unbound after the vertex array otherwise the vertex array will not have an associated elements buffer array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return VAO;
}

void DrawTri(GLuint vaoID)
{
	glBindVertexArray(vaoID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
