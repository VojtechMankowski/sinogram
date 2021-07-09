#include <stdio.h>
#include <math.h>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

enum CHANNELS { RED, GREEN, BLUE, ALPHA, NUM_CHANNELS };

void draw_channel(unsigned char* input_image, int width, int height, int channels, enum CHANNELS offset);

void size_of_rotated_image(int* width_rot, int* height_rot, int height, int width, double angle_deg);

void rotate_image(unsigned char* rotated_image, unsigned char* input_image, double angle_deg, int width, int height, int width_rot, int height_rot, int channels, enum CHANNELS offset);

void rotate_position(double* x, double* y, int pixel_num, double angle_deg, int width_rot, int height_rot, int width, int height);

unsigned char nearest_neighbour(unsigned char* input_image, double x, double y, int width, int height, int channels, enum CHANNELS offset);

unsigned char bilinear_interp(unsigned char* input_image, double x, double y, int width, int height, int channels, enum CHANNELS offset);

int main(void) {
    clock_t start_time;
    double cpu_time_used;
    
    start_time = clock();

    int width, height, channels;
    int width_rot = 0, height_rot = 0;
    char * filename = "letters.png";
    char output_filename[20];
    unsigned char *input_image = stbi_load(filename, &width, &height, &channels, 0);
    int angles = 181;
    enum CHANNELS offset = RED;
    double angle_deg = 30.0;

    /* Use as a check for small images */
    // draw_channel(input_image, width, height, channels, offset);

    /* allocate N bytes for rotated image */
    // unsigned char* rotated_image = malloc(width*height*channels);

    // memcpy(rotated_image, input_image, width*height*channels);
    /* Alternative to MEMCPY: */
    // for (int i = 0; i < width*height*channels; i++){
    //     *(rotated_image+i) = *(input_image+i);
    // }

    /* allocate bytes for sinogram image */
    unsigned char* sinogram = malloc(angles*height*channels);

    for (angle_deg = 0.0; angle_deg < angles; angle_deg += 10) {
        
        /* compute size of rotated image */
        size_of_rotated_image(&width_rot, &height_rot, height, width, angle_deg);

        /* allocate memory for rotated image */
        unsigned char* rotated_image = malloc(width_rot*height_rot*channels);

        /* set black image */
        for (int i = 0; i < width_rot*height_rot*channels; i++){
            *(rotated_image+i) = 0;
        }

        /* loop through all image channels */
        for ( int c = 0; c < NUM_CHANNELS; c++ ) {
            rotate_image(rotated_image, input_image, angle_deg, width, height, width_rot, height_rot, channels, (enum CHANNELS)c);
        }

        sprintf(output_filename, "rotated%d.png", (int)angle_deg);
        printf("%s\n", output_filename);

        stbi_write_png(output_filename, width_rot, height_rot, channels, rotated_image, width_rot*channels);

        free(rotated_image);
    }
    
    stbi_image_free(input_image);
    
    free(sinogram);

    cpu_time_used = ((double) (clock() - start_time)) / CLOCKS_PER_SEC;
    printf("Program took %f seconds to execute.\n", cpu_time_used);

    // getchar();
    return 0;
}

void draw_channel(unsigned char* input_image, int width, int height, int channels, enum CHANNELS offset) {
    int N = width*height;
    int pixel_num = 0;
    int val = 0;

    for (; pixel_num < N; pixel_num++) {
            val = *(input_image + channels*pixel_num + offset);
            
            if ( pixel_num % width == 0) {
                printf("\n");
            }
            if ( val < 10 ) printf("%d   ", val);
            else if ( val < 100 ) printf("%d  ", val);
            else printf("%d ", val);
        }
}

