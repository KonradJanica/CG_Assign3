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
Renderer    *g_renderer;
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

// Send the projection matrix to all shaders
// NB - Make sure to update this loop every time we add a new shader
void UpdateProjection() {
  glm::mat4 projection = glm::perspective(g_fov, float(g_window_x / g_window_y), 0.1f, 100.0f);
  for (unsigned int i = 0; i < 6; i++) {
    glUseProgram(g_program_id[i]);
    int projHandle = glGetUniformLocation(g_program_id[i], "projection_matrix");
    if(projHandle == -1) {
      fprintf(stderr,"Could not find uniform: 'projection_matrix' In: Main - UpdateProjection\n This may cause unexpected behaviour in the program\n");
    }
    glUniformMatrix4fv( projHandle, 1, false, glm::value_ptr(projection) );
  }
}

// Render a frame
void render() {

  // Clear the buffers
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Call the daw function of the controller which handles drawing the objects
  g_controller->Draw();

  // Unbind
  glUseProgram(0);
  glBindVertexArray(0);

  // Swap buffers + flush
  glutSwapBuffers();
  glFlush();
}

void idle() {
}

void timer(int t) {
  // unsigned long long time = glutGet(GLUT_ELAPSED_TIME);
  // // FPS counter, only print FPS in terminal when it is not 60
  // g_frames += 1;
  // if (static_cast<float>(time - g_past) / 1000.0f >= 1.0f) {
  //   if (g_frames != 60) {
  //     std::cout << "FPS: " << g_frames << std::endl;
  //   }
  //   g_frames = 0;
  //   g_past = time;
  // }

  // Let the controller handle updating the state of the game
  g_controller->UpdateGame();
  

  // glUseProgram(g_program_id[3]);
  // int timeHandle = glGetUniformLocation(g_program_id[3], "time");
  // if(timeHandle == -1)
  // {
  //   printf("Could not get handle for time var \n");
  // }
  // //printf("sending time %d\n", time);
  // glUniform1f(timeHandle, time+1); 


  // UpdateProjection();


  glutTimerFunc(14, timer, 0);
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

  // We simply check the key argument against characters we care about, in this case A and D
  switch(key) 
  {
    case 'w':
    case 's':
    case 'a':
    case 'd':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'h':
      g_controller->KeyPressed(key);
      break;  
    case 27: // escape key pressed
      exit(0);
      break;
    case 'c':
      g_camera->CycleState();
      break;
    case 'b':
      g_fov += 5.0f;
      std::cout << "FOV = " << g_fov << std::endl;
      if (g_fov > 120.0f)
        g_fov = 0.0f;
      UpdateProjection();
      glutPostRedisplay();
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

  // Set up GLUT window
  glutInit(&argc, argv);
  glutInitWindowPosition(100, 0);
  glutInitWindowSize(g_window_x, g_window_y);

#ifdef __APPLE__    
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
#endif
  glutCreateWindow("Many Attributes");

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

  // Load in all the shaders
  g_program_id[0] = LoadShaders("shaders/wireframe.vert", "shaders/wireframe.frag");
  if (g_program_id[0] == 0)
    return 1;

  g_program_id[1] = LoadShaders("shaders/coord.vert", "shaders/coord.frag");
  if (g_program_id[1] == 0)
    return 1;

  g_program_id[2] = LoadShaders("shaders/shaded.vert", "shaders/shaded.frag");
  if (g_program_id[2] == 0)
    return 1;

  g_program_id[3] = LoadShaders("shaders/water.vert", "shaders/water.frag");
  if (g_program_id[3] == 0)
    return 1;

  g_program_id[4] = LoadShaders("shaders/sky.vert", "shaders/sky.frag");
  if (g_program_id[4] == 0)
    return 1;

  g_program_id[5] = LoadShaders("shaders/rain.vert", "shaders/rain.frag");
  if (g_program_id[5] == 0)
    return 1;

  g_program_id[6] = LoadShaders("shaders/depthbuffer.vert", "shaders/depthbuffer.frag");
  if (g_program_id[6] == 0)
    return 1;

  // Make renderer with Axis
  g_renderer = new Renderer(g_program_id[6], g_program_id[1], false);

  g_controller = new Controller(g_renderer);

  // Setup camera global
  g_camera = g_controller->camera();

  // Setup terrain
  g_controller->EnableTerrain(g_program_id[2]);

  // Setup skybox
  g_controller->AddSkybox(g_program_id[4]);

  // Setup Water
  g_controller->AddWater(g_program_id[3]);

  // Setup Rain
  g_controller->AddRain(g_program_id[5]);

  // Add starting models
  // g_controller->AddModel(g_program_id[2], "models/Spider-Man/Spider-Man.obj");
  // g_controller->AddModel(g_program_id[2], "models/Aventador/Avent.obj", true);
  // g_controller->AddModel(g_program_id[2], "models/Car/car-n.obj", true);
  g_controller->AddModel(g_program_id[2], "models/Pick-up_Truck/pickup.obj", true);
  // g_controller->AddModel(g_program_id[2], "models/Signs_OBJ/working/curve_left.obj");
  // g_controller->AddModel(g_program_id[2], "models/Signs_OBJ/working/curve_right.obj");
  g_controller->AddModel(g_program_id[2], "models/Signs_OBJ/working/60.obj");

  // TODO fix this - removed and hardcoded for shadows
  // UpdateProjection();

  // Set our GLUT window handler callback functions
  glutKeyboardFunc(keyboardDown);
  glutKeyboardUpFunc(KeyboardUp);
  glutMotionFunc(MotionFunc);
  glutMouseFunc(MouseFunc);
  // glutSpecialFunc(SpecialPressed); // arrow keys etc.
  // glutSpecialUpFunc(SpecialReleased);
  glutIdleFunc(idle);
  glutTimerFunc(1,timer,0);
  glutDisplayFunc(render);
  glutMainLoop();

  delete g_renderer;
  delete g_controller;
  delete g_camera;

  return 0;
}
