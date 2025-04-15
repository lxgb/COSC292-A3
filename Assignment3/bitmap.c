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
	if (!imgPtr || !imgPtr->bmData || !imgPtr->bmHDR || !filePtr)
		return;

	// Get image dimensions and calculate row padding
	int width = imgPtr->bmHDR->lWidth;
	int height = imgPtr->bmHDR->lHeight;
	int rowSize = width * sizeof(PIXEL);
	int padding = (4 - (rowSize % 4)) % 4;

	BYTE* pixelData = (BYTE*)imgPtr->bmData;

	// Use the helper to get file size
	unsigned int fileSize = GetFileSize(filePtr);
	imgPtr->bmHDR->dwClrImportant = fileSize;  // Store the size for later extraction

	// Check if the image has enough pixels to hide the file
	int totalPixels = width * height;
	if (fileSize > totalPixels)
		return;  // Not enough space to hide the file

	BYTE byteToHide = 0;
	unsigned int byteIndex = 0;

	// Loop through each pixel and embed one byte per pixel
	for (int y = 0; y < height && byteIndex < fileSize; y++)
	{
		PIXEL* row = (PIXEL*)(pixelData + y * (rowSize + padding));
		for (int x = 0; x < width && byteIndex < fileSize; x++)
		{
			if (fread(&byteToHide, sizeof(BYTE), 1, filePtr) != 1)
				break;

			BYTE blueBits = (byteToHide & 0xF0) >> 4;  // Top 4 bits
			BYTE greenBits = (byteToHide & 0x0C) >> 2;  // Middle 2 bits
			BYTE redBits = (byteToHide & 0x03);       // Bottom 2 bits

			row[x].bBlu = (row[x].bBlu & 0xF0) | blueBits;
			row[x].bGrn = (row[x].bGrn & 0xFC) | greenBits;
			row[x].bRed = (row[x].bRed & 0xFC) | redBits;

			byteIndex++;
		}
	}
}

void ExtractFileFromImage(IMAGE* imgPtr, FILE* filePtr)
{
	// Validate inputs
	if (!imgPtr || !imgPtr->bmData || !imgPtr->bmHDR || !filePtr)
		return;

	// Get image dimensions and calculate padding
	int width = imgPtr->bmHDR->lWidth;
	int height = imgPtr->bmHDR->lHeight;
	int rowSize = width * sizeof(PIXEL);
	int padding = (4 - (rowSize % 4)) % 4;

	// Get size of hidden file from the header
	unsigned int fileSize = imgPtr->bmHDR->dwClrImportant;

	// Allocate buffer for hidden file
	BYTE* buffer = (BYTE*)malloc(fileSize);
	if (!buffer)
		return;

	BYTE* pixelData = (BYTE*)imgPtr->bmData;
	unsigned int byteIndex = 0;

	// Iterate through the pixels row by row
	for (int i = 0; i < height && byteIndex < fileSize; i++)
	{
		PIXEL* row = (PIXEL*)(pixelData + i * (rowSize + padding));
		for (int j = 0; j < width && byteIndex < fileSize; j++)
		{
			PIXEL pixel = row[j];

			BYTE blueBits = (pixel.bBlu & 0x0F) << 4;  // Lower 4 bits from blue
			BYTE greenBits = (pixel.bGrn & 0x03) << 2;  // Lower 2 bits from green
			BYTE redBits = (pixel.bRed & 0x03);       // Lower 2 bits from red

			BYTE hiddenByte = blueBits | greenBits | redBits;
			buffer[byteIndex++] = hiddenByte;
		}
	}

	// Write the extracted data to file
	fwrite(buffer, sizeof(BYTE), fileSize, filePtr);
	free(buffer);
}


unsigned int GetFileSize(FILE* filePtr)
{
	unsigned int size = 0;
	fseek(filePtr, 0, SEEK_END);
	size = ftell(filePtr);
	rewind(filePtr);
	return size;
}