#ifndef ASSIGN3_SUN_H_
#define ASSIGN3_SUN_H_

#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include "camera.h"

#include "constants.h"

#include "glm/glm.hpp"
#include <GL/glew.h>
#include "shaders/shader_compiler/shader.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/vector_angle.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h> 
#else
#include <GL/glut.h>
#endif

// Calculates and moves the Sun/Moons position for
//   Shadows and appropriate lighting
class Sun {
  public:
    Sun(const Camera * camera, const bool is_debug = false);

    // TODO
    void Update();

    // TODO
    inline char time_of_day() const;
    // TODO
    inline float sun_start() const;
    // TODO
    inline float sun_height() const;
    // TODO
    inline float sun_target_x() const;
    // TODO
    inline float sun_target_z() const;

    // TODO
    glm::vec3 sun_direction() const;
    // TODO
    glm::vec3 sun_diffuse() const;

  private:
    // SUN/MOON MOVE POSITIONAL CONSTANTS
    // The min/max height of the Sun/Moon
    const float kMinHeight;
    const float kMaxHeight;
    // The morning/dawn position of Sun/Moon
    //   It's the position of start/end of
    //   horizon.
    const float kMornPos;
    const float kDawnPos;
    // The amount of ticks to smooth over
    //   to reach next position
    const int kSmoothDuration;
    // The amount of ticks before the hour
    //   gets updated
    const int kTicksPerHour;
    // The Sun/Moon transition amounts
    const float kHeightMove;
    const float kHorizontalMove;
    // LIGHTING CONSTANTS
    // Morning/Dawning constants
    // The growing/shrinking percentage per hour
    const float kDiffuseGreenPercentageGrow;
    const float kDiffuseBluePercentageGrow;
    // Amount of hours to grow/shrink
    //   i.e. morning/dawning range
    const char kMorningStart;
    const char kMorningEnd;
    const char kDawnStart;
    const char kDawnEnd;
    // Min/Max Diffuse
    const glm::vec3 kMinRedning;
    const glm::vec3 kMaxRedning;

    // The time of day
    //   No. is in range [0,24)
    //   [6,18) is Daytime (sun)
    //   [0,6) U [18,24) is Nighttime (moon)
    char time_of_day_;
    // Sun position vars
    float sun_start_;
    float sun_target_x_;
    float sun_target_z_;
    float sun_height_;
    // The amount to move to reach next_
    //   by the kSmoothDuration amount
    float next_sun_start_;
    float next_sun_end_;
    float next_sun_target_x_;
    float next_sun_target_z_;
    float next_sun_height_;
    // Tick counter to reach kSmoothDuration
    int smooth_tick_count_;
    // Tick counter to reach kTicksPerHour
    int hours_tick_count_;

    // Sun color
    glm::vec3 sun_ambient_;
    glm::vec3 sun_diffuse_;

    // The camera object (for target pos)
    const Camera * camera_;

    // Verbose debugging mode
    const bool is_debugging_;

    // TODO
    void UpdateHour();
    // Smooth the positions over kSmoothDuration
    void SmoothPosition();
};

// TODO
inline char Sun::time_of_day() const {
  return time_of_day_;
}
// TODO
inline float Sun::sun_start() const {
  return sun_start_;
}
// TODO
inline float Sun::sun_height() const {
  return sun_height_;
}
// TODO
inline float Sun::sun_target_x() const {
  return sun_target_x_;
}
// TODO
inline float Sun::sun_target_z() const {
  return sun_target_z_;
}

#endif
