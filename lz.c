#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int

uint32_t lz77_compress (uint8_t *uncompressed_text, uint32_t uncompressed_size, uint8_t *compressed_text, uint8_t pointer_length_width)
{
	uint16_t pointer_pos, temp_pointer_pos, output_pointer, pointer_length, temp_pointer_length;
	uint32_t compressed_pointer, output_size, coding_pos, output_lookahead_ref, look_behind, look_ahead;
	uint16_t pointer_pos_max, pointer_length_max;
	pointer_pos_max = pow(2, 16 - pointer_length_width);
	pointer_length_max = pow(2, pointer_length_width);

	*((uint32_t *) compressed_text) = uncompressed_size;
	*(compressed_text + 4) = pointer_length_width;
	compressed_pointer = output_size = 5;
	    
	for(coding_pos = 0; coding_pos < uncompressed_size; ++coding_pos)
	{
		pointer_pos = 0;
		pointer_length = 0;
		for(temp_pointer_pos = 1; (temp_pointer_pos < pointer_pos_max) && (temp_pointer_pos <= coding_pos); ++temp_pointer_pos)
		{
			look_behind = coding_pos - temp_pointer_pos;
			look_ahead = coding_pos;
			for(temp_pointer_length = 0; uncompressed_text[look_ahead++] == uncompressed_text[look_behind++]; ++temp_pointer_length)
				if(temp_pointer_length == pointer_length_max) break;
		
			if(temp_pointer_length > pointer_length)
			{
			    pointer_pos = temp_pointer_pos;
			    pointer_length = temp_pointer_length;
			    if(pointer_length == pointer_length_max)
				break;
			}
		}
	
		coding_pos += pointer_length;
	
		if((coding_pos == uncompressed_size) && pointer_length)
		{
			output_pointer = (pointer_length == 1) ? 0 : ((pointer_pos << pointer_length_width) | (pointer_length - 2));
			output_lookahead_ref = coding_pos - 1;
		}
		else
		{
			output_pointer = (pointer_pos << pointer_length_width) | (pointer_length ? (pointer_length - 1) : 0);
			output_lookahead_ref = coding_pos;
		}
	
		*((uint16_t *) (compressed_text + compressed_pointer)) = output_pointer;
		compressed_pointer += 2;
		*(compressed_text + compressed_pointer++) = *(uncompressed_text + output_lookahead_ref);
		output_size += 3;
	}

	return output_size;
}

uint32_t lz77_decompress (uint8_t *compressed_text, uint8_t *uncompressed_text)
{
	uint8_t pointer_length_width;
	uint16_t input_pointer, pointer_length, pointer_pos, pointer_length_mask;
	uint32_t compressed_pointer, coding_pos, pointer_offset, uncompressed_size;

	uncompressed_size = *((uint32_t *) compressed_text);
	pointer_length_width = *(compressed_text + 4);
	compressed_pointer = 5;

	pointer_length_mask = pow(2, pointer_length_width) - 1;

	for(coding_pos = 0; coding_pos < uncompressed_size; ++coding_pos)
	{
		input_pointer = *((uint16_t *) (compressed_text + compressed_pointer));
		compressed_pointer += 2;
		pointer_pos = input_pointer >> pointer_length_width;
		pointer_length = pointer_pos ? ((input_pointer & pointer_length_mask) + 1) : 0;

		if(pointer_pos)
			for(pointer_offset = coding_pos - pointer_pos; pointer_length > 0; --pointer_length)
				uncompressed_text[coding_pos++] = uncompressed_text[pointer_offset++];

		*(uncompressed_text + coding_pos) = *(compressed_text + compressed_pointer++);
	}

	return coding_pos;
}

long fsize (FILE *in)
{
    long pos, length;
    pos = ftell(in);
    fseek(in, 0L, SEEK_END);
    length = ftell(in);
    fseek(in, pos, SEEK_SET);
    return length;
}

uint32_t file_lz77_compress (char *filename_in, char *filename_out, size_t malloc_size, uint8_t pointer_length_width)
{
    FILE *in, *out;
    uint8_t *uncompressed_text, *compressed_text;
    uint32_t uncompressed_size, compressed_size;

    in = fopen(filename_in, "r");
    if(in == NULL)
        return 0;
    uncompressed_size = fsize(in);
    uncompressed_text = malloc(uncompressed_size);

   
    if((uncompressed_size != fread(uncompressed_text, 1, uncompressed_size, in)))
        return 0;
    fclose(in);

    compressed_text = malloc(malloc_size);
    if(compressed_text==NULL)
    {
  	puts("Out of memory!");
	return 0;
    }

    compressed_size = lz77_compress(uncompressed_text, uncompressed_size, compressed_text, pointer_length_width);

    out = fopen(filename_out, "w");
    if(out == NULL)
        return 0;
    if((compressed_size != fwrite(compressed_text, 1, compressed_size, out)))
        return 0;
    fclose(out);

	free(uncompressed_text);
	free(compressed_text);

    return compressed_size;
}

uint32_t file_lz77_decompress (char *filename_in, char *filename_out)
{
    FILE *in, *out;
    uint32_t compressed_size, uncompressed_size;
    uint8_t *compressed_text, *uncompressed_text;

    in = fopen(filename_in, "r");
    if(in == NULL)
        return 0;
    compressed_size = fsize(in);
    compressed_text = malloc(compressed_size);
    if(fread(compressed_text, 1, compressed_size, in) != compressed_size)
        return 0;
    fclose(in);

    uncompressed_size = *((uint32_t *) compressed_text);
    uncompressed_text = malloc(uncompressed_size);

    if(lz77_decompress(compressed_text, uncompressed_text) != uncompressed_size)
        return 0;

    out = fopen(filename_out, "w");
    if(out == NULL)
        return 0;
    if(fwrite(uncompressed_text, 1, uncompressed_size, out) != uncompressed_size)
        return 0;
    fclose(out);

	free(compressed_text);
	free(uncompressed_text);

    return uncompressed_size;
}

int main (int argc, char *argv[])
{
	uint8_t i;
	int a;
	FILE *in;
	char of[64],rcf[64];

	for(a=1;a<argc;a++)
	{
		sprintf(of,"%sz",argv[a]);
		sprintf(rcf,"%s2",argv[a]);

		in = fopen(argv[a], "r");

		if(in == NULL) return 0;

		printf("Original size: %ld\n", fsize(in));
		fclose(in);

		//for(i = 1; i < 16; ++i)
		//
		i=3;
		        printf("Compressed (%i): %u, decompressed: (%u)\n", i, file_lz77_compress(argv[a],of, 100000, i), file_lz77_decompress(of,rcf));

	}
	
	return 0;
}
