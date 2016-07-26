#ifndef AUTO_LTS_H
#define AUTO_LTS_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>
#include<ctype.h>
#include<sys/stat.h>
#include<dirent.h>

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
struct params
{
	char **ext_param_name;
	char **ext_param_str;
	float **ext_param_step_array;
	//float **ext_param_data_array;
	float **ext_param_permutations;
	float *ext_param_curr;
	int *ext_param_index;

	int num_param;
	unsigned int num_perm;
	int curr_size;
}; 

/*
 * Dynamically allocates memory for the struct
 * as it is unknown at compile-time how much
 * memory to allocate for arrays storing information
 * on the stepped parameters
 */
int initialize_params(struct params *run)
{
	run->num_param = 0;
	run->curr_size = 3;
	run->ext_param_index = malloc(run->curr_size*sizeof(int));
	run->ext_param_curr = malloc(run->curr_size*sizeof(float));
	run->ext_param_step_array = malloc(run->curr_size*sizeof(float*));
	run->ext_param_str = malloc(run->curr_size*sizeof(char*));
	run->ext_param_name = malloc(run->curr_size*sizeof(char*));
	for(int n=0; n < run->curr_size; n++) {
		run->ext_param_step_array[n] = malloc(3*sizeof(float));
		run->ext_param_str[n] = malloc(255*sizeof(char));
		run->ext_param_name[n] = malloc(255*sizeof(char));
	}
	return 0;
}

/*
 * Reallocates the struct's memory blocks
 * when the number of parameters to be stepped
 * is updated. 
 */
int realloc_params(struct params *run)
{
	run->ext_param_index = realloc(run->ext_param_index, run->curr_size*sizeof(int));
	run->ext_param_curr = realloc(run->ext_param_curr, run->curr_size*sizeof(float));
	run->ext_param_step_array = realloc(run->ext_param_step_array, run->curr_size*sizeof(float*));
	run->ext_param_str = realloc(run->ext_param_str, run->curr_size*sizeof(char*));
	run->ext_param_name = realloc(run->ext_param_name, run->curr_size*sizeof(char*));
	for(int n=run->num_param; n < run->curr_size; n++) {
		run->ext_param_step_array[n] = malloc(3*sizeof(float));
		run->ext_param_str[n] = malloc(255*sizeof(char));
		run->ext_param_name[n] = malloc(255*sizeof(char));
	}
	return 0;
}

/*
 * Frees the struct's memory blocks previously 
 * dynamically allocated by initialize_params
 * & realloc_params. Should be used if the struct
 * will not be used any more to prevent memory
 * leakage.
 */
