/* MMN 14, assembler (project)
 *
 * Name:    Itay Krishtal   |   Vladislav Panin
 * ID:      REDACTED       |   REDACTED
 *
 * MMN14 is an assembler program designed to translate an input file written in a pre-defined assembling langauge
 * into an 'assembled' code ready to be transferred to the next stages of assembly. (linking, etc).
 *  This c-file (secondrun.c) will run on the second pass over the input file.
 *
 *  purpose of secondrun is to replace all labels in file with the appropriate defined or external labels.
 *  label_list is a list of addresses of all operands which are possible labels.
 */

#include "assembler.h"

void second_iteration(symbol_table_line *entroot, symbol_table_line *extroot, symbol_table_line *extlist, symbol_table_line *entlist, symbol_table_line *label_list, symbol_table_line *symbolroot, object_line *obj_root, int *errors_flag)
{
	int mask = 0x7FFF; /*15 bit mask*/
	char local_name[32] = { '\0' };
	symbol_table_line *label_local = label_list;/*ptr to label_list list*/
	symbol_table_line *extlist_local = extlist;	/*ptr to node in extlist list*/
	symbol_table_line *sym_local = symbolroot;	/*ptr to node is symbol table which is a label list.*/
	object_line *obj_local = obj_root;	/*iterate through and write to obj nodes*/

	if (label_local->name[0] == '\0')	/*check if there are no labels.*/
		label_local = NULL;	/*skip the next while loop.*/

		/* How this works:
		 * each node of label represents a LABEL: in source file.
		 * cmp each label operand (in label_list) to the available labels,
		 * and write to the obj node pointed to by label list if label exists.
		 */
	while (label_local != NULL)
	{
		memset(local_name, '\0', sizeof(char) * 32);	/*reset local_namee*/
		extlist_local 	= extlist;	/*reset ptrs*/
		sym_local 		= symbolroot;
		obj_local 		= obj_root;

		while (obj_local != NULL && (obj_local->address != label_local->address))	/*if obj address doesnt match label list's address, try next node.*/
			obj_local = obj_local->nextline;	/*point to obj node address of first node of label.*/

		if (obj_local != NULL && (obj_local->address == label_local->address))	/*found the label node. now check if it's external or defined.*/
		{
			while (extlist_local != NULL && (strcmp(extlist_local->name, label_local->name) != 0))
				extlist_local = extlist_local->nextline;	/*check if external*/
			/*will point to NULL if no match*/
			if (extlist_local != NULL) {
				/*external labels are just nodes of ...0000001 which is the External flag on.*/
				obj_local->data = 1; /*...00000001*/
				obj_local->dataptr = &(obj_local->data);

				/*extroot is the file list, extlist provides label name and obj provides address*/
				write_ext(extroot, extlist_local->name, obj_local->address);
			}
			else {	/*it's not an external, check if defined*/
				strcpy(local_name, label_local->name);
				strcat(local_name, ":");	/*sym_local includes ':' sign but label doesn't.*/
				while (sym_local != NULL && (strcmp(sym_local->name, local_name) != 0))
					sym_local = sym_local->nextline;

				if (sym_local != NULL) {
					obj_local->data = sym_local->address;	/*write address of label into node*/
					obj_local->data <<= 3;/*room for A.R.E flags*/
					obj_local->data |= 2; /*0100 A-flag*/
					obj_local->data = mask & obj_local->data; /*limit to 15 bits.*/
					obj_local->dataptr = &(obj_local->data);
				}
				else {	/*label is not external or defined in file. that's an error.*/
					fprintf(stderr, "Error: \"%s\" is a call to neither internal or external label!\n", label_local->name);
					*errors_flag = 1;
				}
			}/*end of check of internal label*/
		}	/*end of label check*/
		label_local = label_local->nextline;
	}	/*end of label list iterator*/

	/*compile entry list (entroot) for printing.*/
	check_entry(symbolroot, entlist, entroot, extlist, errors_flag);
}	/*end of function*/

























