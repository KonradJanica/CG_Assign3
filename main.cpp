/**
 * Computer Graphics Assignment 3 - 
 * Mitchell Anderson, Andrew Pham, Konrad Janica
 *
 * This is the 3rd (group) assignment for Computer Graphics 3014 
 * University of Adelaide
 * 
 * This is a cliff driving game with procedurally generated terrain,
 * dynamic lighting, skybox, multi-texturing, physics, sound
 * collision detection, water, 

 * main.cpp - Driver function for assignment 3
 * 
 * 
**/

#include <iostream>
#include <stdlib.h>
#include <iomanip>
#include <stdio.h>
#include <fstream>
#include <GL/glew.h>
// #include <GL/glx.h> //vsync glx


#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "model_data.h"
#include "model.h"
#include "renderer.h"
#include "camera.h"
#include "controller.h"
#include "Skybox.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image/stb_image.h"

// Renderer, Controller and Camera objects
Controller  *g_controller;
Camera      *g_camera;

// Array for the shader programs
GLuint      g_program_id[6];

// FOV global
float       g_fov = 75.0f;

// Rendering Toggle Vars
bool        g_coord_axis = true;
  
// Default window properies
int         g_window_x = 640;
int         g_window_y = 480;

// FPS Counter Vars
long        g_past = 0;
int         g_frames = 0;

// Render a frame
void render() {

  // Clear the buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.0f,0.0f,0.0f,0.0f);
  glViewport(0,0,640,480);

  // Call the daw function of the controller which handles drawing the objects
  g_controller->Draw();

  // Unbind
  glUseProgram(0);
  glBindVertexArray(0);

  // Swap buffers + flush
  glutSwapBuffers();
  // No point using glFlush unless single buffer or using opengl over a network
  //   // Konrad
  // glFlush();
}

void idle() {
}

void timer(int t) {
  // Let the controller handle updating the state of the game
  g_controller->UpdateGame();

  glutTimerFunc(14, timer, 0);
  glutPostRedisplay();
}

void ReshapeFunc(GLint new_width, GLint new_height) {
  g_camera->ChangeWidthHeight(new_width, new_height);
  glutPostRedisplay();
}

void MouseFunc(int button, int state, int x, int y) {
  if (button == GLUT_RIGHT_BUTTON) {
    g_camera->set_is_right_button(true);
  } else {
    g_camera->set_is_right_button(false);
  }
  if (button == GLUT_LEFT_BUTTON) {
    g_camera->set_is_left_button(true);
  } else {
    g_camera->set_is_left_button(false);
  }
  g_camera->UpdatePreviousMouse(x,y);
}

void MotionFunc(int x, int y) {
  if (g_camera->is_right_button()) {
    g_camera->ChangeZoom(y);
  }
  if (g_camera->is_left_button()) {
    g_camera->ChangeDirection(x,y);
  }
  glutPostRedisplay();
  g_camera->UpdatePreviousMouse(x,y);
}

void SpecialPressed( int key, int x, int y )
{
  g_controller->KeyPressed(key);
}

void SpecialReleased( int key, int x, int y )
{
  g_controller->KeyReleased(key);
}

/**
 * Called while a keyboard key release is detected
 * This GLUT functions is not OpenGL specific, but allows interactivity to our programs
 * @param key, the keyboard key that made the event
 * @param x, not used
 * @param y, not used
 */
void KeyboardUp(unsigned char key, int x, int y) {
  g_controller->KeyReleased(key);
}

/**
 * Called while a keyboard key press is detected
 * This GLUT functions is not OpenGL specific, but allows interactivity to our programs
 * @param key, the keyboard key that made the event
 * @param x, not used
 * @param y, not used
 */
void keyboardDown(unsigned char key, int x, int y) {

  g_controller->KeyPressed(key);
  // We simply check the key argument against characters we care about, in this case A and D
  switch(key) 
  {
    case 27: // escape key pressed
      exit(0);
      break;
    case 'c':
      g_camera->CycleState();
      break;
    case 'p':
      // TODO fix this
      if (g_coord_axis) {
        g_coord_axis = false;
      } else {
        g_coord_axis = true;
      }
      glutPostRedisplay();
      break;
  }
}

/**
 * Program entry. Sets up OpenGL state, GLSL Shaders and GLUT window and function call backs
 * Takes no arguments
 */
int main(int argc, char **argv) {

  std::cout << "Movement: WASD keys to control the car\n";
  std::cout << "Movement: IJKL keys to control the camera when in free mode\n";
  std::cout << "Gameplay: H key to beep horn\n";
  std::cout << "Movement: Hold and drag left mouse to change camera direction in free mode\n";
  std::cout << "Controls: 'c' key to toggle Camera mode\n";
  std::cout << "Controls: 'Esc' key to Quit\n\n";

  // Check whether vec3 can be loaded into VAO - otherwise no point continuing
  assert(sizeof(glm::vec3) == sizeof(GLfloat) * 3);

  // Set up GLUT window
  glutInit(&argc, argv);
  glutInitWindowPosition(100, 0);
  glutInitWindowSize(g_window_x, g_window_y);

#ifdef __APPLE__    
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
#endif
  glutCreateWindow("Assign3");

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    return -1;
  }
  // glXSwapIntervalEXT(1); //linux vsync
  // wglSwapIntervalEXT(1); //windows vsync
  // glXSwapIntervalMESA(1); //doesnt work but should

  // std::cout << glGetString(GL_EXTENSIONS) << std::endl;

  // #if defined(WIN32)
  //   wglSwapIntervalEXT(1); //1 for on
  // #else
  //   glXSwapIntervalSGI(1);
  // #endif

  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);

  // Moved to stack for speed
  Controller controller(g_window_x, g_window_y, false);
  g_controller = &controller;
  // g_controller = new Controller();
  // Setup camera global
  g_camera = g_controller->camera();

  // Set our GLUT window handler callback functions
  glutKeyboardFunc(keyboardDown);
  glutKeyboardUpFunc(KeyboardUp);
  glutMotionFunc(MotionFunc);
  glutMouseFunc(MouseFunc);
  glutReshapeFunc(ReshapeFunc);
  // glutSpecialFunc(SpecialPressed); // arrow keys etc.
  // glutSpecialUpFunc(SpecialReleased);
  glutIdleFunc(idle);
  glutTimerFunc(1,timer,0);
  glutDisplayFunc(render);
  glutMainLoop();

  return 0;
}
