#ifndef AUTO_LTS_2_H
#define AUTO_LTS_2_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>
#include<ctype.h>
#include<sys/stat.h>

/*
 * This struct stores the information
 * characterizing the paramters that
 * will be incremented. ext_param_name
 * & ext_param_str are pointers to arrays
 * of char* strings. ext_param_step_array
 * is an array of arrays, so that each array
 * characterizes the lowest value, the highest
 * value and the step size for that nth parameter
 * i.e. ext_param_step_array[parameter][low=0|high=1|step=2]
 * ext_param_data_array is an array made up of each
 * value that each paramter can take. This array's
 * values are created with the generate_data_array
 * function. ext_param_permutations gives the combinations
 * of each data array, so that iterating through ext_param_permutations
 * produces the unique paramter values for that simulation.
 * ext_param_index is an array of the char indices that each
 * parameter is defined on the .param line for so that later 
 * each new paramter value obtained from ext_param_permutations
 * can be subbed into the correct place on the .param spice directive.
 */
 
enum {
	MAX_CHAR = 255
};

char MAX_CURRENT[] = "-5m";
char MIN_CURRENT[] = "0";
char IN_PULSE[] = "InPulse";
char IN_REF_PULSE[] = "InRefPulse";
char RISE_FALL[] = "200p";
char *DELTA_DELAY[] = {"1n", "2n", "3n", "4n", "5n", "6n", "7n", "8n", "9n", "10n", "1.5n"};

struct netlist
{
	char **param_name;
	char **param_strvalue;
	int *vin;
	char **save_str;
	
	float *param_fvalue;
	
	char tran_str[MAX_CHAR];
	char ac_str[MAX_CHAR];
	
	long tran_pos;
	long ac_pos;
	
	char in_label[MAX_CHAR];
	char in_ref_label[MAX_CHAR];
	char v_rise[MAX_CHAR];
	char v_fall[MAX_CHAR];
	char v_on[MAX_CHAR];
	char ac_amplitude[MAX_CHAR];
	float delta_delay;
	//char **delay;
	char **current_str;
	
	int is_tran;
	
	int num_sources;
	int sources_size;

	int save_size;
	int num_save;
	
	int num_currents;
	
	int num_param;
	int param_size;
}; 

/* Function prototypes */
int initialize_netlist(struct netlist *net, int argc, char **argv);
int realloc_netlist(struct netlist *net);
int free_netlist(struct netlist *net);
float str_to_expo(char *str);
int expo_to_str(char *str, float expo);
int prepare_netlist(struct netlist *net, FILE *tl_netlist);

/*
 * Dynamically allocates memory for the struct
 * as it is unknown at compile-time how much
 * memory to allocate for arrays storing information
 * on the stepped parameters
 */
