
/*
libpng info:
http://www.libpng.org/pub/png/libpng-1.4.0-manual.pdf
http://signbit01.wordpress.com/2012/05/27/libpng-1-5-10-access-violation-on-visual-studio-2010/
http://www.piko3d.net/tutorials/libpng-tutorial-loading-png-files-from-streams/

source: 
http://forums.x-plane.org/index.php?showtopic=66819
http://forums.x-plane.org/index.php?showtopic=51503
alternate use http://forums.x-plane.org/index.php?showtopic=44573	

an alternative to libnpg might be sdl_image

building info:

Generated Code Setting (/MD, /MTd...) must be the same for target, libpng16.lib and zlib.lib 
no Optimization (/GL)
Configuration must be Release (even though I cant see any differences in configs)
 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include <XPLMDefs.h>
#include <XPLMGraphics.h>
#include "png.h"   //not found when Configuration "Debug"
#include "zlib.h"


GLuint png_texture_load(const char * file_name, int * width, int * height)
{
    png_byte header[8];
 
    FILE *fp = fopen(file_name, "rb");
    if (fp == 0)
    {
        perror(file_name);
        return 0;
    }
 
    // read the header
    fread(header, 1, 8, fp);
 
    if (png_sig_cmp(header, 0, 8))
    {
        fprintf(stderr, "error: %s is not a PNG.\n", file_name);
        fclose(fp);
        return 0;
    }
 
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
        fprintf(stderr, "error: png_create_read_struct returned 0.\n");
        fclose(fp);
        return 0;
    }
 
    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        return 0;
    }
 
    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        return 0;
    }
 
    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "error from libpng\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }
 
    // init png reading
    png_init_io(png_ptr, fp);
 
    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);
 
    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);
 
    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;
 
    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type,
        NULL, NULL, NULL);
 
    if (width){ *width = temp_width; }
    if (height){ *height = temp_height; }
 
    //printf("%s: %lux%lu %d\n", file_name, temp_width, temp_height, color_type);
 
    if (bit_depth != 8)
    {
        fprintf(stderr, "%s: Unsupported bit depth %d.  Must be 8.\n", file_name, bit_depth);
        return 0;
    }
 
    GLint format;
    switch(color_type)
    {
    case PNG_COLOR_TYPE_RGB:
        format = GL_RGB;
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        format = GL_RGBA;
        break;
    default:
        fprintf(stderr, "%s: Unknown libpng color type %d.\n", file_name, color_type);
        return 0;
    }
 
    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);
 
    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);
 
    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes-1) % 4);
 
    // Allocate the image_data as a big block, to be given to opengl
    png_byte * image_data = (png_byte *)malloc(rowbytes * temp_height * sizeof(png_byte)+15);
    if (image_data == NULL)
    {
        fprintf(stderr, "error: could not allocate memory for PNG image data\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }
 
    // row_pointers is for pointing to image_data for reading the png with libpng
    png_byte ** row_pointers = (png_byte **)malloc(temp_height * sizeof(png_byte *));
    if (row_pointers == NULL)
    {
        fprintf(stderr, "error: could not allocate memory for PNG row pointers\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(image_data);
        fclose(fp);
        return 0;
    }
 
    // set the individual row_pointers to point at the correct offsets of image_data
    for (unsigned int i = 0; i < temp_height; i++)
    {
        row_pointers[temp_height - 1 - i] = image_data + i * rowbytes;
    }
 
    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);
 
    // Generate the OpenGL texture object
    //GLuint texture;
    //XPLMGenerateTextureNumbers((GLint*)&texture, 1);
    //glGenTextures(1, &texture);
    //XPLMBindTexture2d(texture, 0);
    //glBindTexture(GL_TEXTURE_2D, texture);
    //glTexImage2D(GL_TEXTURE_2D, 0, format, temp_width, temp_height, 0, format, GL_UNSIGNED_BYTE, image_data);
	int TextureId;
	XPLMGenerateTextureNumbers(&gTexture[TextureId], 1);
	XPLMBindTexture2d(gTexture[TextureId], 0);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, temp_width, temp_height, GL_RGB, GL_UNSIGNED_BYTE, image_data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(image_data);
    free(row_pointers);
    fclose(fp);
    //return texture;
	return TextureId;
}