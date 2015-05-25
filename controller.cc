#include "controller.h"

// Constructor, initializes an empty axis coordinate VAO to optimize Render()
//   Allows for Verbose Debugging Mode
//   @param bool debug_flag, true will enable verbose debugging
//   @warn assert will end program prematurely
Controller::Controller(const Renderer * r, const bool &debug_flag) 
  : renderer_(r), game_state_(kAutoDrive), road_y_rotation_(0), light_pos_(glm::vec4(0,0,0,0)),
  delta_time_(0), last_frame_(0), is_debugging_(debug_flag) {
    camera_ = new Camera();
    light_controller_ = new LightController();
    is_key_pressed_hash_.reserve(256);
    is_key_pressed_hash_.resize(256);

  }

void Controller::AddWater(const GLuint &program_id)
{
  water_ = new Water(program_id);
}

void Controller::AddSkybox(const GLuint &program_id)
{
  skybox_ = new Skybox(program_id);
}
// Adds a model to the member vector
//   @param program_id, a shader program
//   @param model_filename, a string containing the path of the .obj file
//   @warn the model is created on the heap and memory must be freed afterwards
//   TODO split Car into it's own
void Controller::AddModel(const GLuint &program_id, const std::string &model_filename, const bool &is_car) {
  if (is_car) {
    car_ = new Model(program_id, model_filename,
        glm::vec3(1.12f, 0.3f, 15.0f), // Translation  move behind first tile (i.e. start on 2nd tile)
        glm::vec3(0.0f,  0.0f, 0.0f),  // Rotation
        glm::vec3(0.3f,  0.3f, 0.3f),  // Scale
        60, false); // starting speed and debugging mode
    // This block fixes car being moved to the wrong spot initially
    UpdateCollisions();
    prev_left_lane_midpoint_ = left_lane_midpoint_;
    GLfloat current_frame = glutGet(GLUT_ELAPSED_TIME);
    delta_time_ = current_frame - last_frame_;
    last_frame_ = current_frame;
  } else {
    Object * object = new Model(program_id, model_filename,
        glm::vec3(0.0f, 0.0f, 0.0f), // Translation
        glm::vec3(0.0f, 0.0f, 0.0f), // Rotation
        glm::vec3(0.6f, 0.6f, 0.6f)); // Scale
    objects_.push_back(object);
  }
}

// Renders all models in the vector member
//   Should be called in the render loop
void Controller::Draw() {
  // Lights need to be transformed with view/normal matrix
  PositionLights();
  //NB MitchNote - DO NOT MOVE WHERE THIS IS RENDERED, IT MUST BE RENDERED FIRST!!!
  renderer_->RenderSkybox(skybox_, camera_);
  // Spider-man
  renderer_->Render(objects_.at(0), camera_);
  // Aventador
  renderer_->Render(objects_.at(1), camera_);
  // Car with physics
  renderer_->Render(car_, camera_);
  // Terrain
  renderer_->Render(terrain_, camera_);

  renderer_->RenderWater(water_, camera_);
  // Axis
  // TODO Toggle
  renderer_->RenderAxis(camera_);

  car_->UpdateModelMatrix();
}

