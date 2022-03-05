/* MMN 14, assembler (project)
 *
 * Name:    Itay Krishtal   |   Vladislav Panin
 * ID:      REDACTED       |   REDACTED
 *
 * MMN14 is an assembler program designed to translate an input file written in a pre-defined assembling langauge
 * into an 'assembled' code ready to be transferred to the next stages of assembly. (linking, etc).
 *  This c-file (firstrun.c) will run on the first pass over the input file.
 */


#include "assembler.h"


void first_iteration(FILE* fp, object_line *obj_root, symbol_table_line *symbolroot, symbol_table_line *extlist, symbol_table_line *entlist, int *line_num, int *errors_flag, int *dataCounter, int *instruCounter, symbol_table_line *label_list)
{
	int i = 0, j = 0, label_flag = 0, curr_arg_group;	/*i,j used in loops, and flags.*/
	char line[MAXLINE] = { 0 };
	char *oneword = NULL;
	char *tokens[MAXLINE] = { NULL };
	object_line *local_obj = obj_root;
	symbol_table_line *local_sym = symbolroot;
	while (fgets(line, MAXLINE, fp) != NULL) {    /*parse each line*/
		*line_num = *line_num + 1;              /*increment line_num to the current line number*/
		memset(tokens, '0', sizeof(tokens));         /*reset token array for each line*/
		oneword = NULL;
		/*if(oneword != NULL)*/
		/*memset(oneword, '0', MAXLINE/2);  */  /*reset word array for each line*/
		/* START Added 20.3.2020 */
		label_flag = 0;            /*reset flags*/
		curr_arg_group = -1;
		j = 0;
		/* END Added 20.3.2020 */


		reduce_spaces(line, line_num);        /*make sure there are no extra spaces*/

		/*check if comment*/
		if (line[0] == ';')
			continue;

		/* after reduce_spaces(), line will have either 1 or 2 spaces, (if a label (:) exists),
		 * any other space will be deleted.		i.e LABEL: OPCODE PARAM1,PARAM2*/
		/* from now on, token[i] will point to different words in the phrase*/
		for (i = 0; i < MAXLINE; i++) /* after this, LABEL: OPCODE PARAM1 PARAM2 */
		{
			if (line[i] == ',')
				line[i] = ' ';
		}
		for (i = 0; i < MAXLINE; i++)
		{
			if (line[i] == ' ' && line[i+1] == ' ') {
				for(j = i+1; j < MAXLINE; j++) {
					line[j] = line[j+1];
				}
				i--;
			}
		}

		tokens[0] = strtok(line, " \t");
		for (i = 1; (tokens[i] = strtok(NULL, " \t")) && (i < MAXLINE); i++);	/*tokenize each space to create single-word parameters.*/
		/*oneword is the current word being processed*/
		oneword = tokens[0];

		for  (i = 0; i < MAXLINE-1 && oneword != NULL; i++, oneword = tokens[i]) {
			if (i == 0)	{/*check if first word is label or opcode*/
                for (j = 0; oneword[j] != '\0' && oneword[j+1] != '\0'; j++);	/*j points to last char of word*/
                if (oneword[j] == ':')  /*check if label*/
                {
                    if (check_label_validity(oneword, errors_flag) == 2) {
                        label_flag = 1;
                        write_symbol(oneword, symbolroot, obj_root, extlist, line_num, errors_flag);
                        continue;
                    }
                    else {
                    	fprintf(stderr, "Error: in line %d, label can not be reserved word, too long or start with non-alphabetical.\n", *line_num);
                    	*errors_flag = 1;
                        break;
                    }
                }

                if ((check_dot(oneword, tokens, line_num, obj_root, errors_flag) == 1))	/*check if dot (.) symbol. skips if not a dot operator.*/
                {
						parse_data(tokens, line_num, obj_root, errors_flag, i, dataCounter, instruCounter, extlist, entlist, symbolroot);
						break;      /*it's a .data/.string/.extern/.entry, handled by parse_data(). goto next line*/
                }

				else	/*should be an opcode, check and write accordingly*/
				{
                    if ((curr_arg_group = parse_opcode(oneword, obj_root, errors_flag, instruCounter)) != -1); /*will check and write if true, go to 'else' if check fails.*/
                    else
                    {
                        fprintf(stderr, "Error: in line %d, non-existant opcode. (%s)\n", *line_num, oneword);
                        break;  /*if opcode invalid, we can't check other arguments.*/
                    }
				}
			}
			else if (i == 1)
			{
				if (label_flag == 1)    /*if first word was a label*/
				{
                    if ((check_dot(oneword, tokens, line_num, obj_root, errors_flag) == 1))	/*check if dot (.) symbol. skips if not a dot operator.*/
                    {
                            parse_data(tokens, line_num, obj_root, errors_flag, i, dataCounter, instruCounter, extlist, entlist, symbolroot);
                            break;  /*it's a .data/.string/.extern, handled by parse_data()*/
                    }

					else if (curr_arg_group == -1 && ((curr_arg_group = parse_opcode(oneword, obj_root, errors_flag, instruCounter)) != -1)) { /*first word was a label, it's an opcode*/
						/*put last obj's address in appropriate label*/
						while (local_obj->nextline != NULL)
							local_obj = local_obj->nextline;	/*get last obj*/
						while (local_sym != NULL)
						{
							if (strcmp(local_sym->name, tokens[0]) != 0)	/*find correct label*/
								local_sym = local_sym->nextline;
							else
							{	/*label found, write address.*/
								local_sym->address = local_obj->address;
								break;
							}
						}
					    continue;
					}    /*will check if opcode and write it if true, go to 'else' if check fails.*/
					else
                    {
                        fprintf(stderr, "Error: in line %d, non-existant opcode. (%s)\n", *line_num, oneword);
                        break;  /*if opcode invalid, we can't check other arguments.*/
                    }
				}
				else    /*first word wasn't a label, this is operand1*/
				{
					/* Operand 1 is checked here */
					/* We check if the operand answers to the required argmethods. */
					if(check_operand_validity(oneword, curr_arg_group, 1, line_num, errors_flag) == -1) {
						fprintf(stderr ,"Error: in line %d, missing or wrong type of argument:(\"%s\")\n", *line_num, oneword);
						*errors_flag = 1;
                        break;  /*missing arguments.*/
					}
					if(check_operand_validity(oneword, curr_arg_group, 1, line_num, errors_flag)) {
						write_operand_method(obj_root, oneword, line_num, errors_flag, 1, curr_arg_group); /*write the method of the operand to the opcode node.*/
						write_operand(obj_root, label_list, oneword, line_num, errors_flag, 1, curr_arg_group, instruCounter);        /*creates a new node and writes operand to it.*/
					} /* If the operand does comply with the policy of the opcode before it, then write accordingly */
					else {
						fprintf(stderr ,"Error: in line %d, invalid or extraneous operand: %s\n", *line_num, oneword);
						*errors_flag = 1;
                        break;  /*if opcode invalid, we can't check other arguments.*/
					} /* Operand does not comply - throw an error and set the flag.  */
				}
			}
			else if (i == 2)
			{
				if (label_flag == 1)
				{
                    /* Operand 1 is checked here */
                    /* We check if the operand answers to the required argmethods. */
					if(check_operand_validity(oneword, curr_arg_group, 1, line_num, errors_flag) == -1) {
						fprintf(stderr ,"Error: in line %d, missing or wrong type of argument:(\"%s\")\n", *line_num, oneword);
						*errors_flag = 1;
                        break;  /*missing arguments.*/
					}

                    if(check_operand_validity(oneword, curr_arg_group, 1, line_num, errors_flag)) {
                        write_operand_method(obj_root, oneword, line_num, errors_flag, 1, curr_arg_group); /*write the method of the operand to the opcode node.*/
                        write_operand(obj_root, label_list, oneword, line_num, errors_flag, 1, curr_arg_group, instruCounter);        /*creates a new node and writes operand to it.*/
                    } /* If the operand does comply with the policy of the opcode before it, then write accordingly */
                    else {
                        fprintf(stderr ,"Error: in line %d, invalid or extraneous operand: %s\n", *line_num, oneword);
                        *errors_flag = 1;
                        break;  /*if opcode invalid, we can't check other arguments.*/
                    } /* Operand does not comply - throw an error and set the flag.  */
				}
				else
				{
					/* Operand 2 is checked here */
					/* We check if the operand answers to the required argmethods. */
					if(check_operand_validity(oneword, curr_arg_group, 2, line_num, errors_flag) == -1) {
						fprintf(stderr ,"Error: in line %d, missing or wrong type of argument:(\"%s\")\n", *line_num, oneword);
						*errors_flag = 1;
                        break;  /*missing arguments.*/
					}
					if(check_operand_validity(oneword, curr_arg_group, 2, line_num, errors_flag)) {
                        write_operand_method(obj_root, oneword, line_num, errors_flag, 2, curr_arg_group); /*write the method of the operand to the opcode node.*/
                        write_operand(obj_root, label_list, oneword, line_num, errors_flag, 2, curr_arg_group, instruCounter);        /*creates a new node and writes operand to it.*/
						continue;
					} /* If the operand does comply with the policy of the opcode before it, then write accordingly */
					else {
						fprintf(stderr ,"Error: in line %d, invalid or extraneous operand: %s\n", *line_num, oneword);
						*errors_flag = 1;
						break;  /*if opcode invalid, we can't check other arguments.*/
					} /* Operand does not comply - throw an error and set the flag.  */
				}
			}
			else if (i == 3)
			{
				if (label_flag == 1)
				{
					/* Operand 2 is checked here */
					/* We check if the operand answers to the required argmethods. */
					if(check_operand_validity(oneword, curr_arg_group, 2, line_num, errors_flag) == -1) {
						fprintf(stderr ,"Error: in line %d, missing or wrong type of argument:(\"%s\")\n", *line_num, oneword);
						*errors_flag = 1;
                        break;  /*missing arguments.*/
					}
					if(check_operand_validity(oneword, curr_arg_group, 2, line_num, errors_flag)) {
                        write_operand_method(obj_root, oneword, line_num, errors_flag, 2, curr_arg_group); /*write the method of the operand to the opcode node.*/
                        write_operand(obj_root, label_list, oneword, line_num, errors_flag, 2, curr_arg_group, instruCounter);        /*creates a new node and writes operand to it.*/
						continue;
					} /* If the operand does comply with the policy of the opcode before it, then write accordingly */
					else {
						fprintf(stderr ,"Error: in line %d, invalid or extraneous operand. (%s)\n", *line_num, oneword);
						*errors_flag = 1;
						break;  /*if opcode invalid, we can't check other arguments.*/
					} /* Operand does not comply - throw an error and set the flag.  */
				}
				else
				{
					fprintf(stderr ,"Error: in line %d, unexpected expression: %s\n", *line_num, oneword);
					*errors_flag = 1;
					break;
				}
			}
			else
			{
				fprintf(stderr, "Error: in line %d, extraneous parameters. (%s)\n", *line_num, oneword);
				*errors_flag = 1;
				break;
			}
		}

		if(oneword == NULL && i != 0 && curr_arg_group != 0 && curr_arg_group != ARG_GROUP_6) {
			if (curr_arg_group < ARG_GROUP_3 && ((label_flag == 1 && i <= 3) || (label_flag == 0 && i <= 2))) {
				fprintf(stderr, "Error: in line %d, too few parameters.\n", *line_num);
				*errors_flag = 1;
			}
			else if (curr_arg_group >= ARG_GROUP_3 && ((label_flag == 1 && i <= 2) || (label_flag == 0 && i <= 1))) {
				fprintf(stderr, "Error: in line %d, too few parameters.\n", *line_num);
				*errors_flag = 1;
			}
		}


	}
}
