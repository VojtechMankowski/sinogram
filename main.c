#include <stdio.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

enum CHANNELS { RED, GREEN, BLUE, ALPHA, NUM_CHANNELS };

void draw_image(unsigned char* data, int width, int height, int channels, enum CHANNELS offset);

void rotate_image(unsigned char* data, double angle_deg, int width, int height, int channels, enum CHANNELS offset);

void rotate_position(double* x, double* y, int pixel_num, double angle_deg, int width, int height);

unsigned char nearest_neighbour(unsigned char* data, double x, double y, int width, int height, int channels, enum CHANNELS offset);


int main(void) {

    int width, height, channels;
    char * filename = "letters.png";
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 0);
    
    enum CHANNELS offset = RED;

    double angle_deg = -30.0;
    
    // draw_image(data, width, height, channels, offset);

    for ( int c = 0; c < NUM_CHANNELS; c++ ) {
        rotate_image(data, angle_deg, width, height, channels, (enum CHANNELS)c);
    }

    // printf("\n\nROTATED:\n\n");

    // draw_image(data, width, height, channels, offset);

    stbi_write_png("written.png", width, height, channels, data, width*channels);

    stbi_image_free(data);

    getchar();
    return 0;
}

void draw_image(unsigned char* data, int width, int height, int channels, enum CHANNELS offset) {
    int N = width*height;
    int pixel_num = 0;
    int val = 0;

    for (; pixel_num < N; pixel_num++) {
            val = *(data + channels*pixel_num + offset);
            
            if ( pixel_num % width == 0) {
                printf("\n");
            }
            if ( val < 10 ) printf("%d   ", val);
            else if ( val < 100 ) printf("%d  ", val);
            else printf("%d ", val);
        }
}

void rotate_image(unsigned char* data, double angle_deg, int width, int height, int channels, enum CHANNELS offset) {
    int pixel_num, rot_pixel_num;
    int N = width*height;
    double x,y;
    unsigned char val;

    for (pixel_num = 0; pixel_num < N; pixel_num++) {
        // 1. find rotated position
        rotate_position(&x, &y, pixel_num, angle_deg, width, height);

        // 2. compute value (NEAREST)
        val = nearest_neighbour(data, x, y, width, height, channels, offset);

        // 3. assign value
        *(data + channels*pixel_num + offset) = val;
    }
}

void rotate_position(double* x, double* y, int pixel_num, double angle, int width, int height) {
    double x_rot, y_rot;
    
    /* convert to radians */
    angle *= ( M_PI / 180.0 );
    
    /* compute pixel position*/
    *x = pixel_num % width;
    *y = pixel_num / width;

    /* center around middle of image */
    *x = *x - 1.0*width;
    *y = *y - 1.0*height;

    /* compute pixel position after rotation */
    x_rot = (*x) * cos(angle) - (*y) * sin(angle);
    y_rot = (*x) * sin(angle) + (*y) * cos(angle);
    
    /* move origin back to (0,0)*/
    x_rot = x_rot + 1.0*width;
    y_rot = y_rot + 1.0*height;

    *x = x_rot;
    *y = y_rot;
}

unsigned char nearest_neighbour(unsigned char* data, double x, double y, int width, int height, int channels, enum CHANNELS offset) {
    int pixel_num;
    unsigned char val;

    if ( x < 0.0 || y < 0.0 || x > (width-1) || y > (height-1) ) {
        return 0;
    }

    x = round(x);
    y = round(y);
    pixel_num = x + y*width;
    val = *(data + channels*pixel_num + offset); 
    return val;
}