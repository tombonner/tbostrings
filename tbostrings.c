/*

	File:		tbostrings.c

	Author:		Tom Bonner (tom.bonner@gmail.com)

	Date:		09-May-2016

	Version:	0.1

	Purpose:	Dump printable ASCII/UNICODE strings from a given file in a single pass.

	Copyright (C) 2016, Tom Bonner.

	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS
	#define _CRT_DISABLE_PERFCRIT_LOCKS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _FLAG_SHOW_OFFSETS	0x00000001	/* Display string offsets */
#define _FLAG_SHOW_FILENAME	0x00000002	/* Display filename */

#define _MAX_NULLS			3

/*

	User supplied options

*/

typedef struct OPTIONS
{
	char *			FileName;		/* Name of the file being processed */
	size_t			MinLength;		/* Mimimum length of strings to find */
	size_t			MaxLength;		/* Maximum length of strings to find */
	unsigned int	Flags;			/* See _FLAG_SHOW_OFFSETS and _FLAG_SHOW_FILENAME */

} OPTIONS, *POPTIONS;

/* Table of printable ASCII characters */

static unsigned char _IsPrintable[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*

	Open/read file and dump printable strings

*/

int TBO_StringsFile(POPTIONS Options)
{
	FILE *			Stream = NULL;
	size_t			Offset = 0;
	size_t			FirstOffset = 0;
	size_t			LastOffset = 0;
	long			Char = 0;
	long			PrevChar = 0;
	unsigned char *	Buffer = NULL;
	size_t			Length = 0;
	size_t			Nulls = 0;
	size_t			ExpectedNulls = 0;

	/* Check parameters */

	if (Options == NULL || Options->FileName == NULL || Options->MinLength == 0)
	{
		return -1;
	}
	
	/* Open the input file */

	Stream = fopen(Options->FileName, "rb");

	if (Stream == NULL)
	{
		perror (Options->FileName);

		return -1;
	}

	/* Allocate enough memory to store the minimum string length (and null terminator) */

	Buffer = (unsigned char *)malloc(sizeof(unsigned char) * (Options->MinLength + 1));

	if (Buffer != NULL)
	{
		/* Iterate over each byte in the input file */

		for (Offset = 0; ; Offset++)
		{
			/* Read next character from the input file */

			Char = getc(Stream);
		
			if (Char == EOF)
			{
				/* End of file, bail */

				break;
			}
			else if (Char == 0 && Nulls < _MAX_NULLS)
			{
				/* Skip (up too _MAX_NULLS) null bytes */

				Nulls++;

				/* Count any expected nulls after the first character */

				if (Length == 1)
				{
					ExpectedNulls++;
				}

				continue;
			}
			else if (_IsPrintable[Char & 0xff])
			{
				/* Not counting nulls for the first char */

				if (Length == 0)
				{
					Nulls = 0;
					ExpectedNulls = 0;
					
					/* Store the offset of the first matching char */

					FirstOffset = Offset;
				}
				else
				{
					/* Is the count of nulls as expected (i.e. same as the first char)? */

					if (Nulls != ExpectedNulls)
					{
						/* Last found character is now the start of the potential match */

						Buffer[0] = Buffer[Length - 1];
						Length = 1;
						Buffer[Length++] = (unsigned char)Char;

						FirstOffset = LastOffset;

						/* Update null counts */

						ExpectedNulls = Nulls;
						Nulls = 0;

						continue;
					}
				}

				/* Store printable character */

				Buffer[Length++] = (unsigned char)Char;

				/* Store the last offset in case we have to revert */

				LastOffset = Offset;
				
				/* Reset the count of expected nulls */

				Nulls = 0;

				/* Is the minimum length buffer full? */

				if (Length >= Options->MinLength)
				{
					/* Print filename? */

					if (Options->Flags & _FLAG_SHOW_FILENAME)
					{
						printf("%s: ", Options->FileName);
					}

					/* Print offset? */

					if (Options->Flags & _FLAG_SHOW_OFFSETS)
					{
#ifdef _MSC_VER
						printf("%7lu ", FirstOffset);
#else
						printf("%7zu ", FirstOffset);
#endif
					}

					/* Flush buffer of printable characters */

					Buffer[Length] = 0x00;
					fputs((char *)Buffer, stdout);

					/* Iterate over remaining characters in the string */

					while (++Offset)
					{
						/* Read next character from the input file */

						Char = getc(Stream);

						if (Char == 0 && Nulls < _MAX_NULLS)
						{
							/* Skip (up too _MAX_NULLS) null bytes */

							Nulls++;

							continue;
						}
						else if (_IsPrintable[Char & 0xff])
						{
							/* Is the count of null bytes as expected?.. */

							if (ExpectedNulls != Nulls)
							{
								/* No, reset */

								Length = 0;

								FirstOffset = Offset;

								/* This could be another ASCII string? */

								if (ExpectedNulls != 0)
								{
									Buffer[Length++] = (unsigned char)PrevChar;
									FirstOffset--;
								}

								/* Terminate the string */

								putchar('\n');
								Nulls = 0;
								ExpectedNulls = 0;

								/* Character becomes the start of the next potential match */

								Buffer[Length++] = (unsigned char)Char;
								
								break;
							}
							else
							{
								/* Yes, output character */

								putchar(Char);

								Length++;
								Nulls = 0;
								PrevChar = Char;
							}
						}
						else
						{
							/* Invalid character, terminate the string */

							putchar('\n');

							Length = 0;
							Nulls = 0;
							ExpectedNulls = 0;

							break;
						}
					}
				}
			}
			else
			{
				/* Invalid character, reset any match and counts */

				Length = 0;
				Nulls = 0;
				ExpectedNulls = 0;
			}
		}

		/* Any I/O errors? */

		if (ferror(Stream)) 
		{
			perror (Options->FileName);
		}

		/* Free the minimum string length buffer */

		free(Buffer);
	}

	/* Close the input file */

	if (fclose (Stream) == EOF)
	{
		perror (Options->FileName);

		return -1;
	}

	return 0;
}

/*

	Display program usage

*/

static int _Usage(char * ProgramName)
{
	printf("Display printable ASCII/UNICODE strings in a file.\n");
	printf("Usage: %s [options] [files]\n", ProgramName);
	printf("-n [num]   Minimum string length\n");
	printf("-o         Display offsets\n");
	printf("-f         Display filename\n");
	printf("-w         Display whitespace\n");
	printf("-h --help  Display usage information\n");

	return -1;
}

/*
	Main program entrypoint

*/

int main(int argc, char ** argv)
{
	OPTIONS	Options;
	int		i = 0;

	/* Ensure we have enough arguments */

	if (argc < 2)
	{
		return _Usage(argv[0]);
	}

	/* Set default options */

	Options.MinLength = 4;
	Options.MaxLength = 0;
	Options.Flags = 0;

	/* Parse command line arguments */

	for (i = 1; i < argc; i++)
	{
		/* Minimum string length */

		if(strcmp(argv[i],"-n") == 0)
		{
			if (++i < argc)
			{
				/* Get user specified length */

				Options.MinLength = (int)strtoul((char *)argv[i], NULL, 0);

				/* Is length sane? */

				if (Options.MinLength < 2 || Options.MinLength > 0xffff)
				{
					/* Bail */

					printf("Invalid length\n");

					return -1;
				}
			}
			else
			{
				return _Usage(argv[0]);
			}
		}

		/* Show offsets */

		else if(strcmp(argv[i],"-o") == 0)
		{
			Options.Flags |= _FLAG_SHOW_OFFSETS;
		}

		/* Show file name */

		else if(strcmp(argv[i],"-f") == 0)
		{
			Options.Flags |= _FLAG_SHOW_FILENAME;
		}

		/* Show whitespace characters */

		else if(strcmp(argv[i],"-w") == 0)
		{
			/* Tab */

			_IsPrintable[0x09] = 1;
			
			/* Space */

			_IsPrintable[0x20] = 1;
		}
		
		/* Display usage */

		else if(strcmp(argv[i],"-h") == 0 || strcmp(argv[i],"--help") == 0 || strcmp(argv[i],"-?") == 0)
		{
			return _Usage(argv[0]);
		}

		/* Scan file for strings */

		else
		{
			/* Get the filename */

			Options.FileName = argv[i];

			/* Scan the file for printable strings */

			if (TBO_StringsFile(&Options) != 0)
			{
				return -1;
			}
		}
	}

	return 0;
}