int initialize_netlist(struct netlist *net, int argc, char **argv)
{
	printf("Allocating netlist memory...\n");
	
	net->param_size = 3;
	net->num_param = 0;

	net->param_name = malloc(net->param_size*sizeof(char*));
	net->param_strvalue = malloc(net->param_size*sizeof(char*));
	net->param_fvalue = malloc(net->param_size*sizeof(float));
	for(int n=0; n < net->param_size; n++) {
		net->param_name[n] = malloc(MAX_CHAR*sizeof(char));
		net->param_strvalue[n] = malloc(MAX_CHAR*sizeof(char));
	}	
	
	printf("Determining the number of pulses...\n");
	int curr_index = 0;
	int max_size = 5;
	char *p_tok;
	p_tok = strtok(argv[9], " ");
	net->vin = malloc(max_size*sizeof(char*));
	while(p_tok != NULL) {
		if(curr_index >= max_size) {
			max_size += 3;
			net->vin = realloc(net->vin, max_size*sizeof(char*));
		}
		//net->vin_str[curr_index] = malloc(MAX_CHAR*sizeof(char));
		net->vin[curr_index] = atoi(p_tok);
		//strcpy(net->vin_str[curr_index], p_tok);
		curr_index++;
		p_tok = strtok(NULL, " ");
	}
	net->num_currents = curr_index;
	net->vin = realloc(net->vin, net->num_currents*sizeof(char*));
	//net->delay = malloc(net->num_currents*sizeof(char*));
	net->current_str = malloc(net->num_currents*sizeof(char*));
	printf("Determining the delay between pulses...\n");
	
	char delay_str[MAX_CHAR];
	float delta_delay = str_to_expo(argv[10]);
	float rise_fall = str_to_expo(RISE_FALL);
	char buffer_str[MAX_CHAR];

	strcpy(net->ac_amplitude, "1");
	
	float on = delta_delay;
	char on_str[MAX_CHAR];
	float delay = 0;
	net->sources_size = 5;
	net->num_sources = 0;
	net->current_str = malloc(net->sources_size*sizeof(char*));
	for(int n=0; n < net->sources_size; n++)
		net->current_str[n] = malloc(MAX_CHAR*sizeof(char));
	
	net->num_sources = 0;
	for(int n=0; n < net->num_currents; n++) {
		if(net->num_sources >= net->sources_size) {
			net->sources_size += 3;
			net->current_str = realloc(net->current_str, net->sources_size*sizeof(char*));
			for(int n=net->num_sources; n < net->sources_size; n++)
				net->current_str[n] = malloc(MAX_CHAR*sizeof(char));
		}
		
		if(n == (net->num_currents-1)) {
			if(net->vin[n] == 1)
				strcpy(buffer_str, MAX_CURRENT);
			else
				strcpy(buffer_str, MIN_CURRENT);
			expo_to_str(delay_str, delay);
			expo_to_str(on_str, on - rise_fall);
			printf("I%d %s %s PULSE(0 %s %s %s %s %s) AC %s\n", (net->num_sources+1), IN_PULSE, IN_REF_PULSE, 
					buffer_str, delay_str, RISE_FALL, "0", on_str, net->ac_amplitude);
			sprintf(net->current_str[net->num_sources], "I%d %s %s PULSE(0 %s %s %s %s %s) AC %s\n", (net->num_sources+1), IN_PULSE, IN_REF_PULSE, 
					buffer_str, delay_str, RISE_FALL, "0", on_str, net->ac_amplitude);
			delay += on;
			net->num_sources++;
		}
		else if(net->num_sources == 0 && net->vin[n] != net->vin[n+1]) {
			if(net->vin[n] == 1)
				strcpy(buffer_str, MAX_CURRENT);
			else
				strcpy(buffer_str, MIN_CURRENT);
			expo_to_str(delay_str, delay);
			expo_to_str(on_str, on - rise_fall);
			printf("I%d %s %s PULSE(0 %s %s %s %s %s) AC %s\n", (net->num_sources+1), IN_PULSE, IN_REF_PULSE, 
					buffer_str, delay_str, "0", RISE_FALL, on_str, net->ac_amplitude);
			sprintf(net->current_str[net->num_sources], "I%d %s %s PULSE(0 %s %s %s %s %s) AC %s\n", (net->num_sources+1), IN_PULSE, IN_REF_PULSE, 
					buffer_str, delay_str, "0", RISE_FALL, on_str, net->ac_amplitude);
			delay += on;
			on = delta_delay;
			net->num_sources++;
		}
		else if(net->vin[n] != net->vin[n+1]) {
			if(net->vin[n] == 1)
				strcpy(buffer_str, MAX_CURRENT);
			else
				strcpy(buffer_str, MIN_CURRENT);
			expo_to_str(delay_str, delay);
			expo_to_str(on_str, on - 2*rise_fall);
			printf("I%d %s %s PULSE(0 %s %s %s %s %s) AC %s\n", (net->num_sources+1), IN_PULSE, IN_REF_PULSE, 
					buffer_str, delay_str, RISE_FALL, RISE_FALL, on_str, net->ac_amplitude);
			sprintf(net->current_str[net->num_sources], "I%d %s %s PULSE(0 %s %s %s %s %s) AC %s\n", (net->num_sources+1), IN_PULSE, IN_REF_PULSE, 
					buffer_str, delay_str, RISE_FALL, RISE_FALL, on_str, net->ac_amplitude);
			delay += on;
			on = delta_delay;
			net->num_sources++;
		}
		else {
			on += delta_delay;
		}
	}
	for(int n=net->num_sources; n < net->sources_size; n++)
		free(net->current_str[n]);
	
	printf("Number of sources: %d\n", net->num_sources);
	net->current_str = realloc(net->current_str, net->num_sources*sizeof(char*));
/*
	for(int n=0; n < net->num_currents; n++) {
		//net->delay[n] = malloc(MAX_CHAR*sizeof(char));
		net->current_str[n] = malloc(MAX_CHAR*sizeof(char));
		expo_to_str(delay_tmp_str, delay_tmp);
		//printf("Delay[%d] = %s\n", n, net->delay[n]);
		if(n != 0 && n != (net->num_currents-1)) {
			if(net->vin[n] == net->vin[n-1] && net->vin[n] == net->vin[n+1])
				sprintf(net->current_str[n], "I%d %s %s PULSE(0 %s %s %s %s %s) AC %s\n", (n+1), IN_PULSE, IN_REF_PULSE, 
					buffer_str, delay_tmp_str, "0", "0", DELTA_DELAY[atoi(argv[10])]);
			else if(net->vin[n] != net->vin[n-1] && net->vin[n] == net->vin[n+1])
				sprintf(net->current_str[n], "I%d %s %s PULSE(0 %s %s %s %s %s) AC %s\n", (n+1), IN_PULSE, IN_REF_PULSE, 
					buffer_str, delay_tmp_str, net->v_rise, "0", on1);
			else if(net->vin[n] == net->vin[n-1] && net->vin[n] != net->vin[n+1])
				sprintf(net->current_str[n], "I%d %s %s PULSE(0 %s %s %s %s %s) AC %s\n", (n+1), IN_PULSE, IN_REF_PULSE, 
					buffer_str, delay_tmp_str, "0", RISE_FALL, on1);
			else
				sprintf(net->current_str[n], "I%d %s %s PULSE(0 %s %s %s %s %s) AC %s\n", (n+1), IN_PULSE, IN_REF_PULSE, 
					buffer_str, delay_tmp_str, RISE_FALL, RISE_FALL, on2);
		}
		else if(n == (net->num_currents-1))
			if(net->vin[n] == net->vin[n-1])
				sprintf(net->current_str[n], "I%d %s %s PULSE(0 %s %s %s %s %s) AC %s\n", (n+1), IN_PULSE, IN_REF_PULSE, 
					buffer_str, delay_tmp_str, "0", "0", DELTA_DELAY[atoi(argv[10])], net->ac_amplitude);
			else
				sprintf(net->current_str[n], "I%d %s %s PULSE(0 %s %s %s %s %s) AC %s\n", (n+1), IN_PULSE, IN_REF_PULSE, 
					buffer_str, delay_tmp_str, RISE_FALL, "0", on1, net->ac_amplitude);
		else
			sprintf(net->current_str[n], "I%d %s %s PULSE(0 %s %s %s %s %s) AC %s\n", (n+1), IN_PULSE, IN_REF_PULSE, 
								buffer_str, delay_tmp_str, "0", "0", DELTA_DELAY[atoi(argv[10])], net->ac_amplitude);
		delay_tmp += delta_delay;
	}
*/
	net->is_tran = 0;
	if((strcmp(argv[2], "tran")==0) || (strcmp(argv[2], "TRAN")==0))
		net->is_tran = 1;

	net->save_size = 8;
	net->num_save = 0;
	net->save_str = malloc(net->save_size*sizeof(char*));
	for(int n=0; n < net->save_size; n++)
		net->save_str[n] = malloc(MAX_CHAR*sizeof(char));
	
	char tran_time[MAX_CHAR];
	//simulation windows in 1.5 times longer than all the pulses together
	expo_to_str(tran_time, 1.5*delay);
	printf("Delay tmp: %e to %s\n", delay, tran_time);
	sprintf(net->tran_str, ".tran 0 %s\n", tran_time);
	
	return 0;
}

