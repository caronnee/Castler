// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* GWindow;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <source/shader.hpp>
#include <source/texture.hpp>
#include <source/controls.hpp>
#include <source/objloader.hpp>
#include <source/vboindexer.hpp>

// TODO error when 
void ReportError(const char * message )
{
  fprintf( stderr, message );
}

#include <ShObjIdl.h>

static char * appDir = NULL;

std::string GetFullPath(const char * rel)
{
  if (appDir == NULL)
  {
    appDir = new char[256];
    GetModuleFileName(NULL,appDir,256);
    char * c = strrchr(appDir,'\\');
    *c = '\0';
  }
  char cc[256];
  GetCurrentDirectory(256,cc);
  SetCurrentDirectory(appDir);
  char in[256];
  _fullpath(in,rel,256);
  SetCurrentDirectory(cc);
  std::string ret(in);
  return in;
};

bool Init()
{
  // Initialise GLFW
  if( !glfwInit() )
  {
    ReportError("Failed to initialize GLFW");
    return false;
  }

  GWindow = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "Model creator", NULL, NULL);

  // Open a window and create its OpenGL context
  if( GWindow == NULL ){
    ReportError( "Failed to open GLFW window." );
    return false;
  }
  glfwMakeContextCurrent(GWindow);

  // Initialize GLEW
  if (glewInit() != GLEW_OK) {
    ReportError("Failed to initialize GLEW");
    return false;
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(GWindow, GLFW_STICKY_KEYS, GL_TRUE);
  glfwSetCursorPos(GWindow, WINDOW_WIDTH/2, WINDOW_HEIGHT/2);

  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS); 

  // Cull triangles which normal is not towards the camera
  glEnable(GL_CULL_FACE);

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  
  return true;
}

void ShutDown()
{
  // deinitialize all
  glfwTerminate();
}

#include "source/tiny_obj_loader.h"

class Model
{
  std::vector<glm::vec3> _vertices;
  std::vector<glm::vec3> _normals;
  glm::vec4 _centerMin, _centerMax,_center;
  std::vector<int> _indices;

public:
  void Load(const char * name)
  {
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> mats;
    std::string s = GetFullPath(name);
    std::string matfolder = s.substr(0,s.rfind('\\')+1);
    std::string err = tinyobj::LoadObj(shapes,mats,s.c_str(), matfolder.c_str());
    if (!err.empty())
    {
      ReportError(err.c_str());
      return;
    }
    
    // first mesh only
    for ( size_t i =0; i< shapes[0].mesh.positions.size(); i+=3 )
      _vertices.push_back( glm::vec3(shapes[0].mesh.positions[i],
                                     shapes[0].mesh.positions[i+1],
                                     shapes[0].mesh.positions[i+2]) );

    for ( size_t i =0; i< shapes[0].mesh.indices.size(); i++ )
    {
      _indices.push_back( shapes[0].mesh.indices[i] );
    }
    for ( size_t i =0; i< shapes[0].mesh.indices.size(); i+=3 )
    {
      glm::vec3 & a1 = _vertices[shapes[0].mesh.indices[i]];
      glm::vec3 & a2 = _vertices[shapes[0].mesh.indices[i]];
      glm::vec3 & a3 = _vertices[shapes[0].mesh.indices[i]];
      _normals.push_back( glm::cross(a2-a1,a3-a1));
    }
    // center of the model
    glm::vec3 BbCenterMn = _vertices[0], BbCenterMx = _vertices[0];
    for ( size_t i =1; i <  _vertices.size(); i++)
    { 
      if ( BbCenterMx[0] < _vertices[i][0] )
        BbCenterMx[0] = _vertices[i][0];
      if ( BbCenterMx[1] < _vertices[i][1] )
        BbCenterMx[1] = _vertices[i][1];
      if ( BbCenterMx[2] < _vertices[i][2] )
        BbCenterMx[2] = _vertices[i][2];

      if ( BbCenterMn[0] > _vertices[i][0] )
        BbCenterMn[0] = _vertices[i][0];
      if ( BbCenterMn[1] > _vertices[i][1] )
        BbCenterMn[1] = _vertices[i][1];
      if ( BbCenterMn[2] > _vertices[i][2] )
        BbCenterMn[2] = _vertices[i][2];
    }
    _centerMin = glm::vec4( BbCenterMn,1);
    _centerMax = glm::vec4(BbCenterMx,1);
    _center = glm::vec4 ((BbCenterMn + BbCenterMx)*0.5f,1);
  }

