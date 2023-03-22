// Copies a BMP file

#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"

void bottom_up(int height, int width, RGBTRIPLE image[height][width]);

int main(int argc, char *argv[])
{
    	// Ensure proper usage
    	if (argc != 3)
    	{
        	printf("Usage: copy infile outfile\n");
        	return 1;
    	}

    	// Remember filenames
    	char *infile = argv[1];
    	char *outfile = argv[2];

    	// Open input file
    	FILE *inptr = fopen(infile, "r");
    	if (inptr == NULL)
    	{
        	printf("Could not open %s.\n", infile);
        	return 2;
    	}

    	// Open output file
    	FILE *outptr = fopen(outfile, "w");
    	if (outptr == NULL)
    	{
        	fclose(inptr);
        	printf("Could not create %s.\n", outfile);
        	return 3;
    	}

    	// Read infile's BITMAPFILEHEADER
    	BITMAPFILEHEADER bf;
    	fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    	// Read infile's BITMAPINFOHEADER
    	BITMAPINFOHEADER bi;
    	fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    	// Ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    	if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        	bi.biBitCount != 24 || bi.biCompression != 0)
    	{
        	fclose(outptr);
        	fclose(inptr);
        	printf("Unsupported file format.\n");
        	return 4;
    	}

    	int height = abs(bi.biHeight);
	int width = bi.biWidth;

	// alocate memory for image
	RGBTRIPLE (*image)[width] = calloc(height, width * sizeof(RGBTRIPLE));
	if (image == NULL)
	{
		printf("Not enough memory to store image.\n");
		fclose(outptr);
		fclose(inptr);
		return 5;
	}

    	// Determine padding for scanlines
    	int padding = (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    	// Iterate over infile's scanlines
    	for (int i = 0; i < height; i++)
    	{
		// read row into pixel array
		fread(image[i], sizeof(RGBTRIPLE), width, inptr);

        	// Skip over padding, if any
        	fseek(inptr, padding, SEEK_CUR);

    	}

	// Write outfile's BITMAPFILEHEADER
    	fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

	// Write outfile's BITMAPINFOHEADER
	fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);

	// Turn image upside down
	bottom_up(height, width, image);

	// Write new pixels to outfile
	for (int i = 0; i < height; i++)
	{
		fwrite(image[i], sizeof(RGBTRIPLE), width, outptr);

		// Write padding at end of row
		for (int k = 0; k < padding; k++)
		{
			fputc(0x00, outptr);
		}
	}

	// Free memory for image
	free(image);

    	// Close infile
    	fclose(inptr);

    	// Close outfile
    	fclose(outptr);

    	// Success
    	return 0;
}

void bottom_up(int height, int width, RGBTRIPLE image[height][width])
{
	RGBTRIPLE image_rev[height][width];

	// create reversed array of image[i][j]
	for (int i = height - 1; i >= 0; i--)
	{
		for (int j = width - 1; j >= 0; j--)
		{
			int k = height - i - 1, l = width - j - 1;
			image_rev[k][l].rgbtRed = image[i][j].rgbtRed;
			image_rev[k][l].rgbtGreen = image[i][j].rgbtGreen;
			image_rev[k][l].rgbtBlue = image[i][j].rgbtBlue;
		}
	}

	// replace image[i][j] with image_rev[i]j]
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			image[i][j].rgbtRed = image_rev[i][j].rgbtRed;
			image[i][j].rgbtGreen = image_rev[i][j].rgbtGreen;
			image[i][j].rgbtBlue = image_rev[i][j].rgbtBlue;
		}
	}
}