// Assumes SetupLighting() has been called, only updates essential light properties
void Controller::PositionLights() {
  glm::mat4 view_matrix = camera_->view_matrix();
  glm::mat4 car_mv_matrix = view_matrix * car_->model_matrix();
  glm::mat3 norm_matrix = glm::mat3(view_matrix);

  DirectionalLight dirLight;
  dirLight.DiffuseIntensity = glm::vec3(0.3f, 0.3f, 0.3f);
  dirLight.Direction = norm_matrix * glm::vec3(0.3f, -1.0f, -0.3f);

  // Point lights
  std::vector<PointLight> pointLights;
  // Main car brake lights
  for (unsigned int i = 0; i < 2; i++) {
    glm::mat4 brakeLightTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f + i * 1.0f, 0.5f, -3.0f));

    PointLight brakeLight;
    brakeLight.DiffuseIntensity = glm::vec3(1.0f, 0.0f, 0.0f);
    brakeLight.Position = glm::vec3(car_mv_matrix * brakeLightTranslation * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    brakeLight.Attenuation.Constant = 0.01f;
    brakeLight.Attenuation.Linear = 3.0f;
    brakeLight.Attenuation.Exp = 5.0f;

    pointLights.push_back(brakeLight);
  }

  // Spot lights
  std::vector<SpotLight> spotLights;
  // Main car headlights
  for (unsigned int i = 0; i < 2; i++) {
    glm::mat4 headlightTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f + i * 1.0f, 0.0f, 1.45f));
    glm::mat3 headlightNormMatrix = glm::mat3(car_mv_matrix);

    SpotLight headlight;
    headlight.DiffuseIntensity = glm::vec3(1.0f, 1.0f, 1.0f);
    headlight.SpecularIntensity = glm::vec3(1.0f, 1.0f, 1.0f);
    headlight.Position = glm::vec3(car_mv_matrix * headlightTranslation * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    headlight.Direction = headlightNormMatrix * glm::vec3(0.0f, -0.3f, 1.0f);
    headlight.CosineCutoff = cos(DEG2RAD(30.0f));
    headlight.Attenuation.Constant = 0.5f;
    headlight.Attenuation.Linear = 0.1f;
    headlight.Attenuation.Exp = 0.01f;

    spotLights.push_back(headlight);
  }

  light_controller_->SetDirectionalLight(car_->program_id(), dirLight);
  light_controller_->SetPointLights(car_->program_id(), pointLights.size(), &pointLights[0]);
  light_controller_->SetSpotLights(car_->program_id(), spotLights.size(), &spotLights[0]);
}

// Creates the Terrain object for RenderTerrain()
//   Creates Terrain VAOs
//   @warn terrain_ on heap, must be deleted after
void Controller::EnableTerrain(const GLuint &program_id) {
  terrain_ = new Terrain(program_id);
}

// The main control tick
//   Controls everything: camera, inputs, physics, collisions
void Controller::UpdateGame() {
  // calculate delta time
  GLfloat current_frame = glutGet(GLUT_ELAPSED_TIME);
  delta_time_ = current_frame - last_frame_;
  last_frame_ = current_frame;

  UpdateCamera();
  if (!is_collision_)
    UpdatePhysics();

  if (game_state_ == kCrashingFall) {
    // delta_time_ /= 5; //slowmo
    CrashAnimation();
    return;
  }

  if (is_key_pressed_hash_.at('w') || is_key_pressed_hash_.at('s')
      || is_key_pressed_hash_.at('a') || is_key_pressed_hash_.at('d')) {
    game_state_ = kStart;
  }
}

// The controllers camera update tick
//   Uses car position (for chase and 1st person view)
//   and checks keypresses for freeview
//   @warn should be called before car movement
void Controller::UpdateCamera() {
  // CAMERA CONTROLS
  // Freeview movement
  camera_->Movement(delta_time_, is_key_pressed_hash_); 
  // Point at car
  camera_->UpdateCarTick(car_);
  // Update camera lookAt
  camera_->UpdateCamera();
}

