#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>
#include "SkyBox.hpp"

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
glm::mat4 modelBulb;
glm::mat4 modelBulb2;
glm::mat4 modelBulb3;
GLuint modelLoc;
GLuint modelLocBulb;
GLuint modelLocBulb2;
GLuint modelLocBulb3;
glm::mat4 view;
GLuint viewLoc;
GLuint viewLocBulb;
GLuint viewLocBulb2;
GLuint viewLocBulb3;
glm::mat4 projection;
GLuint projectionLoc;
GLuint projectionLocBulb;
GLuint projectionLocBulb2;
GLuint projectionLocBulb3;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
GLuint normalMatrixLoc1;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;



glm::vec3 ball(0.59597f, 0.12688f ,0.83744f);
//glm::vec3 tail(0.87799f, 0.2801f, 0.31136f);

glm::vec3 bulb(0.82458f, 1.4362f, -0.75341f);
glm::vec3 bulb2(-0.25307f, 0.58215f, -1.5573f);
glm::vec3 bulb3(-1.9157f, 3.0268f, -1.0758f);
GLuint bulbLoc;
GLuint bulbLoc2;
GLuint bulbLoc3;

gps::Camera myCamera(
				glm::vec3(3.2357f, 0.32853f, 0.2953f), //camera position
				glm::vec3(2.9914f, 0.32853f, 0.2953f),  //camera target
				glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.1f;

bool pressedKeys[1024];
float angleY = 0.0f;
float tailAngle = 0.0f;
GLfloat lightAngle;

gps::Model3D nanosuit;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D scene;
gps::Model3D ball_o;
gps::Model3D tail_o;
gps::Model3D bulb1_o;
gps::Model3D bulb2_o;
gps::Model3D bulb3_o;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader ballShader;
gps::Shader bulbShader;
gps::Shader bulbShader2;
gps::Shader bulbShader3;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

double mvm = 0.0f;
float lastX = 400.0f;
float lastY = 300.0f;
float yaw = -90.0f;
float pitch = 0.0f;

int fogInit = 1;
GLfloat fogDensity = 0.01f;

float var = 0.02f;
float speed = 1.0f;
float var_tail = 1.2f;
float speed_tail = 1.0f;


GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.5f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_5]) {
		mvm += 0.05f;
	}

	if (pressedKeys[GLFW_KEY_6]) {
		mvm -= 0.05f;
	}
	if (pressedKeys[GLFW_KEY_F]) {
		fogInit = -1 * fogInit;
	}

	if (pressedKeys[GLFW_KEY_G]) {
		fogDensity = glm::min(fogDensity + 0.005f, 1.0f);
	}
	
	if (pressedKeys[GLFW_KEY_H]) {
		fogDensity = glm::max(fogDensity - 0.005f, 0.0f);
	}

	if (pressedKeys[GLFW_KEY_P]) {
		tailAngle+= 2.2f;
		std::cout << tailAngle << std::endl;
	}

	if (pressedKeys[GLFW_KEY_O]) {
		tailAngle -= 2.2f;
		std::cout << tailAngle << std::endl;
	}

	// Solid
	if (glfwGetKey(glWindow, GLFW_KEY_Z)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// Wireframe
	if (glfwGetKey(glWindow, GLFW_KEY_X)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	// Point
	if (glfwGetKey(glWindow, GLFW_KEY_C)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

}


bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	scene.LoadModel("objects/scene/scene.obj", "objects/scene/");
	ground.LoadModel("objects/ground/ground.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	tail_o.LoadModel("objects/scene/tail.obj", "objects/scene/");
	ball_o.LoadModel("objects/scene/untitled.obj", "objects/scene/");
	bulb1_o.LoadModel("objects/scene/bulb1.obj", "objects/scene/");
	bulb2_o.LoadModel("objects/scene/bulb2.obj", "objects/scene/");
	bulb3_o.LoadModel("objects/scene/bulb3.obj", "objects/scene/");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();

	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();

	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();

	//ballShader.loadShader("shaders/ball.vert", "shaders/ball.frag");


	skyboxShader.loadShader("shaders/skyBox.vert", "shaders/skyBox.frag");
	skyboxShader.useShaderProgram();

	bulbShader.loadShader("shaders/shaderBulb.vert", "shaders/shaderBulb.frag");
	bulbShader.useShaderProgram();

	bulbShader2.loadShader("shaders/shaderBulb2.vert", "shaders/shaderBulb.frag");
	bulbShader2.useShaderProgram();

	bulbShader3.loadShader("shaders/shaderBulb3.vert", "shaders/shaderBulb.frag");
	bulbShader3.useShaderProgram();


}

void initUniforms() {
	myCustomShader.useShaderProgram();

	//model1 = glm::rotate(glm::mat4(1.0f), glm::radians(0), glm::vec3(0.0f, 1.0f, 0.0f));

	model = glm::mat4(1.0f);
	
	bulbLoc = glGetUniformLocation(myCustomShader.shaderProgram, "bec");
	glUniform3fv(bulbLoc, 1, glm::value_ptr(bulb));

	bulbLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "bec2");
	glUniform3fv(bulbLoc2, 1, glm::value_ptr(bulb2));

	bulbLoc3 = glGetUniformLocation(myCustomShader.shaderProgram, "bec3");
	glUniform3fv(bulbLoc3, 1, glm::value_ptr(bulb3));
	 
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);	
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(35.544f, 56.56f, -60.048f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	//lightColor = glm::vec3(0.349f, 0.349f, 0.349f); //gray light
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	bulbShader.useShaderProgram();
	modelBulb = glm::mat4(1.0f);
	modelLocBulb = glGetUniformLocation(bulbShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLocBulb, 1, GL_FALSE, glm::value_ptr(modelBulb));

	viewLocBulb = glGetUniformLocation(bulbShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLocBulb, 1, GL_FALSE, glm::value_ptr(view));

	projectionLocBulb = glGetUniformLocation(bulbShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLocBulb, 1, GL_FALSE, glm::value_ptr(projection));


	bulbShader2.useShaderProgram();
	modelBulb2 = glm::mat4(1.0f);
	modelLocBulb2 = glGetUniformLocation(bulbShader2.shaderProgram, "model");
	glUniformMatrix4fv(modelLocBulb, 1, GL_FALSE, glm::value_ptr(modelBulb2));

	viewLocBulb2 = glGetUniformLocation(bulbShader2.shaderProgram, "view");
	glUniformMatrix4fv(viewLocBulb, 1, GL_FALSE, glm::value_ptr(view));

	projectionLocBulb2 = glGetUniformLocation(bulbShader2.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLocBulb2, 1, GL_FALSE, glm::value_ptr(projection));


	bulbShader3.useShaderProgram();
	modelBulb3 = glm::mat4(1.0f);
	modelLocBulb3 = glGetUniformLocation(bulbShader3.shaderProgram, "model");
	glUniformMatrix4fv(modelLocBulb3, 1, GL_FALSE, glm::value_ptr(modelBulb));

	viewLocBulb3 = glGetUniformLocation(bulbShader3.shaderProgram, "view");
	glUniformMatrix4fv(viewLocBulb3, 1, GL_FALSE, glm::value_ptr(view));

	projectionLocBulb3 = glGetUniformLocation(bulbShader3.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLocBulb3, 1, GL_FALSE, glm::value_ptr(projection));
	
}

void initFBO() {
	// Create the FBO, the depth texture and attach the depth texture to the FBO
	GLuint shadowMapFBO;
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);
	GLuint depthMapTexture;

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	const GLfloat near_plane = 0.1f, far_plane = 800.0f;
	float vari = 800.0f;
	glm::mat4 lightProjection = glm::ortho(-vari, vari, -vari, vari, near_plane, far_plane);

	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

void minge() {
	if (mvm > 0.3f or mvm < 0.0f)
	{
		var = -var;
	}

	mvm += var;
}

void tail_function() {
	if (tailAngle > 11.0f or tailAngle < -19.8f)
	{
		var_tail = -var_tail;
	}
	tailAngle += var_tail;
}

void drawObjects(gps::Shader shader, bool depthPass) {
		
	shader.useShaderProgram();
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
    if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	//ground.Draw(shader);
	scene.Draw(shader);

	minge();
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, mvm, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	ball_o.Draw(shader);

	tail_function();
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.87799f, 0.2801f, 0.32763f));
	model = glm::rotate(model, glm::radians(tailAngle), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.87799f, -0.2801f, -0.32763f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	tail_o.Draw(shader);

	bulbShader.useShaderProgram();
	glUniformMatrix4fv(viewLocBulb, 1, GL_FALSE, glm::value_ptr(view));
	bulb1_o.Draw(bulbShader);

	bulbShader2.useShaderProgram();
	glUniformMatrix4fv(viewLocBulb2, 1, GL_FALSE, glm::value_ptr(view));
	bulb2_o.Draw(bulbShader2);

	bulbShader3.useShaderProgram();
	glUniformMatrix4fv(viewLocBulb3, 1, GL_FALSE, glm::value_ptr(view));
	bulb3_o.Draw(bulbShader3);
}



void renderScene() {
	myCustomShader.useShaderProgram();

	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "fogInit"), fogInit);
	glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);

	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	drawObjects(depthMapShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, retina_width, retina_height);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	myCustomShader.useShaderProgram();

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	

	drawObjects(myCustomShader, false);

	mySkyBox.Draw(skyboxShader, view, projection);
}
void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

void initSkyBox() {
	faces.push_back("skybox/right.tga");
	faces.push_back("skybox/left.tga");
	faces.push_back("skybox/top.tga");
	faces.push_back("skybox/bottom.tga");
	faces.push_back("skybox/back.tga");
	faces.push_back("skybox/front.tga");

	mySkyBox.Load(faces);

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkyBox();
	

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();		

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
