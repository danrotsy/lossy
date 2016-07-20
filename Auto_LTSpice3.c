#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>
#include<ctype.h>
	
#include"Auto_LTS.h"

int main(int argc, char* argv[])
{
	/*
	 * The one command-line argument given
	 * is the netlist / circuit file for
	 * spice to simulate with parameters 
	 * stepped as provided in netlist file.
	 * the .step directives MUST be commented
	 * out though, or else ltsputil will mess
	 * up the CSV files outputted. If not enough
	 * arguments are given, exit immediately
	 * or else argv[1] will be undefined
	 */
	if(argc < 2) {
		printf("Insufficient number of command-line arguments\n");
		return 1;
	}
	FILE* tl_netlist = fopen(filename_in, "r+");
	if(!tl_netlist) {
		printf("Error in Opening Spice file, %s: %s\n", filename_in, strerror(errno));
		return 1;
	}
	struct params run;
	initialize_params(&run);
	
	char filename_in[255];
	strcpy(filename_in, argv[1]);
	
	char cmd_ltspice[255];
	sprintf(cmd_ltspice, "./scad3.exe -b %s", filename_in);
	
	char filename_out[255];
	char filename_raw[255];
	char filename_buffer[255];
	char buffer[255];
	char str_fl[20];
	
	long tran_pos;
	long ac_pos;
	long param_pos;
	long tmp_pos;

	char *p_ch;
	char *p_tok;
	char *p_end;
		
	int param_index = 0;
	int data_index = 0;
	
	float tmp_data = 0;

	FILE* tl_csv;
	/*
	 * Loop through the input file looking
	 * for the lines that specify .tran, .ac, 
	 * .param and .step spice directives
	 * After determining if the current line
	 * is any of one of those, the file stream
	 * "cursor" is saved specifying which directive
	 * was on that line. For the .param line the char
	 * index of each stepped variable is saved, as to
	 * sub in the new values each simualation run,
	 * so the .param line MUST come after the .step
	 * param directives in the input file
	 */
	while(!feof(tl_netlist)) {
		tmp_pos = ftell(tl_netlist);
		fgets(buffer, 255, tl_netlist);
		p_tok = strtok(buffer, " ");
		if(strcmp(buffer+1, "tran")==0) {
			printf("Transient data\n");
			buffer[5] = ' ';
			tran_pos = tmp_pos;
		} else if(strcmp(buffer+1, "ac")==0) {
			printf("AC data\n");
			buffer[3] = ' ';
			ac_pos = tmp_pos;
		} else if(strcmp(buffer, ".param")==0) {
			printf("Parameter data\n");
			buffer[6] = ' ';
			param_pos = tmp_pos;
			p_ch = buffer;
			/*
			 * This loop 
			do {
				for(int n=0; n < run.num_param; n++) {
					int length = strlen(run.ext_param_name[n]);
					if(memcmp(p_ch-length, run.ext_param_name[n], length)==0)
						run.ext_param_index[n] = p_ch - buffer;
				}
			} while(*(++p_ch));
		} else if(strcmp(buffer+1, "step")==0) {
			strtok(NULL, " ");
			p_tok = strtok(NULL, " ");
			printf("Strcpy %s into run.ext_param_name[%d]\n", p_tok, run.num_param);
			printf("Current size: %d\n", run.curr_size);
			strcpy(run.ext_param_name[run.num_param], p_tok);
			data_index = 0;
			while(p_tok!=NULL) {
				p_end = p_tok;
				printf("Strcpy p_tok: %s into str_fl...\n", p_tok);
				strcpy(str_fl, p_tok);
				p_ch = str_fl;
				do {
					switch(*p_ch) {
					case 'a': strcpy(p_ch, "E-18"); break;
					case 'f': strcpy(p_ch, "E-15"); break;
					case 'p': strcpy(p_ch, "E-12"); break;
					case 'n':strcpy(p_ch, "E-9"); break;
					case 'u':strcpy(p_ch, "E-6"); break;
					case 'm':strcpy(p_ch, "E-3"); break;
					case 'k': strcpy(p_ch, "E3"); break;
					case 'M':strcpy(p_ch, "E6"); break;
					}
				} while(*(++p_ch));
				tmp_data = strtof(str_fl, &p_end);
				if (str_fl != p_end) {
					run.ext_param_step_array[run.num_param][data_index] = tmp_data;
					data_index++;
				}
				p_tok = strtok(NULL, " ");
			}
			run.num_param++;
			if(run.num_param >= run.curr_size) {
				run.curr_size = run.num_param + 3;
				printf("Reallocate param memory...\n");
				realloc_params(&run);
			}
		}
	}
	for(int i=0; i < run.num_param; i++) {
		printf("\n%s\n", run.ext_param_name[i]);
		for(int j=0; j < 3; j++) 
			printf("%e ", run.ext_param_step_array[i][j]);
	}
	printf("\n");
	
	for(int n=0; n < run.num_param; n++)
		run.ext_param_curr[n] = run.ext_param_step_array[n][0];
	
	printf("Generating data arrays for .step params...\n");
	for(int n=0; n < run.num_param; n++)
		generate_data_array(run.ext_param_step_array[n][0], run.ext_param_step_array[n][1], run.ext_param_step_array[n][2], run.ext_param_data_array[n]);
	
	unsigned int num_perm = 1;
	printf("Generating the number of run.ext_param_permutations for .step params...\n");
	for(int n=0; n < run.num_param; n++) {
		num_perm *= (unsigned int)((run.ext_param_step_array[n][1]-run.ext_param_step_array[n][0])/run.ext_param_step_array[n][2]+1);
	}
	printf("Allocating memory for an array of size %u with float* to store run.ext_param_permutations...\n", num_perm);
	run.ext_param_permutations = malloc(num_perm*sizeof(float*));
	if(!run.ext_param_permutations)
		printf("Not enough memory to allocate run.ext_param_permutations array\n");
	
	for(unsigned int i=0; i < num_perm; i++) {
		run.ext_param_permutations[i] = malloc(run.num_param*sizeof(float));
		for(int j=0; j < run.num_param; j++) {
			run.ext_param_permutations[i][j] = run.ext_param_curr[j];
		}
		update_run(&run);
	}
	for(int n=0; n < run.num_param; n++)
		run.ext_param_curr[n] = run.ext_param_step_array[n][0];
	
	printf("Number of run.ext_param_permutations: %d\n", num_perm);
	
	/*
	 * step one: run all ltspice netlists with varying parameters in length, cdrp and ac vs tran
	 * 0: tran
	 * 1: ac
	 */
	sprintf(cmd_ltspice, "./scad3.exe -b %s", filename_in);
	strcpy(filename_raw, filename_in);
	strcpy(filename_raw+strlen(filename_raw)-3, "raw");
	
	for(int i=0; i < 2; i++) {
		tl_netlist = freopen(filename_in, "r+", tl_netlist);
		printf("tl_netlist==NULL: %d\n", tl_netlist==NULL);
		printf("ac_pos: %ld, tran_pos: %ld\n", ac_pos, tran_pos);
		if(i == 0) { 
			printf("\nBeginning transient analysis...\n");
			if(fseek(tl_netlist, tran_pos, SEEK_SET))
					perror("Error in tran fseek");
		}
		else if(i == 1) { 
			printf("\nBeginning ac analysis...\n");
			if(fseek(tl_netlist, ac_pos, SEEK_SET))
					perror("Error in ac fseek");
		}
		fputc('.', tl_netlist);
		/* put ';' to comment out the other analysis line */
		if(i == 0) {
			if (fseek(tl_netlist, ac_pos, SEEK_SET))
					perror("Error in 2nd ac fseek");
		}
		else if(i == 1) {
			if (fseek(tl_netlist, tran_pos, SEEK_SET))
					perror("Error in 2nd tran fseek");
		}
		fputc(';', tl_netlist);
		for(unsigned int j=0; j < num_perm; j++) {
			tl_netlist = freopen(filename_in, "r+", tl_netlist);
			printf("\n");
			if(fseek(tl_netlist, param_pos, SEEK_SET))
				perror("Error in param fseek");
			fgets(buffer, 255, tl_netlist);
			param_expo_to_str(&run, j);
			for(int k=0; k < run.num_param; k++) {
				p_ch = buffer + run.ext_param_index[k];
				memcpy(p_ch+1, run.ext_param_str[k], strlen(run.ext_param_str[k]));
			}
			printf("Completed buffer: %s", buffer);
			if(fseek(tl_netlist, param_pos, SEEK_SET))
				perror("Error in second param fseek");
			fputs(buffer, tl_netlist);
			if(fclose(tl_netlist))
				perror("Error in closing file");
			sprintf(filename_out, "%s", i ? "ac" : "tran");
			for(int n=0; n < run.num_param; n++) {
				sprintf(filename_buffer, "_%s=%s", run.ext_param_name[n], run.ext_param_str[n]);
				strcat(filename_out, filename_buffer);
			}
			strcat(filename_out, ".raw");
			printf("Raw file: %s\n", filename_out);
			system(cmd_ltspice);
			rename(filename_raw, filename_out);
		}
	}
	fclose(tl_netlist);
	return 0;
}