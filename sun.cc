#include "sun.h"

Sun::Sun(const Camera * camera, const bool is_debug) :
  // Initialize Constants
  kMinHeight(0.0f),
  kMaxHeight(20.0f),
  kMornPos(30.0f),
  kDawnPos(-40.0f),
  kSmoothDuration(150), //Must be less than kTicksPerHour
  kTicksPerHour(200),
  kHeightMove((kMaxHeight-kMinHeight)/6.0f/kSmoothDuration),
  kHorizontalMove((kDawnPos-kMornPos)/12.0f/kSmoothDuration),
  // Lighting constants
  kDiffuseGreenPercentageGrow(5),
  kDiffuseBluePercentageGrow(4),
  kMorningStart(6),
  kMorningEnd(9),
  kDawnStart(18),
  kDawnEnd(21),
  kMinRedning(glm::vec3(255.0f,253.0f,214.0f)),
  kMaxRedning(glm::vec3(255.0f,127.0f,112.0f)),
  // Default vars
  time_of_day_(6),
  sun_start_(camera->cam_pos().x + kMornPos),
  sun_target_x_(camera->cam_pos().x),
  sun_target_z_(camera->cam_pos().z),
  sun_height_((kMaxHeight-kMinHeight)/6.0f),
  smooth_tick_count_(kSmoothDuration + 1),
  hours_tick_count_(0),
  sun_diffuse_(kMaxRedning),
  // The camera
  camera_(camera),
  is_debugging_(is_debug) {

}

// TODO
void Sun::Update() {
  SmoothPosition();
  if (hours_tick_count_ < kTicksPerHour) {
    ++hours_tick_count_;
  } else {
    hours_tick_count_ = 0;
    UpdateHour();
  }
}

// TODO
void Sun::UpdateHour() {
  time_of_day_ = (time_of_day_+1) % 24;
  smooth_tick_count_ = 0;

  // Update the ambient and diffuse of the Sun/Moon


  // Update next position adders
  next_sun_start_ = kHorizontalMove;
  if (time_of_day_ == 0 || time_of_day_ == 12) // Reset once reach horizon
    sun_start_ = camera_->cam_pos().x + kMornPos;
  next_sun_target_x_ = (camera_->cam_pos().x-sun_target_x_) / kSmoothDuration;
  next_sun_target_z_ = (camera_->cam_pos().z-sun_target_z_) / kSmoothDuration;
  if (time_of_day() < 6)
    next_sun_height_ = -kHeightMove;
  else if (time_of_day() < 12)
    next_sun_height_ = kHeightMove;
  else if (time_of_day() < 18)
    next_sun_height_ = -kHeightMove;
  else if (time_of_day() < 24)
    next_sun_height_ = kHeightMove;

  if (is_debugging_) {
    printf("Updating Hour = %d\n", time_of_day());
    printf("Current sun:\nstart = %f\ntarget_x = %f\ntarget_z = %f\nheight = %f\n", sun_start_, sun_target_x_, sun_target_z_, sun_height_);
  }
}

// Smooth the positions over kSmoothDuration
void Sun::SmoothPosition() {
  if (smooth_tick_count_ < kSmoothDuration) {
    sun_start_ += next_sun_start_;
    sun_target_x_ += next_sun_target_x_;
    sun_target_z_ += next_sun_target_z_;
    sun_height_ += next_sun_height_;

    ++smooth_tick_count_;
  }
}

glm::vec3 Sun::sun_direction() const {
  glm::vec3 dir = glm::normalize(glm::vec3(
        sun_target_x_-sun_start_,
        -10.0f - sun_height_,
        0));
  return dir;
}

// TODO
glm::vec3 Sun::sun_diffuse() const {
  glm::vec3 converted = sun_diffuse_;
  converted /= 255.0f;
  return converted;
}
