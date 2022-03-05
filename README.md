# Assembly
This is the Israel Open University System Programming Lab course project - the final project in the course, Semester 2020 Aleph.

The authors of this project are me (Vladislav Panin) and Itay Krishtal.



The goal of the project is to create an assembler for an artificial machine language.
In the documentation folder you'll find the specification of the machine language/codes, as well as the required instruction set, in the file Specification.

We have created a two-pass assembler to reach the goal.

After you have compiled the assembler you can run it via the main program. Provide the names of the source files as arguments. 

For example:

main x y test

The command above will run the assembler on the files x.as, y.as and test.as and will produce several .ob files (object files in octal base machine code) as output, as well as .ent and .ext files if they are required.
