#include "object.h"

// Construct with position setting parameters
Object::Object(const glm::vec3 &translation,
               const glm::vec3 &rotation,
               const glm::vec3 &scale,
               float default_speed, bool debugging_on)
  : kDefaultHeight(translation.y),
  translation_(translation), rotation_(rotation), scale_(scale),
  displacement_(0), speed_(default_speed), centri_speed_(0), default_speed_(default_speed), 
  centripeta_velocity_x_(0.0f), centripeta_velocity_z_(0.0f),
  velocity_x_(0.0f), velocity_z_(0.1f), //Not zero for starting camera (pos behind car)
  is_debugging_(debugging_on) {
    UpdateModelMatrix();
  }

// Works out the maximum speed achieveable per gear
//   @param  the gear ratio
//   @return  the max speed of given gear ratio
float Object::MaxSpeedPerGear(float g_num) {
  const float DIFFERENTIALRATIO = 3.42;
  // const float TRANSMISSIONEFFICIENCY = 0.7;
  // const float WHEELRADIUS = 0.33; //metres  radius of tire
  const float WHEELROTATIONDISTANCE = 2.14; //metres
  const float MAXRPM = 4400;
  float MAXSPEEDCONVERT = MAXRPM/g_num/DIFFERENTIALRATIO/60*WHEELROTATIONDISTANCE*3.6;
  return MAXSPEEDCONVERT;
}

// Works out the maximum force per gear
//   @param  the gear ratio
//   @return  the maximum force per gear
float Object::MaxEngineForcePerGear(float g_num, float max_torque) {
  // float MAXTORQUE = 475; //N.m  depending on MAXRPM
  const float MAXTORQUE = max_torque; //N.m  depending on MAXRPM
  const float DIFFERENTIALRATIO = 3.42, TRANSMISSIONEFFICIENCY = 0.7;
  const float WHEELRADIUS = 0.33; //metres  radius of tire
  float ENGINEFORCE = MAXTORQUE*g_num*DIFFERENTIALRATIO*TRANSMISSIONEFFICIENCY/WHEELRADIUS;
  return ENGINEFORCE;
}

