/* MMN 14, assembler (project)
 *
 * Name:    Itay Krishtal   |   Vladislav Panin
 * ID:      REDACTED       |   REDACTED
 *
 * MMN14 is an assembler program designed to translate an input file written in a pre-defined assembling langauge
 * into an 'assembled' code ready to be transferred to the next stages of assembly. (linking, etc).
 * This c file will hold functions which will help write better code for main, first and second_iteration.
 * mainly, functions to help work on dynamic lists,
 * for example: add a new member to the list, edit a member, print data from a member
 */


#include <ctype.h>
#include <math.h>
#include "assembler.h"
#define BYTES 8

/* binword_to_int turns the binword[2]
 * from struct object_line (2 chars, bit-specific representation) to a decimal int.
 * calculates each bit into decimal using (bit value)*(2^bit number).
 * returns an int.
 * takes a ptr to phrase parameter.*/
int binword_to_int(object_line *node)
{
	int i = 0, j = 0, val = 0, mask = 1;	/*00000001*/

	/*loop will go through binword bits, adding them to val.*/
	for (i = 0; i < BYTES; i++) {
		val += ((((mask & node->binword[0]))/mask) * (int)(pow(2, i)));
		mask <<= 1;
	}
	mask = 1;	/*00000001*/
	j = i;
	for (i = 0; i < BYTES; i++) {	/*loop for binword[1]*/
		val += (((mask & node->binword[1])/mask) * (int)(pow(2, j)));
		j++;
		mask <<= 1;
	}
	return val;
}

/* print_obj will create the obj file, according to the required format.
 * parameters are root of object_line*, instructions counter, data counter and filename.
 * returns void (creates a new file) */
void print_obj(object_line* root, int instructions, int data, char* filename)
{
	FILE* fp = NULL;
	object_line* current = root;
	int val = 0;

	char newfilename[MAX_NAME_LENGTH] = { '0' };	/*append file extension.*/
	strcpy(newfilename, filename);
	strcat(newfilename, ".obj");

	/* first, create the file, check for fails, write first line (number of instruction words (decimal), tab, number of data words (decimal)
 	* afterwards, each line will write the mem address (should start with 0100), tab, word in octal. */
	if ((fp = fopen(newfilename, "w")) == NULL) {
		fprintf(stderr, "Error: failed to create obj file.\n");
		return;
	}

	fprintf(fp, "%4d %d\n", instructions, data);	/*write the first line, which is the instruc cnt, and data cnt.*/
	while (current != NULL) {
		if (current->string_char != '\0')	/*print char*/
			val = current->string_char;

		else if (current->dataptr != NULL)	/*print int*/
			val = (current->data);	/*points to null */

		else	/*print binword*/
			val = binword_to_int(current);

		fprintf(fp, "%04d %05o\n", current->address, val);
		current = current->nextline;
	}
	return;
}

/* print_ext will create the ext file, according to the required format.
 * parameters are root of extroot, and filename.
 * returns void (creates a new file) */
void print_ext(symbol_table_line *extroot, char* filename)
{
	symbol_table_line *current = extroot;	/*to iterate through the lines*/
	FILE *fp = NULL;	/*fp for file creation*/

	char newfilename[MAX_NAME_LENGTH] = { '0' };	/*append file extension.*/
	strcpy(newfilename, filename);
	strcat(newfilename, ".ext");

	if ((fp = fopen(newfilename, "w")) == NULL) {
		fprintf(stderr, "Error: failed to create externals file.\n");
		return;
	}

	while (current != NULL) {
		fprintf(fp, "%s\t%04d\n", current->name, current->address);
		current = current->nextline;
	}
	return;
}

/* print_ent will create the ent file, according to the required format.
 * parameters are root of entroot, and filename.
 * returns void (creates a new file) */
void print_ent(symbol_table_line *entroot, char* filename)
{
	symbol_table_line *current = entroot;	/*to iterate through the lines*/
	FILE *fp = NULL;	/*fp for file creation*/

	char newfilename[MAX_NAME_LENGTH] = { '0' };	/*append file extension.*/
	strcpy(newfilename, filename);
	strcat(newfilename, ".ent");

	if ((fp = fopen(newfilename, "w")) == NULL) {
		fprintf(stderr, "Error: failed to create entries file.\n");
		return;
	}

	while (current != NULL) {
		fprintf(fp, "%s\t%04d\n", current->name, current->address);
		current = current->nextline;
	}
	return;
}

/*	reduces unnecessary spaces in input line.
 *	takes a line parameter.
 	returns void (alters given parameter)*/
void reduce_spaces(char* line, int *line_num)
{
	int current = 0, next = 0, i = 0, j = 0, colon_flag = 0, space_flag = 0;

	for (current = 0; line[current] != '\0'; current++)	/*run through line, until \0 encountered.*/
	{
		if (line[current] == ':' && space_flag == 0)	/*if char is a colon*/
		{
			if (colon_flag == 1)	/*if there's more than one colon*/
			{
				fprintf(stderr, "Error: extraneous colon(:) sign in string: %d %s\n", *line_num, line);
				continue;
			}
			colon_flag = 1;
			continue;
		}
		/*test if string starts with spaces*/
		else if(isspace(line[current]) && (current == 0)) {	/*every time current is space, shift all chars backwards once.*/
			for(j = 0; j < MAXLINE-1; j++) {
				line[j] = line[j+1];
			}
			current--; /*make current stay in the same place to re-check for more spaces*/
		}
						/*tests if multiple spaces exist in series*/
		else if (isspace(line[current]) && line[current+1] != '\0' && isspace(line[current+1]))
		{	/*delete any multiple-space occurrences.*/
			for (next = current + 1; line[next] != '\0'; next++) {
				i = next + 1;
				line[next] = line[i];
			}
			current--;    /*make current stay in the same place to re-check for more spaces*/
		}	/*from here on, line has only single-spaces.*/
		else if (isspace(line[current]) && ((colon_flag == 1 && space_flag < 2) || (colon_flag == 0 && space_flag < 1)))
		{
			space_flag++;
			continue;
		}
		else if (isspace(line[current]))
		{
			for (next = current; line[next] != '\0'; next++) {	/*shift all chars << 1*/
				i = next + 1;
				line[next] = line[i];
			}
		}
	}

	for(current = 0; current < MAXLINE; current++) {
		if(line[current] == '\n') {
			line[current] = 0;
			break;
		}
	}
	return;
}

