#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glob.h>
#include <unistd.h>
#include "dbg.h"

#define MAX_DATA 512

int or_search(FILE *cur_file, int keywordc, char *keyword[])
{
	char line[MAX_DATA];

	for (int i = 0; i < keywordc; i++) {
		
		rewind(cur_file);

		while (fgets(line, sizeof(line), cur_file)) {
			if(strstr(line, keyword[i])) {
				printf("Found %s \n", keyword[i]);
				printf("Now end the search...\n");
				return 1;
			}
		}
		printf("Cannot find the keyword in this file \n");
		printf("Start searching with next keyword \n");
	}
	printf("Cannot find any keyword in this file \n");
	printf("Now end the search \n");
	return 0;
}

int and_search(FILE *cur_file, int keywordc, char *keyword[])
{
	char line[MAX_DATA];

	//"and search"(Default)
	for (int i = 0; i < keywordc; i++) {
		int found = 0;

		rewind(cur_file);

		while (fgets(line, sizeof(line), cur_file)) {
			if(strstr(line, keyword[i])) {
			   printf("Found %s \n", keyword[i]);
			   found = 1;
			   break;
			}
		}

		if (!found) {
		//If unable to find one keyword, no need to search other keyword.
		printf("Cannot find %s \n", keyword[i]);
		printf("Now end the search...\n");
		return 0;
	}
	}	

	printf("Successfully found all keywords in this file.\n");
	return 1;
}

int open_file(char *file_path, int keywordc, char *keyword[], int or_option) 
{
	FILE *cur_file;
	cur_file = fopen(file_path, "r");
	check(cur_file != NULL, "Fail to open the file.");	

	//fread(file_content, sizeof(char), MAX_DATA, cur_file);
	//printf("%s \n", file_content);	

	//Determine if it is an "or search" or "and search"	
	
	if (or_option) {
		or_search(cur_file, keywordc, keyword);
	} else { and_search(cur_file, keywordc, keyword); }

	printf("Now closing the file:%s \n", file_path);
	fclose(cur_file);

	return 0;
error:
	return -1;	
}

void read_logfind_line(FILE *file, int keywordc, char *keyword[], int or_option)
{
	char line[MAX_DATA];
	while (fgets(line, sizeof(line), file) != NULL) {

		// Remove the newline character
		line[strcspn(line, "\n")] = 0;

		glob_t results;
		int rc = glob(line, GLOB_TILDE, NULL, &results);
		
		if (rc == 0) {
			for (size_t i = 0; i < results.gl_pathc; i++) {
				printf("Found log file: %s\n", results.gl_pathv[i]);
				open_file(results.gl_pathv[i], keywordc, keyword, or_option);
			}
		} else if (rc == GLOB_NOMATCH) {
			printf("No files matched pattern: %s\n", line);
		} else {
			printf("Error reading pattern: %s\n", line);
		}

		globfree(&results);
	}
}

int main(int argc, char *argv[])
{
	int or_option = 0;
	int opt;

	while ((opt = getopt(argc, argv, "o")) != -1) {
		switch (opt) {
			case 'o':
				or_option = 1;
				break;
			default:
				fprintf(stderr, "Usage: %s [-o] keyword...\n", argv[0]);
				return 1;
		}
	}
	
	//optind return the index of first non-option argument
	int keywordc = argc - optind;
	check(keywordc >= 1, "You should at least type one keyword");

	char **keywords = argv + optind;

	char *home = getenv("HOME"); //returns "/home/yourusername"
	check(home != NULL, "HOME environment variable not found.");

	char path[MAX_DATA];	
	snprintf(path, sizeof(path), "%s/.logfind", home);
		
	printf("Your ~/.logfind path is %s \n", path);

	FILE *logfind = fopen(path, "r");
	check(logfind != NULL, "Failed to load ~/.logfind\n");
	
	read_logfind_line(logfind, keywordc, keywords, or_option);

	fclose(logfind);

	return 0;
error:
	return -1;
}
