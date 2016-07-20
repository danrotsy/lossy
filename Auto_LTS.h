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
	float **ext_param_data_array;
	float **ext_param_permutations;
	float *ext_param_curr;
	int *ext_param_index;

	int num_param;
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
	for(int n=0; n < run->num_params; n++) {
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
#endif /* AUTO_LTS_H */