/*creates a new object_line node at the end of object_line list
 * returns a pointer to the new (last) object_line node
 */
object_line *obj_newnode(object_line *obj_root)
{
	object_line *obj_new = malloc(sizeof(object_line));	/*pointer to new node*/
	object_line *obj_ptr = obj_root;    /*a copy of pointer to root*/
	int lastnode_address = 0;           /*will be the new node's address*/

	while (obj_ptr->nextline != NULL)   /*find last node*/
		obj_ptr = obj_ptr->nextline;

	lastnode_address = obj_ptr->address;/*copy last node's address*/

	obj_ptr->nextline = obj_new;        /*add to list*/

	/*set all members.*/
	obj_new->nextline = NULL;
	obj_new->address = lastnode_address+1;
	obj_new->string_char = '\0';
	obj_new->data = 0;
	obj_new->dataptr = NULL;
	obj_new->binword[0] = '\0';	/*set all bits to 0*/
	obj_new->binword[1] = '\0';
	return obj_new;	/*return pointer to new node*/
}

/* reset the object type node pointed to.
 * requires a pointer to node.
 * returns a pointer to the object_line node.*/
object_line *obj_zeroize_node(object_line *obj_node)
{
	/*set all members.*/
	obj_node->nextline = NULL;
	obj_node->address = 100;			/*first address is 0100*/
	obj_node->string_char = '\0';
	obj_node->data = 0;
	obj_node->dataptr = NULL;
	obj_node->binword[0] = '\0';	/*set all bits to 0*/
	obj_node->binword[1] = '\0';
	return obj_node;	/*return pointer to new node*/
}

/*checks if obj line was already written to or is clean.
 * requires pointer to obj line.
 * returns 1 if clean, 0 otherwise. */
int check_obj_clean(object_line *obj_root)
{
    if (obj_root->data == 0 && obj_root->nextline == NULL && obj_root->string_char == '\0' && obj_root->binword[0] == '\0' && obj_root->binword[1] == '\0')
        return 1;
    else
        return 0;
}

/* parses data types (checks if .data or .string, adds to obj_root list accordingly)
 * parameters are a pointer to an array of tokens in a phrase, number of line being parsed, obj_root, errors flag, line_num
 * and which word is the dot operator aka word_count (first, or second).
 * returns void (modifies lists).*/
void parse_data(char **tokens, int *line_num, object_line *obj_root, int *errors_flag, int word_count, int *dataCounter, int *instruCounter, symbol_table_line *extlist, symbol_table_line *entlist, symbol_table_line *symbolroot)
{
	int flag_type = 0, curr_val = 0, i = 0, j = 0, mask = 0x7FFF;
	char label[32] = { '\0' };
    object_line *localobj = obj_root; /*local pointer to iterate through the list*/
	symbol_table_line *local_extlist = extlist;
	symbol_table_line *local_entlist = entlist;
	symbol_table_line *locallabel = symbolroot;

	/* tokens[0] is either the data type symbol or a label.
	 * tokens[1] is either the data type symbol or parameters
	 * tokens[2] is either null or parameters.*/

	/*check data type*/
	if (!strcmp(tokens[word_count], ".data"))
		flag_type = 1;
	else if (!strcmp(tokens[word_count], ".string"))
		flag_type = 2;
	else if (!strcmp(tokens[word_count], ".extern"))
		flag_type = 3;
	else if (!strcmp(tokens[word_count], ".entry"))
		flag_type = 4;

	/*parse, check and add each operand*/
	if (tokens[word_count + 1] == NULL) {	/*no operands*/
	    fprintf(stderr, "Error: in line %d: missing %s operand.\n", *line_num, tokens[word_count]);
	    return;
    }
	else if (flag_type == 1) {	/*params are ints*/
	    /*check params include only digits and commas*/
	    for (i = 0; !isspace(tokens[word_count+1][i]) && tokens[word_count+1][i] != '\0' ; i++)
        {
            /*if it's not a signed int or a comma*/
	        if (!((tokens[word_count+1][i] >= '0' && tokens[word_count+1][i] <= '9') || tokens[word_count+1][i] == '-') || (tokens[word_count+1][i] == '-' && tokens[word_count+1][i+1] == '-'))
            {
                *errors_flag = 1;
                fprintf(stderr, "Error: in line %d, .data operands can be be integers only, negatives with single minus sign.\n", *line_num);
                return;
            }
			if (tokens[word_count+1][i+1] == '\0' && ((tokens[word_count+1][i+2] >= '0' && tokens[word_count+1][i+2] <= '9') || tokens[word_count+1][i+2] == '-'))
				i++;
        }

		if (i != 0) {	/*i == 0 means there are no arguments*/
			j = 1;
			while (tokens[word_count+j] != NULL) {
				curr_val = strtol(tokens[word_count+j], NULL, 10);   /*string to int stored in curr_val*/
				if (curr_val > 16383 || curr_val < -16384) {
					*errors_flag = 1;
					fprintf(stderr, "Error: in line %d, .data operand size beyond limits\n", *line_num);
				}

				if (!check_obj_clean(obj_root))
					localobj = obj_newnode(obj_root);                 /*create a new node*/
				localobj->data = curr_val;                           /*set the node's char to the string's char*/
				if (curr_val < 0)	/*if negative number*/
					localobj->data = mask & localobj->data;			/*int is bigger than 15 bits. this is a 15 bit mask.*/
				localobj->dataptr = &(localobj->data);
				*dataCounter += 1;
				j++;
			}
		}
	    else
		{
	    	*errors_flag = 1;
	    	fprintf(stderr, "Error: in line %d, .data without operands.\n", *line_num);
		}
    }

	else if (flag_type == 2) {	/*param is a string*/
	    /*check for two " signs, add each char. */
        if (tokens[word_count+1] != NULL)
        {
            for (i = 0; tokens[word_count+1][i] != '\0'; i++); /*move i to \n (length of string)*/
            if (tokens[word_count+1][0] == '"' && tokens[word_count+1][i-1] == '"')   /*check both first and last chars are " */
            {
                /*add chars to obj list*/
                for (j = 1; j < i-1; j++)
                {
                    if (!check_obj_clean(obj_root))
                        localobj = obj_newnode(obj_root);                 /*create a new node*/
                    localobj->string_char = tokens[word_count+1][j];   /*set the node's char to the string's char*/
					*dataCounter += 1;
                }
                obj_newnode(obj_root);   /*this node is pre-set to 0.*/
				*dataCounter += 1;
            }
            else    /*argument exists but is not according to foramt ("...")*/
            {
                *errors_flag = 1;
                fprintf(stderr, "Error: in line %d: .string parameter invalid (\"%s\")\n", *line_num, tokens[word_count+1]);
                return;
            }
        }
        else    /*no token[1] means no arguments*/
        {
            *errors_flag = 1;
            fprintf(stderr, "Error: in line %d: .string does not have parameters\n", *line_num);
            return;
        }
    }

    else if (flag_type == 3)	/*.extern param to be added to extlist after cmp*/
	{
    	/*check if operand is valid label name*/
    	if (check_label_validity(tokens[word_count+1], errors_flag) == 1)
		{
    		/*check if extern wasn't defined as label earlier.*/
    		strcpy(label, tokens[word_count+1]);
    		strcat(label, ":");
			while (locallabel != NULL && strcmp(label, locallabel->name))
				locallabel = locallabel->nextline;
			if (locallabel != NULL) {
				*errors_flag = 1;
				fprintf(stderr,"Error: in line %d, external label already defined as internal. (%s)\n", *line_num, tokens[word_count+1]);
				return;
			}

    		/*write*/
			if (local_extlist->name[0] == '\0')	/*write to root if clean*/
				strcpy(local_extlist->name, tokens[word_count+1]);
			else    /*otherwise, write to new node*/
			{
				local_extlist = sym_newnode(local_extlist);	/*create new node*/
				strcpy(local_extlist->name, tokens[word_count+1]);	/*copy label name*/
			}
		}
    	else
    		fprintf(stderr, "Error: in line %d, bad label operand.\n", *line_num);
	}
	else	/*it's an .entry*/
	{
		/*add the entry operand to entlist so that it can be compared at secondrun
		 * and if label will exist, add it to entroot.*/

		/*check if operand is valid label name*/
		if (check_label_validity(tokens[word_count+1], errors_flag))
		{
			if (local_entlist->name[0] == '\0')    /*write to root if clean*/
			{
				strcpy(local_entlist->name, tokens[word_count+1]);
			}
			else    /*otherwise, write to new node*/
			{
				local_entlist = sym_newnode(local_entlist);	/*create new node*/
				strcpy(local_entlist->name, tokens[word_count+1]);	/*copy label name*/
			}
		}
	}
	return;
}


