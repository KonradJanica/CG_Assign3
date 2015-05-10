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

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "model_data.h"
#include "model.h"
#include "renderer.h"
#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image/stb_image.h"

Renderer *g_renderer;

// Our shader program
GLuint g_program_id[3];

// Background controls
enum Colours {
  kBlue = 0,
  kGrey,
  kBlack,
  kOldGold
};
int g_background = kOldGold;
glm::vec3 g_colour = glm::vec3(0.71,0.61,0.23);

// Lighting Globals
int g_lighting_mode = 1;

// Fog global
int g_fog_mode = 0;

// Rendering Toggle Vars
bool g_coord_axis = true;

// Camera object
Camera * g_camera;

int g_window_x = 640;
int g_window_y = 480;

void UpdateProjection() {
    glm::mat4 projection = glm::perspective(g_camera->aspect(), float(g_window_x / g_window_y), 0.1f, 100.0f);
    for (unsigned int i = 0; i < 3; i++) {
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

  g_camera->UpdateCamera();

  UpdateProjection();

  // Render ALL
  //g_renderer->Render();

  // This renders model 0
  g_renderer->Render(0);
  // Renders the car model
  g_renderer->DrawCar();

  g_renderer->skyBox(g_program_id[2]);

  g_renderer->RenderTerrain();

  // Render axis last so on top
  if (g_coord_axis) {
    g_renderer->RenderAxis();
  }

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

  // Keyboard camera movement tick
  g_camera->Movement();

  // Headlight dynamic lighting
  if (g_lighting_mode == 0) {
    float x,y,z;
    x = g_camera->cam_pos().x;
    y = g_camera->cam_pos().y;
    z = g_camera->cam_pos().z;
    g_renderer->SetLightPosition(x,y,z,1.0f);
  }

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
  g_camera->KeyPressed(key);
}

void SpecialReleased( int key, int x, int y )
{
  g_camera->KeyReleased(key);
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
    case 'd': // Toggle depth test
      if ( glIsEnabled( GL_DEPTH_TEST ) ) {
        glDisable( GL_DEPTH_TEST );
      }
      else {
        glEnable( GL_DEPTH_TEST );
      }
      glutPostRedisplay();
      break;
    case 's':
      glutPostRedisplay();
      break;
    case 'c':
      if (g_coord_axis) {
        g_coord_axis = false;
      } else {
        g_coord_axis = true;
      }
      glutPostRedisplay();
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
    case 'l':
      if (g_lighting_mode == 0) {
        std::cout << "Lighting Mode = Overhead (Directional) Static" << std::endl;
        g_lighting_mode = 1;
        float overhead_light = (g_renderer->GetMax(0, Renderer::kY) + 1) * 2;
        // Ambient light disabled as per specs
        g_renderer->SetupLighting(g_program_id[2], glm::vec3(0,0,0), glm::vec3(0.7,0.7,1), glm::vec3(1,1,1));
        g_renderer->SetLightPosition(0,overhead_light,0,0);
      } else if (g_lighting_mode == 1) {
        std::cout << "Lighting Mode = Headlight (Point) Static" << std::endl;
        g_lighting_mode = 2;
        float front_light = (g_renderer->GetMax(0, Renderer::kZ) + 1) * 2;
        // Ambient light disabled as per specs
        g_renderer->SetupLighting(g_program_id[2], glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(1,1,1));
        g_renderer->SetLightPosition(0,0,front_light,1.0f);
      } else if (g_lighting_mode == 2){
        std::cout << "Lighting Mode = None (Texturing Only)" << std::endl;
        g_lighting_mode = 3;
        g_renderer->SetupLighting(g_program_id[2], glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(0,0,0), 0);
      } else {
        std::cout << "Lighting Mode = Headlight (Point) Dynamic (Follows Camera)" << std::endl;
        g_lighting_mode = 0;
        g_renderer->SetupLighting(g_program_id[2], glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(1,1,1));
        // Dynamic headlight lighting in render
        //   Triggered by g_light_mode == 0
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

  // assert(argc > 1 && "provide Arg1 of .obj file");

  std::cout << "Movement: Arrow keys move forward/backward and strafe left/right\n";
  std::cout << "Movement: Hold left mouse button to change direction\n";
  std::cout << "Movement: Hold right mouse button to zoom\n";
  std::cout << "Controls: 'b' key to change Background Colour\n";
  std::cout << "Controls: 'd' key to toggle Depth Testing\n";
  std::cout << "Controls: 'c' key to toggle Axis Coordinates\n";
  std::cout << "Controls: 'l' key to Cycle Lighting\n\n";
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


  g_renderer = new Renderer(g_window_x,g_window_y);
  //Texture Shader
  // g_renderer->AddModel(g_program_id[2], std::string(argv[1]));
  g_renderer->AddModel(g_program_id[2], "models/Spider-Man/Spider-Man.obj"); 
  g_renderer->AddModel(g_program_id[2], "models/Car/car-n.obj", true); 
  //Construct Axis VAO
  g_renderer->EnableAxis(g_program_id[1]);
  //Setup default overhead light
  float overhead_light = (g_renderer->GetMax(0, Renderer::kY) + 1) * 2;
  // Last 0  =>  w = 0, meaning light doesn't have position  => directional light
  g_renderer->SetLightPosition(0,overhead_light,0,0);
  // Setup camera global
  g_camera = g_renderer->camera();

  // Setup terrain
  g_renderer->EnableTerrain(g_program_id[2]);

  // Here we set a new function callback which is the GLUT handling of keyboard input
  glutKeyboardFunc(keyboardDown);
  glutMotionFunc(MotionFunc);
  glutMouseFunc(MouseFunc);
  glutSpecialFunc(SpecialPressed);
  glutSpecialUpFunc(SpecialReleased);
  glutIdleFunc(idle);
  glutDisplayFunc(render);
  glutMainLoop();

  return 0;
}
