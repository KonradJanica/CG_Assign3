#include "collision_controller.h"

// Constructor, initializes an empty axis coordinate VAO to optimize Render()
//   Allows for Verbose Debugging Mode
//   @param bool debug_flag, true will enable verbose debugging
//   @warn assert will end program prematurely
CollisionController::CollisionController() :
  // State and var defaults
  is_collision_(false),
  road_y_rotation_(0) {

    // Initialize Dummy Index for first equilavence check in collisions
    prev_colisn_pair_idx_ = 0;

}

// The animation played when the car drives off the road on left (cliff) side
//   Is calculated using 1 row of vertices stored by terrain
//   Finds the closest vertice to car and calculates determinate, when determinate
//     of middle of road and car position are opposite there is a colisn
//   A recovery occurs when both determinates are opposite but closest dis is too far
// @warn This collision can fall through if delta time makes the car velocity larger
//       than the catch dis, may be an issue for systems with poor performance
kGameState CollisionController::CrashAnimationCliff(
    Camera * camera_, const Terrain * terrain_, Car * car_,
    float delta_time_, const std::vector<bool> &is_key_pressed_hash_) {

  // Lock camera state
  // camera_.ChangeState(Camera::kFreeView);
  const glm::vec3 &car_pos = car_->translation();

  // Calc next movement (avoid going through)
  glm::vec3 dir = car_->direction();
  float dt = delta_time_ / 1000;
  float scale = 10.0f;
  // Move car in its direction
  float vel_x = dir.x * car_->speed();
  vel_x += car_->centripeta_velocity_x();
  vel_x /= scale;
  float x_pos = car_pos.x;
  x_pos += vel_x * dt;
  float vel_z = dir.z * car_->speed();
  vel_z += car_->centripeta_velocity_z();
  vel_z /= scale;
  float z_pos = car_pos.z;
  z_pos += vel_z * dt;
  float y_pos = car_pos.y + dir.y * car_->speed()/10.0f*dt;
  if (y_pos < 0.3f)
    y_pos = 0.3f;

  float dis = 0.0f;

  // Check for collision
  if (!is_cliff_hit_) {
    typedef Terrain::anim_vec w_vec;
    typedef Terrain::anim_container w_list;
    const w_list * cliff = terrain_->colisn_lst_cliff();
    glm::vec3 closest_vertice = cliff->begin()->at(0);
    glm::vec3 snd_clst_vertice = cliff->begin()->at(1);
    dis = glm::distance(glm::vec2(closest_vertice.x,closest_vertice.z),glm::vec2(x_pos,z_pos));
    float snd_dis = glm::distance(glm::vec2(snd_clst_vertice.x,snd_clst_vertice.z),glm::vec2(x_pos,z_pos));

    w_list::const_iterator it = cliff->begin();
    for (unsigned int i = 0; i < 2; ++i) {

      const w_vec &vertices = *it;
      for (w_vec::const_iterator x = vertices.begin()+2; 
          x != vertices.end(); ++x) {

        const glm::vec2 cur_vec = glm::vec2(x->x,x->z); // current vector 
        float cur_dis = glm::distance(cur_vec, glm::vec2(x_pos, z_pos));
        if (cur_dis < dis) {
          snd_dis = dis;
          dis = cur_dis;
          snd_clst_vertice = closest_vertice;
          closest_vertice = *x;
        } else if (cur_dis < snd_dis && *x != closest_vertice) {
          snd_dis = cur_dis;
          snd_clst_vertice = *x;
        }
      }
      it++;
    }
    // printf("dis=%f\n",dis);

    // Check for collision on closest 2 points
    glm::vec3 a, b;
    a = closest_vertice;
    b = snd_clst_vertice;
    float road_det = (b.x-a.x)*(left_lane_midpoint_.z-a.z)-(b.z-a.z)*(left_lane_midpoint_.x-a.x);
    bool road_sign = true; //true for positive or 0
    if (road_det < 0)
      road_sign = false;
    // printf("road det = %f\n", road_det);
    float det = (b.x-a.x)*(z_pos-a.z)-(b.z-a.z)*(x_pos-a.x);
    if (det == 0) {
      // printf("a = (%f,%f,%f)\n",a.x,a.y,a.z);
      // printf("b = (%f,%f,%f)\n",b.x,b.y,b.z);
      is_cliff_hit_ = true;
    }
    float det_sign = true;
    if (det < 0)
      det_sign = false;
    if (det_sign != road_sign) {
      is_cliff_hit_ = true;
      impact_speed_ = car_->speed();
    }
    // printf("car det = %f\n", det);
    // printf("mp = (%f,%f)\n", left_lane_midpoint_.x, left_lane_midpoint_.z);
  }

  // printf("dis = %f\n", dis);
  // Give second chance
  if (is_cliff_hit_ && dis > 0.1f
      || colisn_anim_ticks_ > 10 && !is_cliff_hit_ && dis > 0.50f) {
    // printf("CRUDE RECOVERY (IE BUGGY 2ND CHANCE)\n");
    // Reset game state
    is_collision_ = false;
    camera_->ChangeState(camera_state_); // users previous camera
    return kGameState::kStart;
  }

  if (!is_cliff_hit_) {
    // No collision yet => move car towards cliff
    car_->set_translation(glm::vec3(x_pos, y_pos, z_pos));
    // car_->ReduceSpeed(car_->speed() * car_->speed()/100.0f*dt);
    // car_->ReduceCentriVelocity(50);
  } else {
    // Collided crash into wall animation
    // Move car in roads direction
    float dt = delta_time_ / 1000;
    // glm::vec3 cliff_dir = closest_vertice - snd_clst_vertice;
    glm::vec3 cliff_dir = road_direction_;
    float velocity_x = road_direction_.x * car_->speed()/10.0f*dt;
    float velocity_z = road_direction_.z * car_->speed()/10.0f*dt;
    velocity_x = cliff_dir.x * car_->speed()/10.0f*dt;
    velocity_z = cliff_dir.z * car_->speed()/10.0f*dt;
    float x_pos = car_->translation().x + velocity_x;
    float y_pos = car_->translation().y - dir.y * car_->speed()/30.0f*dt;
    float z_pos = car_->translation().z + velocity_z;
    car_->set_translation(glm::vec3(x_pos, y_pos, z_pos));
    if (car_->speed() > 10.0f) {
      car_->ReduceSpeed(car_->speed() * car_->speed()/10.0f*dt);
      car_->ReduceCentriVelocity(0); //0 side speed
    } else {
      car_->ReduceSpeed(car_->speed()); //0 speed
    }
    // Rotation
    if (car_->speed() > impact_speed_ / 1.8f) {
      float viol = car_->speed() / 600; //violence factor
      float x_rot = car_->speed() * viol;
      float y_rot = car_->rotation().y;
      float z_rot = car_->speed() * viol;
      car_->set_rotation(glm::vec3(x_rot, y_rot, z_rot));
      // printf("x_rot = %f, z_rot = %f\n",x_rot, z_rot);
    }
    // printf("spe = %f\n",car_->speed());
  }

  // Restart game
  if (colisn_anim_ticks_ > 400 && is_cliff_hit_) {
    if (is_key_pressed_hash_.at('w') || is_key_pressed_hash_.at('s')
        || is_key_pressed_hash_.at('a') || is_key_pressed_hash_.at('d')
        || colisn_anim_ticks_ > 2000) {
      // Reset car
      car_->set_translation(glm::vec3(left_lane_midpoint_.x, car_->kDefaultHeight, left_lane_midpoint_.z));
      car_->set_rotation(glm::vec3(0.0f,road_y_rotation_,0.0f));
      car_->ResetPhysics();
      // Reset game state
      is_collision_ = false;
      camera_->ChangeState(camera_state_); // users previous camera
      return kGameState::kAutoDrive;
    }
  }
  colisn_anim_ticks_ += delta_time_ / 5;
  // printf("tm = %f\n", colisn_anim_ticks_);
  return kGameState::kCrashingCliff;
}

