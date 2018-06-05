#ifndef __TESTMANAGER_H__
#define __TESTMANAGER_H__

////////////////////////////////////////////////////////////////////////////////
/// \copiright ox223252, 2017
///
/// This program is free software: you can redistribute it and/or modify it
///     under the terms of the GNU General Public License published by the Free
///     Software Foundation, either version 2 of the License, or (at your
///     option) any later version.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
///     ANY WARRANTY; without even the implied of MERCHANTABILITY or FITNESS FOR
///     A PARTICULAR PURPOSE. See the GNU General Public License for more
///     details.
///
/// You should have received a copy of the GNU General Public License along with
///     this program. If not, see <http://www.gnu.org/licenses/>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \file testManager.h
/// \brief library to automate UNITARY tests
/// \author ox223252
/// \date 2017-07
/// \copyright GPLv2
/// \version 0.1
/// \warning NONE
/// \bug NONE
////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdlib.h>

/// \note in this structs all parameters are set by
/// user and read by system unless if param is precedes
/// by (R) or (R/W)

typedef enum
{
	UNITAIRE,                               ///< use to define a unit test
	INTEGRATION                             ///< use to define a functional test
} typeOfTest;

typedef struct _menu_el menu_el;
typedef struct _test_el test_el;

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

struct _menu_el
{
	menu_el * menu;                         ///< pointer on sub menu
	test_el * test;                         ///< pointer on test
	char * comment;                         ///< comment to explain what for
};

////////////////////////////////////////////////////////////////////////////////
/// \fn uint8_t mainMenuTest ( menu_el * const el, const char * logFile );
/// \param[ in ] menu_el * const el : menu that would be displaied
/// \param[ in ] const char * logFile : file where log would be saved
/// \brief will display the main menu where you could coice to launch manul or
///     automatic tests, if log File set will save some log inside
/// \details will present two choices auto tests or manual tests, logFile is
///     used in auto test mode if test function is set as quiet, for more
///     details refer to the relative section
/// \retrun 0 : OK
////////////////////////////////////////////////////////////////////////////////
uint8_t mainMenuTest ( menu_el * const el, const char * logFile );



////////////////////////////////////////////////////////////////////////////////
/// \example
////////////////////////////////////////////////////////////////////////////////
/// #include <stdio.h>
/// #include "testManager/testManager.h"
///
///
/// uint8_t a ( void * arg )
/// {
/// 	static uint8_t i = 0;
/// 	printf ( "function A\n" );
/// 	i++;
/// 	return ( ( i%10 ) == 0 );
/// }
///
/// uint8_t b ( void * arg )
/// {
/// 	printf ( "function b : %s\n", ( char * ) arg );
/// 	return ( 0 );
/// }
///
/// uint8_t c ( void * arg )
/// {
/// 	return ( 0 );
/// }
///
/// uint8_t d ( void * arg )
/// {
/// 	static uint8_t i = 0;
/// 	if ( i < 5 )
/// 	{
/// 		i++;
/// 		return ( 0 );
/// 	}
/// 	return ( 1 );
/// }
///
/// int main ( void )
/// {
/// 	uint8_t result;
/// 	test_el A = { UNITAIRE, a, ( void * )NULL, ( uint8_t * )&result, { 1, 0, 0, 0, 1, 0 }, "funct A", "return error", 0, 0 };
/// 	test_el B = { UNITAIRE, b, "arg from param", ( uint8_t * )&result, { 1, 1, 1, 0, 1, 0 },  "funct B", "commentaire", 0, 0 };
/// 	test_el C = { UNITAIRE, c, "arg from param", ( uint8_t * )&result, { 0, 0, 0, 0, 0, 0 },  "funct C", "commentaire", 0, 0 };
/// 	test_el D = { UNITAIRE, d, "arg from param", ( uint8_t * )&result, { 1, 0, 1, 0, 1, 0 },  "funct D", "return error & stop on error", 0, 0 };
///
/// 	menu_el subMenu[] = {
/// 		{ NULL, &C, "comentaire C"  },
/// 		{ NULL, &D, "commentaire D" },
/// 		{ NULL, NULL, NULL }
/// 	};
/// 	menu_el menu[] = {
/// 		{ NULL, &A, "comentaire A" },
/// 		{ NULL, &B, "comentaire B" },
/// 		{ subMenu, NULL, "menu" },
/// 		{ NULL, NULL, NULL }
/// 	};
/// 	mainMenuTest ( menu, "logFile" );
/// 	return ( 0 );
/// }
////////////////////////////////////////////////////////////////////////////////

#endif

