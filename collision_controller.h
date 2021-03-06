#ifndef ASSIGN3_COLLISION_CONTROLLER_H_
#define ASSIGN3_COLLISION_CONTROLLER_H_

#include <string>
#include <vector>
#include <cassert>
#include <cstdlib>
#include "camera.h"
#include "terrain.h"
#include "object.h"
#include "roadsign.h"

#include "constants.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/vector_angle.hpp"

#ifdef __APPLE__
#include <GLUT/glut.h> 
#else
#include <GL/glut.h>
#endif

class CollisionController {
  public:
    CollisionController();

    // TODO comment
    kGameState UpdateCollisions(
        const Object * car_, Terrain * terrain_,
        Camera * camera_, RoadSign * road_sign,
        kGameState current_state);

    // TODO comment
    void AutoDrive(Object * car, const float delta_time);

    // ANIMATION FUNCTIONS
    // The animation played when the car falls off the right (water) side
    //   Is calculated using the vertices stored by terrain
    //   Finds the closest vertice to car and doesn't allow it to go below it
    //   Once complete resets the state to kAutoDrive
    // @warn Pretty inefficent way of checking for collisions but it's only
    //       calculated during this state.
    kGameState CrashAnimationFall(
        Camera * camera_, const Terrain * terrain_, Object * car_,
        float delta_time_, const std::vector<bool> &is_key_pressed_hash_);

    // The animation played when the car drives off the road on left (cliff) side
    //   Is calculated using 1 row of vertices stored by terrain
    //   Finds the closest vertice to car and calculates determinate, when determinate
    //     of middle of road and car position are opposite there is a colisn
    //   A recovery occurs when both determinates are opposite but closest dis is too far
    // @warn This collision can fall through if delta time makes the car velocity larger
    //       than the catch dis, may be an issue for systems with poor performance
    kGameState CrashAnimationCliff(
        Camera * camera_, const Terrain * terrain_, Object * car_,
        float delta_time_, const std::vector<bool> &is_key_pressed_hash_);

    // TODO comment
    inline bool is_collision() const;

  private:
    // The users camera state (for animations)
    Camera::State camera_state_;
    // The state of the previous collision tick
    bool is_collision_;
    // The midpoint of the road where the car is
    glm::vec3 left_lane_midpoint_;
    // The previous midpoint, updated during autodrive
    glm::vec3 prev_left_lane_midpoint_;
    // The direction vector of the road where the car is
    glm::vec3 road_direction_;
    // The rotation of the road where the car is
    float road_y_rotation_;
    // The angle of the car and the direction of road
    //   Angle is clockwise from facing of road
    //   This includes the cars centripetal direction
    float car_angle_;
    // Has the car hit the right side cliff yet?
    //   Used in left (cliff) animation
    bool is_cliff_hit_;
    // Used in left (cliff) animation for collision detection using determinate
    bool is_prev_positive_;
    // The impact speed of the car for the left (cliff) side animation
    float impact_speed_;

    // TODO comment
    unsigned char prev_colisn_pair_idx_;

    // TODO comment
    float colisn_anim_ticks_;

    // COLLISION HELPERS
    // Checks whether car is between boundary pair
    //   Creates 4 triangles out of the 4 points of the given square and returns 
    //   @param car, the car object (to find it's position)
    //   @param bp, 2x pairs (ie. 2x2 points), each pair is the horizontal bound
    //   @return true if car is inside corner of rectangle
    //   @warn input must be square for accurate results
    bool IsInside(const glm::vec3 &car, const std::pair<Terrain::boundary_pair,Terrain::boundary_pair> &bp);
    // Checks whether car is between a pair of points
    //   @param car, the car vec3 (to find it's position)
    //   @param bp, 2x pairs (ie. 2x2 points), each pair is the horizontal bound
    //   @return true if car is within the points
    bool IsInside(const glm::vec3 &car, const std::pair<glm::vec3,glm::vec3> &bp);

};

// TODO comment
inline bool CollisionController::is_collision() const {
  return is_collision_;
}

#endif