/* func will check if opcode is valid, and add it as a node to obj list if it is.
 * otherwise, will raise error flag.
 * returns -1 if not an opcode, or the appropriate ARG_GROUP (int) otherwise..*/
int parse_opcode(char *opcode, object_line *obj_root, int *errors_flag, int *instruCounter) {
	object_line *localobj = obj_root;

	if (!strcmp(opcode, "mov")) {
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		*instruCounter += 1;
	}
	else if(!strcmp(opcode, "cmp")) {
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x8;		/*writes 1*/
		*instruCounter += 1;
	}

	else if(!strcmp(opcode, "add")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x10;	/*writes 2*/
	}

	else if (!strcmp(opcode, "sub")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x18;    /*writes 3*/
	}

	else if(!strcmp(opcode, "lea")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x20;    /*writes 4*/
	}

	else if(!strcmp(opcode, "clr")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x28;    /*writes 5*/
	}

	else if(!strcmp(opcode, "not")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x30;    /*writes 6*/
	}

	else if(!strcmp(opcode, "inc")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x38;    /*writes 7*/
	}

	else if(!strcmp(opcode, "dec")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x40;    /*writes 8*/
	}

	else if(!strcmp(opcode, "jmp")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x48;    /*writes 9*/
	}

	else if(!strcmp(opcode, "bne")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x50;    /*writes 10*/
	}

	else if(!strcmp(opcode, "red")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x58;    /*writes 11*/
	}

	else if(!strcmp(opcode, "prn")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x60;    /*writes 12*/
	}

	else if(!strcmp(opcode, "jsr")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x68;    /*writes 13*/
	}

	else if(!strcmp(opcode, "rts")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x70;    /*writes 14*/
	}

	else if(!strcmp(opcode, "stop")) {
		*instruCounter += 1;
		if (!check_obj_clean(obj_root))
			localobj = obj_newnode(obj_root);   /*create new node*/
		localobj->binword[1] |= 0x78;    /*writes 15*/
	}

	else { /*not an opcode*/
		*errors_flag = *errors_flag+1;
		return -1;
	}

	localobj->binword[0] |= 0x4;	/*writes the A flag.*/
	return check_arg_group(opcode);
}

/*checks if dot is valid, and of what type.
 * params are the word being processed, the tokens arr, line number value, and a pointer to root of list of obj lines.
 * returns 0 for error (prints to stderr),
 * returns 1 for .data or .string and .extern types.
 */
int check_dot(char *oneword, char **tokens, int *line_num, object_line *obj_root, int *errors_flag)
{
    int j = 0;
    if (oneword[j] == '.')	/*check if .data or .string symbol*/
    {
        if (!strcmp(oneword, ".data") || !strcmp(oneword, ".string")) /*data / string symbol*/
            return 1;   /*it's data / string, handled completely by parse_data.*/
        else if (!strcmp(oneword, ".extern") || !strcmp(oneword, ".entry"))
            return 1;   /*it's ext or ent*/
    }
    return 0;   /*not a dot operator.*/
}

/* check_arg_group takes an opcode string.
 * It returns the argument group, to which the opcode belongs, or 0, if invalid opcode is provided.
 * Available argument groups are listed on page 30 of the course's taskbook (different combinations of methods).
 * There are seven argument groups in total.
 *
 * Note: The opcode provided should be checked beforehand - this function is not intended to be the checker. 
 * Also, the string MUST be properly terminated!
 */
