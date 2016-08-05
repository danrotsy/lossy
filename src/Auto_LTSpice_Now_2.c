#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<errno.h>
#include<math.h>
#include<ctype.h>

#include"Auto_LTS_2.h"

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
	 * ARGS: num_dropoffs, analysis, cdrp, lenline, r, l, c, signal, quoted bits with spaces, bit time index
	 */
	if(argc < 10) {
		printf("Not enough Command-line arguments supplied\n");
		printf("ARGS: num_dropoffs, analysis, cdrp, lenline, r, l, c, signal, vin_arg_str\n");
		return 1;
	}
	
	struct netlist net;
	initialize_netlist(&net, argc, argv);
	
	char filename_in[MAX_CHAR];
	char filename_tmp[MAX_CHAR] = "..\\data\\tmp.cir";

	char *p_ch;
	char *p_tok;
	char *p_end;
	char *pch;
	
	long tmp_pos;

	//Determine the netlist to open for reading
	int num_dropoffs = (int) strtol(argv[1], NULL, 10);
	sprintf(filename_in, "..\\data\\transmission_line_%d_dropoffs.cir", num_dropoffs);
	FILE* tl_netlist = fopen(filename_in, "rb");
	if(!tl_netlist) {
		printf("Error in Opening Spice file, %s: %s\n", filename_in, strerror(errno));
		return 1;
	}
	//Create a new temporary netlist to write changes to in case the file changes length
	FILE* tmp_netlist = fopen(filename_tmp, "wb+");
	if(!tmp_netlist) {
		printf("Error in Opening or Creating Spice file, %s: %s\n", filename_tmp, strerror(errno));
		return 1;
	}
	
	/* printf("Number of currents: %d\n", net.num_currents); */
		
	float cdrp_arg = str_to_expo(argv[3]);
	float lenline_arg = str_to_expo(argv[4]);
	float r_arg = str_to_expo(argv[5]);
	float l_arg = str_to_expo(argv[6]);
	float c_arg = str_to_expo(argv[7]);
	
	char cmd_ltspice[MAX_CHAR];
	
	char filename_out[MAX_CHAR];
	char filename_raw[MAX_CHAR];
	char filename_buffer[MAX_CHAR];
	char filename_buffer2[MAX_CHAR];
	char buffer_str[MAX_CHAR];
	char filename_v[MAX_CHAR];
	char cmd_ltsputil[MAX_CHAR];
	char cmd_cd[MAX_CHAR];
	char cmd_mkdir[MAX_CHAR];
	char buffer[MAX_CHAR];
	char v_first[MAX_CHAR];
	char v_middle[MAX_CHAR];
	char v_last[MAX_CHAR];
	char name_dir[MAX_CHAR];
	
	if(num_dropoffs == 10) {
		strcpy(v_first, "V(n001)");
		strcpy(v_middle, "V(n006)");
		strcpy(v_last, "V(n010)");
	} else if(num_dropoffs == 14) {
		strcpy(v_first, "V(n001)");
		strcpy(v_middle, "V(n008)");
		strcpy(v_last, "V(n014)");
	} else {
		strcpy(v_first, "V(n001)");
		strcpy(v_middle, "V(n014)");
		strcpy(v_last, "V(n026)");
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
		
	float progress_percent = 0;

	/*
	 * Loop through the input file looking
	 * for the lines that specify .tran, .ac, 
	 * .param and .step spice directives
	 * After determining if the currents line
	 * is any of one of those, the file stream
	 * "cursor" is saved specifying which directive
	 * was on that line. For the .param line the char
	 * index of each stepped variable is saved, as to
	 * sub in the new values each simualation net,
	 * so the .param line MUST come after the .step
	 * param directives in the input file
	 */
	printf("Preparing netlist struct...\n");
	prepare_netlist(&net, tl_netlist);
	/* 
	 * step one: net all ltspice netlists with varying parameters in length, cdrp and ac vs tran
	 * 0: tran
	 * 1: ac
	 */	
	sprintf(cmd_ltspice, "scad3.exe -b %s", filename_in);
	/* 
	 * scad3 names the output raw file the same as the netlist file
	 * This is important as we later rename that file to specify
	 * what the stepped parameter values were
	 */
	/* Copy the schematic into tmp.cir until the voltage / current source is found,
	 * as that is where the lengths of lines might change
	 * This means the only lines after the voltage / current source MUST be lines
	 * that Auto_LTSpice_Now manipulates or else they won't be copied into the
	 * temporary netlist
	 */
	rewind(tl_netlist);
	while(!feof(tl_netlist)) {
		fgets(buffer, 255, tl_netlist);
		if(buffer[0] == 'I' || buffer[0] == 'V')
			break;
		fputs(buffer, tmp_netlist);
	}
	/* Next comes the insertion of the voltage / current source
	 * into the temporary netlist
	 */
	printf("Substitutuing input currents...\n");
	for(int n=0; n < net.num_sources; n++) {
		/*
		printf("Currents[%d]: %s\n", n, net.vin_str[n]);
		printf("Delay[%d]: %s\n", n, net.delay[n]);
		*/
		if(net.current_str[n] == NULL)
			printf("String is null!\n");
		printf("Current String: %s", net.current_str[n]);
		fputs(net.current_str[n], tmp_netlist);
	}
	/* Next the transient and ac analysis spice directives
	 * are inserted into the tmp.cir netlist
	 */
	printf("Selecting proper analysis...\n");
	
	net.tran_pos = ftell(tmp_netlist);
	/*
	for(int n=0; n < 30; n++)
		printf("%d", net.tran_str[n]);
	printf("\n");
	*/
	fputs(net.tran_str, tmp_netlist);
	tmp_pos = ftell(tmp_netlist);
	if(fseek(tmp_netlist, net.tran_pos, SEEK_SET))
		perror("Error in seeking transient analysis line");
	fputc(net.is_tran ? '.' : ';', tmp_netlist);
	fseek(tmp_netlist, net.tran_pos, SEEK_SET);
	fgets(buffer, MAX_CHAR, tmp_netlist);
	printf("Transient Analysis: %s", buffer);
	
	if(fseek(tmp_netlist, tmp_pos, SEEK_SET))
		perror("Error in seeking tmp position");
	
	net.ac_pos = ftell(tmp_netlist);
	fputs(net.ac_str, tmp_netlist);
	tmp_pos = ftell(tmp_netlist);
	/* printf("AC_str: %s", net.ac_str); */
	if(fseek(tmp_netlist, net.ac_pos, SEEK_SET))
		perror("Error in seeking ac analysis line");
	fputc(net.is_tran ? ';' : '.', tmp_netlist);
	fseek(tmp_netlist, net.ac_pos, SEEK_SET);
	fgets(buffer, MAX_CHAR, tmp_netlist);
	printf("AC Analysis: %s", buffer);
	
	if(fseek(tmp_netlist, tmp_pos, SEEK_SET))
		perror("Error in seeking tmp position");
	
	/* The next line is the .param line */
	expo_to_str(net.param_strvalue[0], cdrp_arg);
	//expo_to_str(net.param_strvalue[1], l_arg);
	expo_to_str(net.param_strvalue[2], lenline_arg);
	expo_to_str(net.param_strvalue[3], r_arg);
	expo_to_str(net.param_strvalue[4], l_arg);
	expo_to_str(net.param_strvalue[5], c_arg);
	printf("Substituting new paramters into netlist...\n");
	sprintf(buffer, ".param");
	for(int k=0; k < net.num_param; k++) {
		/*
		p_ch = buffer + net.ext_param_index[k];
		pch = p_ch + 1;
		do {
			if((memcmp(pch, "Rline", 4)==0) || (memcmp(pch, "Lline", 4)==0) || (memcmp(pch, "Cline", 5)==0) || (memcmp(pch, "Cdrp", 4)==0)
					|| (memcmp(pch, "Lenline", 7)==0) || (memcmp(pch, "Rterm", 4)==0)) {
				break;
			}
			if(*pch != ' ' && *pch != '\n' && *pch != '\r')
				*pch = ' ';
		} while(*(++pch));
		*/
		printf(" %s=%s\n", net.param_name[k], net.param_strvalue[k]);
		sprintf(buffer_str, " %s=%s", net.param_name[k], net.param_strvalue[k]);
		strcat(buffer, buffer_str);
		//sprintf(buffer_str, "%s", net.ext_param_str[k]);
		//memcpy(p_ch+1, buffer_str, strlen(buffer_str));
	}
	strcat(buffer, "\n");
	printf("Completed buffer: %s", buffer);
	fputs(buffer, tmp_netlist);
	
	strcpy(buffer, ".save");
	for(int n=0; n < net.num_save; n++) {
		sprintf(buffer_str, " %s", net.save_str[n]);
		strcat(buffer, buffer_str);
	}
	fputs(buffer, tmp_netlist);
	fputs(".model LossyTL  LTRA(len=Lenline R=Rline L=Lline C=Cline)\n", tmp_netlist);
	fputs(".backanno\n", tmp_netlist);
	fputs(".end\n", tmp_netlist);
	/* 
	 * tl_netlist must be closed when scad3 nets since
	 * both programs can't be updating the same file
	 * as far as I know, as scad3 will give an error
	 * that it can't open the input deck netlist
	 */
	if(fclose(tl_netlist))
		perror("Error in closing original netlist file");
	if(fclose(tmp_netlist))
		perror("Error in closing temporary netlist file");
	
	if(remove(filename_in) != 0)
		perror("Couldn't remove input file");
	
	if(rename(filename_tmp, filename_in) !=0 )
		perror("Failed to rename tmp file as input file");
	
	strcpy(filename_raw, filename_in);
	strcpy(filename_raw+strlen(filename_raw)-3, "raw");
	sprintf(filename_buffer2, "%s", net.is_tran ? "tran" : "ac");
	for(int n=0; n < net.num_param; n++) {
		sprintf(filename_buffer, "_%s=%s", net.param_name[n], net.param_strvalue[n]);
		strcat(filename_buffer2, filename_buffer);
	}
	sprintf(filename_buffer, "_Vin=%d", net.vin[0]);
	strcat(filename_buffer2, filename_buffer);
	for(int n=1; n < net.num_currents; n++) {
		sprintf(filename_buffer, "_%d", net.vin[n]);
		strcat(filename_buffer2, filename_buffer);
	}
	sprintf(filename_buffer, "_BitTime=%s", argv[10]);
	printf("Filename buffer: %s\n", filename_buffer);
	strcat(filename_buffer2, filename_buffer);
	strcat(filename_buffer2, ".raw");
	system(cmd_ltspice);
	system("timeout 2");
	strcpy(filename_in+strlen(filename_in)-4, "\0");

	/* printf("file_in: %s, file_buffer2: %s\n", filename_in, filename_buffer2); */
	sprintf(filename_out, "%s\\Multidrop%s\\%s", filename_in, net.is_tran ? "Transient" : "Bode", filename_buffer2);
	sprintf(cmd_mkdir, "mkdir %s\\Multidrop%s", filename_in, net.is_tran ? "Transient" : "Bode");
	system(cmd_mkdir);
	
	char *s = filename_out;
	do {
		if(*s == '/' || *s == ':' || *s == '*' || *s == '?' || *s == '"' || *s == '<' || *s == '>' || *s == '|')
			*s = '_';
	} while(*(++s));
	
	printf("filename out: %s\n", filename_out);
	printf("filename raw: %s\n", filename_raw);
	remove(filename_out);
	
	if(rename(filename_raw, filename_out)!=0)
		perror("Failed to rename RAW file");
	
	strcpy(filename_raw, filename_out);
	sprintf(filename_v, "_%s.csv", argv[8]);
	strcpy(filename_out+strlen(filename_out)-4, filename_v);

	if(net.is_tran) {
		sprintf(cmd_ltsputil, "ltsputil.exe -xo2 %s %s \"%%14.6e\" \",\" \"#DATA\" \"time\" \"%s\"", 
				filename_raw, filename_out, v_signal[v_signal_index]);
				
		system(cmd_ltsputil);
	} else {
		sprintf(cmd_ltsputil, "ltsputil.exe -xo2dp %s %s \"%%14.6e\" \",\" \"#DATA\" \"frequency\" \"%s\"", 
				filename_raw, filename_out, v_signal[v_signal_index]);
		
		system(cmd_ltsputil);
	}
	
	remove("tmp$x$y%z&_0.tmp");
	remove("tmp$x$y%z&_1.tmp");
	remove("tmp$x$y%z&_3.tmp");
	remove("tmp$x$y%z&_4.tmp");
	
	return 0;
}