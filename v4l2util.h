#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <gd.h>

int base64_encode(char *text, int numBytes, char **encodedText);
void add_image_bayer(u_int16_t *dst, u_int8_t *img, u_int32_t length, u_int32_t w, u_int32_t h);
void createPng(int width, int height, void *data, int length, FILE *stream);
void png2base64(char *buffer, int length, FILE *dst);