  glm::vec4 Center() const
  {
    return _center;
  }

  int NNormals()
  {
    return _normals.size();
  }

  int NVertices()
  {
    return _vertices.size();
  }

  glm::vec3 * Vertices() 
  {
    return &_vertices[0];
  }

  glm::vec3 * Normals() 
  {
    return &_normals[0];
  }
};

class Engine
{
  glm::vec4 camPos, camDir;
  GLuint programID;

  GLuint MatrixID;
  GLuint ViewMatrixID;
  GLuint ModelMatrixID;

  // Get a handle for our buffers
  GLuint vertexPosition_modelspaceID;
  GLuint vertexUVID;
  GLuint vertexNormal_modelspaceID;
  GLuint lightPowerID;
  GLuint vertexbuffer;
  GLuint normalbuffer;
  GLuint LightID;
  GLuint NormalScaledMID;

  int _nvertices;
public:
  void SwitchToModel(Model & model)
  {
    camPos = getModelMatrix() * model.Center();
    camDir = getModelMatrix() * ( model.Center() - camPos);
    camDir = glm::normalize(camDir);
    
    SetCameraPosition( glm::vec3(camPos.x,camPos.y,camPos.z) - glm::vec3( camDir.x,camDir.y,camDir.z) *14.0f, glm::vec3( camDir.x,camDir.y,camDir.z));

    SetCenter(model.Center());

    // Load it into a VBO
    
    glBufferData(GL_ARRAY_BUFFER, model.NVertices() * sizeof(glm::vec3), model.Vertices(), GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, model.NNormals() * sizeof(glm::vec3), model.Normals(), GL_STATIC_DRAW);
    _nvertices = model.NVertices();
  }

  void Draw()
  {
    
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

    //glm::vec3 lightPos = glm::vec3(4,4,4);
    glm::vec3 lightPos = GetLightPos();
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

    glUniform1f(lightPowerID, GetPower());

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
    glDrawArrays(GL_TRIANGLES, 0, _nvertices );

    glDisableVertexAttribArray(vertexPosition_modelspaceID);
    glDisableVertexAttribArray(vertexNormal_modelspaceID);

  }

  void Init()
  {
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "shaders/StandardShading.vertexshader", "shaders/StandardShading.fragmentshader" );

    glUseProgram(programID);

    // Get a handle for our "MVP" uniform
    MatrixID = glGetUniformLocation(programID, "MVP");
    ViewMatrixID = glGetUniformLocation(programID, "V");
    ModelMatrixID = glGetUniformLocation(programID, "M");

    // Get a handle for our buffers
    vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
    vertexUVID = glGetAttribLocation(programID, "vertexUV");
    vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");

    lightPowerID = glGetUniformLocation(programID, "LightPower");

    glGenBuffers(1, &normalbuffer);
    glGenBuffers(1, &vertexbuffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);

    LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    NormalScaledMID = glGetUniformLocation(programID, "NormalScaledM");
  }

  void ShutDown()
  {
    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteProgram(programID);
  }
};

int main( void )
{
  if ( !Init())
  {
    return -1;
  }

  Engine engine;
  Model m;
  m.Load("..\\..\\Models\\Cube\\cube.obj");
 
  engine.Init();
  engine.SwitchToModel(m);

  // Get a handle for our "LightPosition" uniform
	do{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    engine.Draw();

		// Swap buffers
		glfwSwapBuffers(GWindow);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(GWindow, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(GWindow) == 0 );

  engine.ShutDown();
  ShutDown();
  return 0;
}

