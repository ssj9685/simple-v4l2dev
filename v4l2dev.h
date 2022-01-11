#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <gd.h>

typedef struct{
	char *name;
	char *value;
} src_option_t;

typedef struct{
	/* Source Options */
	char *source;
	u_int8_t type;
	
	void *state;
	
	/* Last captured image */
	u_int32_t length;
	void *img;
	
	/* Input Options */
	char    *input;
	u_int8_t  tuner;
	u_int32_t frequency;
	u_int32_t delay;
	u_int32_t timeout;
	char     use_read;
	
	/* List Options */
	u_int8_t list;
	
	/* Image Options */
	int palette;
	u_int32_t width;
	u_int32_t height;
	u_int32_t fps;
	
	src_option_t **option;
	
	/* For calculating capture FPS */
	u_int32_t captured_frames;
	struct timeval tv_first;
	struct timeval tv_last;
	
} src_t;

typedef struct {
	
	char *name;
	
	u_int8_t flags;
	
	int (*open)(src_t *);
	int (*close)(src_t *);
	int (*grab)(src_t *);
	
} src_mod_t;

typedef struct Buffer Buffer;
struct Buffer{
  void *start;
  int length;
};

typedef struct Webcam Webcam;
struct Webcam{
  char *devName;
  int fd;
  Buffer buffer;
  int imageWidth;
  int imageHeight;
  int imageFormat;

  void (*openDevice)(Webcam *);
  void (*closeDevice)(Webcam *);
  void (*initFormat)(Webcam *);
  void (*initMmap)(Webcam *);
  void (*startCapture)(Webcam *);
  void (*stopCapture)(Webcam *);
	void (*getDesc)(Webcam *);
};
