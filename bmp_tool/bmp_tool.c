/*
This is an exercise proposed by Teufelchen#2781 to help me (BigBrainAFK#0001) become the next CLang god

So objectives:
- Understand the file format
- Write software that creates empty bmps
- Write functions to draw line/rects/circles
- Draw funny pictures
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>
#include <stdbool.h>

void drawRect(struct bmp bmp_file, struct color line_color, int32_t x0, int32_t y0, int32_t x1, int32_t y1, bool filled);
void drawLine(struct bmp bmp_file, struct color line_color, int32_t x0, int32_t y0, int32_t x1, int32_t y1);
void fillImage(struct bmp bmp_file, struct color color);
size_t align_up(size_t size, size_t alignment);

#pragma pack(push, 1)
struct bmp_header {
	uint16_t type;
	uint32_t file_size;
	uint32_t reserved;
	uint32_t pixel_offset;
} bmp_header;
#pragma pack(pop)

struct bmp_dib {
	uint32_t size;
	int32_t width;
	int32_t height;
	uint16_t planes;
	uint16_t bit_depth;
	uint32_t compression;
	uint32_t size_image;
	int32_t x_ppm;
	int32_t y_ppm;
	uint32_t color_used;
	uint32_t color_important;
} bmp_dib;

struct bmp {
	struct bmp_header header;
	struct bmp_dib dib_header;
	uint8_t* imageData;
};

struct color {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} color;

int main()
{
	struct bmp_header header = {
		(uint16_t)0x4D42, // set type to "BM" which is the most common in any mdoern system/software
		0, // size of the whole bitmap in bytes
		0, // reserved and only used by certain software
		54 // offset where the image data starts which here is fixed due to not much functionality being used
	};

	struct bmp_dib dib_header = {
		40, //size of header
		0, //width
		0, //height
		1, //planes should always be 1
		24, //bit depth usually best left at 24
		0, //compression
		0, //size of image in bytes
		2835, //ppm in x axis in this case its equal to 72ppi
		2835, //ppm in y axis in this case its equal to 72ppi
		0, // color table used
		0 // important colors used
	};

	// set size to JFR can go f**k himself
	const size_t width = 1231;
	const size_t height = 1234;
	
	// adjust bitmap DIB header accordingly
	dib_header.width = width;
	dib_header.height = -height;

	//calculate line length in bytes
	size_t line_length = width * 3 * sizeof(uint8_t);
	line_length = align_up(line_length, 4);

	//printf("line_length: %zd\n", line_length);

	//calculate size of image data
	dib_header.size_image = (uint32_t) (line_length * height);
	header.file_size = sizeof(header) + sizeof(dib_header) + dib_header.size_image;

	//allocate memory for image data
	uint8_t* imageData = (uint8_t*) malloc(dib_header.size_image);

	struct bmp bmp_file = {
		header,
		dib_header,
		imageData
	};

	struct color white = {
		0xFF,
		0xFF,
		0xFF
	}; // black for the line

	//memset so the microsoft compiler doesnt put 0xCD everywhere but its actually 0x00
	//memset(bmp_file.imageData, 0xFF, dib_header.size_image);
	fillImage(bmp_file, white);

	struct color line_color = {
		0x00,
		0x00,
		0x00
	}; // black for the line

	struct color background_color = {
		0xFF,
		0x00,
		0xFF
	}; // this is pure pink

	fillImage(bmp_file, background_color);
	drawLine(bmp_file, line_color, 0, 0, width, height);
	drawRect(bmp_file, line_color, 100, 100, 400, 400, false);
	drawRect(bmp_file, line_color, 400, 400, 700, 700, true);

	//printf("ImageData %d\n", imageData);

	//printf("size of header: %zu\n", sizeof(header));
	//printf("size of dib_header: %zu\n", sizeof(dib_header));
	//printf("Size of imageData: %zu\n", sizeof(&imageData));

	// open file write stream
	FILE* f = fopen("file.bmp", "wb");

	// check pointer real quick
	if (f != NULL) {
		// write garbage data to disk
		fwrite(&header, sizeof(header), 1, f);
		fwrite(&dib_header, sizeof(dib_header), 1, f);
		fwrite(imageData, dib_header.size_image, 1, f);

		// nicely close file otherwise JFR gets mad
		fclose(f);
	}

	// free data so JFR doesnt get memleaks in the think ways.
	// and since jfr keeps compalining i might aswell rm -rf / --no-preserve-root right now just to save myself from his nitpics
	free(imageData);

	return 0;
}

void drawRect(struct bmp bmp_file, struct color line_color, int32_t x0, int32_t y0, int32_t x1, int32_t y1, bool filled) {
	//calculate line length in bytes
	size_t line_length = bmp_file.dib_header.width * 3 * sizeof(uint8_t);
	line_length = align_up(line_length, 4);

	// loop through all lines and the entire width in pixels and set each pixel to the color given
	size_t line, row;
	for (line = y0; line <= y1; line++) { //renegate height since we store a negative value for top-to-bottom pixel rows
		for (row = x0; row <= x1; row++) {
			if (line > y0 && line < y1 && row > x0 && row < x1 && filled == false) continue;
			uint8_t* pixel = &bmp_file.imageData[line * line_length + row * 3 * sizeof(uint8_t)];
			pixel[0] = color.red; //red
			pixel[1] = color.green; //green
			pixel[2] = color.blue; //blue
		}
	}
}

void drawLine(struct bmp bmp_file, struct color line_color, int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
	//calculate line length in bytes
	size_t line_length = bmp_file.dib_header.width * 3 * sizeof(uint8_t);
	line_length = align_up(line_length, 4);

	// calculate delta of x and y to know the direction of the line
	int32_t deltax= x1 - x0;
	int32_t deltay = y1 - y0;
	int32_t D = 2 * deltay - deltax;
	
	// loop through the entire lines "width" from x0 to x1
	int32_t y = y0;
	int32_t x;
	for (x = x0; x <= x1; x++) {
		uint8_t* pixel = &bmp_file.imageData[y * line_length + x * 3 * sizeof(uint8_t)];
		pixel[0] = line_color.red; //red
		pixel[1] = line_color.green; //green
		pixel[2] = line_color.blue; //blue

		if (D > 0) {
			y = y + 1;
			D = D - 2 * deltax;
		}

		D = D + 2 * deltay;
	}
}

void fillImage(struct bmp bmp_file, struct color color) {
	//calculate line length in bytes
	size_t line_length = bmp_file.dib_header.width * 3 * sizeof(uint8_t);
	line_length = align_up(line_length, 4);

	// loop through all lines and the entire width in pixels and set each pixel to the color given
	size_t line, row;
	for (line = 0; line < -bmp_file.dib_header.height; line++) { //renegate height since we store a negative value for top-to-bottom pixel rows
		for (row = 0; row < bmp_file.dib_header.width; row++) {
			uint8_t* pixel = &bmp_file.imageData[line * line_length + row * 3 * sizeof(uint8_t)];
			pixel[0] = color.red; //red
			pixel[1] = color.green; //green
			pixel[2] = color.blue; //blue
		}
	}
}

size_t align_up(size_t size, size_t alignment) {
	// calculate remainder to align properly
	size_t remainder = size % alignment;
	size_t padding = remainder > 0 ? alignment - remainder : 0;
	// return inverse which is the missing bits
	return size + padding;
}