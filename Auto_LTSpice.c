#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>
#include<ctype.h>
	
int exp_to_str(char *str, float expo)
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
	if(exp%3) {
		if(exp > 6) {
			significand *= pow(10, exp-6);
			exp = 6;
		}
		else if(exp > 3 && exp < 6) {
			significand *= pow(10, exp-3);	
			exp = 3;
		} 
		else if(exp > 0 && exp < 3) {
			significand *= pow(10, exp);
			exp = 0;
		}
		else if(exp > -3 && exp < 0) {
			significand *= pow(10, exp+3);
			exp = -3;
		}
		else if(exp > -6 && exp < -3) {
			significand *= pow(10, exp+6);
			exp = -6;
		}
		else if(exp > -9 && exp < -6) {
			significand *= pow(10, exp+9);
			exp = -9;
		}
		else if(exp > -12 && exp < -9) {
			significand *= pow(10, exp+12);
			exp = -12;
		}
		else if(exp > -15 && exp < -12) {
			significand *= pow(10, exp+15);
			exp = -15;
		}
		else if(exp > -18 && exp < -15) {
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
	
int main(int argc, char* argv[])
{
	char file_name_cir[] = "transmission_line.cir";
	char cmd_ltspice[255];
	char cmd_ltsputil[255];
	char cmd_sleep[] = "sleep 1";
	char file_name_txt[17];
	char buffer[255];
	char str_fl[20];
	char cdrp_str[20];
	char length_str[20];

	fpos_t tran_pos;
	fpos_t ac_pos;
	fpos_t param_pos;
	fpos_t temp_pos;

	char *p_ch;
	char *p_tok;
	char *p_end;

	float step_param_data[3][3];
	float step_length[] = {1, 10, 1};
	float step_cdrp[] = {1E-12, 5E-12, 1E-12};

	int length_array_size = (int) ((step_length[1]-step_length[0])/step_length[2]+1);
	int cdrp_array_size = (int) ((step_cdrp[1]-step_cdrp[0])/step_cdrp[2]+1);
	int param_index = 0;
	int data_index = 0;
	int length_index;
	int cdrp_index;
	
	float tmp_data = 0;
	float length_param = step_length[0];
	float cdrp_param = step_cdrp[0];

	float *length_array = malloc(sizeof(float)*length_array_size);
	float *cdrp_array = malloc(sizeof(float)*cdrp_array_size);

	printf("Number of lengths sampled: %i\n", length_array_size);
	printf("Number of cdrps samples: %i\n", cdrp_array_size);
	for(int n=0; n < length_array_size; n++) {
		length_array[n] = length_param;
		printf("Length array[%i] = %f\n", n, length_array[n]);
		length_param += step_length[2];
	}
	for(int n=0; n < cdrp_array_size; n++) {
		cdrp_array[n] = cdrp_param;
		printf("Cdrp array[%i] = %E\n", n, cdrp_array[n]);
		cdrp_param += step_cdrp[2];
	}

	FILE* tl_netlist = fopen(file_name_cir, "r+");
	if(!tl_netlist) {
		printf("Error in Opening Netlist file, %s: %s\n", file_name_cir, strerror(errno));
		return 0;
	}
	while(!feof(tl_netlist)) {
		fgetpos(tl_netlist, &temp_pos);
		fgets(buffer, 255, tl_netlist);
		//printf("File Buffer: %s\n", buffer);
		p_tok = strtok(buffer, " ");
		//unsure if it is commented, so compare after '.'/';'
		if(strcmp(buffer+1, "tran")==0) {
			//Buffer holds transient analysis data
			printf("Transient data\n");
			buffer[5] = ' ';
			tran_pos = temp_pos;
		}
		else if(strcmp(buffer+1, "ac")==0) {
			//Buffer holds ac analysis data
			printf("AC data\n");
			buffer[3] = ' ';
			ac_pos = temp_pos;
		}
		else if(strcmp(buffer, ".param")==0) {
			//Buffer holds parameter data
			printf("Parameter data\n");
			buffer[6] = ' ';
			param_pos = temp_pos;
			p_ch = buffer;
			do {
				if(memcmp(p_ch-7, "Lenline", 7)==0)
					length_index = p_ch - buffer;
				else if(memcmp(p_ch-4, "Cdrp", 4)==0)
					cdrp_index = p_ch - buffer;
			} while(*(++p_ch));
		}
		else if(strcmp(buffer, ".step")==0) {
			//Buffer holds step parameter data
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
						case 'n': strcpy(p_ch, "E-9"); break;
						case 'u': strcpy(p_ch, "E-6"); break;
						case 'm': strcpy(p_ch, "E-3"); break;
						case 'k': strcpy(p_ch, "E3"); break;
						case 'M': strcpy(p_ch, "E6"); break;
					}
				} while(*(++p_ch));
				tmp_data = strtof(str_fl, &p_end);
				if(str_fl != p_end) {
					step_param_data[param_index][data_index] = tmp_data;
					data_index++;
				}
				p_tok = strtok(NULL, " ");
			}
		}
	}
	printf("Parameter step data...\n");
	for(int i=0; i < 3; i++) {
		for(int j=0; j < 3; j++)
			printf("%e ", step_param_data[i][j]);
		printf("\n");
	}
	//step one: run all ltspice netlists with varying parameters in length, cdrp and ac vs tran
	//0: tran
	//1: ac
	sprintf(cmd_ltspice, "./scad3.exe -b %s", file_name_cir);
	//sprintf(cmd_ltsputil, "./ltsputil -xo7 transmission_line.raw transmission_line.txt \"%14.6e\" \",\" \"V(inpulse)\" \"V(out)\" \"V(n001)\" \"V(n014)\" \"V(n026)\"");
	
	printf("LTSpice Cmd: %s\n", cmd_ltspice);
	
	for(int i=1; i > -1; i--) {
		if(!tl_netlist)
			tl_netlist = fopen(file_name_cir, "r+");
		if(i == 0) { 
			printf("\nBeginning transient analysis...\n");
			fsetpos(tl_netlist, &tran_pos);
		}
		else if(i == 1) { 
			printf("\nBeginning ac analysis...\n");
			fsetpos(tl_netlist, &ac_pos);
		}
		//put '.' to overwrite ';', uncommenting that analysis
		fputc('.', tl_netlist);
		//put ';' to comment out the other analysis
		fsetpos(tl_netlist, i ? &tran_pos : &ac_pos);
		fputc(';', tl_netlist);
		fflush(tl_netlist);
		for(int j=0; j < cdrp_array_size; j++) {
			printf("\n");
			fsetpos(tl_netlist, &param_pos);
			fgets(buffer, 255, tl_netlist);
			exp_to_str(cdrp_str, cdrp_array[j]);
			p_ch = buffer + cdrp_index;
			//p_ch points to '=', so add one
			memcpy(p_ch+1, cdrp_str, strlen(cdrp_str));
			for(int k=0; k < length_array_size; k++) {
				exp_to_str(length_str, length_array[k]);
				p_ch = buffer + length_index;
				//p_ch points to '=', so add one
				//excludes '\0' since it is memcpy
				memcpy(p_ch+1, length_str, strlen(length_str));
				printf("%s", buffer);
				//change the .param line
				fsetpos(tl_netlist, &param_pos);
				fputs(buffer, tl_netlist);
				//close the transmission_line.cir file so scad3.exe can use it
				fclose(tl_netlist);
				//style of file_name_txt = tlcdrp={cdrp_str}len={length_str}
				sprintf(file_name_txt, "%s_cdrp=%s_len%s", i ? "ac" : "tran", cdrp_str, length_str);
				/*
				p_ch = cmd_ltsputil;
				do {
					if(memcmp(p_ch, "transmission_line.txt",20)==0 || memcmp(p_ch, "cdrp=", 5)==0) 
						break;
				} while(*(++p_ch));
				*/
				sprintf(cmd_ltsputil, "./ltsputil -xo7 transmission_line.raw %s.csv \"%%14.6e\" \",\" \"time\" \"V(inpulse)\" \"V(out)\" \"V(n001)\" \"V(n014)\" \"V(n026)\"", file_name_txt);
				//memcpy(p_ch, file_name_txt, strlen(file_name_txt));
				printf("CMD LTSputil: %s\n", cmd_ltsputil);
				system(cmd_ltspice);
				system(cmd_sleep);
				//system("./Auto_LTSpiceBatch.sh");
				system(cmd_ltsputil);
			}
		}
	}		
	//step two: run ltsputil on .raw files to export waveforms as comma separated
	free(length_array);
	free(cdrp_array);
	fclose(tl_netlist);
	return 0;
}