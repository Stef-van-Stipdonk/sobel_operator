#include <complex.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *image_buffer;

void get_image_type() {
  char *type_string = malloc(3 * sizeof(char));
  strncpy(type_string, image_buffer, 2);
  type_string[2] = '\0';

  if (strcmp(type_string, "P2") != 0) {
    printf("Incorrect filetype");
  }
  free(type_string);
}

int main(int argc, char **args) {
  FILE *image_file = fopen(args[1], "r");

  if (NULL == image_file) {
    printf("%d: %s", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  fseek(image_file, 0, SEEK_END);
  long fsize = ftell(image_file);
  fseek(image_file, 0, SEEK_SET); /* same as rewind(f); */

  image_buffer = malloc(fsize + 1);
  fread(image_buffer, fsize, 1, image_file);
  fclose(image_file);

  image_buffer[fsize] = 0;

  get_image_type();
  free(image_buffer);
}
