/* MMN 14, assembler (project)
 *
 * Name:    Itay Krishtal   |   Vladislav Panin
 * ID:      REDACTED       |   REDACTED
 *
 * MMN14 is an assembler program designed to translate an input file written in a pre-defined assembling langauge
 * into an 'assembled' code ready to be transferred to the next stages of assembly. (linking, etc).
 *
 * NOTE: this program partially fixes some possible user errors, such as multiple and extraneous spaces and commas.
 * for example: (cmp r 1, r 2) will be fixed and is a valid line of input.*/


#include <stdlib.h>
#include "assembler.h"
int main(int argc, char* argv[])
{
	int i = 0, instruCounter = 0, dataCounter = 0;	/*instruction and data counters*/
	int line_num = 0;
	int errors_flag = 0;
	char newfilename[MAX_NAME_LENGTH] = { '0' };	/*append file extension.*/
	FILE* fp = NULL;								/*file pointer*/
	object_line objectroot;							/*root ptr for list of obj*/
	symbol_table_line symbolroot, extroot, entroot;	/*ext, ent, and sym tables to be printed.*/
	symbol_table_line label_list;					/*collectes all labels in first run to find them in secondrun*/
	symbol_table_line entlist;						/*lists all entries so that their label can be filled in later.*/
	symbol_table_line extlist;			/*holds a list of all .externs from first run
  										so that it can be compared to in secondrun*/

	if (argc < 2)	/*in case user did not input any files.*/
	{
		fprintf(stderr, "\nError: no input files received!\n");
		return 1;
	}

	for (i = 1; i < argc; i++)	/*run through all input file names.*/
	{
		/*variables and list resets*/
		instruCounter = 0; dataCounter = 0;	/*reset before each file.*/
		/*reset roots.*/
		obj_zeroize_node(&objectroot);			/*prevent trash data, address miscalculation, etc*/
		sym_zeroize(&extlist);
		sym_zeroize(&entroot);
		sym_zeroize(&extroot);
		sym_zeroize(&entlist);
		sym_zeroize(&symbolroot);
		sym_zeroize(&label_list);

		/*hold string of .as file for opening*/
		strcpy(newfilename, argv[i]);
		strcat(newfilename, ".as");
		if (!(fp = fopen(newfilename, "r"))) {		/*open file in argv[i] and assign pointer to it (fp)*/
			/*in case failed to open*/
			printf("Error: failed to open file %s, please check file name and directory.\n", argv[i]);
			continue;	/*to next file parameter*/
		}

		/*iterations*/
		first_iteration(fp, &objectroot, &symbolroot, &extlist, &entlist, &line_num, &errors_flag, &dataCounter, &instruCounter,&label_list);		/*first run through input file.*/
		line_num = 0;	/*reset line_num for next iteration*/
		second_iteration(&entroot, &extroot, &extlist, &entlist, &label_list, &symbolroot, &objectroot, &errors_flag);		/*second run through input file.*/

		if (!errors_flag)		/*if there are no bugs in input*/
		{
			print_obj(&objectroot, instruCounter, dataCounter, argv[i]);	/*create object file*/
			if (extroot.name[0] != '\0')	/*and create external and entry files if exists.*/
				print_ext(&extroot, argv[i]);
			if (entroot.name[0] != '\0')
				print_ent(&entroot, argv[i]);
		}

		free_symbol(&symbolroot, &extroot, &entroot, &label_list, &extlist);	/*free nodes*/
		free_obj(&objectroot);	/*free nodes*/

	}	/*end of loop through files*/
	return 0;
}	/*end of main*/