/*
 * Reallocates the struct's memory blocks
 * when the number of parameters to be stepped
 * is updated. 
 */
int realloc_netlist(struct netlist *net)
{
	printf("Reallocating netlist param string memory...\n");	
	net->param_size += 3;
	
	net->param_name = realloc(net->param_name, net->param_size*sizeof(char*));
	net->param_strvalue = realloc(net->param_strvalue, net->param_size*sizeof(char*));
	net->param_fvalue = realloc(net->param_fvalue, net->param_size*sizeof(float));
	for(int n=net->num_param; n < net->param_size; n++) {
		net->param_name[n] = malloc(MAX_CHAR*sizeof(char));
		net->param_strvalue[n] = malloc(MAX_CHAR*sizeof(char));
	}

	return 0;
}

int realloc_save(struct netlist *net)
{
	printf("Reallocating netlist save string memory...\n");
	net->save_size += 3;
	
	net->save_str = realloc(net->save_str, net->save_size*sizeof(char*));
	for(int n=net->num_save; n < net->save_size; n++) 
		net->save_str[n] = malloc(MAX_CHAR*sizeof(char));
	
	return 0;
}

/*
 * Frees the struct's memory blocks previously 
 * dynamically allocated by initialize_params
 * & realloc_params. Should be used if the struct
 * will not be used any more to prevent memory
 * leakage.
 */