void Controller::CrashAnimation() {
  // Lock camera state
  camera_->ChangeState(Camera::kFreeView);
  // Rotate car
  float x_rot = car_->rotation().x + car_->speed() * 0.0004f * delta_time_;
  // printf("dt = %f\n",delta_time_);
  float z_rot = car_->rotation().z + car_->speed() * 0.0004f * delta_time_;

  // Move car in direction
  // TODO make member in object for this
  glm::vec3 dir = car_->direction();
  float dt = delta_time_ / 1000;
  float x_pos = car_->translation().x + dir.x * car_->speed()/10.0f*dt;
  float y_pos = car_->translation().y + dir.y * car_->speed()/10.0f*dt;
  float z_pos = car_->translation().z + dir.z * car_->speed()/10.0f*dt;

  typedef std::vector<glm::vec3> w_vec;
  typedef std::list<w_vec> w_list;
  const w_list &water = terrain_->colisn_lst_water();
  float dis = FLT_MAX;
  glm::vec3 closest_vertice;
  glm::vec3 snd_clst_vertice = glm::vec3(0,0,0); // just incase not found

  w_list::const_iterator it = water.begin();
  for (unsigned int i = 0; i < 2; ++i) {

    const w_vec &vertices = *it;
    for (w_vec::const_iterator x = vertices.begin(); 
        x != vertices.end(); ++x) {

      const glm::vec2 cur_vec = glm::vec2(x->x,x->z); // current vector 
      float cur_dis = glm::distance(cur_vec, glm::vec2(x_pos, z_pos));
      if (cur_dis < dis) {
        dis = cur_dis;
        snd_clst_vertice = closest_vertice;
        closest_vertice = *x;
      }
    }
    it++;
  }
  // printf("yp = %f, cv = %f\n",y_pos, closest_vertice.y);
  if (y_pos > closest_vertice.y) {
    car_->set_translation(glm::vec3(x_pos, y_pos, z_pos));
    car_->set_rotation(glm::vec3(x_rot, car_->rotation().y, z_rot));
  } else {
    // Calc crude rebound angle
    glm::vec3 vec_a = closest_vertice - snd_clst_vertice;
    x_rot = -asin(vec_a.x/dir.x);
    if (x_rot != x_rot) { // NaN catch
      x_rot = -0.3f;
    }
    z_rot += 0.3f;
    car_->set_rotation(glm::vec3(x_rot, car_->rotation().y, z_rot));
    // Calc crude bounce
    x_pos = car_->translation().x;
    y_pos = car_->translation().y + car_->speed() / 220;
    z_pos = car_->translation().z;
    car_->set_translation(glm::vec3(x_pos, y_pos, z_pos));
    // Reduce speed due to bounce
    car_->ReduceSpeed(car_->speed() * 0.5f);
  }
  if (colisn_anim_ticks_ > 400) {
    if (is_key_pressed_hash_.at('w') || is_key_pressed_hash_.at('s')
        || is_key_pressed_hash_.at('a') || is_key_pressed_hash_.at('d')
        || colisn_anim_ticks_ > 2000) {
    // Reset car
    // TODO constant for y translation
    car_->set_translation(glm::vec3(left_lane_midpoint_.x, 0.3f, left_lane_midpoint_.z));
    car_->set_rotation(glm::vec3(0.0f,road_y_rotation_,0.0f));
    car_->ResetPhysics();
    // Reset game state
    game_state_ = kAutoDrive;
    is_collision_ = false;
    camera_->ChangeState(camera_state_); // users previous camera
    return;
    }
  }
  colisn_anim_ticks_ += delta_time_ / 5;
  // printf("tm = %f\n", colisn_anim_ticks_);
}

// Checks whether car is between the biggest rectangle than can be formed
// @return  true  if can is inside corner of rectangle
// @warn input must be square for accurate results
bool Controller::IsInside(const glm::vec3 &car, std::pair<Terrain::boundary_pair,Terrain::boundary_pair> &bp) {
  Terrain::boundary_pair curr = bp.first;
  Terrain::boundary_pair next = bp.second;
  glm::vec3 a = curr.first;
  glm::vec3 b = curr.second;
  glm::vec3 c = next.second;
  glm::vec3 d = next.first;
  glm::vec3 arr[] = {b,c,d};

  float min_x = a.x, max_x = a.x;
  float min_z = a.z, max_z = a.z;
  for (int i = 0; i < 3; ++i) {
    // Find max x bounding box
    if (arr[i].x > max_x)
      max_x = arr[i].x;
    // Find min x bounding box
    if (arr[i].x < min_x)
      min_x = arr[i].x;
    // Find max z bounding box
    if (arr[i].z > max_z)
      max_z = arr[i].z;
    // Find min x bounding box
    if (arr[i].z < min_z)
      min_z = arr[i].z;
  }

  return !(car.x < min_x || car.x > max_x || car.z < min_z || car.z > max_z);
}