int check_arg_group(char* opcode) {
	if(!strcmp(opcode, "mov") || !strcmp(opcode, "sub") || !strcmp(opcode, "add"))
		return ARG_GROUP_0;
	else if(!strcmp(opcode, "cmp"))
		return ARG_GROUP_1;
	else if(!strcmp(opcode, "lea"))
		return ARG_GROUP_2;
	else if(!strcmp(opcode, "clr") || !strcmp(opcode, "not") || !strcmp(opcode, "inc") || !strcmp(opcode, "dec") || !strcmp(opcode, "red"))
		return ARG_GROUP_3;
	else if(!strcmp(opcode, "jmp") || !strcmp(opcode, "bne") || !strcmp(opcode, "jsr"))
		return ARG_GROUP_4;
	else if(!strcmp(opcode, "prn"))
		return ARG_GROUP_5;
	else if(!strcmp(opcode, "rts") || !strcmp(opcode, "stop"))
		return ARG_GROUP_6;
	else
		return 0;
}


/* checks if all entry labels exist, and what's their address.
 * if there is a match, add it to entry root list (entroot).
 * requires a symbol root, entry list root node and entry root node.
 * returns void, alters list entroot. */
void check_entry(symbol_table_line *sym_root, symbol_table_line *entlist, symbol_table_line *entroot, symbol_table_line *extlist, int *errors_flag) {
	char localent_name[32] = { '\0' };
	symbol_table_line *extlist_local = extlist;
	symbol_table_line *entlist_local = entlist;
	symbol_table_line *entroot_local = entroot;
	symbol_table_line *sym_local = sym_root;

	/*check if root is empty*/
	if (entlist_local->name[0] == '\0')
		return;

	/*check if entry exists in extlist*/
	while (entlist_local != NULL) {
		extlist_local = extlist;
		while (extlist_local != NULL) {
			if (!strcmp(extlist_local->name, entlist_local->name)) {
				fprintf(stderr, "Error: .entry label can not be external. (%s)\n", entlist_local->name);
				*errors_flag = 1;
			}
			extlist_local = extlist_local->nextline;
		}
		entlist_local = entlist_local->nextline;
	}

	entlist_local = entlist;	/*reset*/
	while (entlist_local != NULL)
	{
		memset(localent_name, '\0', sizeof(char) * 32);	/*reset localent_name*/
		sym_local = sym_root;
		while (sym_local != NULL) {
			strcpy(localent_name, entlist_local->name);
			strcat(localent_name, ":");	/*sym_local includes ':' sign but label doesn't.*/
			if (strcmp(localent_name, sym_local->name))
			{
				sym_local = sym_local->nextline;
				continue;
			}
			else	/*found a matching label*/
			{
				if (entroot->name[0] == '\0')	/*entroot is zeroized*/
				{
					strcpy(entroot->name, sym_local->name);	/*write to node*/
					entroot->address = sym_local->address;
				}
				else	/*entroot has data*/
				{
					entroot_local = sym_newnode(entroot);			/*create new node*/
					strcpy(entroot_local->name, sym_local->name);	/*write to new node*/
					entroot_local->address = sym_local->address;
				}
				break;
			}
		}
		if (sym_local == NULL)
		{
			fprintf(stderr, "Error: call to non-existent label \"%s\"\n", entlist_local->name);
			*errors_flag = 1;
		}
		entlist_local = entlist_local->nextline;
	}
}

/*checks if label is a reserved word.
 * compares label to reserved words.
 * requires ptr to label.
 * returns 1 if reserved, 0 if not reserved.*/
int check_label_reservedword(char *label)
{	/*str compares split into multiple if-else statements for comfort of reading.
 * if str is identical to a reserved word, return 1.*/
	if(!strcmp(label, "mov:") || !strcmp(label, "sub:") || !strcmp(label, "add:") || !strcmp(label, "cmp:") || !strcmp(label, "lea:"))
		return 1;
	else if(!strcmp(label, "clr:") || !strcmp(label, "not:") || !strcmp(label, "inc:") || !strcmp(label, "dec:") || !strcmp(label, "red:"))
		return 1;
	else if(!strcmp(label, "jmp:") || !strcmp(label, "bne:") || !strcmp(label, "jsr:") || !strcmp(label, "prn:") || !strcmp(label, "rts:") || !strcmp(label, "stop:"))
		return 1;
	else if(!strcmp(label, "r0:") || !strcmp(label, "r1:") || !strcmp(label, "r2:") || !strcmp(label, "r3:") || !strcmp(label, "r4:") || !strcmp(label, "r5:") || !strcmp(label, "r6:") || !strcmp(label, "r7:"))
		return 1;
	else if(!strcmp(label, "*r0:") || !strcmp(label, "*r1:") || !strcmp(label, "*r2:") || !strcmp(label, "*r3:") || !strcmp(label, "*r4:") || !strcmp(label, "*r5:") || !strcmp(label, "*r6:") || !strcmp(label, "*r7:"))
		return 1;
	else
		return 0;
}


/* check_label_validity takes a suspected label string.
 * It returns 0 if the label does not comply with the label policy, and 1 if it does.
 * The policy is described on pages 25-26 of the course's taskbook.
 * returns 0 if bad label, returns 2 if terminated with ':' and 1 if without.
 *
 * Note: The string MUST be properly terminated! */
int check_label_validity(char* label, int *errors_flag) {
	int i = 0, count = 0;
	int is_terminated = 0; /* 1 if it's a label, that ends with :, 0 if not */

	/*check if label exists and if it starts with alphabetical letter.*/
	if(label[0] == '\0' || !isalpha(label[0])) {
		*errors_flag = 1;
		return 0;
	}

	/*check label is not a reserved word*/
	if (check_label_reservedword(label))
	{
		*errors_flag = 1;
		return 0;
	}

	for(i = 0; label[i] != '\0'; i++) {
		if(count >= 32) {
			*errors_flag = 1;
			return 0;
		}
		if(!isalpha(label[i]) && !isdigit(label[i])) {
			if(label[i] == ':') {
				if(label[i+1] != '\0') {
					*errors_flag = 1;
					return 0;
				}
				else {
					is_terminated = 1;
					break;
				}
			}
			else {
				*errors_flag = 1;
				return 0;
			}
		}
		count++;
	}
	if(is_terminated == 1)
		return 2;
	return 1;
}