int free_netlist(struct netlist *net)
{
	for(int n=0; n < net->num_param; n++) {
		free(net->param_name[n]);
		free(net->param_strvalue[n]);
	}
	free(net->param_name);
	free(net->param_strvalue);
	free(net->param_fvalue);

	return 0;
}

float str_to_expo(char *str)
{
	char *pch;
	float expo = strtof(str, &pch);
	switch(*pch) {
		case 'Y': expo *= 1E24; break;
		case 'Z': expo *= 1E21; break;
		case 'E': expo *= 1E18; break;
		case 'P': expo *= 1E15; break;
		case 'T': expo *= 1E12; break;
		case 'G': expo *= 1E9; break;
		case 'M': expo *= 1E6; break;
		case 'k': expo *= 1E3; break;
		case 'd': expo *= 1E-1; break;
		case 'c': expo *= 1E-2; break;
		case 'm': expo *= 1E-3; break;
		case 'u': expo *= 1E-6; break;
		case 'n': expo *= 1E-9; break;
		case 'p': expo *= 1E-12; break;
		case 'f': expo *= 1E-15; break;
		case 'a': expo *= 1E-18; break;
		case 'z': expo *= 1E-21; break;
		case 'y': expo *= 1E-24; break;
	}
	return expo;
}

/*
 * Converts a float in scientific notation
 * i.e. exponent notation into a string
 * appended with: m(illi), k(ilo), etc.
 * One exception is for mega, Meg is appended
 * to the significand.
 */
int expo_to_str(char *str, float expo)
{
	char *pch;
	char buffer[MAX_CHAR];
	char buffer2[MAX_CHAR];
	float significand;
	int exp;
	int offset = 0;
	sprintf(buffer, "%E", expo);
	buffer[8] = 'X';
	strcpy(buffer2, buffer);
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
	if(significand < 10)
		offset = 3;
	else if(significand >= 10 && significand < 100)
		offset = 4;
	else
		offset = 5;
	
	switch(exp) {
		case -18: strcpy(str+offset, "a"); break;
		case -15: strcpy(str+offset, "f"); break;
		case -12: strcpy(str+offset, "p"); break;
		case -9: strcpy(str+offset, "n"); break; //copies the sci letter right after the 3rd digit, including periods
		case -6: strcpy(str+offset, "u"); break;
		case -3: strcpy(str+offset, "m"); break;
		case 0: strcpy(str+offset, ""); break;
		case 3: strcpy(str+offset, "k"); break;
		case 6: strcpy(str+offset, "Meg"); break;
		default: strcpy(str+offset, ""); printf("Error in exp_to_str in switch statement!\n"); return 1;
	}
	return 0;
}
	
