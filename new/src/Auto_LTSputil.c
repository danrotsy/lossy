#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<dirent.h>
#include<errno.h>

#include"Auto_LTS.h"

int main(int argc, char* argv[])
{
	if(argc < 2) {
		printf("Command-line argument for netlist filename required\n");
		return 1;
	}
	float progress_percent = 0;
	
	char filename_in[255];
	char filename_raw[255];
	char filename_out[255];
	char filename_buffer[255];
	char cmd_ltsputil[255];
	char cmd_cd[255];
	char name_dir[255];
	char *v_signal[] = {"V(inpulse)", "V(out)", "V(n001)", "V(n014)", "V(n026)"};
	DIR *out_dir;
	strcpy(filename_in, argv[1]);
	FILE *tl_netlist = fopen(filename_in, "r");
	if(!tl_netlist) {
		printf("Could not open netlist file: %s, %s\n", filename_in, strerror(errno));
		return 1;
	}
	long tran, ac, param;
	struct params run;
	printf("Initializing struct params run...\n");
	initialize_params(&run);
	printf("Preparing struct params run...\n");
	/* 
	 * Goes through the input tl_netlist determining
	 * the parameters to be stepped and the low, high
	 * and step size for those parameters. Because we
	 * are not changing the netlist, the tran, ac, and 
	 * param line position don't matter.
	 */
	prepare_run(&run, tl_netlist, &tran, &ac, &param);
	printf("\n");
	for(unsigned int j=0; j < run.num_perm; j++) {
		param_expo_to_str(&run, j);
		/* 
		 * The ouput file names will look like:
		 * StepInformationRline=1.02Lline=1.659nCline=17.82p
		 */
		
		sprintf(filename_out, "StepInformation");
		sprintf(filename_raw, "\0");
		for(int n=0; n < run.num_param; n++) {
				sprintf(filename_buffer, "_%s=%s", run.ext_param_name[n], run.ext_param_str[n]);
				strcat(filename_raw, filename_buffer);
				
				if(n < (run.num_param-2)) {
					sprintf(filename_buffer, "%s=%s", run.ext_param_name[n], run.ext_param_str[n]);
					strcat(filename_out, filename_buffer);
				}
		}
		/* 
		 * Display a Progress Bar to monitor
		 * far the program has to go
		 */
		progress_percent = (float) (100*((double) j/run.num_perm));
	
		printf("\r\033[A\033[A");
		printf("%30s\n", filename_out);
		printf("%4.2f%% |", progress_percent);
		for(int n=0; n < 100; n++) 
			printf("%c", (n < progress_percent) ? '#' : '-');
		printf("|\n");
		fflush(stdout);
		
		for(int n=0; n < 5; n++) {
			sprintf(name_dir, "cdrp=%s_lenline=%s_%s", run.ext_param_str[run.num_param-2], run.ext_param_str[run.num_param-1], v_signal[n]);
			out_dir = opendir(name_dir);
			if(out_dir) {
				closedir(out_dir);
			} else if(ENOENT==errno) {
				if(mkdir(name_dir, S_IRWXU)==-1) {
					printf("Could not create directory, exiting program...\n");
					return 1;
				}
			} else {
				printf("An error has occurred in opening the directory %s", name_dir);
				return 1;
			}
			sprintf(cmd_cd, "cd %s", name_dir);
			
			sprintf(cmd_ltsputil, "./ltsputil -xo2 tran_%s.raw %s.csv \"%%14.6e\" \",\" \"#DATA\" \"time\" \"%s\"", 
					filename_raw, filename_out, v_signal[n]);
			
			/*
			printf("cd MultidropTransient\n");
			printf("%s\n", cmd_cd);
			printf("%s\n", cmd_ltsputil);
			printf("sleep 1\n");
			printf("cd ../..\n");
			*/
			system("cd MultidropTransient");
			system(cmd_cd);
			system(cmd_ltsputil);
			system("sleep 1");
			system("cd ../..");
			
			sprintf(cmd_ltsputil, "./ltsputil -xo2dp ac_%s.raw %s.csv \"%%14.6e\" \",\" \"#DATA\" \"frequency\" \"%s\"", 
					filename_raw, filename_out, v_signal[n]);
			
			/*			
			printf("cd MultidropBode\n");
			printf("%s\n", cmd_cd);
			printf("%s\n", cmd_ltsputil);
			printf("sleep 1\n");
			printf("cd ../..\n");
			*/
			system("cd MultidropBode");
			system(cmd_cd);
			system(cmd_ltsputil);
			system("sleep 1");
			system("cd ../..");
			
			system("Sleep 0.1");
		}
	}	
	return 0;
}