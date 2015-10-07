// Include GLFW
#include <glfw3.h>
extern GLFWwindow* GWindow; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

float power = 50;

float GetPower()
{
  return power;
}

// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 0, 0.5 ); 
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
float horizontalAngleO = 3.14f;
float horizontalAngleL = 3.14f;

// Initial vertical angle : none
float verticalAngle = 0.0f;
float verticalAngleO = 0.0f;
float verticalAngleL = 0.0f;

// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;


void SetCameraPosition(glm::vec3 newCamPos, glm::vec3 newCamDir )
{
  position = newCamPos;

  //
  /*cos(verticalAngleL) * sin(horizontalAngleL), 
    sin(verticalAngleL),
    cos(verticalAngleL) * cos(horizontalAngleL)
*/
  newCamDir = glm::normalize(newCamDir);
  verticalAngle = verticalAngleO = asin(newCamDir.y);
  if (newCamDir.y == 0)
  {
    horizontalAngle = horizontalAngleO = acos(newCamDir.z);
  }
  else
  {
    horizontalAngle = horizontalAngleO = asin( newCamDir.x/newCamDir.y);
  }
}

struct KeyPressed
{
  int key;
  bool isPressed;
  bool enabled;
  KeyPressed( int kkey ): key(kkey),isPressed(false), enabled(false){}
};

KeyPressed keys[Keys] = { GLFW_KEY_C, GLFW_KEY_A,GLFW_KEY_S,GLFW_KEY_D,GLFW_KEY_L,GLFW_KEY_P,GLFW_KEY_R,GLFW_KEY_N,GLFW_KEY_T,GLFW_KEY_KP_ADD,GLFW_KEY_KP_SUBTRACT, GLFW_KEY_B, GLFW_KEY_M, GLFW_KEY_O, GLFW_KEY_I, GLFW_KEY_K };

glm::vec3 lightPos = glm::vec3(0.5f,2,2);
glm::vec3 aroundPoint = glm::vec3(0,0,0);

void SetLightPos(glm::vec3 lp)
{
  lightPos = lp;
}

glm::vec3 GetObjectPos()
{
  return aroundPoint;
}

glm::vec3 GetLightDir()
{
  // Direction : Spherical coordinates to Cartesian coordinates conversion
  glm::vec3 direction(
    cos(verticalAngleL) * sin(horizontalAngleL), 
    sin(verticalAngleL),
    cos(verticalAngleL) * cos(horizontalAngleL)
    );
  return direction;
}

glm::vec3 GetLightStrafe()
{
  // Right vector
  glm::vec3 right = glm::vec3(
    sin(horizontalAngleL - 3.14f/2.0f), 
    0,
    cos(horizontalAngleL - 3.14f/2.0f)
    );
  return right;
}

glm::vec3 lightFocusedPos = glm::vec3(0,0,0);

glm::vec3 GetLightObjectPos()
{
  return lightFocusedPos;
}

glm::vec3 GetLightPos()
{
  if ( IsEnabled(Key_M) )
    return lightPos;

  glm::vec3 direction = GetLightDir();
  glm::vec3 right = GetLightStrafe();

  // Up vector
  glm::vec3 up = glm::cross( right, direction );

  float len = glm::length (GetLightObjectPos() - lightPos);
  //calculate new light position, rotate around our object we light
  return GetLightObjectPos() - direction * len;
}

bool GetAround()
{
  return keys[Key_C].enabled;
}

float totalAngle = 0;
float GetAngle()
{
  return totalAngle;
}

static vec3 startScaleVec = glm::vec3(0.1f,0.1f,0.1f);
static vec3 scaleRange = glm::vec3( 0.08f, -0.02f, 0.0f); 
static vec3 scaleVec = startScaleVec;

static glm::vec2 lastPos(WINDOW_WIDTH/2,WINDOW_HEIGHT/2);

float HandlePressed()
{
  bool CEnabled = GetAround();

  for ( int i =0; i < Keys; i++)
  {
    if( (glfwGetKey(GWindow, keys[i].key ) == GLFW_PRESS) && !keys[i].isPressed )
    {
      keys[i].isPressed = true;
      keys[i].enabled = !keys[i].enabled;
    }
    if( (glfwGetKey(GWindow, keys[i].key ) == GLFW_RELEASE) )
    {
      keys[i].isPressed = false;
    }
  }

  if ( CEnabled != GetAround() )
  {
    // switch to original camera position
    if ( CEnabled )
    {
      horizontalAngle = horizontalAngleO;
      verticalAngle = verticalAngleO;
    }
    else
    {
      // save the current position, now we will move with camera
      horizontalAngleO = horizontalAngle;
      verticalAngleO = verticalAngle;

      horizontalAngle = 3.14f;
      verticalAngle = 0.0f;
    }
  }

  // handle dependent on delta time - mouse move
  // glfwGetTime is called only once, the first time this function is called
  static double lastTime = glfwGetTime();

  // Get mouse position
  double xpos, ypos;
  glfwGetCursorPos(GWindow, &xpos, &ypos);

  // Compute time difference between current and last frame
  double currentTime = glfwGetTime();
  float deltaTime = float(currentTime - lastTime);

  //if ( IsEnabled(Key_T))
  //  totalAngle += deltaTime * 3.14f;

  static float slowDown = 0.1f;
  if ( IsEnabled(Key_P))
  {
    static float scaleTime = 0.0f;
    scaleTime += deltaTime*slowDown;
    if ( scaleTime > 1.0 )
    {
      //going down
      scaleTime = 2-scaleTime;
      slowDown = -0.1f;
    }
    if ( scaleTime < 0 )
    {
      //going up
      scaleTime = 0;
      slowDown = 0.1f;
    }
    scaleVec = startScaleVec + scaleRange * scaleTime;
  }

  if ( IsEnabled(Key_R))
  {
    horizontalAngleL += mouseSpeed * float( lastPos.x - xpos );
    verticalAngleL   += mouseSpeed * float( lastPos.y - ypos );
  }
  else
  {
    // Compute new orientation
    horizontalAngle += mouseSpeed * float( lastPos.x - xpos );
    verticalAngle   += mouseSpeed * float( lastPos.y - ypos );
  }

  lastPos =  glm::vec2(xpos,ypos);
  // For the next frame, the "last time" will be "now"
  lastTime = currentTime;
  return deltaTime;
}