/* 
 * Converts every parameter value as given
 * by permutations for the jth permutations
 * into a string
 */
/*
int param_expo_to_str(struct params *net, unsigned int j)
{
	for(int n=0; n < net->num_param; n++)
		expo_to_str(net->ext_param_str[n], net->ext_param_permutations[j][n]);
	return 0;
}
*/
/*
 * Updates the parameter values iteratively
 * for each net to generate the values for
 * ext_param_permutations
 */
/*
int update_net(struct params *net)
{	
	net->ext_param_curr[0] += net->ext_param_step_array[0][2];
	for(int n=0; n < net->num_param-1; n++) {
		if(net->ext_param_curr[n] > net->ext_param_step_array[n][1]) {
			net->ext_param_curr[n+1] += net->ext_param_step_array[n+1][2];
			net->ext_param_curr[n] = net->ext_param_step_array[n][0];
		}
	}
	if(net->ext_param_curr[net->num_param-1] > net->ext_param_step_array[net->num_param-1][1]) {
		net->ext_param_curr[net->num_param-1] = net->ext_param_step_array[net->num_param-1][0];
		return 1;
	}
	else
		return 0;
}
*/
/*
 * Goes through the input file, tl_netlist, extracting the necessary
 * step parameters and put them into net. Also the long pointers 
 * store the position that the .tran, .ac, .param spice directives
 * appear in the file, so that they can be accessed later
 */