int free_params(struct params *run)
{
	for(int n=0; n < run->num_param; n++) {
		free(run->ext_param_name[n]);
		free(run->ext_param_str[n]);
		free(run->ext_param_step_array[n]);
	}
	free(run->ext_param_index);
	free(run->ext_param_curr);
	free(run->ext_param_step_array);
	free(run->ext_param_str);
	free(run->ext_param_name);
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
	
/*
 * Dynamically allocate memory to hold
 * an array of the values characterized
 * by low, high and step for the parameters
 */
int generate_data_array(float low, float high, float step, float *step_array)
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

/* 
 * Converts every parameter value as given
 * by permutations for the jth permutations
 * into a string
 */
int param_expo_to_str(struct params *run, unsigned int j)
{
	for(int n=0; n < run->num_param; n++)
		expo_to_str(run->ext_param_str[n], run->ext_param_permutations[j][n]);
	return 0;
}

/*
 * Updates the parameter values iteratively
 * for each run to generate the values for
 * ext_param_permutations
 */
int update_run(struct params *run)
{	
	run->ext_param_curr[0] += run->ext_param_step_array[0][2];
	for(int n=0; n < run->num_param-1; n++) {
		if(run->ext_param_curr[n] > run->ext_param_step_array[n][1]) {
			run->ext_param_curr[n+1] += run->ext_param_step_array[n+1][2];
			run->ext_param_curr[n] = run->ext_param_step_array[n][0];
		}
	}
	if(run->ext_param_curr[run->num_param-1] > run->ext_param_step_array[run->num_param-1][1]) {
		run->ext_param_curr[run->num_param-1] = run->ext_param_step_array[run->num_param-1][0];
		return 1;
	}
	else
		return 0;

	return 0;
}

/*
 * Goes through the input file, tl_netlist, extracting the necessary
 * step parameters and put them into run. Also the long pointers 
 * store the position that the .tran, .ac, .param spice directives
 * appear in the file, so that they can be accessed later
 */
int prepare_run(struct params *run, FILE* tl_netlist, long *tran_pos, long *ac_pos, long *param_pos)
{	
	char buffer[255];
	char str_fl[20];
	char *p_ch;
	char *p_tok;
	char *p_end;
	int param_index = 0;
	int data_index = 0;
	long tmp_pos;
	float tmp_data = 0;
	
	while(!feof(tl_netlist)) {
		tmp_pos = ftell(tl_netlist);
		fgets(buffer, 255, tl_netlist);
		p_tok = strtok(buffer, " ");
		if(strcmp(buffer+1, "tran")==0) {
			printf("Transient data\n");
			buffer[5] = ' ';
			*tran_pos = tmp_pos;
		} else if(strcmp(buffer+1, "ac")==0) {
			printf("AC data\n");
			buffer[3] = ' ';
			*ac_pos = tmp_pos;
		} else if(strcmp(buffer, ".param")==0) {
			printf("Parameter data\n");
			buffer[6] = ' ';
			*param_pos = tmp_pos;
			p_ch = buffer;
			/*
			 * Create comment for do { } while loop
			 */
			do {
				for(int n=0; n < run->num_param; n++) {
					int length = strlen(run->ext_param_name[n]);
					if(memcmp(p_ch-length, run->ext_param_name[n], length)==0)
						run->ext_param_index[n] = p_ch - buffer;
				}
			} while(*(++p_ch));
		} else if(strcmp(buffer+1, "step")==0) {
			strtok(NULL, " ");
			p_tok = strtok(NULL, " ");
			printf("Strcpy %s into run.ext_param_name[%d]\n", p_tok, run->num_param);
			printf("Current size: %d\n", run->curr_size);
			strcpy(run->ext_param_name[run->num_param], p_tok);
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
					run->ext_param_step_array[run->num_param][data_index] = tmp_data;
					data_index++;
				}
				p_tok = strtok(NULL, " ");
			}
			run->num_param++;
			if(run->num_param >= run->curr_size) {
				run->curr_size = run->num_param + 3;
				printf("Reallocate param memory...\n");
				realloc_params(run);
			}
		}
	}
	for(int i=0; i < run->num_param; i++) {
		printf("\n%s\n", run->ext_param_name[i]);
		for(int j=0; j < 3; j++) 
			printf("%e ", run->ext_param_step_array[i][j]);
	}
	printf("\n");
	
	for(int n=0; n < run->num_param; n++)
		run->ext_param_curr[n] = run->ext_param_step_array[n][0];
	/*
	printf("Generating data arrays for .step params...\n");
	for(int n=0; n < run->num_param; n++)
		generate_data_array(run->ext_param_step_array[n][0], run->ext_param_step_array[n][1], run->ext_param_step_array[n][2], run->ext_param_data_array[n]);
	*/
	run->num_perm = 1;
	printf("Generating the number of run.ext_param_permutations for .step params...\n");
	for(int n=0; n < run->num_param; n++) {
		run->num_perm *= (unsigned int)((run->ext_param_step_array[n][1]-run->ext_param_step_array[n][0])/run->ext_param_step_array[n][2]+1);
	}
	printf("Allocating memory for an array of size %u with float* to store run.ext_param_permutations...\n", run->num_perm);
	run->ext_param_permutations = malloc(run->num_perm*sizeof(float*));
	if(!run->ext_param_permutations)
		printf("Not enough memory to allocate run.ext_param_permutations array\n");
	
	for(unsigned int i=0; i < run->num_perm; i++) {
		run->ext_param_permutations[i] = malloc(run->num_param*sizeof(float));
		for(int j=0; j < run->num_param; j++) {
			run->ext_param_permutations[i][j] = run->ext_param_curr[j];
		}
		update_run(run);
	}
	for(int n=0; n < run->num_param; n++)
		run->ext_param_curr[n] = run->ext_param_step_array[n][0];
	
	printf("Number of run.ext_param_permutations: %d\n", run->num_perm);
	return 0;
}
#endif /* AUTO_LTS_H */