// The animation played when the car falls off the right (water) side
//   Is calculated using the vertices stored by terrain
//   Finds the closest vertice to car and doesn't allow it to go below it
//   Once complete resets the state to kAutoDrive
// @warn Pretty inefficent way of checking for collisions but it's only
//       calculated during this state.
kGameState CollisionController::CrashAnimationFall(
    Camera * camera_, const Terrain * terrain_, Car * car_,
    float delta_time_, const std::vector<bool> &is_key_pressed_hash_) {

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
  float scale = 10.0f;
  float vel_x = dir.x * car_->speed();
  vel_x += car_->centripeta_velocity_x();
  vel_x /= scale;
  float x_pos = car_->translation().x;
  x_pos += vel_x * dt;
  float y_pos = car_->translation().y + dir.y * car_->speed()/10.0f*dt;
  float vel_z = dir.z * car_->speed();
  vel_z += car_->centripeta_velocity_z();
  vel_z /= scale;
  float z_pos = car_->translation().z;
  z_pos += vel_z * dt;

  typedef Terrain::anim_vec w_vec;
  typedef Terrain::anim_container w_list;
  const w_list * water = terrain_->colisn_lst_water();
  const w_list * cliff = terrain_->colisn_lst_cliff();
  glm::vec3 closest_vertice = cliff->begin()->at(0);
  glm::vec3 snd_clst_vertice = cliff->begin()->at(1);
  float dis = glm::distance(glm::vec2(closest_vertice.x,closest_vertice.z),glm::vec2(x_pos,z_pos));
  float snd_dis = glm::distance(glm::vec2(snd_clst_vertice.x,snd_clst_vertice.z),glm::vec2(x_pos,z_pos));

  w_list::const_iterator it = water->begin();
  for (unsigned int i = 0; i < 2; ++i) {

    const w_vec &vertices = *it;
    for (w_vec::const_iterator x = vertices.begin()+2;
        x != vertices.end(); ++x) {

      const glm::vec2 cur_vec = glm::vec2(x->x,x->z); // current vector 
      float cur_dis = glm::distance(cur_vec, glm::vec2(x_pos, z_pos));
      if (cur_dis < dis) {
        snd_dis = dis;
        dis = cur_dis;
        snd_clst_vertice = closest_vertice;
        closest_vertice = *x;
      } else if (cur_dis < snd_dis && *x != closest_vertice) {
        snd_dis = cur_dis;
        snd_clst_vertice = *x;
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
    // TODO this needs fixing I think (x_rot makes no sense)
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
    car_->ReduceCentriVelocity(50); //percent reduction
  }
  if (colisn_anim_ticks_ > 400) {
    if (is_key_pressed_hash_.at('w') || is_key_pressed_hash_.at('s')
        || is_key_pressed_hash_.at('a') || is_key_pressed_hash_.at('d')
        || colisn_anim_ticks_ > 2000) {
      // Reset car
      car_->set_translation(glm::vec3(left_lane_midpoint_.x, car_->kDefaultHeight, left_lane_midpoint_.z));
      car_->set_rotation(glm::vec3(0.0f,road_y_rotation_,0.0f));
      car_->ResetPhysics();
      // Reset game state
      is_collision_ = false;
      camera_->ChangeState(camera_state_); // users previous camera
      return kGameState::kAutoDrive;
    }
  }
  colisn_anim_ticks_ += delta_time_ / 5;
  // printf("tm = %f\n", colisn_anim_ticks_);
  return kGameState::kCrashingFall;
}

// Checks whether car is between the biggest rectangle than can be formed
//   @param car, the car object (to find it's position)
//   @param bp, 2x pairs (ie. 2x2 points), each pair is the horizontal bound
// @return  true  if can is inside corner of rectangle
// @warn input must be square for accurate results
bool CollisionController::IsInside(const glm::vec3 &car, const std::pair<Terrain::boundary_pair,Terrain::boundary_pair> &bp) {
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

// Checks whether car is between the biggest rectangle than can be formed
//   @param car, the car vec3 (to find it's position)
//   @param bp, 2x pairs (ie. 2x2 points), each pair is the horizontal bound
//   @return  true  if can is inside corner of rectangle
//   @warn input must be square for accurate results
bool CollisionController::IsInside(const glm::vec3 &car, const std::pair<glm::vec3,glm::vec3> &bp) {
  const glm::vec3 &a = bp.first;
  const glm::vec3 &b = bp.second;

  float dot_product = (car.x - a.x) * (b.x - a.x) + (car.z - a.z) * (b.z - a.z);
  if (dot_product < 0)
    return false;
  float squared_dis = (b.x - a.x) * (b.x - a.x) + (b.z - a.z) * (b.z - a.z);
  if (dot_product > squared_dis)
    return false;

  return true;
}

// TODO comment
//   Also calculates the middle of the road and it's direction if game state is autodrive
kGameState CollisionController::UpdateCollisions(
    const Car * car_, Terrain * terrain_,
    Camera * camera_, RoadSign * road_sign,
    kGameState current_state) {
  // Setup vars
  const circular_vector<Terrain::colisn_vec> * col = terrain_->colisn_boundary_pairs();
  const glm::vec3 &car = car_->translation();
  const Terrain::colisn_vec &head = col->front();
  Terrain::colisn_vec::const_iterator it = head.begin()+prev_colisn_pair_idx_;

  // Find closest edge point
  Terrain::boundary_pair closest_pair;
  float dis = FLT_MAX;
  Terrain::colisn_vec::const_iterator closest_it = head.begin();

  --prev_colisn_pair_idx_;
  while (it != head.end()) {
    const glm::vec3 &cur_vec = it->first; // current vector pair
    float cur_dis = glm::distance(cur_vec, car);
    if (cur_dis > dis) {
      break;
    } else {
      dis = cur_dis;
      closest_pair = *it;
      closest_it = it;
    }
    it++;
    ++prev_colisn_pair_idx_;
  }
  Terrain::boundary_pair next_pair;
  // Get vertice pair next to closest
  //   but make sure it isn't the last pair overwise pop
  it = closest_it;
  it++;
  // Reduce autodrive jerking
  if (it != head.end())
    it++;

  if (it == head.end()) {
    // Get next pair from next vector in circular_vector
    closest_it = (*col)[1].begin(); // reassign to find new midpoint etc.
    it = closest_it;
    it++; // We want next point (i.e. end-1 == begin)
    next_pair = *it;

    terrain_->colisn_pop();
    terrain_->ProceedTiles();
    road_sign->ShiftIndexes();
    // Try to spawn a road sign
    road_sign->SignSpawn();
    prev_colisn_pair_idx_ = 0;
  } else {

    // If conditions flow through then make box with neighbours of the closest pair
    //   and check if car is inside the box
    next_pair = *it;
  }
  // Make boundary box the neighbours of current pair

  // Check if car is in range
  if (IsInside(car, *closest_it)) {
    //inside bounds
    is_collision_ = false;
  } else {
    printf("collision on edge of road!\n");
    is_collision_ = true;

  }

  // Calculate middle of road and it's direction
  // Get the next points to smooth it
  it = closest_it;
  it++;
  // Find midpoint
  glm::vec3 road_midpoint = ((*it).first+(*it).second);
  road_midpoint /= 2;
  road_midpoint.y = car_->translation().y;
  // Find lane midpoints
  left_lane_midpoint_ = road_midpoint + (*it).second;
  left_lane_midpoint_ /= 2;
  // Find road direction
  glm::vec3 first_point = closest_it->first;
  glm::vec3 next_point = next_pair.first;
  glm::vec3 direction = next_point - first_point;
  road_direction_ = glm::normalize(direction);
  road_y_rotation_ = RAD2DEG(atan2(direction.x, direction.z)); // atan2 handles division by 0 and proper quadrant

  // BLOCK BELOW IS UNUSED BUT COULD BE USED FOR BLOCKING TURNING AROUND
  // Find angle between car dir and road dir
  //   Angle is clockwise from direction of road
  // glm::vec2 dir = glm::vec2(car_->direction().x, car_->direction().z);
  // TODO remove top line (saved for getting facing angle not velo angle)
  // dir = glm::vec2(car_->velocity_x(), car_->velocity_z());
  // dir = glm::normalize(dir);
  // glm::vec2 road_dir = glm::vec2(road_direction_.x, road_direction_.z);
  // car_angle_ = -glm::orientedAngle(dir, road_dir);
  // printf("ang = %f\n",car_angle_);

  // Find collision point closest to car
  //  warning closest_pair, and dis cannot be overridden above
  float dis_opposite_point = glm::distance(closest_pair.second, car);
  bool is_water_closest = true;
  if (dis > dis_opposite_point)
    is_water_closest = false;
  // Decide which type of animation to play
  //   i.e. cliff scrape or water bounce
  // @warn also sets camera position for the crash
  if (is_collision_) {
    camera_state_ = camera_->state();
    if (is_water_closest) {
      current_state = kGameState::kCrashingFall;
      camera_->ChangeState(Camera::kFirstPerson);
    } else {
      current_state = kGameState::kCrashingCliff;
      is_cliff_hit_ = false;
      is_prev_positive_ = true;
    }
    colisn_anim_ticks_ = 0;
    // UpdateCamera(); // Camera needs to be updated to change position
  }

  return current_state;
}

// TODO comment
void CollisionController::AutoDrive(Car * car, float delta_time) {
    car->set_rotation(glm::vec3(car->rotation().x,road_y_rotation_,car->rotation().z));
    if (left_lane_midpoint_ == prev_left_lane_midpoint_) {
      // These position updates are from object movement tick
      //   i.e. p = p + dt*v, v /= SPEEDSCALE, v = speed * direction;
      // TODO put constants somewhere
      float dt = delta_time / 1000;
      float x_pos = car->translation().x + road_direction_.x * car->default_speed()/10.0f*dt;
      float y_pos = car->translation().y;
      float z_pos = car->translation().z + road_direction_.z * car->default_speed()/10.0f*dt;
      car->set_translation(glm::vec3(x_pos, y_pos, z_pos));
    } else {
      glm::vec3 next_pt_dir = left_lane_midpoint_ - car->translation();
      next_pt_dir = glm::normalize(next_pt_dir);
      // These position updates are from object movement tick
      //   i.e. p = p + dt*v, v /= SPEEDSCALE, v = speed * direction;
      // TODO put constants somewhere
      float dt = delta_time / 1000;
      float x_pos = car->translation().x + next_pt_dir.x * car->default_speed()/10.0f*dt;
      float y_pos = car->translation().y;
      float z_pos = car->translation().z + next_pt_dir.z * car->default_speed()/10.0f*dt;
      car->set_translation(glm::vec3(x_pos, y_pos, z_pos));
    }
    prev_left_lane_midpoint_ = left_lane_midpoint_;
    // Set speed to default speed
    // TODO only need to call this once when change state
    car->ResetPhysics();
}