// TODO comment
//   Also calculates the middle of the road and it's direction if game state is autodrive
void Controller::UpdateCollisions() {
  // Setup vars
  const std::queue<Terrain::colisn_vec> &col = terrain_->collision_queue_hash();
  const glm::vec3 &car = car_->translation();
  const Terrain::colisn_vec head = col.front();
  Terrain::colisn_vec::const_iterator it = head.begin();

  // Find closest edge point
  Terrain::boundary_pair closest_pair;
  float dis = FLT_MAX;
  Terrain::colisn_vec::const_iterator closest_it = head.begin();

  while (it != head.end()) {
    const glm::vec3 &cur_vec = it->first; // current vector pair
    float cur_dis = glm::distance(cur_vec, car);
    if (cur_dis < dis) {
      dis = cur_dis;
      closest_pair = *it;
      closest_it = it;
    }
    it++;
  }
  // If closest is the first then something went wrong
  if (closest_it == head.begin()) {
    printf("ERROR IN COLLISION CHECK! - CLOSEST POINT IS BEGIN()\n");
  }
  // Get vertice pair next to closest
  //   but make sure it isn't the last pair overwise pop
  it = closest_it;
  it++;
  it++; // pop 1 vertex early
  if (it == head.end()) {
    printf("assuming end of tile reached, pop!\n");
    // TODO fix check end of tile...
    terrain_->col_pop();
    // TODO obviously this needs to be done further back from view space
    terrain_->ProceedTiles();

    // Check collision for next tile
    UpdateCollisions();
    return;
  }

  // If conditions flow through then make box with neighbours of the closest pair
  //   and check if car is inside the box
  Terrain::boundary_pair next_pair = *it;
  it = closest_it;
  it--;
  Terrain::boundary_pair previous_pair = *it;
  // Make boundary box the neighbours of current pair
  std::pair<Terrain::boundary_pair,Terrain::boundary_pair> bounding_box(previous_pair, next_pair);
  // Check if car is in range
  if (IsInside(car, bounding_box)) {
    //inside bounds
    is_collision_ = false;
  } else {
    printf("collision on edge of road!\n");
    is_collision_ = true;
    game_state_ = kCrashingFall;
    colisn_anim_ticks_ = 0;
    // Camera effect
    camera_state_ = camera_->state();
    camera_->ChangeState(Camera::kFirstPerson);
    UpdateCamera(); // Camera needs to be updated to change position
  }
  // Calculate middle of road in autodrive mode
  if (game_state_ == kAutoDrive || is_collision_) {
    // Get the next points to smooth it
    closest_it++;
    closest_it++;
    // Find midpoint
    glm::vec3 road_midpoint = ((*closest_it).first+(*closest_it).second);
    road_midpoint /= 2;
    road_midpoint.y = car_->translation().y;
    // Find lane midpoints
    left_lane_midpoint_ = road_midpoint + (*closest_it).second;
    left_lane_midpoint_ /= 2;
    // Find road direction
    glm::vec3 first_point = previous_pair.first;
    glm::vec3 next_point = next_pair.first;
    glm::vec3 direction = next_point - first_point;
    road_direction_ = glm::normalize(direction);
    road_y_rotation_ = RAD2DEG(atan2(direction.x, direction.z)); // atan2 handles division by 0 and proper quadrant
  }
}

// The controllers physics update tick
//   Checks keypresses and calculates acceleration
void Controller::UpdatePhysics() {

  if (game_state_ == kStart) {
    // Sets variables required for camera
    //  i.e. camera has access to car in UpdateCarTick(car_)
    car_->ControllerMovementTick(delta_time_, is_key_pressed_hash_);
  }

  UpdateCollisions();

  if (game_state_ == kAutoDrive) {
    car_->set_rotation(glm::vec3(car_->rotation().x,road_y_rotation_,car_->rotation().z));
    if (left_lane_midpoint_ == prev_left_lane_midpoint_) {
      // These position updates are from object movement tick
      //   i.e. p = p + dt*v, v /= SPEEDSCALE, v = speed * direction;
      // TODO put constants somewhere
      float dt = delta_time_ / 1000;
      float x_pos = car_->translation().x + road_direction_.x * car_->default_speed()/10.0f*dt;
      float y_pos = car_->translation().y;
      float z_pos = car_->translation().z + road_direction_.z * car_->default_speed()/10.0f*dt;
      car_->set_translation(glm::vec3(x_pos, y_pos, z_pos));
    } else {
      glm::vec3 next_pt_dir = left_lane_midpoint_ - car_->translation();
      next_pt_dir = glm::normalize(next_pt_dir);
      // These position updates are from object movement tick
      //   i.e. p = p + dt*v, v /= SPEEDSCALE, v = speed * direction;
      // TODO put constants somewhere
      float dt = delta_time_ / 1000;
      float x_pos = car_->translation().x + next_pt_dir.x * car_->default_speed()/10.0f*dt;
      float y_pos = car_->translation().y;
      float z_pos = car_->translation().z + next_pt_dir.z * car_->default_speed()/10.0f*dt;
      car_->set_translation(glm::vec3(x_pos, y_pos, z_pos));
    }
    prev_left_lane_midpoint_ = left_lane_midpoint_;
    // Set speed to default speed
    // TODO only need to call this once when change state
    car_->ResetPhysics();
  }
}
