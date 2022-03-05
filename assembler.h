/* MMN 14, assembler (project)
 *
 * Name:    Itay Krishtal   |   Vladislav Panin
 * ID:      REDACTED       |   REDACTED
 *
 * MMN14 is an assembler program designed to translate an input file written in a pre-defined assembling langauge
 * into an 'assembled' code ready to be transferred to the next stages of assembly. (linking, etc).
 * this h file will hold all declarations for assist functions for the assembler to function.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef ASSEMBLER_ASSEMBLER_H
#define ASSEMBLER_ASSEMBLER_H

#endif

#define MAX_NAME_LENGTH sizeof(int)*8
#define MAXLINE 100

/* 0,1,2,3 | 1,2,3 */
#define ARG_GROUP_0 700 

/* 0,1,2,3 | 0,1,2,3 */
#define ARG_GROUP_1 701 

/* 1 | 1.2,3 */
#define ARG_GROUP_2 702

/* 1,2,3 | NaN */
#define ARG_GROUP_3 703

/* 1,2 | NaN */
#define ARG_GROUP_4 704

/* 0,1,2,3 | NaN */
#define ARG_GROUP_5 705

/* NaN | NaN */
#define ARG_GROUP_6 706

typedef struct object_line{	/*a node in a list of lines for obj file.*/
    unsigned char binword[2];    	/*use a an array of two chars, (16 bit) and use bitwise operations on them.*/
    char string_char;  		/*use in .string opcodes. */
    int data;
    int *dataptr;           	/*use in .data opcodes.*/
	int address;        	/*starts at 0100, and goes up...*/
	struct object_line *nextline;  /*pointer to next obj line*/
} object_line;

typedef struct symbol_table_line{		/*a node in a list of lines for symbol_tables (ent and ext)*/
	char name[32];
	int address;
	struct symbol_table_line* nextline;
} symbol_table_line;

/* first_iteration iterates through the input .as file to create the symbol table and write opcodes.
 * returns void.
 */
void first_iteration(FILE* fp, object_line *obj_root, symbol_table_line *symbol_table, symbol_table_line *extlist,symbol_table_line *entlist,int *line_num, int *errors_flag, int *dataCounter, int *instruCounter, symbol_table_line *label_list);

/*
 * second_iteration  iterates through the input .as file to add parameters and memory addresses according to symbol table.
 * returns void.
 */
void second_iteration(symbol_table_line *entroot, symbol_table_line *extroot, symbol_table_line *extlist, symbol_table_line *entlist, symbol_table_line *label_list, symbol_table_line *symbolroot, object_line *obj_root, int *errors_flag);

/*creates a new object_line node at the end of object_line list
 * requires pointer to root of obj list.
 * returns a pointer to the new (last) object_line node*/
object_line *obj_newnode(object_line *obj_root);

/* create a new symbol node at the end of symbol list.
 * returns pointer to new node. */
symbol_table_line *sym_newnode(symbol_table_line* root);

/* creates a new node in label_list, and writes to it the name of the label and address where it can be found.
 * requires ptr to obj root to find lateset address, ptr to label_list to add new node and name of label.
   returns ptr to new node.*/
symbol_table_line *label_list_newnode(symbol_table_line *label_list,object_line *root, char *operand);

/* reset the object type node pointed to.
 * requires a pointer to node.
 * returns a pointer to the object_line node.*/
object_line *obj_zeroize_node(object_line *obj_node);

/* zeroize a symbol node.
 * requires pointer to node.
 * returns pointer to node.*/
symbol_table_line *sym_zeroize(symbol_table_line* node);

/*checks if a symbol node is empty, returns 1 if it is.*/
int symbol_node_clean(symbol_table_line *sym_root);

/* binword_to_int turns the binword[2] from struct object_line (2 chars, bit-specific representation) to a decimal int.
 * returns an int.
 * takes a ptr to phrase parameter.*/
int binword_to_int(object_line *node);

/*	print_ext will create the external file with an ext extension.
 * 	returns void(creates new file if successful)
 	takes as arguments a root of externals list, and file name to be created.*/
void print_ext(symbol_table_line *extroot, char* filename);

/* print_obj creates the object (.obj) file. requires a pointer to start of list of assembler lines.
 * (mem address and binary representation)
 * returns void.*/
void print_obj(object_line* root, int instructions, int data, char* filename);

/*	print_ent will create the entries file with an ent extension.
 * 	returns void(creates new file if successful)
 	takes as arguments a root of entries list, and file name to be created.*/
void print_ent(symbol_table_line *entroot, char* filename);

/*	reduces unnecessary spaces in input line.
 *	takes a line parameter.
 returns void (alters given parameter)*/
void reduce_spaces(char* line, int *line_num);

/* parses data types (checks if .data or .string, adds to obj_root list accordingly)
 * parameters are a pointer to an array of tokens in a line, number of line being parsed, obj_root and errors flag.
 * returns void (modifies lists). */
void parse_data(char **tokens, int *line_num, object_line *obj_root, int *errors_flag, int word_count, int *dataCounter, int *instruCounter, symbol_table_line *extlist, symbol_table_line *entlist, symbol_table_line *label_list);

/* func will check if opcode is valid, and add it as a node to obj list if it is.
 * otherwise, will raise error flag.
 * returns -1 if not an opcode, or the appropriate ARG_GROUP (int) otherwise..*/
