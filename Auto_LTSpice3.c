#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>
#include<ctype.h>
	
#include"Auto_LTS.h"

int main(int argc, char* argv[])
{
	if(argc < 2) {
		printf("Insufficient number of command-line arguments\n");
		return 1;
	}
	char filename_in[255];
	strcpy(filename_in, argv[1]);
	char cmd_ltspice[255];
	char cmd_ltsputil[255];
	char filename_out[17];
	char buffer[255];
	char str_fl[20];
	char cdrp_str[20];
	char length_str[20];

	long tran_pos;
	long ac_pos;
	long param_pos;
	long tmp_pos;

	char *p_ch;
	char *p_tok;
	char *p_end;

	char **step_name;
	float *step_data[3];
	int num_step = 3;
	int curr_size = 3;
	for(int n=0; n < 3; n++)
		step_data[n] = malloc(curr_size*sizeof(float));
	step_name = malloc(curr_size*sizeof(char*));
	for(int i=0; i < curr_size; i++) 
		step_name[i] = malloc(255*sizeof(char));
	int param_index = 0;
	int data_index = 0;
	int *ext_param_index = malloc(curr_size*sizeof(int));
	
	float tmp_data = 0;
	
	float length_low = 1;
	float length_high = 10;
	float length_step = 1;
	
	float cdrp_low = 1E-12;
	float cdrp_high = 5E-12;
	float cdrp_step = 1E-12;

	float **ext_array;
	
	FILE* tl_csv;
	FILE* tl_netlist = fopen(filename_in, "r+");
	if(!tl_netlist) {
		printf("Error in Opening Spice file, %s: %s\n", filename_in, strerror(errno));
		return 0;
	}
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
			do {
				for(int n=3; n < num_step; n++) {
					int length = strlen(step_name[n]);
					if(memcmp(p_ch-length, "Lenline", length)==0)
						ext_param_index[n-3] = p_ch - buffer;
				}
			} while(*(++p_ch));
		} else if(strcmp(buffer, ".step")==0) {
			strtok(buffer, " ");
			p_tok = strtok(buffer, " ");
			strcpy(step_name[num_step], p_tok);
			data_index = 0;
			while(p_tok!=NULL) {
				p_end = p_tok;
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
					step_data[num_step][data_index] = tmp_data;
					data_index++;
				}
				p_tok = strtok(NULL, " ");
			}
			num_step++;
			if(num_step > curr_size) {
				curr_size = num_step + 3;
				for(int i=0; i < 3; i++) {
						step_data[i] = realloc(step_data[i], curr_size*sizeof(float));
						if(!step_data[i]) {
							printf("Could not realloc memory for step_data\n");
							return 1;
						}
				}
				step_name = realloc(step_name, curr_size*sizeof(char*));
				for(int i=num_step; i < curr_size; i++)
					step_name[i] = malloc(255*sizeof(char));
				ext_param_index = realloc(ext_param_index, curr_size*sizeof(int));
			}
		}
	}
	for(int i=0; i < num_step; i++) {
		printf("%s\n", step_name[i]);
		for(int j=0; j < 3; j++) 
			printf("%e", step_data[j][i]);
	}
	for(int n=3; n < num_step; n++)
		generate_step_array(step_data[0][n], step_data[1][n], step_data[2][n], ext_array[n]);

/*

	 * step one: run all ltspice netlists with varying parameters in length, cdrp and ac vs tran
	 * 0: tran
	 * 1: ac

	sprintf(cmd_ltspice, "./scad3.exe -b %s", filename_in);
	for(int i=0; i < 2; i++) {
		tl_netlist = freopen(filename_in, "r+", tl_netlist);
		printf("tl_netlist==NULL: %d\n", tl_netlist==NULL);
		printf("ac_l: %ld, tran_l: %ld\n", ac_l, tran_l);
		if(i == 0) { 
			printf("\nBeginning transient analysis...\n");
			if(fseek(tl_netlist, tran_l, SEEK_SET))
					perror("Error in tran fseek");
		}
		else if(i == 1) { 
			printf("\nBeginning ac analysis...\n");
			if(fseek(tl_netlist, ac_l, SEEK_SET))
					perror("Error in ac fseek");
		}
		fputc('.', tl_netlist);
		* put ';' to comment out the other analysis line *
		if(i == 0) {
			if (fseek(tl_netlist, ac_l, SEEK_SET))
					perror("Error in 2nd ac fseek");
		}
		else if(i == 1) {
			if (fseek(tl_netlist, tran_l, SEEK_SET))
					perror("Error in 2nd tran fseek");
		}
		fputc(';', tl_netlist);
		for(int j=0; j < cdrp_array_size; j++) {
			tl_netlist = freopen(filename_in, "r+", tl_netlist);
			printf("\n");
			if(fseek(tl_netlist, param_l, SEEK_SET))
					perror("Error in cdrp param fseek");
			fgets(buffer, 255, tl_netlist);
			expo_to_str(cdrp_str, cdrp_array[j]);
			p_ch = buffer + cdrp_index;
			memcpy(p_ch+1, cdrp_str, strlen(cdrp_str));
			for(int k=0; k < length_array_size; k++) {
				tl_netlist = freopen(filename_in, "r+", tl_netlist);
				expo_to_str(length_str, length_array[k]);
				p_ch = buffer + length_index;
				memcpy(p_ch+1, length_str, strlen(length_str));
				printf("Buffer: %s", buffer);
				printf("param_l: %ld\n", param_l);
				if(fseek(tl_netlist, param_l, SEEK_SET))
					perror("Error in len param fseek");
				fputs(buffer, tl_netlist);
				if(fclose(tl_netlist))
					perror("Error in closing file");
				sprintf(file_name_out, "%s_cdrp=%s_len%s.raw", i ? "ac" : "tran", cdrp_str, length_str);
				printf("Raw file: %s\n", file_name_out);
				system("./scad3.exe -b transmission_line.cir");
				rename("transmission_line.raw", file_name_out);
			}
		}
	}
*/
	free(ext_array);
	fclose(tl_netlist);
	return 0;
}