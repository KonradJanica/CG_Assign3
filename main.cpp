/*********************************************************
  Version 1.100

  Code provided by Gustavo Carneiro, Anthony Dick
  for the course 
  COMP SCI 3014/7090 Computer Graphics
  School of Computer Science
  University of Adelaide

  Permission is granted for anyone to copy, use, modify, or distribute this
  program and accompanying programs and documents for any purpose, provided
  this copyright notice is retained and prominently displayed, along with
  a note saying that the original programs are available from the aforementioned 
  course web page. 

  The programs and documents are distributed without any warranty, express or
  implied.  As the programs were written for research purposes only, they have
  not been tested to the degree that would be advisable in any important
  application.  All use of these programs is entirely at the user's own risk.
 *********************************************************/

/**
 * Draws a single cube in front of the camera.
 * Toggles Projection matrix, and depth buffer.
 */

#include <iostream>
#include <stdlib.h>
#include <iomanip>
#include <stdio.h>
#include <fstream>

#include <GL/glew.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
// Our wav file
Mix_Chunk *wave = NULL;
// Our music file
Mix_Music *music = NULL;
#define SOUND_PATH "./sounds/"
#define CAR_HORN SOUND_PATH "/car_horn_final.wav"
#define WAV_PATH "./sounds/testwav.wav"
#define MUS_PATH "./sounds/testogg.ogg"

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

Renderer *g_renderer;
Controller *g_controller;

// Our shader program
GLuint g_program_id[5];

// Background controls
enum Colours {
  kBlue = 0,
  kGrey,
  kBlack,
  kOldGold
};
int g_background = kOldGold;
glm::vec3 g_colour = glm::vec3(0.71,0.61,0.23);


// Fog global
int g_fog_mode = 0;

// Rendering Toggle Vars
bool g_coord_axis = true;

// Camera object
Camera * g_camera;

int g_window_x = 640*2;
int g_window_y = 480*2;

void UpdateProjection() {
  glm::mat4 projection = glm::perspective(75.0f, float(g_window_x / g_window_y), 0.1f, 100.0f);
  for (unsigned int i = 0; i < 5; i++) {
    glUseProgram(g_program_id[i]);
    int projHandle = glGetUniformLocation(g_program_id[i], "projection_matrix");
    assert(projHandle != -1 && "Uniform: projection_matrix was not an active uniform label - See EnableAxis in Renderer");
    glUniformMatrix4fv( projHandle, 1, false, glm::value_ptr(projection) );
  }
}

/**
 * Renders a frame of the state and shaders we have set up to the window
 */
