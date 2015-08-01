#ifndef ASSIGN3_HELPERS_H_
#define ASSIGN3_HELPERS_H_

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
}

#endif
