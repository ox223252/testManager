# testManager
Used to manage auto tests

This code is designed to manage auto test, you can set test function, what will be executedmanualy or in a loop, the function output can be redirected in a logFile.

## How to use:
two elements exists:
 - *menu_el*:
	```C
	typedef menu_el struct _menu_el;
	struct _menu_el
	{
		menu_el * menu;                         ///< pointer on sub menu
		test_el * test;                         ///< pointer on test
		char * comment;                         ///< comment to explain what for
	}
	```
	The *menu* field should be used to set a submenu, the *test* field should be used to set tests functions, and the comment is the text information displayed close to the menu's display. Only *menu* or *test* should be set, if they are both set only *menu* will be used.

 - *test_el*:
	```C
	struct _test_el
	{
		typeOfTest type;                        ///< type of test UNITAIRE / INTEGRATION ( no used now )
		uint8_t ( * function )( void * arg );   ///< tested function
		void * arg;                             ///< arg of the tested function
		uint8_t * result;                       ///< (R/W) pointer on result of function
		struct
		{
			uint8_t loopTest:1,                 ///< active auto test in loop ( 1 auto test / 0 manual test )
				exitOnError:1,                  ///< exit of programme if haveError == 1 ( 1 On / 0 Off )
				stopOnError:1,                  ///< stop test of this function if haveError == 1 ( 1 On / 0 Off )
				haveError:1,                    ///< (R) set to 1 if error occured in cycle of test ( 1 error occur / 0 no error occur
				quietOnLoop:1,                  ///< disable printf in tested function ( 1 quiet / 0 verbose )
				isOnError:1;					///< (R) notify if on the last auto loop the function return an error
		}bits;
		char * functionName;                    ///< function name used in description
		char * commentaire;                     ///< comment displayed in case of auto loop test failled
		uint32_t nbLoop;                        ///< (R) use to store number of test loop made
		uint32_t nbError;						///< (R) use to store number of error
	};
	```
	For the test part, only UNITARY tests are available (for now). The *result* pointer could be set to NULL (but not recommended), all *result* fields can be set with the same variable in function of code implementation.

	The flag bit field is the most important config part:
	 - *loopTest*: is used to allow a test function to be used in the auto tests, if it's set to 0 then, only manual test is valid for it,
	 - *exitOnErrot*: is used to determine a fatal error, if an error occured (funtion return != 0) then main test program stop with a *exit(0)*,
	 - *stopOnError*: is used to determine a fatal error on the function, if an error occured, this function stop its tests but the main program continue,
	 - *haveError*: is used to remember if the function failed at least once during auto test,
	 - *quietOnLoop*: is used to drop all function print output, if set to 1, the print will be put in the trash else, the print will be send to the logfile,
	 - *isOnError*: is used to store the last function return status

	 The *funtionName* is used to improve log analysis like the *commentaire* field.

	 The *nbLoop* and *nbError* fields are used to store the number of test on one function and its fail number, it will be usefull when multithreading will be implemented.

## Need to be done:
 - [ ] added *INTEGRATION* tests
 - [ ] added multithreading tests

## Example:
```C
#include <stdio.h>
#include "testManager/testManager.h"

uint8_t a ( void * arg )
{
	static uint8_t i = 0;
	printf ( "function A\n" );
	i++;
	return ( ( i%10 ) == 0 );
}

uint8_t b ( void * arg )
{
	printf ( "function b : %s\n", ( char * ) arg );
	return ( 0 );
}

uint8_t c ( void * arg )
{
	return ( 0 );
}

uint8_t d ( void * arg )
{
	static uint8_t i = 0;
	if ( i < 5 )
	{
		i++;
		return ( 0 );
	}
	return ( 1 );
}

int main ( void )
{
	uint8_t result;
	test_el A = { UNITAIRE, a, ( void * )NULL, NULL, { 1, 0, 0, 0, 1, 0 }, "funct A", "return error but conitnue", 0, 0 };
	test_el B = { UNITAIRE, b, "arg from param", ( uint8_t * )&result, { 1, 1, 1, 0, 1, 0 },  "funct B", "commentaire", 0, 0 };
	test_el C = { UNITAIRE, c, "arg from param", ( uint8_t * )&result, { 0, 0, 0, 0, 0, 0 },  "funct C", "commentaire", 0, 0 };
	test_el D = { UNITAIRE, d, "arg from param", ( uint8_t * )&result, { 1, 0, 1, 0, 1, 0 },  "funct D", "return error & stop on error", 0, 0 };
	menu_el subMenu[] = {
		{ NULL, &C, "comentaire C"  },
		{ NULL, &D, "commentaire D" },
		{ NULL, NULL, NULL }
	};
	menu_el menu[] = {
		{ NULL, &A, "comentaire A" },
		{ NULL, &B, "comentaire B" },
		{ subMenu, NULL, "menu" },
		{ NULL, NULL, NULL }
	};
	mainMenuTest ( menu, "logFile" );
	return ( 0 );
}
```

