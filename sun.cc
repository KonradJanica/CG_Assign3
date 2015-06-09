#include "sun.h"

Sun::Sun(const Camera * camera, const bool is_debug) :
  // Initialize Constants
  kMinHeight(10.0f),
  kMaxHeight(30.0f),
  kMornPos(30.0f),
  kDawnPos(-40.0f),
  kSmoothDuration(200), //Must be less than kTicksPerHour
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
  kIntensityPercentageGrow(1.0f / 6.0f / kSmoothDuration),
  // Default vars
  time_of_day_(6),
  sun_start_(camera->cam_pos().x + kMornPos),
  sun_start_displacement_(sun_start_),
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
  // next_sun_start_ = ((camera_->cam_pos().x+kMornPos)+sun_start_displacement_) / 2;
  // next_sun_start_ = kHorizontalMove;
  // next_sun_start_ =  kHorizontalMove;
  if (time_of_day_ == 6 || time_of_day_ == 18) { // Reset once reach horizon
    sun_start_displacement_ = camera_->cam_pos().x + kMornPos;
    // sun_start_ = sun_start_displacement_;
    // sun_start_ = camera_->cam_pos().x + kMornPos;
  }
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
    // sun_start_ += next_sun_start_;
    sun_start_ = sun_start_displacement_ + kHorizontalMove * kSmoothDuration;
    sun_start_displacement_ = (camera_->cam_pos().x + kMornPos + sun_start_displacement_) / 2.0f;
    sun_target_x_ += next_sun_target_x_;
    sun_target_z_ += next_sun_target_z_;
    sun_height_ += next_sun_height_;

    ++smooth_tick_count_;
  }
}

glm::vec3 Sun::sun_direction() const {
  glm::vec3 dir = glm::normalize(glm::vec3(
        sun_target_x_-sun_start_,
        -10.0f - (sun_height_ - 50.0f), //-30 for cool effect on diffuse light
        0));
  dir *= -1;
  return dir;
}

// TODO
glm::vec3 Sun::sun_diffuse() const {
  glm::vec3 converted = sun_diffuse_;
  converted /= 255.0f;
  return converted;
}

// TODO
bool Sun::IsDay() const {
  return (time_of_day() >= 6 && time_of_day() < 18);
}

// TODO
float Sun::LightIntensityMultiplier() const {
  float multi;
  const float current_smooth_tick_intensity =
    kIntensityPercentageGrow * (smooth_tick_count_ + 0);
  switch(time_of_day()) {
    // Going up
    case 11:
    case 23:
      multi = 1.0f;
      break;
    case 10:
    case 22:
      multi = (5.0f/6.0f + current_smooth_tick_intensity);
      break;
    case 9:
    case 21:
      multi = (4.0f/6.0f + current_smooth_tick_intensity);
      break;
    case 8:
    case 20:
      multi = (3.0f/6.0f + current_smooth_tick_intensity);
      break;
    case 7:
    case 19:
      multi = (2.0f/6.0f + current_smooth_tick_intensity);
      break;
    case 6:
    case 18:
      multi = (1.0f/6.0f + current_smooth_tick_intensity);
      break;
    // Going down
    case 12:
    case 0:
      multi = (1.0f - current_smooth_tick_intensity);
      break;
    case 13:
    case 1:
      multi = (5.0f/6.0f - current_smooth_tick_intensity);
      break;
    case 14:
    case 2:
      multi = (4.0f/6.0f - current_smooth_tick_intensity);
      break;
    case 15:
    case 3:
      multi = (3.0f/6.0f - current_smooth_tick_intensity);
      break;
    case 16:
    case 4:
      multi = (2.0f/6.0f - current_smooth_tick_intensity);
      break;
    case 17:
    case 5:
      multi = (1.0f/6.0f - current_smooth_tick_intensity);
      break;

    default:
      multi = 1.0f;
      if (is_debugging_)
        printf("LIGHTING SMOOTHING SWITCH FAIL IN SUN\n");
  }

  if (!IsDay()) // less lighting at night
    multi *= 0.8f;
  // printf("mult = %f\n",multi);

  return multi;
}