/*  get_operand_method takes an operand string and returns it's method, or -1 if the operand does not comply with any.
 * requires string, and pointers to errors flag and line num.
 * returns number of operand's addressing method.
 * Note: The string MUST be properly terminated!
 */
int get_operand_method(char* arg, int *errors_flag, int *line_num) {
	int i;
	int method0 = 1;
	int method1 = 1;
	int method2 = 1;
	int method3 = 1;
	int blank_val = 5;
	
	if (arg[0] == '\0')	/*empty case*/
		return -1;

	/* Check if this is a base 10 integer constant. Example: #-9 */
	if(arg[0] != '#')
		method0 = 0;
	else if (arg[0] == '#')
	{
		if(arg[1] == '\0')
			method0 = 0;

		if(arg[1] == '-') {	/*negative number*/
			for(i = 2; arg[i] != '\0'; i++) {
				if(!isdigit(arg[i])) {
					method0 = 0;
					*errors_flag = 1;
					fprintf(stderr, "Error: in line %d, operand of this type must include decimals only. (%s)\n", *line_num, arg);
				}
			}
		}
		else {	/*non-negative number*/
			for(i = 1; arg[i] != '\0'; i++) {
				if(!isdigit(arg[i])) {
					method0 = 0;
					*errors_flag = 1;
					fprintf(stderr, "Error: in line %d, operand of this type must include decimals only. (%s)\n", *line_num, arg);
				}
			}
		}
	}

	/* Check if label. */
	method1 = check_label_validity(arg, &blank_val);

	/* Method 2 - this is a indirect registry method.
	 * It gives an access to a memory cell via a pointer in the registry.
	 * There are seven registries from 0 to 7.
	 * Example of a compliant operand of this method:
	 * *r1
	 * And another one:
	 * *r5
	 */
	if(arg[1] == '\0' || arg[2] == '\0')
		method2 = 0;
	else
		method2 = (arg[0] == '*' && arg[1] == 'r' && (arg[2] <= '7' && arg[2] >= '0') && arg[3] == '\0');
	
	/* Method 3 - this is a direct registry method.
	 * It gives an access to a content of the registry.
	 * There are seven registries from 0 to 7.
	 * Example of a compliant operand of this method:
	 * r1
	 * And another one:
	 * r5 */
	if(arg[0] == '\0' || arg[1] == '\0' || arg[2] != '\0')
		method3 = 0;
	else
		method3 = (arg[0] == 'r' && isdigit(arg[1]) && (arg[1] <= '7' && arg[1] >= '0') && arg[2] == '\0');
	
	if(method0) 
		return 0;
	if(method2) 
		return 2;
	if(method3)
		return 3;
	if(method1)
		return 1;
	*errors_flag = 1;
	return -1;
}

/*
 * check_operand_validity takes an operand, and returns 1 if the operand (must be a valid operand) complies
 * with the given arg_group. Also takes operand's position.
 *
 * Note: The string MUST be properly terminated! */
int check_operand_validity(char* operand, int arg_group, int operand_pos, int *line_num, int *errors_flag) {
	int operand_method = get_operand_method(operand, errors_flag, line_num);
	if(operand_method == -1)
		return -1;
	switch(arg_group) {
		case ARG_GROUP_0: /* 0,1,2,3 | 1,2,3 */
			switch(operand_method) {
				case 0:
					if(operand_pos == 2)
						return 0;
					else	
						return 1;
				case 1:
				case 2:
				case 3:
					return 1;
				default:
					return 0;
			}
		
		case ARG_GROUP_1: /* 0,1,2,3 | 0,1,2,3 */
			return 1;
		case ARG_GROUP_2: /* 1 | 1,2,3 */
			switch(operand_method) {
				case 0:
					return 0;
				case 1:
					return 1;
				case 2:
				case 3:
					if(operand_pos == 2)
						return 1;
					else	
						return 0;
				default:
					return 0;
			}
		
		case ARG_GROUP_3: /* 1,2,3 | NaN */
			if(operand_pos == 2)
				return 0;
			else {
				switch(operand_method) {
					case 1:
					case 2:
					case 3:
						return 1;
					default:	/*also case 0*/
						return 0;
				}
			}
		
		case ARG_GROUP_4: /* 1,2 | NaN */
			if(operand_pos == 2)
				return 0;
			else {
				switch(operand_method) {
					case 1:	/*continues to case 2*/
					case 2:
						return 1;
					case 0:	/*continues to default*/
					case 3:
					default:
						return 0;
				}
			}
		
		case ARG_GROUP_5: /* 0,1,2,3 | NaN */
			if(operand_pos == 2)
				return 0;
			else {
				switch(operand_method) {
					case 0:	/*continues to case 3*/
					case 1:
					case 2:
					case 3:
						return 1;
					default:
						return 0;
				}
			}
		
		case ARG_GROUP_6: /* NaN | NaN */
		default:
			return 0;
	}
}

/*writes the operand method to the last node in the obj list.
 * requires root of obj list, the operand (string), line num, pointer to error flag, and number indicating which operand it is.
 *returns void. */
void write_operand_method(object_line *root, char *operand, int *line_num, int *errors_flag, int op, int curr_arg_group)
{
	int method = 0;
	object_line *last = root;

	if ((method = get_operand_method(operand, errors_flag, line_num)) < 0) {
		return;
	}
	else {	/*valid operand method*/
		if (op == 2)	/*opcode is 1 before last, point to it*/
			while (last->nextline->nextline != NULL)
				last = last->nextline;
		else
			while (last->nextline != NULL)
				last = last->nextline;	/*set 'last' to last node.*/

		if (op == 1 && !(curr_arg_group > ARG_GROUP_2 && curr_arg_group < ARG_GROUP_6))	/*source operand which is not an only operand*/
		{
			switch (method) {
				case 0:
					last->binword[0] |= 0x80; /*turn on the bit for method 0, 0001*/
					return;

				case 1:
					last->binword[1] |= 0x1; /*turn on the bit for method 1, 0010*/
					return;

				case 2:
					last->binword[1] |= 0x2; /*turn on the bit for method 2, 0100*/
					return;

				case 3:
					last->binword[1] |= 0x4; /*turn on the bit for method 3, 1000*/
					return;
			}
		}
		else	/*destination operand*/
		{
			switch (method) {
				case 0:
					last->binword[0] |= 0x8; /*turn on the bit for method 0, 0001*/
					return;

				case 1:
					last->binword[0] |= 0x10; /*turn on the bit for method 1, 0010*/
					return;

				case 2:
					last->binword[0] |= 0x20; /*turn on the bit for method 2, 0100*/
					return;

				case 3:
					last->binword[0] |= 0x40; /*turn on the bit for method 3, 1000*/
					return;
			}
		}
	}
}

