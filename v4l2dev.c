#include "v4l2dev.h"
#include "v4l2util.h"

#define SKIP_FRAME_NUM 2

void openDevice(Webcam *webcam){
  webcam->fd = open(webcam->devName, O_RDWR);
}

void closeDevice(Webcam *webcam){
  close(webcam->fd);
}

void initFormat(Webcam *webcam){
  struct v4l2_capability cap = {0};
  ioctl(webcam->fd, VIDIOC_QUERYCAP, &cap);

  struct v4l2_format fmt = {0};
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = webcam->imageWidth;
  fmt.fmt.pix.height = webcam->imageHeight;
  fmt.fmt.pix.pixelformat = webcam->imageFormat;
  fmt.fmt.pix.field = V4L2_FIELD_NONE;
  ioctl(webcam->fd, VIDIOC_S_FMT, &fmt);
};

void initMmap(Webcam *webcam){
  struct v4l2_requestbuffers req = {0};
  req.count = 1;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  ioctl(webcam->fd, VIDIOC_REQBUFS, &req);

  struct v4l2_buffer buf = {0};
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = 0;
  ioctl(webcam->fd, VIDIOC_QUERYBUF, &buf);

  webcam->buffer.length = buf.length;
  webcam->buffer.start = 
    mmap(
      0, 
      buf.length, 
      PROT_READ | PROT_WRITE, 
      MAP_SHARED, 
      webcam->fd, 
      buf.m.offset
    );
};

void startCapture(Webcam *webcam){
  for(int i=0; i < SKIP_FRAME_NUM; ++i){
    struct v4l2_buffer buf = {0};
    int fd = webcam->fd;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    ioctl(fd, VIDIOC_QBUF, &buf);

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMON, &type);
    ioctl(fd, VIDIOC_DQBUF, &buf);

    int width = webcam->imageWidth;
    int height = webcam->imageHeight;
    void *data = webcam->buffer.start;
    int length = webcam->buffer.length;

    unsigned char *buffer = calloc(length, 1);
    FILE *stream = fmemopen(buffer, length, "w+b");
    createPng(width, height, data, length, stream);

    FILE *fp = fopen("test.txt", "w+");
    png2base64(buffer, length, fp);
  }
};

void stopCapture(Webcam *webcam){
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl(webcam->fd, VIDIOC_STREAMOFF, &type);
}

void getDesc(Webcam *webcam){
  struct v4l2_fmtdesc fmtdesc;
  fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl(webcam->fd, VIDIOC_ENUM_FMT, &fmtdesc);
  printf("%s\n",fmtdesc.description);
}

int main(){
  Webcam webcam = {
    .devName = "/dev/video0",
    .imageWidth = 320,
    .imageHeight = 240,
    .imageFormat = V4L2_PIX_FMT_SGRBG8,
    .openDevice = openDevice,
    .closeDevice = closeDevice,
    .initFormat = initFormat,
    .initMmap = initMmap,
    .startCapture = startCapture,
    .stopCapture = stopCapture,
    .getDesc = getDesc,
  };
  webcam.openDevice(&webcam);
  webcam.initFormat(&webcam);
  webcam.getDesc(&webcam);
  webcam.initMmap(&webcam);
  webcam.startCapture(&webcam);
  webcam.stopCapture(&webcam);
  webcam.closeDevice(&webcam);
  return 0;
}