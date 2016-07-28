#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
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
	 *
	 * ARGS: num_dropoffs, analysis, cdrp, lenline, r, l, c, signal
	 */
	if(argc < 9) {
		printf("Command-line argument for netlist filename is required\n");
		return 1;
	}
	char filename_in[255];

	char *p_ch;
	char *p_tok;
	char *p_end;

	//strcpy(filename_in, argv[1]);
	int num_dropoffs = (int) strtol(argv[1], NULL, 10);
	sprintf(filename_in, "transmission_line_%d_dropoffs.cir", num_dropoffs);
	FILE* tl_netlist = fopen(filename_in, "rb+");
	if(!tl_netlist) {
		printf("Error in Opening Spice file, %s: %s\n", filename_in, strerror(errno));
		return 1;
	}
	int is_tran = 0;
	if((strcmp(argv[2], "tran")==0) || (strcmp(argv[2], "TRAN")==0))
		is_tran = 1;
	
	float cdrp_arg = str_to_expo(argv[3]);
	float lenline_arg = str_to_expo(argv[4]);
	float r_arg = str_to_expo(argv[5]);
	float l_arg = str_to_expo(argv[6]);
	float c_arg = str_to_expo(argv[7]);

	struct params run;
	initialize_params(&run);
	
	char cmd_ltspice[255];
	sprintf(cmd_ltspice, "\"C:\\Program Files (x86)\\LTC\\LTspiceIV\\scad3.exe\" -b %s", filename_in);
	
	char filename_out[255];
	char filename_raw[255];
	char filename_buffer[255];
	char filename_buffer2[255];
	char filename_v[255];
	char cmd_ltsputil[255];
	char cmd_cd[255];
	char cmd_mkdir[255];
	char buffer[255];
	char v_first[255];
	char v_middle[255];
	char v_last[255];
	char name_dir[255];
	
	if(num_dropoffs == 10) {
		strcpy(v_first, "V(N001)");
		strcpy(v_middle, "V(N006)");
		strcpy(v_last, "V(N010)");
	} else if(num_dropoffs == 14) {
		strcpy(v_first, "V(N001)");
		strcpy(v_middle, "V(N008)");
		strcpy(v_last, "V(N014)");
	} else {
		strcpy(v_first, "V(N001)");
		strcpy(v_middle, "V(N014)");
		strcpy(v_last, "V(N026)");
	}
	
	char *v_signal[] = {v_first, v_middle, v_last, "V(out)", "V(inpulse)"};
	int v_signal_index = 0;
	if(strcmp(argv[8], "Vfirst")==0)
		v_signal_index = 0;
	else if(strcmp(argv[8], "Vmiddle")==0)
		v_signal_index = 1;
	else if(strcmp(argv[8], "Vlast")==0)
		v_signal_index = 2;
	else if(strcmp(argv[8], "Vout")==0)
		v_signal_index = 3;
	else if(strcmp(argv[8], "Vin")==0)
		v_signal_index = 4;
		
	long tran_pos;
	long ac_pos;
	long param_pos;
	
	float progress_percent = 0;

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
	printf("Finished preparing run\n");
	/* 
	 * step one: run all ltspice netlists with varying parameters in length, cdrp and ac vs tran
	 * 0: tran
	 * 1: ac
	 */	
	sprintf(cmd_ltspice, "scad3.exe -b %s", filename_in);
	/* 
	 * scad3 names the output raw file the same as the netlist file
	 * This is important as we later rename that file to specify
	 * what the stepped parameter values were
	 */
	strcpy(filename_raw, filename_in);
	strcpy(filename_raw+strlen(filename_raw)-3, "raw");
	printf("Selecting proper analysis...\n");
	if(is_tran) {
		if(fseek(tl_netlist, tran_pos, SEEK_SET))
			perror("Error in tran fseek");
		fputc('.', tl_netlist);
		if(fseek(tl_netlist, ac_pos, SEEK_SET))
			perror("Error in ac fseek");
		fputc(';', tl_netlist);
	} else {
		fseek(tl_netlist, ac_pos, SEEK_SET);
		fputc('.', tl_netlist);
		if(fseek(tl_netlist, tran_pos, SEEK_SET))
			perror("Error in tran fseek");
		fputc(';', tl_netlist);
	}
	
	if(fseek(tl_netlist, param_pos, SEEK_SET))
		perror("Error in param fseek");
	fgets(buffer, 255, tl_netlist);
	printf("Buffer: %s\n", buffer);
	printf("Translating parameter floats to strings...\n");
	expo_to_str(run.ext_param_str[0], r_arg);
	expo_to_str(run.ext_param_str[1], l_arg);
	expo_to_str(run.ext_param_str[2], c_arg);
	expo_to_str(run.ext_param_str[3], cdrp_arg);
	expo_to_str(run.ext_param_str[4], lenline_arg);
	printf("Substituting new paramters into netlist...\n");
	for(int k=0; k < run.num_param; k++) {
		printf("run.ext_param_str[k]: %s\n", run.ext_param_str[k]);
		p_ch = buffer + run.ext_param_index[k];
		printf("p_ch: %s\n", p_ch);
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
	sprintf(filename_buffer2, "%s", is_tran ? "tran" : "ac");
	for(int n=0; n < run.num_param; n++) {
		sprintf(filename_buffer, "_%s=%s", run.ext_param_name[n], run.ext_param_str[n]);
		strcat(filename_buffer2, filename_buffer);
	}
	strcat(filename_buffer2, ".raw");
	printf("%s\n", filename_buffer2);
	system(cmd_ltspice);
	system("sleep 2");
	strcpy(filename_in+strlen(filename_in)-4, "\0");

	sprintf(filename_out, "%s\\Multidrop%s\\%s", filename_in, is_tran ? "Transient" : "Bode", filename_buffer2);
	printf("Old: %s to New: %s\n", filename_raw, filename_out);
	sprintf(cmd_mkdir, "mkdir %s\\Multidrop%s", filename_in, is_tran ? "Transient" : "Bode");
	printf("%s\n", cmd_mkdir);
	system(cmd_mkdir);
	
	if(rename(filename_raw, filename_out)!=0)
		perror("Failed to rename RAW file");
	
	strcpy(filename_raw, filename_out);
	sprintf(filename_v, "_%s.csv", argv[8]);
	strcpy(filename_out+strlen(filename_out)-4, filename_v);

	if(is_tran) {
		sprintf(cmd_ltsputil, "ltsputil.exe -xo2 %s %s \"%%14.6e\" \",\" \"#DATA\" \"time\" \"%s\"", 
				filename_raw, filename_out, v_signal[v_signal_index]);
		
		printf("%s\n", cmd_ltsputil);
		printf("timeout 2\n");
				
		system(cmd_ltsputil);
		system("timeout 2");
	} else {
		sprintf(cmd_ltsputil, "ltsputil.exe -xo2dp %s %s \"%%14.6e\" \",\" \"#DATA\" \"frequency\" \"%s\"", 
				filename_raw, filename_out, v_signal[v_signal_index]);
		
		printf("%s\n", cmd_ltsputil);
		printf("timeout 2\n");
		
		system(cmd_ltsputil);
		system("timeout 2");
	}
	return 0;
}