void size_of_rotated_image(int* width_rot, int* height_rot, int height, int width, double angle) {
    double x[4], y[4];
    double x_rot[4], y_rot[4];

    /* convert to radians */
    angle *= ( M_PI / 180.0 );

    /* define 4 corners */
    x[0] = -0.5*(double)width;
    x[1] = -x[0];
    x[2] = x[1];
    x[3] = x[0];

    y[0] = 0.5*(double)height;
    y[1] = y[0];
    y[2] = -y[1];
    y[3] = y[2];

    /* find rotated positions */
    x_rot[0] = x[0] * cos(angle) - y[0] * sin(angle);
    y_rot[0] = x[0] * sin(angle) + y[0] * cos(angle);

    x_rot[1] = x[1] * cos(angle) - y[1] * sin(angle);
    y_rot[1] = x[1] * sin(angle) + y[1] * cos(angle);

    x_rot[2] = x[2] * cos(angle) - y[2] * sin(angle);
    y_rot[2] = x[2] * sin(angle) + y[2] * cos(angle);

    x_rot[3] = x[3] * cos(angle) - y[3] * sin(angle);
    y_rot[3] = x[3] * sin(angle) + y[3] * cos(angle);

    /* get maximum width and height */
    *(width_rot) = (int) 2 * round( fmax( fmax(abs(x_rot[0]), abs(x_rot[1])), fmax(abs(x_rot[2]), abs(x_rot[3])) ) );
    *(height_rot) = (int) 2 * round( fmax( fmax(abs(y_rot[0]), abs(y_rot[1])), fmax(abs(y_rot[2]), abs(y_rot[3])) ) );
}

void rotate_image(unsigned char* rotated_image, unsigned char* input_image, double angle_deg, int width, int height, int width_rot, int height_rot, int channels, enum CHANNELS offset) {
    int pixel_num;
    int N = width_rot*height_rot;
    double x,y;
    unsigned char val;

    for (pixel_num = 0; pixel_num < N; pixel_num++) {
        // 1. find rotated position
        rotate_position(&x, &y, pixel_num, angle_deg, width_rot, height_rot, width, height);

        // 2. compute value (NEAREST)
        val = nearest_neighbour(input_image, x, y, width, height, channels, offset);
        // val = bilinear_interp(input_image, x, y, width, height, channels, offset);

        // 3. assign value
        *(rotated_image + channels*pixel_num + offset) = val;
    }
}

void rotate_position(double* x, double* y, int pixel_num, double angle, int width_rot, int height_rot, int width, int height) {
    double x_rot, y_rot;
    
    /* convert to radians */
    angle *= ( M_PI / 180.0 );
    
    /* compute pixel position*/
    *x = pixel_num % width_rot;
    *y = pixel_num / width_rot;

    /* center around middle of image */
    *x = *x - 0.5*width_rot;
    *y = *y - 0.5*height_rot;

    /* compute pixel position after rotation */
    x_rot = (*x) * cos(angle) - (*y) * sin(angle);
    y_rot = (*x) * sin(angle) + (*y) * cos(angle);
    
    /* move origin back to (0,0) in coords of input image*/
    x_rot = x_rot + 0.5*width;
    y_rot = y_rot + 0.5*height;

    *x = x_rot;
    *y = y_rot;
}

unsigned char nearest_neighbour(unsigned char* input_image, double x, double y, int width, int height, int channels, enum CHANNELS offset) {
    int pixel_num;
    unsigned char val = 0;

    /* outside image case */
    if ( x < 0.0 || y < 0.0 || x > (width-1) || y > (height-1) ) {
        return val;
    }

    x = round(x);
    y = round(y);
    pixel_num = x + y*width;
    val = *(input_image + channels*pixel_num + offset); 
    return val;
}

unsigned char bilinear_interp(unsigned char* input_image, double x, double y, int width, int height, int channels, enum CHANNELS offset) {
    int pixel_num;
    unsigned char val = 0;
    float val1, val2, val3, val4;
    float val12, val34;

    /* outside image case */
    if ( x < 0.0 || y < 0.0 || x > (width-1) || y > (height-1) ) {
        return val;
    }

    /* left top corner */
    pixel_num = floor(x) + floor(y)*width;
    val1 = (float) *(input_image + channels*(pixel_num) + offset);

    /* right top corner */
    pixel_num = ceil(x) + floor(y)*width;
    val2 = (float) *(input_image + channels*pixel_num + offset);
    
    /* left bottom corner */
    pixel_num = floor(x) + ceil(y)*width;
    val3 = (float) *(input_image + channels*pixel_num + offset);
    
    /* right bottom corner*/
    pixel_num = ceil(x) + ceil(y)*width;
    val4 = (float) *(input_image + channels*pixel_num + offset);
    
    /* for pixel grid the denominator (x2-x1) = 1 */
    val12 = val1 + (val2-val1)*(x-floor(x));
    val34 = val3 + (val4-val3)*(x-floor(x));

    val = (unsigned char) round( val12 + (val34-val12)*(y-floor(y)) );
    return val;
}