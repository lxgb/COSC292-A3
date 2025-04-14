#define _CRT_SECURE_NO_WARNINGS
#include "bitmap.h"
#include <malloc.h>
#include <math.h>

#define MAX_FILENAME_SIZE 256

/*

	-- Assignment 3 - Steganography
	-- Author: Alex G.
	-- Date: 2025-01-13
	-- Email: gobrin3707@saskpolytech.ca

*/

// GetFile function to prompt the user for a file name and open it
FILE* GetFile(const char* cPrompt, const char* cMode)
{
	FILE* aFile = NULL;
	char cFileName[MAX_FILENAME_SIZE];

	printf("%s", cPrompt);
	gets_s(cFileName, MAX_FILENAME_SIZE);

	aFile = fopen(cFileName, cMode);
	return aFile;
}

// Function to read an image from a file
void ReadImage(IMAGE* imgPtr, FILE* infile)
{
	ReadHeader(imgPtr, infile);
	if (imgPtr->bmHDR != NULL)
	{
		// Read in the data
		ReadData(imgPtr, infile);
	}
}

// Function to read the image data from the file
void ReadData(IMAGE* imgPtr, FILE* infile)
{
	unsigned int padding = imgPtr->bmHDR->lWidth % 4;
	unsigned int imageSize = (imgPtr->bmHDR->lWidth * sizeof(PIXEL) + padding) * imgPtr->bmHDR->lHeight;

	// Display calculated image size
	printf("Calculated image size: %d\n", imageSize);

	// Check if the file size matches the expected size
	if (imageSize == imgPtr->bmHDR->dwImageSize)
	{
		imgPtr->bmData = (PIXEL*)malloc(imageSize);
		if (imgPtr->bmData != NULL)
		{
			if (fread(imgPtr->bmData, imageSize, 1, infile) != 1)
			{
				printf("Reading data failed");
			}
		}
		
		else
		{
			free(imgPtr->bmHDR);
			imgPtr->bmHDR = NULL;
		}	
	}

	else
	{
		printf("Invalid image\n");
		free(imgPtr->bmHDR);
		imgPtr->bmHDR = NULL;
	}
}

// Function to read the header of the image
void ReadHeader(IMAGE* imgPtr, FILE* infile)
{
	imgPtr->bmHDR = (BITMAPHDR*)malloc(sizeof(BITMAPHDR));

	if (imgPtr->bmHDR != NULL)
	{
		if (fread(imgPtr->bmHDR, sizeof(BITMAPHDR), 1, infile) != 1)
		{
			free(imgPtr->bmHDR);
			imgPtr->bmHDR = NULL;
			printf("Error reading header from file.\n");
		}
	}
}

// Function to print the header information
void PrintHeader(BITMAPHDR* headerPtr)
{
	printf("First two characters: %x\n", headerPtr->wType);
	printf("File size: %d\n", headerPtr->dwFileSize);
	printf("Data offset: %d\n", headerPtr->dwDataOffset);
	printf("Header size: %d\n", headerPtr->dwHeaderSize);
	printf("Dimensions: %d by %d\n", headerPtr->lWidth, headerPtr->lHeight);
	printf("Planes: %d\n", headerPtr->wPlanes);
	printf("Colour depth: %d\n", headerPtr->wBitCount);
	printf("Compression: %d\n", headerPtr->dwCompression);
	printf("Image size: %d\n", headerPtr->dwImageSize);
	printf("Colours used: %d\n", headerPtr->dwClrUsed);
}

// Function to free image memory
void FreeImage(IMAGE* imgPtr)
{
	free(imgPtr->bmHDR);
	imgPtr->bmHDR = NULL;
	if (imgPtr->bmData != NULL)
	{
		free(imgPtr->bmData);
		imgPtr->bmData = NULL;
	}
}

// Function to write modified image to a file
void WriteImage(IMAGE* imgPtr, FILE* outfile)
{
    if (fwrite(imgPtr->bmHDR, sizeof(BITMAPHDR), 1, outfile) != 1)
    {
        printf("Failed to write image header\n");
    }
    else
    {
        // Write the image data
        DWORD imageSize = imgPtr->bmHDR->dwImageSize;

        if (imgPtr->bmData == NULL)
        {
            printf("Image data is NULL\n");
            return;
        }

        if (fwrite(imgPtr->bmData, imageSize, 1, outfile) != 1)
        {
            printf("Failed to write image data\n");
        }
    }
}

void HideInImage(IMAGE* imgPtr, FILE* filePtr)
{
	PIXEL* currentPixel = imgPtr->bmData;
	BYTE padding = imgPtr->bmHDR->lWidth % 4;
	BYTE byteToHide = 0;
	int i = 0;

	// Check if the image is large enough to hold the hidden data
	while (fread(&byteToHide, sizeof(BYTE), 1, filePtr) == 1)
	{
		for (int j = 0; j < 8; j++)
		{
			// Check if we have reached the end of the image data
			if (i >= imgPtr->bmHDR->dwImageSize)
			{
				break;
			}
			// Hide the byte in the pixel
			currentPixel->bBlu = (currentPixel->bBlu & 0xF0) | ((byteToHide >> (7 - j)) & 0x0F);
			currentPixel++;
			i++;
			if (i % imgPtr->bmHDR->lWidth == 0)
			{
				currentPixel = (PIXEL*)(((BYTE*)currentPixel) + padding);
			}
		}
	}
}

void ExtractFileFromImage(IMAGE* imgPtr, FILE* filePtr)
{
	// Check if the image pointer and file pointer are valid
	if (!imgPtr || !imgPtr->bmData || !imgPtr->bmHDR || !filePtr)
		return;

	// Check if the image is large enough to hold the hidden data
	int width = imgPtr->bmHDR->lWidth;
	int height = imgPtr->bmHDR->lHeight;
	int rowSize = width * sizeof(PIXEL);
	int padding = (4 - (rowSize % 4)) % 4;

	// Calculate the size of the hidden data
	BYTE* pixelData = (BYTE*)imgPtr->bmData;
	BYTE byteToWrite = 0;
	int bitIndex = 0;

	// Read the hidden data from the image
	for (int y = 0; y < height; y++)
	{
		PIXEL* row = (PIXEL*)(pixelData + y * (rowSize + padding)); // Adjust for padding
		// Read each pixel in the row
		for (int x = 0; x < width; x++)
		{
			// Extract the bits from the pixel
			byteToWrite |= ((row[x].bBlu & 0x01) << (7 - bitIndex));

			bitIndex++;
			if (bitIndex == 8)
			{
				fwrite(&byteToWrite, sizeof(BYTE), 1, filePtr);
				byteToWrite = 0;
				bitIndex = 0;
			}
		}
	}

	// Handle any remaining bits
	if (bitIndex != 0)
	{
		fwrite(&byteToWrite, sizeof(BYTE), 1, filePtr);
	}
}

unsigned int GetFileSize(FILE* filePtr)
{
	unsigned int size = 0;
	fseek(filePtr, 0, SEEK_END);
	size = ftell(filePtr);
	rewind(filePtr);
	return size;
}