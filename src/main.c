#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct {
  char type_string[3];
  uint32_t height;
  uint32_t width;
  uint8_t gradient_depth;
} image_header_t;

FILE *image_file;

void skip_unwanted_characters(FILE *image_file) {
  int ch;
  while ((ch = fgetc(image_file)) != EOF) {
    if (ch == '#') {
      while ((ch = fgetc(image_file)) != EOF && ch != '\n')
        ;
    } else if (ch == '\n' || ch == '\r' || ch == ' ' || ch == '\t') {
      continue;
    } else {
      ungetc(ch, image_file);
      break;
    }
  }
}

char *read_ascii_value(FILE *image_file) {
  skip_unwanted_characters(image_file);

  char buffer[20];
  int i = 0;
  int ch;
  while ((ch = fgetc(image_file)) != EOF && ch != '\n' && ch != '\r' &&
         ch != ' ' && ch != '\t') {
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
  fread(image_header->type_string, sizeof(char), 2, image_file);
  image_header->type_string[2] = '\0';

  char *width_str = read_ascii_value(image_file);
  char *height_str = read_ascii_value(image_file);
  char *gradient_depth_str = read_ascii_value(image_file);

  if (width_str && height_str && gradient_depth_str) {
    image_header->width = strtoul(width_str, NULL, 10);
    image_header->height = strtoul(height_str, NULL, 10);
    image_header->gradient_depth = strtoul(gradient_depth_str, NULL, 10);
  } else {
    free(image_header);
    return NULL;
  }

  free(width_str);
  free(height_str);
  free(gradient_depth_str);

  return image_header;
}

const int8_t gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
const int8_t gy[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

void apply_sobel(image_header_t *header, FILE *output_file) {
  if (!header) {
    fprintf(stderr, "Invalid image header\n");
    return;
  }

  int *image_array = malloc(header->width * header->height * sizeof(int));
  if (!image_array) {
    perror("Failed to allocate memory for image array");
    return;
  }

  for (int i = 0; i < header->width * header->height; i++) {
    char *value_str = read_ascii_value(image_file);
    if (value_str) {
      image_array[i] = strtoul(value_str, NULL, 10);
      free(value_str);
    } else {
      fprintf(stderr, "Failed to read ASCII value\n");
      free(image_array);
      return;
    }
  }

  int *sobel_image = calloc(header->width * header->height, sizeof(int));
  if (!sobel_image) {
    perror("Failed to allocate memory for Sobel image");
    free(image_array);
    return;
  }

  for (uint32_t y = 1; y < header->height - 1; y++) {
    for (uint32_t x = 1; x < header->width - 1; x++) {
      int Gx_sum = 0;
      int Gy_sum = 0;

      for (int ky = -1; ky <= 1; ky++) {
        for (int kx = -1; kx <= 1; kx++) {
          int pixel = image_array[(y + ky) * header->width + (x + kx)];
          Gx_sum += pixel * gx[ky + 1][kx + 1];
          Gy_sum += pixel * gy[ky + 1][kx + 1];
        }
      }

      int magnitude = (int)sqrt(Gx_sum * Gx_sum + Gy_sum * Gy_sum);
      if (magnitude > 255) {
        magnitude = 255;
      }

      sobel_image[y * header->width + x] = magnitude;
    }
  }

  fprintf(output_file, "P2\n%u %u\n%u\n", header->width, header->height,
          header->gradient_depth);
  for (uint32_t y = 0; y < header->height; y++) {
    for (uint32_t x = 0; x < header->width; x++) {
      fprintf(output_file, "%d ", sobel_image[y * header->width + x]);
    }
    fprintf(output_file, "\n");
  }

  free(image_array);
  free(sobel_image);
}

int main(int argc, char **args) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <image_file>\n", args[0]);
    exit(EXIT_FAILURE);
  }

  image_file = fopen(args[1], "r");
  if (NULL == image_file) {
    printf("inputfile: %d: %s", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  errno = 0;
  FILE *output_file = fopen(args[2], "w");
  if (NULL == output_file) {
    printf("outputfile: %d: %s", errno, strerror(errno));
  }

  image_header_t *header = get_image_header();
  if (header) {
    apply_sobel(header, output_file);
    free(header);
  } else {
    fprintf(stderr, "Failed to get image header\n");
  }

  fclose(image_file);
  return 0;
}