/* writes new operands to obj list.
 * requires root of obj list, operand string, argument group, line num, err flag, and which operand it is (src or dest)
 *returns void (alters list).
 */
void write_operand(object_line *root, symbol_table_line *label_list, char *operand, int *line_num, int *errors_flag, int op_num, int curr_arg_group, int *instruCounter)
{
	int op_method = 0;
	object_line *op_node;
	object_line *opcode = root;

	if ((op_method = get_operand_method(operand, errors_flag, line_num)) < 0) {	/*get op's method*/
		fprintf(stderr, "Error: in line %d, operand's addressing method invalid. (%s)\n", *line_num, operand);
		*errors_flag = 1;
		return; 	/*return if bad operand.*/
	}


	if (op_num == 1)
	{
		/*create new node and write to it*/
		op_node = obj_newnode(root);	/*new node*/
		*instruCounter += 1;
		switch (op_method) {
			case 0:
				obj_write_method0(operand, op_node, line_num, errors_flag);
				return;
			case 1:
				/* it's a label. an empty node was added 6 lines ago, node will be written to at secondrun*/
				/*save the address of node and label name to label_list*/
				label_list_newnode(label_list, root, operand);
				return;
			default:	/*cases 2 and 3*/
				obj_write_reg(op_node, operand, op_num, line_num, errors_flag, curr_arg_group);
				return;
		}	/*end switch*/
	}
	else	/*second operand*/
	{
		while (opcode->nextline->nextline != NULL)
			opcode = opcode->nextline;	/*go to opcode node*/

		if (((opcode->binword[0] & 0x20) || (opcode->binword[0] & 0x40)) && ((opcode->binword[1] & 0x2) || (opcode->binword[1] & 0x4)) && (op_method == 2 || op_method == 3))
		{
			/*if both operands are registers, reuse the last node.*/
			op_node = opcode->nextline;
			obj_write_reg(op_node, operand, op_num,  line_num, errors_flag, curr_arg_group);
			return;
		}
		else	/*not register*/
		{
			/*create new node and write to it*/
			op_node = obj_newnode(root);	/*new node*/
			*instruCounter += 1;
			switch (op_method) {
				case 0:
					obj_write_method0(operand, op_node, line_num, errors_flag);
					return;
				case 1:
					/*an empty node was added 6 lines ago, node will be written to at secondrun*/
					label_list_newnode(label_list, root, operand);
					return;
				default:	/*cases 2 and 3*/
					obj_write_reg(op_node, operand, op_num, line_num, errors_flag, curr_arg_group);
					return;
			}	/*end switch*/
		}
	}
}

/* creates a new node in label_list (after the last node), and writes to it the name of the label and address where it can be found.
 * requires ptr to obj root to find lateset address, ptr to label_list to add new node and name of label.
   returns ptr to new node.*/
symbol_table_line *label_list_newnode(symbol_table_line *label_list,object_line *root, char *operand)
{
	int address = 100;	/*iterator, address*/
	object_line *obj_local = root;				/*ptrs to node*/
	symbol_table_line *lab_local = label_list;

	/*point to last obj node*/
	while (obj_local->nextline != NULL)
		obj_local = obj_local->nextline;

	address = obj_local->address; /*acquire last node's address*/

	if (label_list->name[0] == '\0')	/*list empty, write to root.*/
	{
		label_list->address = address;
		strcpy(label_list->name, operand);
		return label_list;
	}
	else	/*add new node*/
	{
		while (lab_local->nextline != NULL)    /*ptr to last lab node*/
			lab_local = lab_local->nextline;

		lab_local->nextline = malloc(sizeof(symbol_table_line));
		lab_local = lab_local->nextline;	/*goto new node.*/
		
		lab_local->address = address;
		lab_local->nextline = NULL;
		strcpy(lab_local->name, operand);
		return lab_local;
	}
}


/* obj_write_reg writes a register's value into the appropriate binword.
 * requires a pointer to node, operand string (register), and number of operand, i.e is it the destination or source.
 * returns void, alters node. */