void render() {

  glClearColor(g_colour.x, g_colour.y, g_colour.z, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  g_controller->Draw();

  glUseProgram(0);

  glBindVertexArray(0);

  glutSwapBuffers();
  glFlush();

}

// FPS Counter Vars
long g_past = 0;
int g_frames = 0;

void idle() {
  long time = glutGet(GLUT_ELAPSED_TIME);

  // FPS counter, only print FPS in terminal when it is not 60
  g_frames += 1;
  if (static_cast<float>(time - g_past) / 1000.0f >= 1.0f) {
    if (g_frames != 60) {
      std::cout << "FPS: " << g_frames << std::endl;
    }
    g_frames = 0;
    g_past = time;
  }

  // Physics movement tick
  g_controller->UpdateGame();

  glUseProgram(g_program_id[3]);
  int timeHandle = glGetUniformLocation(g_program_id[3], "time");
  if(timeHandle == -1)
  {
    printf("Could not get handle for time var \n");
  }
  //printf("sending time %d\n", time);
  glUniform1f(timeHandle, time+1); 

  UpdateProjection();

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
      g_controller->KeyPressed(key);
      break;
    case 'h':
      // system("aplay ./sounds/car_horn_final.wav -q&");
      if (!Mix_Playing(0)) {
        if ( Mix_PlayChannel(0, wave, 0) == -1 ) {
          printf("WAV play fail");
          exit(1);
        }
      }
      break;
    case 'f':
      {    
        g_fog_mode = (g_fog_mode + 1) % 5;
        glUseProgram(g_program_id[2]);
        int fogModeHandle = glGetUniformLocation(g_program_id[2], "fog_mode");
        glUniform1i(fogModeHandle, g_fog_mode);
        glutPostRedisplay(); 
        break;
      }    
    case 27: // escape key pressed
      exit(0);
      break;
    case 'c':
      g_camera->CycleState();
      break;
    case 'b':
      g_background++;
      if (g_background > 3)
        g_background = 0;

      if (g_background == kBlue) {
        g_colour.x = 0;
        g_colour.y = 0;
        g_colour.z = 1;
      } else if (g_background == kGrey) {
        g_colour.x = 0.7;
        g_colour.y = 0.7;
        g_colour.z = 0.7;
      } else if (g_background == kBlack) {
        g_colour.x = 0;
        g_colour.y = 0;
        g_colour.z = 0;
      } else if (g_background == kOldGold) {
        g_colour.x = 0.71;
        g_colour.y = 0.61;
        g_colour.z = 0.23;
      }
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

  // Initialize SDL.
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    std::cout << "SDL_Init() Fail" << std::endl;
    return -1;
  }
  //Initialize SDL_mixer 
  if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 ) {
    std::cout << "SDL_mixer Fail" << std::endl;
    return -1; 
  }

  // Load our sound effect
  wave = Mix_LoadWAV(CAR_HORN);
  if (wave == NULL) {
    std::cout << "WAV load fail" << std::endl;
    return -1;
  }

  // Load our music
  music = Mix_LoadMUS(MUS_PATH);
  if (music == NULL) {
    std::cout << "OGG load fail" << std::endl;
    return -1;
  }

  if ( Mix_PlayChannel(0, wave, 0) == -1 ) {
    std::cout << "WAV play fail" << std::endl;
    return -1;
  }

  if ( Mix_PlayMusic( music, 1) == -1 ) {
    std::cout << "OGG play fail" << std::endl;
    return -1;
  }

  // assert(argc > 1 && "provide Arg1 of .obj file");

  std::cout << "Movement: Arrow keys move forward/backward and strafe left/right\n";
  std::cout << "Movement: Hold left mouse button to change direction\n";
  std::cout << "Movement: Hold right mouse button to zoom\n";
  std::cout << "Controls: 'b' key to change Background Colour\n";
  std::cout << "Controls: 'd' key to toggle Depth Testing\n";
  std::cout << "Controls: 'c' key to toggle Axis Coordinates\n";
  // std::cout << "Controls: 'l' key to Cycle Lighting\n\n";
  // std::cout << "Controls: 's' key to toggle Wireframe\n\n";
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

  // GL stateglUseProgram(g_program_id[i]);
  ;
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glFrontFace(GL_CCW);

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


  g_renderer = new Renderer();
  // Construct Axis VAO
  g_renderer->EnableAxis(g_program_id[1]);

  g_controller = new Controller(g_renderer);

  // Setup camera global
  g_camera = g_controller->camera();

  // Setup terrain
  g_controller->EnableTerrain(g_program_id[2]);

  // Setup skybox
  g_controller->AddSkybox(g_program_id[4]);

  // Setup Water
  g_controller->AddWater(g_program_id[3]);

  // Add starting models
  g_controller->AddModel(g_program_id[2], "models/Spider-Man/Spider-Man.obj");
  g_controller->AddModel(g_program_id[2], "models/Aventador/Avent.obj");
  g_controller->AddModel(g_program_id[2], "models/Car/car-n.obj", true);

  // Here we set a new function callback which is the GLUT handling of keyboard input
  glutKeyboardFunc(keyboardDown);
  glutKeyboardUpFunc(KeyboardUp);
  glutMotionFunc(MotionFunc);
  glutMouseFunc(MouseFunc);
  // glutSpecialFunc(SpecialPressed); // arrow keys etc.
  // glutSpecialUpFunc(SpecialReleased);
  glutIdleFunc(idle);
  glutDisplayFunc(render);
  glutMainLoop();

  return 0;
}
