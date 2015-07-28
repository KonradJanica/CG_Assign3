#include "controller.h"

// Constructor
//   Allows for Verbose Debugging Mode
//   @param bool debug_flag, true will enable verbose debugging
//   @warn assert will end program prematurely
//   @note axis is rendered in debugging mode
Controller::Controller(const int window_width, const int window_height, const bool debug_flag) :
  // Object construction
  renderer_(Renderer(debug_flag)),
  shaders_(renderer_.shaders()),
  camera_(Camera(shaders_, window_width, window_height)),
  sun_(Sun(camera(), debug_flag)),
  light_controller_(new LightController()),
  collision_controller_(CollisionController()),
  terrain_(new Terrain(shaders_->LightMappedGeneric)),
  road_sign_(RoadSign(shaders_, terrain_)),
  car_(AddCar(shaders_->LightMappedGeneric, "models/Pick-up_Truck/pickup_wind_alpha.obj")),
  // State and var defaults
  game_state_(kAutoDrive), light_pos_(glm::vec4(0,0,0,0)),
  frames_past_(0), frames_count_(0), delta_time_(18), is_debugging_(debug_flag) {

    is_key_pressed_hash_.reserve(256);
    is_key_pressed_hash_.resize(256);
    playSound = 1;

    rain_ = new Rain(shaders_->RainGeneric, debug_flag);
    water_ = new Water(shaders_->WaterGeneric);
    skybox_ = new Skybox(shaders_->SkyboxGeneric);

  // Add starting models
  // AddModel(shaders_->LightMappedGeneric, "models/Pick-up_Truck/pickup.obj", true);
  // AddModel(shaders_->LightMappedGeneric, "models/Car/car-n.obj", true);
  // AddModel(shaders_->LightMappedGeneric, "models/Signs_OBJ/working/curve_left.obj");
  // AddModel(shaders_->LightMappedGeneric, "models/Signs_OBJ/working/curve_right.obj");
  // AddModel(shaders_->LightMappedGeneric, "models/Signs_OBJ/working/60.obj");

}

// Adds an object to the member vector
//   @param shader, a shader class holding shader to use and uniforms
//   @param model_filename, a string containing the path of the .obj file
//   @warn the model is created on the heap and memory must be freed afterwards
Object * Controller::AddObject(const Shader &shader, const std::string &model_filename) {
  Object * object = new Object(shader, model_filename,
      glm::vec3(0.95f, 0.55f, 35.0f),     // Translation  move behind first tile (i.e. start on 2nd tile)
      glm::vec3(0.0f, 20.0f, 0.0f),       // Rotation
      glm::vec3(0.4f,  0.4f*1.6f, 0.4f),  // Scale
      60, false); // starting speed and debugging mode

  return object;
}

// Adds a car (object with physics) to the member vector
//   @param shader, a shader class holding shader to use and uniforms
//   @param model_filename, a string containing the path of the .obj file
//   @warn the model is created on the heap and memory must be freed afterwards
Car * Controller::AddCar(const Shader &shader, const std::string &model_filename) {
  Car * car = new Car(shader, model_filename,
      glm::vec3(0.95f, 0.55f, 35.0f),     // Translation  move behind first tile (i.e. start on 2nd tile)
      glm::vec3(0.0f, 20.0f, 0.0f),       // Rotation
      glm::vec3(0.4f,  0.4f*1.6f, 0.4f),  // Scale
      60, false); // starting speed and debugging mode

  return car;
}

