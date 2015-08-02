#ifndef ASSIGN3_HELPERS_H_
#define ASSIGN3_HELPERS_H_

#include "includes.h"

namespace helpers {

  // Creates a single texture pointer from file
  //   @return new_texture, a GLuint texture pointer
  static GLuint LoadTexture(const std::string &filename) {

    // Generate New Texture
    GLuint new_texture;
    glGenTextures( 1, &new_texture );

    // load an image from file as texture 1
    int x, y, n;
    unsigned char *data;
    data = stbi_load(
        filename.c_str(), /*char* filepath */
        // "crate.jpg",
        &x, /*The address to store the width of the image*/
        &y, /*The address to store the height of the image*/
        &n  /*Number of channels in the image*/,
        0   /*Force number of channels if > 0*/
        );

    glBindTexture( GL_TEXTURE_2D, new_texture );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    if (n == 3) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else {
      fprintf(stderr, "Image pixels are not RGB. You will need to change the glTexImage2D command.");
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    stbi_image_free(data);

    glGenerateMipmap(GL_TEXTURE_2D);

    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);
    return new_texture;
  }

  // Checks whether car is between the biggest rectangle than can be formed
  //   @param car, the car object (to find it's position)
  //   @param arr, the array containing the corner points
  // @return  true  if can is inside corner of rectangle
  // @warn input must be square for accurate results
  static bool IsInside(const glm::vec3 &car,
      const std::vector<glm::vec2> &arr) {

    if (arr.size() != 4)
      return false;

    // glm::vec3 arr[] = {b,c,d};

    float min_x = arr[0].x, max_x = arr[0].x;
    float min_z = arr[0].y, max_z = arr[0].y;
    for (int i = 1; i < 3; ++i) {
      // Find max x bounding box
      if (arr[i].x > max_x)
        max_x = arr[i].x;
      // Find min x bounding box
      if (arr[i].x < min_x)
        min_x = arr[i].x;
      // Find max z bounding box
      if (arr[i].y > max_z)
        max_z = arr[i].y;
      // Find min x bounding box
      if (arr[i].y < min_z)
        min_z = arr[i].y;
    }

    return !(car.x < min_x || car.x > max_x || car.z < min_z || car.z > max_z);
  }

  // Checks whether car is between the biggest rectangle than can be formed
  //   @param car, the car vec3 (to find it's position)
  //   @param bp, 2x pairs (ie. 2x2 points), each pair is the horizontal bound
  //   @return  true  if can is inside corner of rectangle
  //   @warn input must be square for accurate results
  static bool IsInside(const glm::vec3 &car, const std::pair<glm::vec3,glm::vec3> &bp) {
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

}

#endif