int prepare_netlist(struct netlist *net, FILE* tl_netlist)
{	
	char buffer[MAX_CHAR];
	char str_fl[MAX_CHAR];
	char *p_ch;
	char *p_tok;
	char *p_end;
	int param_index = 0;
	int data_index = 0;
	long tmp_pos = 0;
	float tmp_data = 0;
	
	while(!feof(tl_netlist)) {
		fgets(buffer, MAX_CHAR, tl_netlist);
		p_tok = strtok(buffer, " ");
		/*
		if(buffer[0] == 'I') {
			 * Assumes at most nine voltage sources,
			 * so that the ID is only one character
			 * Also that all of the sources are
			 * hooked up in parallel
			 *
			int index = atoi(buffer+1);
			printf("Reached a current source!\n");
			printf("Current number: %d\n", index);
			(*currents_pos)[index-1] = tmp_pos;
			p_ch = buffer;
			buffer[2] = ' ';
			do {
				printf("%c", *p_ch);
				if(memcmp(p_ch-5, "PULSE", 5)==0) {
					net->currents_index[index-1] = p_ch - buffer;
					break;
				}
			} while(*(++p_ch));
			printf("\n");
			printf("P_ch: %s\n", p_ch);
		}
		*/
		/*
		if(strcmp(buffer+1, "tran")==0) {
			* printf("Found tran spice directive\n"); *
			buffer[5] = ' ';
			strcpy(net->tran_str, buffer);
		*/
		if(strcmp(buffer+1, "ac")==0) {
			/* printf("Found ac spice directive\n"); */
			buffer[3] = ' ';
			strcpy(net->ac_str, buffer);
		} else if(strcmp(buffer, ".param")==0) {
			/* printf("Found param spice directive\n"); */
			buffer[6] = ' ';
			p_ch = buffer + 7;
			printf("p_ch:%s\n", p_ch);
			char *start, *eq_sign, *end;
			start = eq_sign = end = NULL;
			/*
			 * Create comment for do { } while loop
			 */
			do {
				if(*p_ch != ' ' && *(p_ch-1) == ' ')
					start = p_ch;
				if(*p_ch == '=')
					eq_sign = p_ch;
				if(*p_ch != ' ' && (*(p_ch+1) == ' ' || *(p_ch+1) == '\0'))
					end = p_ch;
				if(start && eq_sign && end) {
					if(net->num_param >= net->param_size) {
						realloc_netlist(net);
						printf("Finished reallocation\n");
					}
					//printf("Copying %.*s into param_name[%d]\n", eq_sign - start, start, k);
					sprintf(net->param_name[net->num_param], "%.*s", eq_sign - start, start);
					//memcpy(net->param_name[k], start, eq_sign - start);
					//printf("Copying %.*s into param_strvalue[%d]\n", end - eq_sign, eq_sign + 1, k);
					sprintf(net->param_strvalue[net->num_param], "%.*s", end - eq_sign, eq_sign + 1);
					//memcpy(net->param_strvalue[k], eq_sign + 1, end - eq_sign);
					//printf("Name: %s, Value: %s\n", net->param_name[net->num_param], net->param_strvalue[net->num_param]);
					start = eq_sign = end = NULL;
					net->num_param++;
				}
			} while(*(++p_ch));
		} else if(strcmp(buffer, ".save") == 0) {
			//printf("Found save spice directive\n");
			buffer[5] = ' ';
			p_ch = strtok(NULL, " ");
			while(p_ch != NULL) {
				if(net->num_save >= net->save_size)
					realloc_save(net);
				
				strcpy(net->save_str[net->num_save], p_ch);
				net->num_save++;
				p_ch = strtok(NULL, " ");
			}
		}
				/*
				for(int n=0; n < net->num_param; n++) {
					int length = strlen(net->ext_param_name[n]);
					if(memcmp(p_ch-length, net->ext_param_name[n], length)==0)
						net->ext_param_index[n] = p_ch - buffer;
				}
				*/
		/*
		} else if(strcmp(buffer+1, "step")==0) {
			strtok(NULL, " ");
			p_tok = strtok(NULL, " ");
			*
			printf("Strcpy %s into net.ext_param_name[%d]\n", p_tok, net->num_param);
			printf("currents size: %d\n", net->curr_size);
			*
			//strcpy(net->param_name[net->num_param], p_tok);
			data_index = 0;
			while(p_tok!=NULL) {
				p_end = p_tok;
				* printf("Strcpy p_tok: %s into str_fl...\n", p_tok); *
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
					net->param_step_array[net->num_param][data_index] = tmp_data;
					data_index++;
				}
				p_tok = strtok(NULL, " ");
			}
			//net->num_param++;
			if(net->num_param >= net->curr_size) {
				net->curr_size = net->num_param + 3;
				realloc_netlist(net);
			}
		}
		*/
	}
	/*
	for(int n=0; n < net->num_param; n++)
		net->ext_param_curr[n] = net->ext_param_step_array[n][0];
	
	net->num_perm = 1;
	printf("Generating the number of net.ext_param_permutations for .step params...\n");
	for(int n=0; n < net->num_param; n++) {
		net->num_perm *= (unsigned int)((net->ext_param_step_array[n][1]-net->ext_param_step_array[n][0])/net->ext_param_step_array[n][2]+1);
	}
	printf("Allocating memory for an array of size %u with float* to store net.ext_param_permutations...\n", net->num_perm);
	net->ext_param_permutations = malloc(net->num_perm*sizeof(float*));
	if(!net->ext_param_permutations)
		printf("Not enough memory to allocate net.ext_param_permutations array\n");
	
	for(unsigned int i=0; i < net->num_perm; i++) {
		net->ext_param_permutations[i] = malloc(net->num_param*sizeof(float));
		for(int j=0; j < net->num_param; j++) {
			net->ext_param_permutations[i][j] = net->ext_param_curr[j];
		}
		update_net(net);
	}
	for(int n=0; n < net->num_param; n++)
		net->ext_param_curr[n] = net->ext_param_step_array[n][0];
	
	printf("Number of net.ext_param_permutations: %d\n", net->num_perm);
	*/
	return 0;
}
#endif /* AUTO_LTS_2_H */