// Renders all models in the vector member
//   Should be called in the render loop
void Controller::Draw() {
  // Draw to shadow buffer
  glBindFramebuffer(GL_FRAMEBUFFER, renderer_.fbo()->FrameBufferShadows);
  glClear(GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, renderer_.fbo()->textureX, renderer_.fbo()->textureY);

  // Car with physics
  renderer_.RenderDepthBuffer(car_, sun_);
  // Road-signs
  const std::vector<Object*> signs = road_sign_.signs();
  const std::vector<int> active_signs = road_sign_.active_signs();

  // Unfortunately this does not work
  // for (unsigned int x = 0; x < signs.size(); ++x) {
  //   // if (active_signs[x] >= 0) // no point
  //   renderer_.RenderDepthBuffer(signs[x], sun_);
  // }
  // Terrain
  renderer_.RenderDepthBuffer(terrain_, sun_);

  // Draw to screen
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // Bind shadow map texture
  glBindTexture(GL_TEXTURE_2D, renderer_.fbo()->DepthTexture);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, camera_.width(), camera_.height());

  // Ordering of renderering is very important due to transparency
  renderer_.RenderSkybox(skybox_, camera_);
  // Water
  renderer_.RenderWater(water_, car_, skybox_, camera_);
  // Terrain
  renderer_.Render(terrain_, camera_, sun_);
  // Road-signs
  for (unsigned int x = 0; x < signs.size(); ++x) {
    // if (active_signs[x] >= 0) // no point
    renderer_.Render(signs[x], camera_, sun_);
  }
  if (camera_.state() == Camera::kFirstPerson) {
    // Rain (particles)
    if (sun_.time_of_day() != 12)
      rain_->Render(camera_, car_, skybox_);
  // Car with physics
  renderer_.Render(car_, camera_, sun_);
  } else {
    // Car with physics
    renderer_.Render(car_, camera_, sun_);
    // Rain (particles)
    if (sun_.time_of_day() != 12)
      rain_->Render(camera_, car_, skybox_);
  }

  // Axis only renders in debugging mode
  renderer_.RenderAxis(camera_);
}

