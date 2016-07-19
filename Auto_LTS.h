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

struct param_run
{
	char *r_str;
	char *l_str;
	char *c_str;
	float r_step_array[3];
	float l_step_array[3];
	float c_step_array[3];
	float r_curr;
	float l_curr;
	float c_curr;
}; 

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
	
int generate_step_array(float low, float high, float step, float *step_array)
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

int update_run(struct param_run *run)
{
	expo_to_str(run->r_str, run->r_curr);
	expo_to_str(run->l_str, run->l_curr);
	expo_to_str(run->c_str, run->c_curr);
	
	run->c_curr += run->c_step_array[2];
	if(run->c_curr > run->c_step_array[1]) {
		run->l_curr += run->l_step_array[2];
		run->c_curr = run->c_step_array[0];
	}
	if(run->l_curr > run->l_step_array[1]) {
		run->r_curr += run->r_step_array[2];
		run->l_curr = run->l_step_array[0];
	}
	if(run->r_curr > run->r_step_array[1]) {
		run->r_curr = run->r_step_array[0];
	}
	return 0;
}
#endif /* AUTO_LTS_H */