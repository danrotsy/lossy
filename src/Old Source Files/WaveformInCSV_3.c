#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<dirent.h>
#include<errno.h>

#include"Auto_LTS.h"

int main(int argc, char* argv[])
{
	//don't include .txt in file name for command-line argument
	//file names are in format of "R=X_L=Y_C=Z"
	//all in directory named after input file
	if(argc < 3) {
		printf("Insufficient number of command-line arguments.\n Exiting program.\n");
		return 1;
	}
	struct param_run run;
	run.r_step_array[0] = 0.02;
	run.r_step_array[1] = 1.8;
	run.r_step_array[2] = 0.2;
	
	run.l_step_array[0] = 0.079E-9;
	run.l_step_array[1] = 7.11E-9;;
	run.l_step_array[2] = 0.79E-9;
	
	run.c_step_array[0] = 0.22E-12;
	run.c_step_array[1] = 19.8E-12;
	run.c_step_array[2] = 2.2E-12;
	
	run.r_curr = 0.02;
	run.l_curr = 0.079E-9;
	run.c_curr = 0.22E-12;
	
	run.r_str = malloc(255);
	run.l_str = malloc(255);
	run.c_str = malloc(255);
	
	char buffer[255];
	char filename_out[255];
	char var_line[255];
	char file_name[255];
	FILE* waveform_in = NULL;
	FILE* waveform_out = NULL;
	DIR* waveform_out_dir = opendir(argv[1]);
	if(waveform_out_dir)
		closedir(waveform_out_dir);
	else if(ENOENT==errno) {
		if(mkdir(argv[1], S_IRWXU)==-1) {
			printf("Could not create directory, exiting program...\n");
			return 1;
		}
	}
	else {
		printf("An error has occurred in opening the directory %s", argv[1]);
		return 1;
	}
	strcpy(var_line, argv[2]);
	strcpy(file_name, argv[1]);
	strcat(file_name, ".csv");
	waveform_in = fopen(file_name, "r");
	if(waveform_in)
		printf("Opened file %s\n", file_name);
	else {
		printf("Could not open file %s\n", file_name);
		return 0;
	}
	while(!feof(waveform_in)) {
		fgets(buffer, 255, waveform_in);
		printf("Buffer: %s", buffer);
		if(memcmp(buffer, "#DATA", 4)==0) {
			update_run(&run);
			sprintf(filename_out, "StepInformationRline=%sLline=%sCline=%s.csv", run.r_str, run.l_str, run.c_str);
			if(waveform_out)
				fclose(waveform_out);
			strcpy(buffer, argv[1]);
			strcat(buffer, "/");
			strcat(buffer, filename_out);
			waveform_out = fopen(buffer, "w");
			printf("Started new waveform_out file: %s\n", filename_out);
			fprintf(waveform_out, "%s\n", var_line);
			continue;
		}
		if(waveform_out)
			fprintf(waveform_out, "%s", buffer);
		else
			printf("Waveform_out is NULL\n");
	}
	fclose(waveform_out);
	fclose(waveform_in);
	return 0;
}