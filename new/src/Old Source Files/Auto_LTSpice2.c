#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>
#include<ctype.h>
	
int expo_to_str(char *str, float expo)
{
	char *pch;
	char buffer[10];
	float significand;
	int exp;
	sprintf(str, "%E", expo);
	strcpy(buffer, str);
	strtok(buffer, "E");
	significand = strtof(buffer, &pch);
	exp = (int) strtol(pch+1, NULL, 10);
	if(exp % 3) {
		if(exp > 6) {
			significand *= pow(10, exp-6);
			exp = 6;
		} else if(exp > 3 && exp < 6) {
			significand *= pow(10, exp-3);	
			exp = 3;
		} else if(exp > 0 && exp < 3) {
			significand *= pow(10, exp);
			exp = 0;
		} else if(exp > -3 && exp < 0) {
			significand *= pow(10, exp+3);
			exp = -3;
		} else if(exp > -6 && exp < -3) {
			significand *= pow(10, exp+6);
			exp = -6;
		} else if(exp > -9 && exp < -6) {
			significand *= pow(10, exp+9);
			exp = -9;
		} else if(exp > -12 && exp < -9) {
			significand *= pow(10, exp+12);
			exp = -12;
		} else if(exp > -15 && exp < -12) {
			significand *= pow(10, exp+15);
			exp = -15;
		} else if(exp > -18 && exp < -15) {
			significand *= pow(10, exp+18);
			exp = -18;
		}
	}
	sprintf(str, "%3.1f", significand);
	switch(exp) {
		case -18: strcpy(str+3, "a"); break;
		case -15: strcpy(str+3, "f"); break;
		case -12: strcpy(str+3, "p"); break;
		case -9: strcpy(str+3, "n"); break;
		case -6: strcpy(str+3, "u"); break;
		case -3: strcpy(str+3, "m"); break;
		case 0: strcpy(str+3, ""); break;
		case 3: strcpy(str+3, "k"); break;
		case 6: strcpy(str+3, "Meg"); break;
		default: strcpy(str+3, ""); printf("Error in exp_to_str in switch statement!\n"); return 1;
	}
	return 0;
}
	
int generate_step_array(float low, float high, float step, float *step_arary)
{
	int array_size = (int) ((high-low)/step+1);
	int tmp = low;
	step_array = malloc(sizeof(float)*array_size);
	if(!step_array)
		return 1;
	for(int n=0; n < array_size; n++) {
		step_array[n] = tmp;
		tmp += step;
	}
}

int main(int argc, char* argv[])
{
	if(argc < 2) {
		printf("Insufficient number of command-line arguments\n");
		return 1;
	}
	char filename_in[255];
	strcpy(filename_in, argv[1]);
	//char file_name_cir[] = "transmission_line.cir";
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

	float step_param_data[3][3];

	int param_index = 0;
	int data_index = 0;
	int length_index;
	int cdrp_index;
	
	float tmp_data = 0;
	
	float length_low = 1;
	float length_high = 10;
	float length_step = 1;
	
	float cdrp_low = 1E-12;
	float cdrp_high = 5E-12;
	float cdrp_step = 1E-12;
	
	float length_param = step_length[0];
	float cdrp_param = step_cdrp[0];

	float *length_array;
	float *cdrp_array;

	generate_step_array(length_low, length_high, length_step, length_array);
	generate_step_array(cdrp_low, cdrp_high, cdrp_step, cdrp_array);

	FILE* tl_csv;
	FILE* tl_netlist = fopen(file_name_cir, "r+");
	if(!tl_netlist) {
		printf("Error in Opening Circuit file, %s: %s\n", file_name_cir, strerror(errno));
		return 0;
	}
	while(!feof(tl_netlist)) {
		temp_l = ftell(tl_netlist);
		fgets(buffer, 255, tl_netlist);
		p_tok = strtok(buffer, " ");
		if(strcmp(buffer+1, "tran")==0) {
			printf("Transient data\n");
			buffer[5] = ' ';
			tran_l = temp_l;
		} else if(strcmp(buffer+1, "ac")==0) {
			printf("AC data\n");
			buffer[3] = ' ';
			ac_l = temp_l;
		} else if(strcmp(buffer, ".param")==0) {
			printf("Parameter data\n");
			buffer[6] = ' ';
			param_l = temp_l;
			p_ch = buffer;
			do {
				if(memcmp(p_ch-7, "Lenline", 7)==0)
					length_index = p_ch - buffer;
				else if(memcmp(p_ch-4, "Cdrp", 4)==0)
					cdrp_index = p_ch - buffer;
			} while(*(++p_ch));
		} else if(strcmp(buffer, ".step")==0) {
			if(p_tok[12] == 'R')
				param_index = 0;
			else if(p_tok[12] == 'L')
				param_index = 1;	
			else if(p_tok[12] == 'C')
				param_index = 2;
			else
				printf("Error!\n");
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
					step_param_data[param_index][data_index] = tmp_data;
					data_index++;
				}
				p_tok = strtok(NULL, " ");
			}
		}
	}
	/*
	 * step one: run all ltspice netlists with varying parameters in length, cdrp and ac vs tran
	 * 0: tran
	 * 1: ac
	 */
	sprintf(cmd_ltspice, "./scad3.exe -b %s", file_name_cir);
	for(int i=0; i < 2; i++) {
		tl_netlist = freopen(file_name_cir, "r+", tl_netlist);
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
		/* put ';' to comment out the other analysis line */
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
			tl_netlist = freopen(file_name_cir, "r+", tl_netlist);
			printf("\n");
			if(fseek(tl_netlist, param_l, SEEK_SET))
					perror("Error in cdrp param fseek");
			fgets(buffer, 255, tl_netlist);
			expo_to_str(cdrp_str, cdrp_array[j]);
			p_ch = buffer + cdrp_index;
			memcpy(p_ch+1, cdrp_str, strlen(cdrp_str));
			for(int k=0; k < length_array_size; k++) {
				tl_netlist = freopen(file_name_cir, "r+", tl_netlist);
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
	free(length_array);
	free(cdrp_array);
	fclose(tl_netlist);
	return 0;
}