#include <complex.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct {
  char *type_string;
  uint32_t height;
  uint32_t width;
  uint32_t gradient_depth;
} image_header_t;

FILE *image_file;

void skip_unwanted_characters(FILE *image_file) {
  int ch;
  while ((ch = fgetc(image_file)) != EOF) {
    if (ch == '#') {
      while ((ch = fgetc(image_file)) != EOF && ch != '\n')
        ;
    } else if (ch == '\n' || ch == '\r') {
      continue;
    } else {
      ungetc(ch, image_file);
      break;
    }
  }
}

void print_image_header(image_header_t header) {
  printf("type: %s\n", header.type_string);
  printf("dimensions: w%u h%u\n", header.width, header.height);
  printf("gradient_depth: %u\n", header.gradient_depth);
}

char *read_ascii_value(FILE *image_file) {
  skip_unwanted_characters(image_file);

  char buffer[20];
  int i = 0;
  int ch;
  while ((ch = fgetc(image_file)) != EOF && ch != '\n' && ch != '\r' &&
         ch != ' ') {
    buffer[i++] = ch;
  }
  buffer[i] = '\0';

  return strdup(buffer);
}

image_header_t *get_image_header() {
  image_header_t *image_header = malloc(sizeof(image_header_t));
  if (!image_header) {
    perror("Failed to allocate memory");
    return NULL;
  }

  skip_unwanted_characters(image_file);
  image_header->type_string = malloc(3 * sizeof(char));
  if (!image_header->type_string) {
    perror("Failed to allocate memory for type_string");
    free(image_header);
    return NULL;
  }
  fread(image_header->type_string, sizeof(char), 2, image_file);
  image_header->type_string[2] = '\0';

  char *width_str = read_ascii_value(image_file);
  char *height_str = read_ascii_value(image_file);
  char *gradient_depth_str = read_ascii_value(image_file);

  if (width_str && height_str && gradient_depth_str) {
    image_header->width = strtoul(width_str, NULL, 10);
    image_header->height = strtoul(height_str, NULL, 10);
    image_header->gradient_depth = strtoul(gradient_depth_str, NULL, 10);
  }

  free(width_str);
  free(height_str);
  free(gradient_depth_str);

  return image_header;
}

int main(int argc, char **args) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <image_file>\n", args[0]);
    exit(EXIT_FAILURE);
  }

  image_file = fopen(args[1], "r");
  if (NULL == image_file) {
    printf("%d: %s", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  image_header_t *header = get_image_header();
  if (header) {
    print_image_header(*header);
    free(header->type_string);
    free(header);
  }

  fclose(image_file);
  return 0;
}