// position around which to rotate
void SetCenter( glm::vec4 a)
{
  aroundPoint = glm::vec3(a[0],a[1],a[2]);
  lightFocusedPos = aroundPoint;
}

void SetCenter( glm::vec3 a)
{
  aroundPoint = a;
}

glm::vec3 GetCameraPosition(glm::vec3& direction,glm::vec3& right, glm::vec3& up)
{
  if ( GetAround() )
  {
    glm::vec3 cDir = GetObjectPos() - position;
    float f = glm::length(cDir);
    glm::vec3 newPos = GetObjectPos() - direction * f;
    return newPos;    
  }
  return position;
}

glm::vec3 GetObjectPosition(glm::vec3& direction,glm::vec3& right, glm::vec3& up)
{
  if ( GetAround() )
    return GetObjectPos();
  return position + direction;
}

glm::mat4 getModelMatrix(glm::mat4 m)
{
  //pulsing
  m =glm::rotate(m,GetAngle(),glm::vec3(1,1,1));
  m =glm::scale(m,scaleVec);
  return m;
}

glm::mat4 getModelNormalMatrix( const glm::mat4 & m )
{
  return glm::transpose(glm::inverse(m));
}

float savedVa;
float savedHa;

glm::vec3 GetCameraDirection()
{
  return glm::vec3 (
    cos(verticalAngle) * sin(horizontalAngle), 
    sin(verticalAngle),
    cos(verticalAngle) * cos(horizontalAngle)
    );
}

void computeMatricesFromInputs(){

  // handle inputs
  float deltaTime = HandlePressed();
	
	// Direction : Spherical coordinates to Cartesian coordinates conversion
  glm::vec3 direction(
    cos(verticalAngle) * sin(horizontalAngle), 
    sin(verticalAngle),
    cos(verticalAngle) * cos(horizontalAngle)
    );

  float len = glm::length(direction);

  // Right vector
  glm::vec3 right = glm::vec3(
    sin(horizontalAngle - 3.14f/2.0f), 
    0,
    cos(horizontalAngle - 3.14f/2.0f)
    );

  // Up vector
  glm::vec3 up = glm::cross( right, direction );

  // handle not dependent on time
  

  if (glfwGetKey( GWindow, GLFW_KEY_SPACE ) == GLFW_PRESS)
  {
    // Reset mouse position for next frame
    glfwSetCursorPos(GWindow, WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
    lastPos = glm::vec2(WINDOW_WIDTH/2, WINDOW_HEIGHT/2);
  }

  //static glm::vec3 d= direction;
  //if ( d!= direction)
  //  __debugbreak();
  //d = direction;
  // Move forward
  if (glfwGetKey( GWindow, GLFW_KEY_UP ) == GLFW_PRESS){
    if (IsEnabled(Key_L))
      lightPos += GetLightDir()* deltaTime * speed;
    else
      position += direction * deltaTime * speed;
  }
  // Move backward
  if (glfwGetKey( GWindow, GLFW_KEY_DOWN ) == GLFW_PRESS){
    if (IsEnabled(Key_L))
      lightPos -= GetLightDir()* deltaTime * speed;
    else
      position -= direction * deltaTime * speed;
  }
  // Strafe right
  if (glfwGetKey( GWindow, GLFW_KEY_RIGHT ) == GLFW_PRESS){
    if (IsEnabled(Key_L))
      lightPos += GetLightStrafe() * deltaTime * speed;
    else
      position += right * deltaTime * speed;
  }
  // Strafe left
  if (glfwGetKey( GWindow, GLFW_KEY_LEFT ) == GLFW_PRESS){
    if (IsEnabled(Key_L))
      lightPos -= GetLightStrafe() * deltaTime * speed;
    else
      position -= right * deltaTime * speed;
  }

  if (glfwGetKey( GWindow, GLFW_KEY_KP_ADD ) == GLFW_PRESS)
    power++;
  if (glfwGetKey( GWindow, GLFW_KEY_KP_SUBTRACT ) == GLFW_PRESS)
    power--;
  if ( power < 0 )
    power = 0;
  if ( power > 100 )
    power = 100;
	
	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 1000.0f);
	// Camera matrix

  ViewMatrix = glm::lookAt(
    GetCameraPosition(direction, right,up),
    GetObjectPosition(direction, right,up),
    up
    );
  
  //ViewMatrix       = glm::lookAt(
	//							position,           // Camera is here
	//							position+direction, // and looks here : at the same position, plus "direction"
	//							up                  // Head is up (set to 0,-1,0 to look upside-down)
	//					   );

}

bool IsEnabled(SwitchKeys key)
{
  return keys[key].enabled;
}