```Shell
gcc main.c testManager.c -Wall && ./a.out
#################################################################
    0 - auto test
    1 - manual test
    2 - print tests
    3   exit
#################################################################
-> 0
enter q or Q to quit
#################################################################
  function name |    test type |                  | result | comment
#################################################################
        funct A |     UNITAIRE | [    0/    1] OK |      . |
        funct B |     UNITAIRE | [    0/    1] OK |      . |
        funct D |     UNITAIRE | [    0/    1] OK |      . |
        funct A |     UNITAIRE | [    0/    2] OK |      . |
        funct B |     UNITAIRE | [    0/    2] OK |      . |
        funct D |     UNITAIRE | [    0/    2] OK |      . |
        funct A |     UNITAIRE | [    0/    3] OK |      . |
        funct B |     UNITAIRE | [    0/    3] OK |      . |
        funct D |     UNITAIRE | [    0/    3] OK |      . |
        funct A |     UNITAIRE | [    0/    4] OK |      . |
        funct B |     UNITAIRE | [    0/    4] OK |      . |
        funct D |     UNITAIRE | [    0/    4] OK |      . |
        funct A |     UNITAIRE | [    0/    5] OK |      . |
        funct B |     UNITAIRE | [    0/    5] OK |      . |
        funct D |     UNITAIRE | [    0/    5] OK |      . |
        funct A |     UNITAIRE | [    0/    6] OK |      . |
        funct B |     UNITAIRE | [    0/    6] OK |      . |
        funct D |     UNITAIRE | [    1/    6] KO |      1 |return error & stop on error
        funct A |     UNITAIRE | [    0/    7] OK |      . |
        funct B |     UNITAIRE | [    0/    7] OK |      . |
        funct A |     UNITAIRE | [    0/    8] OK |      . |
        funct B |     UNITAIRE | [    0/    8] OK |      . |
        funct A |     UNITAIRE | [    0/    9] OK |      . |
        funct B |     UNITAIRE | [    0/    9] OK |      . |
        funct A |     UNITAIRE | [    1/   10] KO |  (nil) |return error
        funct B |     UNITAIRE | [    0/   10] OK |      . |
        funct A |     UNITAIRE | [    1/   11] OK |      . |
        funct B |     UNITAIRE | [    0/   11] OK |      . |
        funct A |     UNITAIRE | [    1/   12] OK |      . |
        funct B |     UNITAIRE | [    0/   12] OK |      . |
        funct A |     UNITAIRE | [    1/   13] OK |      . |
        funct B |     UNITAIRE | [    0/   13] OK |      . |
        funct A |     UNITAIRE | [    1/   14] OK |      . |
        funct B |     UNITAIRE | [    0/   14] OK |      . |
        funct A |     UNITAIRE | [    1/   15] OK |      . |
        funct B |     UNITAIRE | [    0/   15] OK |      . |
        funct A |     UNITAIRE | [    1/   16] OK |      . |
        funct B |     UNITAIRE | [    0/   16] OK |      . |
        funct A |     UNITAIRE | [    1/   17] OK |      . |
        funct B |     UNITAIRE | [    0/   17] OK |      . |
        funct A |     UNITAIRE | [    1/   18] OK |      . |
        funct B |     UNITAIRE | [    0/   18] OK |      . |
        funct A |     UNITAIRE | [    1/   19] OK |      . |
        funct B |     UNITAIRE | [    0/   19] OK |      . |
        funct A |     UNITAIRE | [    2/   20] KO |  (nil) |return error
        funct B |     UNITAIRE | [    0/   20] OK |      . |
        funct A |     UNITAIRE | [    2/   21] OK |      . |
        funct B |     UNITAIRE | [    0/   21] OK |      . |
        funct A |     UNITAIRE | [    2/   22] OK |      . |
        funct B |     UNITAIRE | [    0/   22] OK |      . |
        funct A |     UNITAIRE | [    2/   23] OK |      . |
        funct B |     UNITAIRE | [    0/   23] OK |      . |
        funct A |     UNITAIRE | [    2/   24] OK |      . |
        funct B |     UNITAIRE | [    0/   24] OK |      . |
        funct A |     UNITAIRE | [    2/   25] OK |      . |
        funct B |     UNITAIRE | [    0/   25] OK |      . |
        funct A |     UNITAIRE | [    2/   26] OK |      . |
        funct B |     UNITAIRE | [    0/   26] OK |      . |
        funct A |     UNITAIRE | [    2/   27] OK |      . |
        funct B |     UNITAIRE | [    0/   27] OK |      . |
        funct A |     UNITAIRE | [    2/   28] OK |      . |
        funct B |     UNITAIRE | [    0/   28] OK |      . |
        funct A |     UNITAIRE | [    2/   29] OK |      . |
        funct B |     UNITAIRE | [    0/   29] OK |      . |
        funct A |     UNITAIRE | [    3/   30] KO |  (nil) |return error
        funct B |     UNITAIRE | [    0/   30] OK |      . |
        funct A |     UNITAIRE | [    3/   31] OK |      . |
        funct B |     UNITAIRE | [    0/   31] OK |      . |
        funct A |     UNITAIRE | [    3/   32] OK |      . |
        funct B |     UNITAIRE | [    0/   32] OK |      . |
        funct A |     UNITAIRE | [    3/   33] OK |      . |
        funct B |     UNITAIRE | [    0/   33] OK |      . |
        funct A |     UNITAIRE | [    3/   34] OK |      . |
        funct B |     UNITAIRE | [    0/   34] OK |      . |
        funct A |     UNITAIRE | [    3/   35] OK |      . |
        funct B |     UNITAIRE | [    0/   35] OK |      . |
        funct A |     UNITAIRE | [    3/   36] OK |      . |
        funct B |     UNITAIRE | [    0/   36] OK |      . |
        funct A |     UNITAIRE | [    3/   37] OK |      . |
        funct B |     UNITAIRE | [    0/   37] OK |      . |
        funct A |     UNITAIRE | [    3/   38] OK |      . |
        funct B |     UNITAIRE | [    0/   38] OK |      . |
        funct A |     UNITAIRE | [    3/   39] OK |      . |
        funct B |     UNITAIRE | [    0/   39] OK |      . |
        funct A |     UNITAIRE | [    4/   40] KO |  (nil) |return error
        funct B |     UNITAIRE | [    0/   40] OK |      . |
        funct A |     UNITAIRE | [    4/   41] OK |      . |
        funct B |     UNITAIRE | [    0/   41] OK |      . |
        funct A |     UNITAIRE | [    4/   42] OK |      . |
        funct B |     UNITAIRE | [    0/   42] OK |      . |
        funct A |     UNITAIRE | [    4/   43] OK |      . |
        funct B |     UNITAIRE | [    0/   43] OK |      . |
        funct A |     UNITAIRE | [    4/   44] OK |      . |
        funct B |     UNITAIRE | [    0/   44] OK |      . |
        funct A |     UNITAIRE | [    4/   45] OK |      . |
        funct B |     UNITAIRE | [    0/   45] OK |      . |
        funct A |     UNITAIRE | [    4/   46] OK |      . |
        funct B |     UNITAIRE | [    0/   46] OK |      . |
        funct A |     UNITAIRE | [    4/   47] OK |      . |
        funct B |     UNITAIRE | [    0/   47] OK |      . |
        funct A |     UNITAIRE | [    4/   48] OK |      . |
        funct B |     UNITAIRE | [    0/   48] OK |      . |
        funct A |     UNITAIRE | [    4/   49] OK |      . |
        funct B |     UNITAIRE | [    0/   49] OK |      . |
        funct A |     UNITAIRE | [    5/   50] KO |  (nil) |return error
        funct B |     UNITAIRE | [    0/   50] OK |      . |
        funct A |     UNITAIRE | [    5/   51] OK |      . |
        funct B |     UNITAIRE | [    0/   51] OK |      . |
q
#################################################################
    0 - auto test
    1 - manual test
    2 - print tests
    3   exit
#################################################################
-> 3
```

```Shell
> cat logFile
------------------------------
name: funct A
comment: return error
loop: 1
function A
------------------------------
------------------------------
name: funct B
comment: commentaire
loop: 1
function b : arg from param
------------------------------
------------------------------
name: funct D
comment: return error & stop on error
loop: 1
------------------------------
...
...
...
------------------------------
name: funct A
comment: return error
loop: 51
function A
------------------------------
------------------------------
name: funct B
comment: commentaire
loop: 51
function b : arg from param
------------------------------
>
```