void obj_write_reg(object_line *op_node, char *operand, int operand_num, int *line_num, int *errors_flag, int curr_arg_group)
{
	char *ptr = operand;
	int reg = 0;

	if (*ptr == '*')	/*if register stores an address, skip the delimiter '*' (i.e *r2) */
		ptr = &ptr[1];

	reg = strtol(&ptr[1], NULL,10);	/*translate register num to int*/

	if (operand_num == 1 && !(curr_arg_group > ARG_GROUP_2 && curr_arg_group < ARG_GROUP_6))
	{
		switch (reg)
		{
			case 1:
				op_node->binword[0] |= 0x40;	/*X	01000ARE*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			case 2:
				op_node->binword[0] |= 0x80;	/*X	10000ARE*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			case 3:
				op_node->binword[0] |= 0xC0;	/*X	11000ARE*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			case 4:
				op_node->binword[1] |= 0x01;	/*1	00000ARE*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			case 5:
				op_node->binword[0] |= 0x40;	/*X	01000ARE*/
				op_node->binword[1] |= 0x01;	/*1	00000ARE*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			case 6:
				op_node->binword[0] |= 0x80;	/*X	10000*/
				op_node->binword[1] |= 0x01;	/*1 00000*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			case 7:
				op_node->binword[0] |= 0xC0;	/*X	11000*/
				op_node->binword[1] |= 0x01;	/*1 00000*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			default:
				fprintf(stderr, "Error: in line %d, non-existent register. (%s. r0 - r7 only)\n", *line_num, operand);
				*errors_flag = 1;
				return;
		}
	}
	else /*operand_num is 2*/
	{
		switch (reg)
		{
			case 1:								/*reg*/
				op_node->binword[0] |= 0x08;	/*001*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			case 2:
				op_node->binword[0] |= 0x10;	/*010*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			case 3:
				op_node->binword[0] |= 0x18;	/*011*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			case 4:
				op_node->binword[0] |= 0x20;	/*100*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			case 5:
				op_node->binword[0] |= 0x28;	/*101*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			case 6:
				op_node->binword[0] |= 0x30;	/*110*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			case 7:
				op_node->binword[0] |= 0x38;	/*111*/
				op_node->binword[0] |= 0x4;		/*X	000001RE*/
				return;
			default:
				fprintf(stderr, "Error: in line %d, non-existent register. (%s. r0 - r7 only)\n", *line_num, operand);
				*errors_flag = 1;
				return;
		}
	}
}

/* writes operand into given obj node, in format of addressing method 0.
 * requires the operand and a node to write to.
   returns void, alters node in obj list.*/
void obj_write_method0(char *operand, object_line *op_node, int *line_num, int *errors_flag)
{
	int val = 0, i = 0, bit = 0, mask = 0xFF;
	char *local_op = operand;

	if (local_op[0] == '#')
		local_op = &local_op[1];

	val = strtol(local_op, NULL, 10);
	if (val > 2047 || val < -2048)
	{
		fprintf(stderr, "Error: in line %d, integer out of range. (%s)\n", *line_num, operand);
		*errors_flag = *errors_flag + 1;
		return;
	}


	/*convert to binary and write to binword*/	/*000000|00000ARE*/
	op_node->binword[0] |= 0x4;	/*A flag.*/
	bit = 8;

	if (val >= 0)
	{

		for (i = 0; (i < 5) && (val > 0); i++)	/*5 the amount of iterations required to go through all bits.*/
		{
			if (val%2)
				op_node->binword[0] |= bit;
			val = val/2;/*divide val for next bit*/
			bit *= 2;	/*move to next bit*/
		}
		bit = 1;
		for (i = 0; (i < 6) && (val > 0); i++)	/*7 is the 1 bigger than the amount of iterations required to go through all bits.*/
		{
			if (val%2)
				op_node->binword[1] |= bit;
			val = val/2;/*divide val for next bit*/
			bit *= 2;	/*move to next bit*/
		}
	}
	else if (val < 0)
	{
		val = val * (-1); /*remove negative sign*/

		op_node->binword[1] |= 0x40;	/*signed negative.*/

		for (i = 0; (i < 5); i++) {	/*5 is the amount of iterations required to go through all bits.*/
			if (!(val%2))	/*write if remainder is 0*/
				op_node->binword[0] |= bit;
			val = val/2;/*divide val for next bit*/
			bit *= 2;	/*move to next bit*/
		}

		/*move to next binword char (binword[1])*/
		bit = 1;	/*reset bit mask*/
		for (i = 0; (i < 6); i++)	/*6 is the amount of iterations required to go through all bits.*/
		{
			if (!(val%2))
				op_node->binword[1] |= bit;
			val = val/2;/*divide val for next bit*/
			bit *= 2;	/*move to next bit*/
		}

		/*-----add 1 to complete two's complement-----
		 * adding one is done by finding masking any consecutive 1's, zeroing them,
		 * and incrementing the following bit to 1.
 		 * ex: 10010011 + 1 ==>  10010000 ==> 10010100. */

		/*create mask for first 5 bits of binary word (binwor[0])*/
		for (bit = 8; (bit & op_node->binword[0]) && bit < 128 ; bit *= 2)	/*8 16 32 64 128*/
			mask <<= 1;	/*zero out any 1's*/

		for (i = 0; i < 3; i++)
			mask <<= 1;	/*add 3 1;s to LSB for A.R.E flags.*/

		op_node->binword[0] = mask & op_node->binword[0];	/*zero out 1's.*/

		if (bit < 128)
		{
			op_node->binword[0] = bit | op_node->binword[0];/*if possible, inc next bit.*/
			op_node->binword[0] |= 4;
			return;	/*number was incremented.*/
		}
		else if (bit == 128)
		{
			mask = 0xFF;	/*reset mask*/

			/*create mask for first 6 bits of binary word (binwor[0])*/
			for (bit = 1; (op_node->binword[1] & bit) && bit < 32 ; bit *= 2);	/*1 2 4 8 16 32*/
			mask <<= 1;	/*zero out any 1's in mask*/

			op_node->binword[1] = mask & op_node->binword[1];	/*zero out 1's in binary word.*/
			op_node->binword[0] |= 4;
		}
	}
}


/* writes operand into given obj node, in format of addressing method 1.
 * it writes the address of the label as-is in decimal form, into the struct's 'data' integer.
 * requires the operand and a node to write to.
   returns void, alters node in obj list.*/
void obj_write_method1(char *operand, object_line *node, symbol_table_line *sym_root, int *errors_flag, int *line_num)
{
	symbol_table_line *sym_ptr = sym_root;

	/*loop through sym table and compare to operand.*/
	while (sym_ptr != NULL)	/*loop through sym table*/
	{
		if (!strcmp(operand, sym_ptr->name)) /*compare to operand*/
		{
			node->data = sym_ptr->address;	/*copy data*/
			node->dataptr = &(node->data);
			return;
		}
		sym_ptr = sym_ptr->nextline;	/*go to next line*/
	}
	/*if no match found, error.*/
	fprintf(stderr, "Error: in line %d, operand is a non-existant label.\n", *line_num);
	*errors_flag = 1;
}


/* write_symbol writes a new label to the symbol list.
 * parameters are the label, symbol list root
   returns void, alters symbol list. */
void write_symbol(char *label, symbol_table_line *sym_root, object_line *obj_root, symbol_table_line *extlist, int *line_num, int *errors_flag)
{
	symbol_table_line *localnode = sym_root;
	symbol_table_line *localext = extlist;
	object_line *obj_local = obj_root;

	while (obj_local->nextline != NULL)
		obj_local = obj_local->nextline;	/*point to last node*/

	/*check label is not already defined as extern*/
	while (localext != NULL && strcmp(label, localext->name))
		localext = localext->nextline;
	if (localext != NULL) {
		*errors_flag = 1;
		fprintf(stderr,"Error: in line %d, label already defined as external. (%s)\n", *line_num, label);
		return;
	}

	if (symbol_node_clean(sym_root))
	{
		strcpy(localnode->name, label);
		localnode->address = obj_local->address+1;
		return;
	}

	while (localnode != NULL)
	{
		if (strcmp(label, localnode->name) == 0)	/*compare*/
		{
			*errors_flag = 1;
			fprintf(stderr,"Error: in line %d, label already exists. (%s)\n", *line_num, label);
			return;
		}
		localnode = localnode->nextline;
	}	/*iterate through list.*/

	/*at this point no copy was found. create new node.*/
	localnode = sym_newnode(sym_root);
	strcpy(localnode->name, label);
	localnode->address = obj_local->address+1;
}	/*end of func*/

/*checks if a symbol node is empty, returns 1 if it is.*/
int symbol_node_clean(symbol_table_line *sym_root)
{
	if(sym_root->name[0] == '\0' )
		return 1;	/*is empty*/
	else
		return 0;	/*label exists*/
}

/* create a new symbol node at the end of symbol list.
 * returns pointer to new node.
 */
symbol_table_line *sym_newnode(symbol_table_line* root)
{
	symbol_table_line *iterator = root;
	symbol_table_line *localnode = malloc(sizeof(symbol_table_line));
	if (localnode == NULL) {
		fprintf(stderr, "Error: failed to create new symbol type node.\n");
		return NULL;
	}

	while (iterator->nextline != NULL)
		iterator = iterator->nextline;
	iterator->nextline = localnode;
	localnode->nextline = NULL;
	localnode->name[0] = '\0';
	localnode->address = 0;
	return localnode;
}

/* zeroize a symbol node.
 * requires pointer to node.
 * returns pointer to node.*/
symbol_table_line *sym_zeroize(symbol_table_line* node)
{
	node->address = 0;	/*reset address*/
	node->nextline = NULL;	/*reset nextline*/
	memset(node->name, 0, sizeof(char)*32);	/*reset name*/
	return node;
}

/* writes data to extroot which will later be printed if no errors occure.
 * extroot is the file list, extlist provides label name and obj provides address
 * returns void, edits extroot. */
void write_ext(symbol_table_line *extroot, char *listname, int obj_address)
{
	symbol_table_line *local_extroot;

	if (extroot->name[0] == '\0') {	/*write to extroot if empty, to new node otherwise.*/
		strcpy(extroot->name, listname);
		extroot->address = obj_address;
	}
	else {
		local_extroot = sym_newnode(extroot);
		strcpy(local_extroot->name, listname);
		local_extroot->address = obj_address;
	}
}

/* remove_commas removes commas and reduces them as needed.
  required string, returns void (alters string)*/
void remove_commas(char *line) {
	int i = 0, j = 0, k = 0;

	while (i < MAXLINE)
	{
		if (line[i] != ',')
			continue;
		if (line[i] == ',' && line[i+1] != ',')
			line[i] = '\0';
		else if (line[i] == ',' && line[i+1] == ',')
		{
			line[i] = '\0';
			i++;
			j = i;
			k = i;
			while (j == ',')
			{
				line[j] = '\0';
				j++;
			}
			while (line[j] != '\0' && j < MAXLINE)
			{
				line[k] = line[j];
				k++;
				j++;
			}
		}
		i++;
	}
}

/* free space mallocd for obj list.
 * requires pointer to obj list.
   returns void, free obj list.*/
void free_obj(object_line *objroot)
{
	object_line *local_ptr = NULL, *next_ptr = NULL;

	/*free obj*/
	if (objroot->nextline != NULL)
		local_ptr = objroot->nextline;		/*set ptr to root of list*/
	while (local_ptr != NULL)
	{
		next_ptr = local_ptr->nextline;	/*set the ptr of the next node*/
		free(local_ptr);		/*free space*/
		local_ptr = next_ptr;	/*pull next ptr to current ptr*/
	}
}


/* free malloc'd space for symbol lists
 * requires pointers to symbol lists
   returns void, frees lists.*/
void free_symbol(symbol_table_line *symbolroot, symbol_table_line *extroot, symbol_table_line *entroot, symbol_table_line *label_list, symbol_table_line *extlist)
{
	symbol_table_line *local_ptr = NULL, *next_ptr = NULL;

	/*free symbolroot*/
	if (symbolroot->nextline != NULL)
		local_ptr = symbolroot->nextline;		/*set ptr to root of list*/
	while (local_ptr != NULL)
	{
		next_ptr = local_ptr->nextline;	/*set the ptr of the next node*/
		free(local_ptr);		/*free space*/
		local_ptr = next_ptr;	/*pull next ptr to current ptr*/
	}

	/*free extroot*/
	if (extroot->nextline != NULL)
		local_ptr = extroot->nextline;		/*description of prev loop repeats for these loops.*/
	while (local_ptr != NULL)
	{
		next_ptr = local_ptr->nextline;
		free(local_ptr);
		local_ptr = next_ptr;
	}

	/*free entroot*/
	if (entroot->nextline != NULL)
		local_ptr = entroot->nextline;
	while (local_ptr != NULL)
	{
		next_ptr = local_ptr->nextline;
		free(local_ptr);
		local_ptr = next_ptr;
	}

	/*free label_list*/
	if (label_list->nextline != NULL)
		local_ptr = label_list->nextline;
	while (local_ptr != NULL)
	{
		next_ptr = local_ptr->nextline;
		free(local_ptr);
		local_ptr = next_ptr;
	}

	/*free extlist*/
	if (extlist->nextline != NULL)
		local_ptr = extlist->nextline;
	while (local_ptr != NULL)
	{
		next_ptr = local_ptr->nextline;
		free(local_ptr);
		local_ptr = next_ptr;
	}
}