#ifndef CONTROLS_HPP
#define CONTROLS_HPP

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512

enum SwitchKeys
{
  Key_C,
  Key_A,
  Key_S,
  Key_D,
  Key_L,
  Key_P,
  Key_R,
  Key_N,
  Key_T,
  Key_Plus,
  Key_Minus,
  Key_B, // like force
  Key_M, // like Move light direction 
  Key_O, // like increase normal offset checking 
  Key_I, // like apply according to slope
  Key_K, // like use camera direction as offset
  Keys
};

bool IsEnabled(SwitchKeys key);
void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
void SetCenter( glm::vec3 a);
void SetCenter( glm::vec4 a);
glm::vec3 GetLightPos();
//glm::vec3 GetScale();
//float GetAngle();
float GetPower();
glm::mat4 getModelMatrix( glm::mat4 mat = glm::mat4(1.0f));
glm::mat4 getModelNormalMatrix( const glm::mat4 & m );
void SetCameraPosition(glm::vec3 newCamPos, glm::vec3 newCamDir );
glm::vec3 GetLightDir();
void SetLightPos(glm::vec3 lp);
glm::vec3 GetCameraDirection();
#endif