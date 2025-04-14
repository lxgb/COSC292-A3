#ifndef BITMAP_H
#define BITMAP_H

#include <stdio.h>
#include <stdlib.h>

#define PI 3.14159265358979323846

/*

	-- Assignment 3 - Steganography
	-- Author: Alex G.
	-- Date: 2025-01-13
	-- Email: gobrin3707@saskpolytech.ca

*/

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef signed int LONG;

#pragma pack(push)
#pragma pack(1)
typedef struct
{
	WORD wType;
	DWORD dwFileSize;
	WORD wReserved1;
	WORD wReserved2;
	DWORD dwDataOffset;
	DWORD dwHeaderSize;
	LONG lWidth;
	LONG lHeight;
	WORD wPlanes;
	WORD wBitCount;
	DWORD dwCompression;
	DWORD dwImageSize;
	LONG lXPelsPerMeter;
	LONG lYPelsPerMeter;
	DWORD dwClrUsed;
	DWORD dwClrImportant;
} BITMAPHDR;

typedef struct
{
	BYTE bBlu, bGrn, bRed;
} PIXEL;

#pragma pack(pop)
typedef struct
{
	BITMAPHDR* bmHDR;
	PIXEL* bmData;
} IMAGE;

typedef void (*BM_FUNC_PTR)(PIXEL*);
typedef void (*BM_TWO_PIXELS)(PIXEL*, PIXEL*);

// Read an image into the image pointer from the already opened file pointer
void ReadImage(IMAGE* imgPtr, FILE* infile);

// Read the image header into the image pointer from the already opened file pointer
void ReadHeader(IMAGE* imgPtr, FILE* infile);

// Read an image into the image pointer from the already opened file pointer
void PrintHeader(BITMAPHDR* headerPtr);

void ReadData(IMAGE* imgPtr, FILE* infile);

void FreeImage(IMAGE* imgPtr);

void WriteImage(IMAGE* imgPtr, FILE* outfile);

FILE* GetFile(const char* cPrompt, const char* cMode);

/*
Function: void HideInImage(IMAGE* imgPtr, FILE* filePtr);
- Purpose:		
Loops through the pixels of the image. Hides one byte from the file into each pixel of the image. How do we hide the byte without destroying the image? Simple, just overwrite some of the lower order bits of each color component with some bits from the file’s byte.
Specifically, for each byte in the file to hide, the highest order 4 bits will be placed in the lower 4 bits of the blue component, the next 2 bits in the lower 2 bits of the green component, and the last 2 bits in the lower 2 bits of the red component.

- Parameters:	
imgPtr - pointer to the image structure
filePtr - pointer to the file to hide
*/
void HideInImage(IMAGE* imgPtr, FILE* filePtr);

/*
Function: void ExtractFileFromImage(IMAGE* imgPtr, FILE* filePtr);
- Purpose:
This function will extract the hidden file from the image. First, you will need to get the file size stored in dwClrImportant. Then, dynamically allocate a block of BYTEs to store the encoded file.
Loops through the file passed in. For each pixel, extract the current byte and assign to your dynamically allocated memory.
Once done looping, write the BYTE array out to the file passed in.

- Parameters:
imgPtr - pointer to the image structure
filePtr - pointer to the file to extract to
*/
void ExtractFileFromImage(IMAGE* imgPtr, FILE* filePtr);

/*
Function: unsigned int GetFileSize(FILE* filePtr);
- Purpose:
Determine the size of an open file given its file stream pointer. This is a simple process that involves moving to the end of the file, asking for the current position, and then “rewinding” the file pointer. This is a well-known process using the functions fseek and ftell.

- Parameters:
filePtr - pointer to the file to get size of
*/
unsigned int GetFileSize(FILE* filePtr);

#endif