int parse_opcode(char *opcode, object_line *obj_root, int *errors_flag, int *instruCounter);

/* parses entry types (checks if .entry, adds to ent list accordingly)
 * has to be called after symbol (label) list is complete. (i.e after first iteration)
 * parameters are a pointer to an array of tokens in a phrase, number of line being parsed, obj_root.
 * tokens[1] will always be the operand which is a label.
 * returns void (modifies lists). */
void parse_ent(char **tokens, symbol_table_line *sym_root, symbol_table_line *entroot, int *line_num, int *errors_flag);

/* parses extern types.
 * parameters are a pointer to an array of tokens in a phrase, number of line being parsed, obj_root.
 * returns void (modifies lists). */
int parse_ext(char *operand, object_line *node, symbol_table_line *extroot, symbol_table_line *extlist, int *errors_flag, int address);

/* writes new operands to obj list.
 * requires root of obj list, operand string, argument group, line num, err flag, and which operand it is (src or dest)
 *returns void (alters list).
 */
void write_operand(object_line *root, symbol_table_line *label_list, char *operand, int *line_num, int *errors_flag, int op_num, int curr_arg_group, int *instruCounter);

/* write_symbol writes a new label to the symbol list.
 * parameters are the label, symbol list root
   returns void, alters symbol list. */
void write_symbol(char *label, symbol_table_line *sym_root, object_line *obj_root, symbol_table_line *extlist, int *line_num, int *errors_flag);

/*writes the operand method to the last node in the obj list.
 * requires root of obj list, the operand (string), line num, pointer to error flag, and number indicating which operand it is.
 *returns void. */
void write_operand_method(object_line *root, char *operand, int *line_num, int *errors_flag, int op, int curr_arg_group);

/* writes operand into given obj node, in format of addressing method 0.
 * requires the operand and a node to write to.
   returns void, alters node in obj list.*/
void obj_write_method0(char *operand, object_line *op_node, int *line_num, int *errors_flag);

/* obj_write_reg writes a register's value into the appropriate binword.
 * requires a pointer to node, operand string (register), and number of operand, i.e is it the destination or source.
 * operand is a verified register, i.e not r3241, or r2.3.
 * returns void, alters node. */
void obj_write_reg(object_line *op_node, char *operand, int operand_num, int *line_num, int *errors_flag, int curr_arg_group);

/*checks if a given symbol (label) matches a node in entlist.
 * if there is a match, add it to entry root list (entroot).
 * requires a label node, entry list root node and entry root node.
 * returns void, alters list entroot. */
void check_entry(symbol_table_line *sym_root, symbol_table_line *entlist, symbol_table_line *entroot, symbol_table_line *extlist, int *errors_flag);

/*checks if dot is valid, and of what type.
 * params are the word being processed, the tokens arr, line number value, and a pointer to root of list of obj lines.
 * returns 0 for error (prints to stderr) or for .data/.string types which are handled by parse_data,
 * returns 1 for .ext or .ent types */
int check_dot(char *oneword, char **tokens, int *line_num, object_line *obj_root, int *errors_flag);

/*checks if obj line was already written to or is clean.
 * requires pointer to obj line.
 * returns 1 if clean, 0 otherwise. */
int check_obj_clean(object_line *obj_root);

/* check_label_validity takes a suspected label string.
 * It returns 0 if the label does not comply with the label policy, and 1 if it does.
 * The policy is described on pages 25-26 of the course's taskbook.
 *
 * Note: The string MUST be properly terminated! */
int check_label_validity(char* label, int *errors_flag);

/* writes data to extroot which will later be printed if no errors occure.
 * extroot is the file list, extlist provides label name and obj provides address
 * returns void, edits extroot. */
void write_ext(symbol_table_line *extroot, char *listname, int obj_address);

/* free malloc'd space for symbol lists
 * requires pointers to symbol lists
   returns void, frees lists.*/
void free_symbol(symbol_table_line *symbolroot, symbol_table_line *extroot, symbol_table_line *entroot, symbol_table_line *label_list, symbol_table_line *extlist);

/* free space mallocd for obj list.
 * requires pointer to obj list.
   returns void, free obj list.*/
void free_obj(object_line *objroot);

/* remove_commas removes commas and reduces them as needed.
  required string, returns void (alters string)*/
void remove_commas(char *line);

/* check_arg_group returns argument group of the opcode.
 * takes opcode as a pointer to a string.
 * returns integer, that represents the argument group. */
int check_arg_group(char* opcode);

/* check_arg_validity returns 1, if the arg string corresponds to the given method (arg is a representation of opcode's argument)).
 * takes opcode's alleged argument and method group.
 * returns 1, if they correspond, and 0 if not.*/
int check_arg_validity(char* arg, int method);

/* get_operand_method returns the operand method of a given arg. If the operand is not a valid operand, handles the error.
 * takes alleged operand, errors flag pointer and line number pointer.
 * returns operand's method, if they correspond, and handles the error (and returns 0) if not. */
int get_operand_method(char* arg, int *errors_flag, int *line_num);

/* check_arg_validity returns 1, if the operand string corresponds to the given arg_group.
 * takes operand, it's place as an argument (1 or 2, opernd_pos), the suspected argument group, and errors flag and line number (for handling the error).
 * returns 1, if valid, and 0 if not.*/
int check_operand_validity(char* operand, int arg_group, int operand_pos, int *line_num, int *errors_flag);