// Assumes SetupLighting() has been called, only updates essential light properties
void Controller::PositionLights() {
  glm::mat4 view_matrix = camera_.view_matrix();
  glm::mat4 car_mv_matrix = view_matrix * car_->model_matrix();
  // glm::mat3 norm_matrix = glm::mat3(view_matrix);

  DirectionalLight dirLight;

  dirLight.DiffuseIntensity = glm::vec3(1.00f, 1.00f, 1.00f);
  dirLight.AmbientIntensity = glm::vec3(0.50f, 0.50f, 0.50f);
  dirLight.SpecularIntensity = glm::vec3(0.35f, 0.35f, 0.40f);
  dirLight.DiffuseIntensity *= sun_.LightIntensityMultiplier();
  dirLight.AmbientIntensity *= sun_.LightIntensityMultiplier();
  // dirLight.SpecularIntensity *= 1.0f - sun_.LightIntensityMultiplier();
  // if (!sun_.IsDay()) {
  //   dirLight.SpecularIntensity.x *= 1.0f - sun_.LightIntensityMultiplier();
  //   dirLight.SpecularIntensity.y *= 1.0f - sun_.LightIntensityMultiplier();
  // }
  dirLight.Direction =  sun_.sun_direction();

  // Point lights
  std::vector<PointLight> pointLights;
  // Main car brake lights
  for (unsigned int i = 0; i < 2; i++) {
    glm::mat4 brakeLightTranslation = glm::translate(glm::mat4(1.0f), glm::vec3(-0.85f + i * 1.7f, 0.0f, -3.3f));

    PointLight brakeLight;
    brakeLight.DiffuseIntensity = glm::vec3(1.0f, 0.0f, 0.0f);
    brakeLight.Position = glm::vec3(car_mv_matrix * brakeLightTranslation * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    brakeLight.Attenuation.Constant = 0.001f;
    brakeLight.Attenuation.Linear = 2.0f;
    brakeLight.Attenuation.Exp = 3.0f;

    if (is_key_pressed_hash_.at('s')) {
      brakeLight.Attenuation.Constant = 0.00001f;
      brakeLight.Attenuation.Linear = 0.5f;
      brakeLight.Attenuation.Exp = 3.0f;
    }

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
    headlight.SpecularIntensity = glm::vec3(0.1f, 0.1f, 0.1f);
    headlight.Position = glm::vec3(car_mv_matrix * headlightTranslation * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    headlight.Direction = headlightNormMatrix * glm::vec3(0.0f, -0.3f, 1.0f);
    headlight.CosineCutoff = cos(DEG2RAD(30.0f));
    headlight.Attenuation.Constant = 0.3f;
    headlight.Attenuation.Linear = 0.01f;
    headlight.Attenuation.Exp = 0.01f;
    
    if (is_key_pressed_hash_.at('r')) {
      headlight.DiffuseIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
      headlight.SpecularIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    spotLights.push_back(headlight);
  }

  light_controller_->SetDirectionalLight(car_->shader()->Id, dirLight);
  light_controller_->SetSpotLights(water_->shader().Id, spotLights.size(), &spotLights[0]);
  light_controller_->SetDirectionalLight(water_->shader().Id, dirLight);

  light_controller_->SetPointLights(car_->shader()->Id, pointLights.size(), &pointLights[0]);
  light_controller_->SetSpotLights(car_->shader()->Id, spotLights.size(), &spotLights[0]);
}

// The main control tick
//   Controls everything: camera, inputs, physics, collisions
void Controller::UpdateGame() {
  // Lights need to be transformed with view/normal matrix
  PositionLights();
  // Update the position of the rain
  rain_->UpdatePosition();

  // FPS counter - also determine delta time
  long long current_frame = glutGet(GLUT_ELAPSED_TIME);
  frames_count_ += 1;
  if ((current_frame - frames_past_) > 1000) {
    const int fps = frames_count_ * 1000.0f / (current_frame - frames_past_);
      std::cout << "FPS: " << frames_count_ << std::endl;
    frames_count_ = 0;
    frames_past_ = current_frame;

    // Work out average miliseconds per tick
    const GLfloat delta_time = 1.0f/fps * 1000.0f;
    if (abs(delta_time_ - delta_time) < 5) // Dont allow too high fluc
      delta_time_ = delta_time;
    if (delta_time_ > 20) // Cap at 50 fps delta (will run in slowmo on less fps)
      delta_time_ = 20;
    // printf("dt = %f\n",delta_time_);
  }

  // Update Sun/Moon position
  sun_.Update();

  // Send time for water
  water_->SendTime(current_frame);
  terrain_->GenerationTick();

  // printf("mid = (%f,%f,%f)\n",left_lane_midpoint_.x,left_lane_midpoint_.y,left_lane_midpoint_.z);
  // printf("car = (%f,%f,%f)\n",car_->translation().x,car_->translation().y,car_->translation().z);
  if (!collision_controller_.is_collision()) {
    UpdatePhysics();
    game_state_ = collision_controller_.UpdateCollisions(car_, terrain_, &camera_, &road_sign_, game_state_);
  } else {
    // TODO add car off road shaking
  }
  car_->UpdateModelMatrix();

  UpdateCamera();

  if (game_state_ == kCrashingFall) {
    // delta_time_ /= 5; //slowmo
    if(playSound)
      system("aplay ./sounds/metal_crash.wav -q &");
    playSound = 0;
    game_state_ = collision_controller_.CrashAnimationFall(&camera_, terrain_, car_, delta_time_, is_key_pressed_hash_);
    return;
  }
  if (game_state_ == kCrashingCliff) {
    // delta_time_ /= 5; //slowmo
    if(playSound)
      system("aplay ./sounds/metal_crash.wav -q &");
    playSound = 0;
    game_state_ = collision_controller_.CrashAnimationCliff(&camera_, terrain_, car_, delta_time_, is_key_pressed_hash_);
    return;
  }

  if (is_key_pressed_hash_.at('w') || is_key_pressed_hash_.at('s')
      || is_key_pressed_hash_.at('a') || is_key_pressed_hash_.at('d')) {
    game_state_ = kStart;
    playSound = 1;
  }
}

// The controllers camera update tick
//   Uses car position (for chase and 1st person view)
//   and checks keypresses for freeview
//   @warn should be called before car movement
void Controller::UpdateCamera() {
  // CAMERA CONTROLS
  // Freeview movement
  camera_.Movement(delta_time_, is_key_pressed_hash_); 
  // Point at car
  camera_.UpdateCarTick(car_);
  // Update camera lookAt
  camera_.UpdateCamera();

}

// The controllers physics update tick
//   Checks keypresses and calculates acceleration
void Controller::UpdatePhysics() {

  if (game_state_ == kStart) {
    // Sets variables required for camera
    //  i.e. camera has access to car in UpdateCarTick(car_)
    car_->ControllerMovementTick(delta_time_, is_key_pressed_hash_);
  }

  if (game_state_ == kAutoDrive) {
    collision_controller_.AutoDrive(car_, delta_time_);
  }

}
