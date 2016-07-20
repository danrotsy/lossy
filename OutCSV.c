#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>
#include<dirent.h>
#include<errno.h>
		
int main(int argc, char* argv[])
{
	//don't include .txt in file name for command-line argument
	//file names are in format of "R=X_L=Y_C=Z"
	//all in directory named after input file
	if(argc < 2) {
		printf("Insufficient number of command-line arguments.\n Exiting program.\n");
		return 0;
	}
	char buffer[255];
	char* mini_buffer = NULL;
	char* p_buf = NULL;
	char var_line[255];
	char file_name[255];
	char* s = NULL;
	char* d = NULL;
	FILE* waveform_in = NULL;
	FILE* waveform_out = NULL;
	strcpy(file_name, argv[1]);
	strcat(file_name, ".txt");
	waveform_in = fopen(file_name, "r");
	if(waveform_in)
		printf("Opened file %s\n", file_name);
	else {
		printf("Could not open file %s\n", file_name);
		return 0;
	}
	strcpy(file_name+strlen(file_name)-3, "csv");
	waveform_out = fopen(file_name, "w");
	while(1) {
		while(1) {
			if(feof(waveform_in))
				goto exit;
			//for frequency plots, Voltage is in polar form with magnitude in dB, so dB and ° units must be removed from buffer
			fgets(buffer, 255, waveform_in);
			s = buffer;
			d = buffer;
			do {
				while(*s == '°' || (memcmp(s, "dB", 2)==0) || (memcmp(s-1, "dB", 2)==0) || *s == '(' || *s == ')' || *s==':') 
					s++; 
				if(*s==' ' || *s=='\t' || *s=='/')
					*s=',';
				if(memcmp(s, "Run", 3)==0)
					*s='\0';
			}
			while(*d++ = *s++);
			if(buffer[0] == 'S')
				break;
			else if(buffer[0] == 't' || buffer[0] == 'F') {
				strcpy(var_line, buffer);
				continue;
			}
			fprintf(waveform_out, "%s", buffer);
		}
	}
exit:
	fclose(waveform_out);
	fclose(waveform_in);
	return 0;
}