// Updates the all the movement data for the object
// @warn should be called in controller tick
void Object::ControllerMovementTick(float delta_time_in, const std::vector<bool> &is_key_pressed_hash) {
  // Convert delta_time to ticks per second
  //   Currently ticks per milisecond
  const float delta_time = delta_time_in / 1000;
  float TURNRATE = 100 * delta_time;
  float ENGINEFORCE = 9000; //newtons (real 1st Gear value is 9000)
  // TODO put into separate constants class
  const float MASS = 1500; //kg
  const float BRAKINGFORCE = ENGINEFORCE * 5; //newtons
  const float AIRRESSISTANCE = 0.4257;  //proportional constant
  const float FRICTION = AIRRESSISTANCE * 30;
  const float SPEEDSCALE = 10; //the conversions from real speed to game movement
  const float WEIGHT = MASS * 9.8; // m * g
  const float LENGTH = 4.8; //metres  length of car
  const float HEIGHT = 0.7; //metres  height of CG (centre of gravity)

  // GEAR RATIOS
  //   @warn dummy gear[0]
  const float GEAR_RATIOS[] = { -1, 2.66, 1.78, 1.30, 1.00, 0.74, 0.50 };
  // Max torque per gear
  //   @warn dummy gear[0]
  const float GEAR_TORQUE[] = { -1, 4400, 2900, 2200, 1650, 1250, 700 };

  // SETUP VARS
  // Used to update camera
  displacement_ = glm::vec3(0,0,0);
  // The current force added to velocity
  float force_x = 0;
  float force_z = 0;
  // The current velocity vector
  float direction_x = direction().x;
  float direction_z = direction().z;
  velocity_x_ = speed_ * direction_x;
  velocity_z_ = speed_ * direction_z;

  if (is_key_pressed_hash.at('h'))
  {
    system("aplay ./sounds/car_horn_final.wav -q&");
  }

  if (is_key_pressed_hash.at('w')) {
    // simulated gear shifting
    // real car physics use rpm
    if (speed() < MaxSpeedPerGear(GEAR_RATIOS[1])) {
      ENGINEFORCE = MaxEngineForcePerGear(GEAR_RATIOS[1], GEAR_TORQUE[1]);
    } else if (speed() < MaxSpeedPerGear(GEAR_RATIOS[2])) {
      ENGINEFORCE = MaxEngineForcePerGear(GEAR_RATIOS[2], GEAR_TORQUE[2]);
    } else if (speed() < MaxSpeedPerGear(GEAR_RATIOS[3])) {
      ENGINEFORCE = MaxEngineForcePerGear(GEAR_RATIOS[3], GEAR_TORQUE[3]);
    } else if (speed() < MaxSpeedPerGear(GEAR_RATIOS[4])) {
      ENGINEFORCE = MaxEngineForcePerGear(GEAR_RATIOS[4], GEAR_TORQUE[4]);
    } else if (speed() < MaxSpeedPerGear(GEAR_RATIOS[5])) {
      ENGINEFORCE = MaxEngineForcePerGear(GEAR_RATIOS[5], GEAR_TORQUE[5]);
    } else {
      ENGINEFORCE = MaxEngineForcePerGear(GEAR_RATIOS[6], GEAR_TORQUE[6]);
    }
    // if (is_debugging_) {
    //   printf("ENGINEFORCE = %f\n",ENGINEFORCE);
    // }
    force_x += ENGINEFORCE * direction_x;
    force_z += ENGINEFORCE * direction_z;
    // TODO switch gear shift different accelerations
  }
  if (is_key_pressed_hash.at('s') && speed() > 0) {
    force_x -= BRAKINGFORCE * direction_x;
    force_z -= BRAKINGFORCE * direction_z;
  }

  // Rolling resistance (friction of tires)
  force_x -= AIRRESSISTANCE * velocity_x_ * speed_;
  force_z -= AIRRESSISTANCE * velocity_z_ * speed_;
  // Air resistance x
  force_x -= FRICTION * velocity_x_;
  force_z -= FRICTION * velocity_z_;

  // CALCULATE ACCELERATION => a = F/M
  float acceleration_x = force_x / MASS;
  float acceleration_z = force_z / MASS;
  float acceleration_combined = sqrt(acceleration_x * acceleration_x + acceleration_z * acceleration_z);
  if (acceleration_x * direction_x < 0 && acceleration_z * direction_z < 0) {
    acceleration_combined = -acceleration_combined;
    acceleration_combined /= 40/speed_; //reduce braking pitch
  }
  if (is_debugging_) {
    printf("forward acceleration magnitude = %f\n",acceleration_combined);
  }

  // CALCULATE CENTRE OF GRAVITY (PITCH OF CAR)
  float weight_rear = 0.5*WEIGHT + HEIGHT/LENGTH*MASS*acceleration_combined;
  float pitch = WEIGHT/2 - weight_rear;
  pitch /= WEIGHT/2; //normalize pitch [-1,1]
  if (is_debugging_)
    printf("pitch = %f\n", pitch);
  set_rotation(glm::vec3(pitch, rotation().y, rotation().z));

  // CALCULATE WHEEL SPINS
  if (is_key_pressed_hash.at('w')) {
    float force_combined = sqrt(force_x * force_x + force_z * force_z);
    if (force_combined > weight_rear * 4.0) {
      // TODO SPIN ANIMATION AND SOUND
      //
      if (is_debugging_) {
        printf("SPIN SPIN WHEELS SPIN\n");
      }

      // CALCULATE NON-SPINNING ACCELERATION
      force_x = weight_rear * 4.0 * direction_x;
      force_z = weight_rear * 4.0 * direction_z;
      acceleration_x = force_x / MASS;
      acceleration_z = force_z / MASS;
    }
  }

  // CALCULATE CENTRIPETAL FORCE (Add to the centri velo)
  float v = sqrt(velocity_x_*velocity_x_ + velocity_z_*velocity_z_);
  if (v < 45) {
    v = 45;
  }
  float dt = delta_time; //milisecond per tick
  dt = 1/delta_time; //tick per milisecond
  dt /= 1000; //tick per second
  float w = TURNRATE * dt; //radians per second
  float r = v/w;
  // printf("r = %f\n",r);
  float a = v*v/r;
  if (is_debugging_) {
    printf("centripetal acceleration = %f\n",a);
  }
  // Increase drifitng on W (oversteer)
  if (is_key_pressed_hash.at('w')) {
    a *= 3;
    // Decrease drifinting on S (understeer)
  } else if (is_key_pressed_hash.at('s')) {
    a *= -1;
  }

  // APPLY CENTRIPETAL FORCE AND ROLL CAR
  float centripeta_acc_x = 0.0f;
  float centripeta_acc_z = 0.0f;
  bool is_turn = false;
  if (speed() > 0) {
    if (is_key_pressed_hash.at('a')) {
      const float rot = 30*TURNRATE/v;
      glm::vec3 centre_of_circle_vector = glm::cross(glm::vec3(direction_x,0.0f,direction_z),glm::vec3(0.0f,1.0f,0.0f));
      centre_of_circle_vector = glm::normalize(centre_of_circle_vector);
      centri_speed_ += a;
      centripeta_acc_x = centre_of_circle_vector.x * a;
      centripeta_acc_z = centre_of_circle_vector.z * a;
      const float z_rot = centri_speed_ * delta_time / 3;
      set_rotation(glm::vec3(rotation().x, rotation().y + rot, z_rot));
      is_turn = true;
    }
    if (is_key_pressed_hash.at('d')) {
      const float rot = 30*TURNRATE/v;
      glm::vec3 centre_of_circle_vector = glm::cross(glm::vec3(direction_x,0.0f,direction_z),glm::vec3(0.0f,1.0f,0.0f));
      centre_of_circle_vector = glm::normalize(centre_of_circle_vector);
      a *= -1;
      centri_speed_ += a;
      centripeta_acc_x = centre_of_circle_vector.x * a;
      centripeta_acc_z = centre_of_circle_vector.z * a;
      const float z_rot = centri_speed_ * delta_time / 3;
      set_rotation(glm::vec3(rotation().x, rotation().y - rot, z_rot));
      is_turn = true;
    }
  }
  // GRADUAL CENTRIPETAL SPEED RELAX
  if (!is_turn) {
    // centripeta_velocity_x_ *= 124.8f * delta_time; //122.5f comes from my (Konrads) laptop .98f (98%, i.e. 2% decrease per tick)
    // centripeta_velocity_z_ *= 124.8f * delta_time;
    centri_speed_ *= 0.9f;
    const float z_rot = centri_speed_ * delta_time / 3;
    set_rotation(glm::vec3(rotation().x, rotation().y, z_rot));
    centripeta_velocity_x_ *= 0.9f;
    centripeta_velocity_z_ *= 0.9f;
  }
  if (is_debugging_)
    printf("centri_speed_ = %f\n",centri_speed_*delta_time);

  // CALCULATE VELOCITY => v = v+dt*a 
  velocity_x_ += delta_time * acceleration_x;
  velocity_z_ += delta_time * acceleration_z;

  // CALCULATE SPEED
  if (velocity_x_ * direction_x < 0 && velocity_z_ * direction_z < 0) {
    speed_ = 0.0f;
  } else {
    speed_ = sqrt(velocity_x_ * velocity_x_ + velocity_z_ * velocity_z_);
    centripeta_velocity_x_ += delta_time * centripeta_acc_x;
    centripeta_velocity_z_ += delta_time * centripeta_acc_z;
    velocity_x_ += centripeta_velocity_x_;
    velocity_z_ += centripeta_velocity_z_;
  }
  if (is_debugging_) {
    printf("speed = %f\n", speed_);
  }
  // convert speed to game world speed
  // TODO put into separate constants class
  velocity_x_ /= SPEEDSCALE;
  velocity_z_ /= SPEEDSCALE;

  // CALCULATE NEW POSITION => p = p+dt*v
  translation_.x += delta_time * velocity_x_;
  translation_.z += delta_time * velocity_z_;
  displacement_.x += delta_time * velocity_x_;
  displacement_.z += delta_time * velocity_z_;

  if (speed() == 0) {
    velocity_x_ = direction_x;
    velocity_z_ = direction_z;
  }
}

