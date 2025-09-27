/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                                                                                  *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Header

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Purpose:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*                                        Module Interface                                          *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

// Source

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
Operation:
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Conventions for this project:
* Casing:
*	Defines & Constexprs: XXX_YYY
*	classes/structs: xxx_yyy_z - where z is s, t, c, etc.
*	enums: 
*		For enums used with ints (like indices): 
*		Do a namespace/enum pair. Do a typedef like so: xxx_yyy_e if instances of the enum are needed
*		For enums not used with ints:
*		Do an enum class like so XXX_YYY
* 
* Typing:
*	Un-inherited classes, structs, and classes with few member functions should be typedef structs
*	Inherited classes should actually be classes
*	Structs should only have a typedef name unless required
*/