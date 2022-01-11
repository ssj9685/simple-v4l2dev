#include "v4l2util.h"

int base64_encode(char *text, int numBytes, char **encodedText)
{
    char *MimeBase64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned char *input = calloc(3, 1);
    unsigned char *output = calloc(4, 1);
    

    int index;
    char *plen = text + numBytes - 1;
    int size = (4 * (numBytes / 3)) + (numBytes % 3 ? 4 : 0) + 1;
    (*encodedText) = calloc(size, sizeof(char *));
    int i = 0;
    int j = 0;
    char *p = text;
    char *tmp = *encodedText;
    for (; p <= plen; ++i, ++p) {
        index = i % 3;
        input[index] = *p;
        if (index == 2 || p == plen) {
            output[0] = ((input[0] & 0xFC) >> 2);
            output[1] = ((input[0] & 0x3) << 4) | ((input[1] & 0xF0) >> 4);
            output[2] = ((input[1] & 0xF) << 2) | ((input[2] & 0xC0) >> 6);
            output[3] = (input[2] & 0x3F);
            
            *(tmp++) = MimeBase64[output[0]];
            *(tmp++) = MimeBase64[output[1]];
            *(tmp++) = index == 0 ? '=' : MimeBase64[output[2]];
            *(tmp++) = index <  2 ? '=' : MimeBase64[output[3]];
            input[0] = 0;
            input[1] = 0;
            input[2] = 0;
        }
        if(p > plen){
          *tmp = '\0';
        }
    }
    free(input);
    free(output);
    return size;
}

void add_image_bayer(u_int16_t *dst, u_int8_t *img, u_int32_t length, u_int32_t w, u_int32_t h)
{
	u_int32_t x = 0, y = 0;
	u_int32_t i = w * h;
	
	while(i-- > 0)
	{
		u_int8_t *p[8];
		u_int8_t hn, vn, di;
		u_int8_t r, g, b;
		int mode;
		
		p[0] = img - w - 1;
		p[1] = img - w;
		p[2] = img - w + 1;
		p[3] = img - 1;
		p[4] = img + 1;
		p[5] = img + w - 1;
		p[6] = img + w;
		p[7] = img + w + 1;
		
		if(!y)              { p[0]=p[5]; p[1]=p[6]; p[2]=p[7]; }
		else if(y == h - 1) { p[5]=p[0]; p[6]=p[1]; p[7]=p[2]; }
		if(!x)              { p[0]=p[2]; p[3]=p[4]; p[5]=p[7]; }
		else if(x == w - 1) { p[2]=p[0]; p[4]=p[3]; p[7]=p[5]; }
		
		hn = (*p[3] + *p[4]) / 2;
		vn = (*p[1] + *p[6]) / 2;
		di = (*p[0] + *p[2] + *p[5] + *p[7]) / 4;
	
		mode = ~(x + y) & 0x01;
		
		if(mode) {
			g = *img;
			if(y & 0x01) {
        r = hn;
        b = vn;
      }
			else {
        r = vn; 
        b = hn;
      }
		}
		else if(y & 0x01) {
      r = *img;
      g = (vn + hn) / 2;
      b = di;
    }
		else {
      b = *img;
      g = (vn + hn) / 2;
      r = di;
    }

			u_int8_t t = r;
			r = b;
			b = t;
		
		*(dst++) += r;
		*(dst++) += g;
		*(dst++) += b;
		
		if(++x == w) {
      x = 0;
      y++;
    }
		img++;
	}
}

void createPng(int width, int height, void *data, int length, FILE *stream){
  gdImage *original = gdImageCreateTrueColor(width, height);
  u_int16_t *abitmap = calloc(length * 3, sizeof(u_int16_t));
  u_int16_t *pbitmap = abitmap;

  add_image_bayer(abitmap, data, length, width, height);

  for(int y = 0; y < height; y++) {
    for(int x = 0; x < width; x++) {
      int color = 0;
      color += *(pbitmap++) << 16;
      color += *(pbitmap++) << 8;
      color += *(pbitmap++);
      gdImageSetPixel(original, x, y, color);
    }
  }
  gdImagePngEx(original, stream, 1);

  free(abitmap);
  fclose(stream);
  gdImageDestroy(original);
}

void png2base64(char *buffer, int length, FILE *dst){
  char *base64;
  base64_encode(buffer, length, &base64);
  fwrite(base64, 1, strlen(base64), dst);
  
  free(buffer);
  free(base64);
  fclose(dst);
}