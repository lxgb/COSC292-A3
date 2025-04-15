#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRTDBG_MAP_ALLOC

#include "bitmap.h"
#include <string.h>
#include <errno.h>
#include <crtdbg.h>

/*

	-- Assignment 3 - Steganography
	-- Author: Alex G.
	-- Date: 2025-01-13
	-- Email: gobrin3707@saskpolytech.ca

*/

// Logic to hide a file in an image
void TestHideInImage()
{
	FILE* infile = NULL;
	FILE* hidefile = NULL;
	FILE* outfile = NULL;

	infile = GetFile("Enter file to hide file in: ", "rb");
	hidefile = GetFile("Enter file to hide: ", "rb");

	// Create an image struct instance
	IMAGE img = { NULL, NULL };

	// Read in the image
	ReadImage(&img, infile);
	fclose(infile);

	if (img.bmHDR->lWidth * img.bmHDR->lHeight >= GetFileSize(hidefile))
	{
		HideInImage(&img, hidefile);
		fclose(hidefile);

		// Write the image with hidden data out to file
		outfile = GetFile("Enter name of image with hidden file: ", "wb");
		WriteImage(&img, outfile);
		fclose(outfile);
	}

	// Free image memory
	FreeImage(&img);
}


// Logic to extract a hidden file from an image
void TestExtractFileFromImage()
{
	FILE* infile = GetFile("Enter image to extract from: ", "rb");
	FILE* outfile = GetFile("Enter file name to extract to: ", "wb");
	IMAGE img = { NULL, NULL };

	// Read in the image
	ReadImage(&img, infile);

	// Close the file
	fclose(infile);

	ExtractFileFromImage(&img, outfile);

	fclose(outfile);

	FreeImage(&img);
}

int main()
{
	//TestHideInImage();
	TestExtractFileFromImage();

	_CrtDumpMemoryLeaks(); // Check for memory leaks
	return EXIT_SUCCESS;
}