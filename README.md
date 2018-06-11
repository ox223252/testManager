
# testManager
Used to manage auto tests

This code is designed to manage auto test, you can set test function, what will be executedmanualy or in a loop, the function output can be redirected in a logFile.

## How to use:
### *menu_el*:
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

### *test_el*:
```C
struct _test_el
{
	typeOfTest type;                        ///< type of test UNITAIRE / INTEGRATION ( no used now ) logical or with testGroup
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
#### *typeOfTest type*:
Only UNITARY tests are available (for now). 

   The UNITARY test could be used with group selection: ``GROUP_1 | UNITAIRE``
    availabe groups are : 
     - NO_GROUP = 0x00: called only by  menu choice: *all*.
     - GROUP_1: called by menu choices: *all* and *group 1*
     - GROUP_2: called by menu choices: *all* and *group 2*
     - GROUP_3: called by menu choices: *all* and *group 3*
     - GROUP_4: called by menu choices: *all* and *group 4*
     - ALL_GROUPS: alled by menu choices: *all*, *group 1*, *group 2*, *group 3* and *group 4*.
     
The groups can be mixed like : ``GROUP_1 | GROUP_3 | UNITAIRE``

> *ALL_GROUPS* = *GROUP_1* | *GROUP_2* | *GROUP_3* | *GROUP_4* = *ALL_GROUPS* | *NO_GROUP*.

#### *function*: 
This is a pointer to the function what should be executed.

#### *arg*:
This is a pointer to the function parameters.

#### *result*:
The pointer could be set to NULL (but not recommended), all *result* fields can be set with the same variable in function of code implementation.

#### *flag*:
This bit field is the most important config part:
 - *loopTest*: is used to allow a test function to be used in the auto tests, if it's set to 0 then, only manual test is valid for it,
 - *exitOnErrot*: is used to determine a fatal error, if an error occured (funtion return != 0) then main test program stop with a *exit(0)*,
 - *stopOnError*: is used to determine a fatal error on the function, if an error occured, this function stop its tests but the main program continue,
 - *haveError*: is used to remember if the function failed at least once during auto test,
 - *quietOnLoop*: is used to drop all function print output, if set to 1, the print will be put in the trash else, the print will be send to the logfile,
 - *isOnError*: is used to store the last function return status

#### *funtionName* / *commentaire*:
These fields are used to improve log analysis.

#### *nbLoop* / *nbError*:

 These fields are used to store the number of test on one function and its fail number, it will be usefull when multithreading will be implemented.

## Need to be done:
 - [x] added group test
 - [ ] added *INTEGRATION* tests
 - [ ] added multithreading tests

## Example:
```C
#include <stdio.h>
#include "lib/testManager/testManager.h"

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
    test_el A = { GROUP_1 | UNITAIRE, a, ( void * )NULL, NULL, { 1, 0, 0, 0, 1, 0 }, "funct A", "return error but conitnue", 0, 0 };
    test_el B = { GROUP_1 | GROUP_2 | UNITAIRE, b, "arg from param", ( uint8_t * )&result, { 1, 1, 1, 0, 1, 0 },  "funct B", "commentaire", 0, 0 };
    test_el C = { UNITAIRE, c, "arg from param", ( uint8_t * )&result, { 0, 0, 0, 0, 0, 0 },  "funct C", "commentaire", 0, 0 };
    test_el D = { ALL_GROUPS | UNITAIRE, d, "arg from param", ( uint8_t * )&result, { 1, 0, 1, 0, 1, 0 },  "funct D", "return error & stop on error", 0, 0 };
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
> gcc main.c testManager.c -Wall && ./a.out
#################################################################
    0 - auto test
    1 - manual test
    2 - print tests
    3   exit
#################################################################
-> 0
#################################################################
    0 - all
    1 - group 1
    2 - group 2
    3   group 3
    4   group 4
    5   exit
#################################################################
-> 1
enter q or Q to quit
#################################################################
  function name |    test type |              | result | comment
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
loop: 4
function A
------------------------------
------------------------------
name: funct B
comment: commentaire
loop: 4
function b : arg from param
------------------------------
------------------------------
name: funct D
comment: return error & stop on error
loop: 4
------------------------------
>
```
