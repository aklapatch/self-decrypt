#include<stdio.h>
#include<stdint.h>
#include <string.h>
#include <inttypes.h>
#include "util.h"

// concats one file to another, with the size of the concatted file appended at the end.
// args file1 file2 out_file_name
int main(int argc, char **argv){

	if (argc != 4){
		printf("Usage: %s file1 file2 out_file\n", argv[0]);
		return 1;
	}

	char *in1 = argv[1], *in2 = argv[2], *out = argv[3];

	// grab the first file and write it to the output file first.
	FILE *fout = fopen(out, "wb");
	if (fout == NULL){
		printf("Opening %s Failed!\n", out);
		return 1;
	}

	FILE *fin1 = fopen(in1, "rb");
	if (fin1 == NULL){
		printf("Opening %s Failed!\n", in1);
		return 1;
	}

	// dump fin1 into fout
	uint8_t file_buf[FREAD_BYTES] = {0};
	uint64_t in1_size = 0;
	for (size_t bytes_read = sizeof(file_buf); bytes_read == sizeof(file_buf);){
		bytes_read = fread(file_buf, 1, sizeof(file_buf), fin1);
		in1_size += bytes_read;
		size_t bytes_out = fwrite(file_buf, 1, bytes_read, fout);
		if (bytes_out != bytes_read){
			printf("Writing to %s failed!\n", out);
			fclose(fin1); fclose(fout);
			return 1;
		}
	}
	fclose(fin1);
	printf("Wrote %" PRIu64 " bytes from %s to %s\n", in1_size, in1, out);

	// do the same for the second file, but keep track of the size
	FILE *fin2 = fopen(in2, "rb");
	if (fin2 == NULL){
		printf("Opening %s Failed!\n", in2);
		fclose(fout);
		return 1;
	}
	uint64_t in2_size = 0;
	for (size_t bytes_read = sizeof(file_buf); bytes_read == sizeof(file_buf);){
		bytes_read = fread(file_buf, 1, sizeof(file_buf), fin2);
		in2_size += bytes_read;
		size_t bytes_out = fwrite(file_buf, 1, bytes_read, fout);
		if (bytes_out != bytes_read){
			printf("Writing to %s failed!\n", out);
			fclose(fin2); fclose(fout);
			return 1;
		}
	}
	printf("Wrote %" PRIu64 " bytes to %s from %s\n", in2_size, out, in2);
	fclose(fin2);

	// append the size of file 2 to fout
	printf("Appending %" PRIu64 "\n", in2_size);
	fwrite(&in2_size, sizeof(in2_size), 1, fout);

	printf("Writing sentinel '%s'\n", END_SENTINEL);
	fwrite(END_SENTINEL, strlen(END_SENTINEL), 1, fout);
	fclose(fout);

	return 0;
}
