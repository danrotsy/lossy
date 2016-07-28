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
		printf("Command-line argument for netlist filename is required\n");
		return 1;
	}
	char filename_in[255];
	strcpy(filename_in, argv[1]);
	FILE* tl_netlist = fopen(filename_in, "r+");
	if(!tl_netlist) {
		printf("Error in Opening Spice file, %s: %s\n", filename_in, strerror(errno));
		return 1;
	}
	struct params run;
	initialize_params(&run);
	
	char cmd_ltspice[255];
	sprintf(cmd_ltspice, "./\"C:\\Program Files (x86)\\LTC\\LTspiceIV\\scad3.exe\" -b %s", filename_in);
	
	char filename_out[255];
	char filename_raw[255];
	char filename_buffer[255];
	char buffer[255];
	
	long tran_pos;
	long ac_pos;
	long param_pos;

	char *p_ch;
	char *p_tok;
	char *p_end;

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
	
	prepare_run(&run, tl_netlist, &tran_pos, &ac_pos, &param_pos);
	
	/* 
	 * step one: run all ltspice netlists with varying parameters in length, cdrp and ac vs tran
	 * 0: tran
	 * 1: ac
	 */	
	sprintf(cmd_ltspice, "./scad3.exe -Run -b %s", filename_in);
	/* 
	 * scad3 names the output raw file the same as the netlist file
	 * This is important as we later rename that file to specify
	 * what the stepped parameter values were
	 */
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
		for(unsigned int j=0; j < run.num_perm; j++) {
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
			/* 
			 * tl_netlist must be closed when scad3 runs since
			 * both programs can't be updating the same file
			 * as far as I know, as scad3 will give an error
			 * that it can't open the input deck netlist
			 */
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
			system("sleep 5");
			rename(filename_raw, filename_out);
		}
	}
	fclose(tl_netlist);
	return 0;
}