// Updates the transform matrix using glLookAt
//  Should be called everytime pos,dir or up changes (but can be optimized to be only called once)
void Object::UpdateModelMatrix() {

  // Scale of object
  glm::mat4 scale = glm::scale(  glm::mat4(1.0f), 
      glm::vec3(scale_.x, scale_.y, scale_.z));

  // Rotation of object - @warn order of rotation matters
  glm::mat4 rotate = glm::mat4(1.0f);
  rotate = glm::rotate(rotate, rotation_.y, glm::vec3(0, 1, 0));
  rotate = glm::rotate(rotate, rotation_.x, glm::vec3(1, 0, 0));
  rotate = glm::rotate(rotate, rotation_.z, glm::vec3(0, 0, 1));

  // Translation of object
  glm::mat4 translate = glm::translate(  glm::mat4(1.0f), 
      glm::vec3(translation_.x, translation_.y, translation_.z));

  model_matrix_ = translate * rotate * scale;
}

// Accessor for the direction vector
//   @warn the roll (z rotation) is not calculated
glm::vec3 Object::direction() const {
  glm::vec3 direction = glm::vec3(
      sin(DEG2RAD(rotation().y)),  //dir.x
      -sin(DEG2RAD(rotation().x)), //dir.y
      cos(DEG2RAD(rotation().y))); //dir.z
  direction = glm::normalize(direction);
  return direction;
}
