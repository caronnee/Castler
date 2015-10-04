// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <source/shader.hpp>
#include <source/texture.hpp>
#include <source/controls.hpp>
#include <source/objloader.hpp>
#include <source/vboindexer.hpp>

glm::vec3 GetSpecular()
{
  if (IsEnabled(Key_S))
    return glm::vec3(0,0,0);
  return glm::vec3(0.3,0.3,0.3);
}

glm::vec3 GetAmbient()
{
  if (IsEnabled(Key_A))
    return glm::vec3(0,0,0);
  return glm::vec3(0.1,0.1,0.1);
}

glm::vec3 GetDiffuseMask()
{
  if (IsEnabled(Key_D))
    return glm::vec3(0,0,0);
  return glm::vec3(1,1,1);
}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);


	// Open a window and create its OpenGL context
	window = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "Tutorial 08 - Basic Shading", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(window, WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "shaders/StandardShading.vertexshader", "shaders/StandardShading.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

	// Get a handle for our buffers
	GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
	GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
  GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");
  
  GLuint ambientID = glGetUniformLocation(programID, "ambient");
  GLuint specularID = glGetUniformLocation(programID, "specular");
  GLuint lightPowerID = glGetUniformLocation(programID, "LightPower");
	GLuint diffuseMaskID = glGetUniformLocation(programID, "diffuseMask");

	// Load the texture
  //GLuint Texture = loadDDS("uvmap.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

	bool res = loadOBJ("c:\\Work\\DP1\\pgr\\Models\\bushes\\01\\bush_01.obj", vertices, uvs, normals);
  GLuint Texture = 0;//loadBMP_custom("vt2.bmp");
  {
    glm::vec3 BbCenterMn = vertices[0], BbCenterMx = vertices[0];
    for ( int i =1; i <  vertices.size(); i++)
    { 
      if ( BbCenterMx[0] < vertices[i][0] )
        BbCenterMx[0] = vertices[i][0];
      if ( BbCenterMx[1] < vertices[i][1] )
        BbCenterMx[1] = vertices[i][1];
      if ( BbCenterMx[2] < vertices[i][2] )
        BbCenterMx[2] = vertices[i][2];

      if ( BbCenterMn[0] > vertices[i][0] )
        BbCenterMn[0] = vertices[i][0];
      if ( BbCenterMn[1] > vertices[i][1] )
        BbCenterMn[1] = vertices[i][1];
      if ( BbCenterMn[2] > vertices[i][2] )
        BbCenterMn[2] = vertices[i][2];
    }
    glm::vec3 oCenter = (BbCenterMn + BbCenterMx)*0.5f;
    SetCenter(oCenter);
    glm::vec4 camPos = getModelMatrix() * glm::vec4(BbCenterMn,1);
    glm::vec4 camDir = getModelMatrix() * (vec4(oCenter,1) - camPos);
    camDir = glm::normalize(camDir);
    SetCameraPosition( glm::vec3(camPos.x,camPos.y,camPos.z) - glm::vec3( camDir.x,camDir.y,camDir.z) *14.0f, glm::vec3( camDir.x,camDir.y,camDir.z));
  }
	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
  GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
  GLuint CorrectNormalID = glGetUniformLocation(programID, "CorrectNormal");
	GLuint NormalScaledMID = glGetUniformLocation(programID, "NormalScaledM");
  
  //uniform bool SwitchNormal;
  //uniform mat4 NormalScaledM;

	do{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

    // first thing - compute matrices
    computeMatricesFromInputs();

		// Compute the MVP matrix from keyboard and mouse input
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
    glm::mat4 ModelMatrix = getModelMatrix();
    glm::mat4 ModelNormalMatrix = getModelNormalMatrix( ModelMatrix );    
    glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

    // Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
    glUniformMatrix4fv(NormalScaledMID, 1, GL_FALSE, &ModelNormalMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
    glUniform1i(CorrectNormalID,IsEnabled(Key_N) ? 1:0);

    //glm::vec3 lightPos = glm::vec3(4,4,4);
		glm::vec3 lightPos = GetLightPos();
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

    glUniform1f(lightPowerID, GetPower());
    // setting uniforms
    glm::vec3 col = GetAmbient();
    glUniform3f(ambientID, col.x, col.y,col.z);
    col = GetDiffuseMask();
    glUniform3f(diffuseMaskID,col.x, col.y,col.z);

    col = GetSpecular();
    glUniform3f(specularID,col.x, col.y,col.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(vertexPosition_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			vertexPosition_modelspaceID,  // The attribute we want to configure
			3,                            // size
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(vertexUVID);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			vertexUVID,                   // The attribute we want to configure
			2,                            // size : U+V => 2
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(vertexNormal_modelspaceID);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			vertexNormal_modelspaceID,    // The attribute we want to configure
			3,                            // size
			GL_FLOAT,                     // type
			GL_FALSE,                     // normalized?
			0,                            // stride
			(void*)0                      // array buffer offset
		);

		// Draw the triangles !
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() );

		glDisableVertexAttribArray(vertexPosition_modelspaceID);
		glDisableVertexAttribArray(vertexUVID);
		glDisableVertexAttribArray(vertexNormal_modelspaceID);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

