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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>


#include "testManager.h"

static uint8_t test ( test_el * const el );
static uint8_t menuManager ( menu_el * const el );
static uint8_t autoExec ( menu_el * const el, const char * logFile, uint32_t group );
static void testPrint ( const test_el * const el );

static uint8_t test ( test_el * const el )
{
	if ( el->result )
	{
		*el->result = el->function ( el->arg );

		if ( *el->result )
		{
			el->bits.haveError = 1;
			el->bits.isOnError = 1;
		}
		else
		{
			el->bits.isOnError = 0;
		}
	}
	else
	{
		el->bits.isOnError = ( el->function ( el->arg ) != 0 );
		el->bits.haveError |= el->bits.isOnError;
	}
	return ( 0 );
}

static uint8_t menuManager ( menu_el * const el )
{
	uint16_t i;                             // loop counter
	char buffer[ 10 ];                      // stdin scanf buffer
	uint32_t choice;                        // buffer cast to int

debut:

	printf ( "#####################################################################\n" );
	#ifdef __REQUEST_H__
		// count number of elements
		for ( i = 0; el[ i ].menu || el[ i ].test || el[ i ].comment; i++ );

		// set choice as exit request
		choice = i;
		
		// create menu
		char **tmpMenu = malloc ( sizeof ( *tmpMenu ) * i + 2 );
		if ( !tmpMenu )
		{
			return ( 1 );
		}

		// create exit label
		tmpMenu[ i ] = malloc ( 14 );
		if ( !tmpMenu[ i ] )
		{
			goto freeTmpMenu;
		}
		sprintf ( tmpMenu[ i ], "%5s   exit", " " );

		// set the last element as NULL, mandatody by temrRequest lib usage
		tmpMenu[ i + 1 ] = NULL;

		// set menu elements
		for ( i = 0; el[ i ].menu || el[ i ].test || el[ i ].comment; i++ )
		{
			tmpMenu[ i ] = malloc ( strlen( el[ i ].comment ) + 16 );

			if ( tmpMenu[ i ] )
			{
				if ( el[ i ].menu )
				{
					sprintf ( tmpMenu[ i ], "%5d - menu : %s", i, el[ i ].comment );
				}
				else if ( el[ i ].test)
				{
					sprintf ( tmpMenu[ i ], "%5d - test : %s", i, el[ i ].comment );
				}
				else
				{
					sprintf ( tmpMenu[ i ], "%5s   %s", " ", el[ i ].comment );
				}
			}
		}

		choice = menu ( 0, tmpMenu, NULL );

		for ( i = 0; el[ i ].menu != NULL || el[ i ].test != NULL || el[ i ].comment != NULL; i++ )
		{
			free ( tmpMenu[ i ] );
			tmpMenu[ i ] = NULL;
		}
freeTmpMenu:
		free ( tmpMenu );
		tmpMenu = NULL;
	#else
		for ( i = 0; el[ i ].menu != NULL || el[ i ].test != NULL || el[ i ].comment != NULL; i++ )
		{
			if ( el[ i ].comment != NULL )
			{
				if ( el[ i ].menu != NULL )
				{
					printf ( "%5d - menu : %s\n", i, el[ i ].comment );
				}
				else if ( el [ i ].test != NULL )
				{
					printf ( "%5d - test : %s\n", i, el[ i ].comment );
				}
				else
				{
					printf ( "%5s   %s\n", " ", el[ i ].comment );
				}
			}
		}
		printf ( "%5d - exit\n", i );
		printf ( "#####################################################################\n-> " );

		do
		{
			scanf ( "%s", buffer );
			choice = ( uint16_t ) atoi ( buffer );

			if ( ( choice <= i ) &&
				( buffer[ 0 ] >= '0' ) &&
				( buffer[ 0 ] <= '9' ) )
			{ // if choice is valid
				break;
			}
			else
			{ // invalid choice
				printf ( "\e[A\e[2K-> " );  // clean entry
			}
		}
		while ( 1 );
	#endif

	if  ( choice == i )
	{
		return ( 0 );
	}
	else if ( el[ choice ].menu != NULL )
	{
		menuManager ( el[ choice ].menu );
	}
	else if ( el[ choice ].test != NULL )
	{
		test ( el[ choice ].test );
	}
	goto debut;
}

static uint8_t autoExec ( menu_el * const el, const char * logFile, uint32_t group )
{
	uint16_t i;                             // loop counter

	fpos_t pos;                             // sert à sauvegarder la position dans stdout  ( si test->bits.quietOnLoop actif )
	int fd;                                 // sert à sauvegrader stdout si test->bits.quietOnLoop actif

	struct
	{
		uint8_t validLogFile : 1;
	}
	flag;

	flag.validLogFile = 0;

	i = 0;
	while ( 1 )
	{
		if ( ( el[ i ].menu == NULL ) && 
			( el[ i ].test == NULL ) && 
			( el[ i ].comment == NULL ) )
		{
			break;
		}

		if ( el[ i ].menu != NULL )
		{
			autoExec ( el[ i ].menu, logFile, group );
		}
		else if ( ( el[ i ].test != NULL ) &&
			( el[ i ].test->bits.loopTest == 1 ) &&
			( ( el[ i ].test->bits.stopOnError == 0 ) ||
			( el[ i ].test->bits.haveError == 0 ) ) )
		{

			if ( group &&
				!( el[ i ].test->type & group ) )
			{
				i++;
				continue;
			}

			// methode pour desactiver la sortie standard, redirige le stdout sur /dev/null
			// tout en gardant une cope de stdout dans fd, pour le restorer par la suite
			// https://stackoverflow.com/questions/13816994/how-to-disable-printf-function
			if ( el[ i ].test->bits.quietOnLoop == 1 )
			{
				fgetpos ( stdout, &pos );               // save pointer position in stdout stream
				fd = dup ( fileno ( stdout ) );         // duplicate stream to redirect original
				if ( logFile == NULL )
				{
					freopen ( "/dev/null", "w", stdout );   // stream to /dev/null
				}
				else if ( !freopen ( logFile, "a",stdout ) )
				{
					freopen ( "/dev/null", "w", stdout );   // stream to /dev/null
				}
				else
				{
					flag.validLogFile = 1;
				}
			}

			el[ i ].test->nbLoop++;

			if ( flag.validLogFile )
			{
				printf ( "------------------------------\n" );
				printf ( "name: %s\n", el[ i ].test->functionName );
				printf ( "comment: %s\n", el[ i ].test->commentaire );
				printf ( "loop: %d\n", el[ i ].test->nbLoop );
			}

			test ( el[ i ].test );

			if ( flag.validLogFile )
			{
				printf ( "------------------------------\n" );
			}

			if ( el[ i ].test->bits.quietOnLoop == 1 )
			{
				fflush ( stdout );                      // jerk out all is pending in stream
				dup2 ( fd, fileno ( stdout ) );         // replace stream on this original place
				close ( fd );                           // close duplicated stream
				clearerr ( stdout ) ;                   // remove error pending on stdout
				setvbuf ( stdout, NULL, _IOLBF, 256 );  // ask for replace stdout as orgonal buffering type
				fsetpos ( stdout, &pos );               // replace pointer on saved position
			}

			printf ( "%15s | ", el[ i ].test->functionName );
			printf ( "%12s | ", ( ( el[ i ].test->type & TEST_MASK ) == UNIT )?"UNIT":"INTEGRATION" );

			if ( el[ i ].test->bits.isOnError )
			{
				el[ i ].test->nbError++;

				if ( el[ i ].test->result )
				{
					printf ( "[%5u/%5u] \e[1;31mKO\e[0m | %6d |",
						el[ i ].test->nbError,
						el[ i ].test->nbLoop,
						*el[ i ].test->result );
				}
				else
				{
					printf ( "[%5u/%5u] \e[1;31mKO\e[0m | %6s |",
						el[ i ].test->nbError,
						el[ i ].test->nbLoop,
						"(nil)" );
				}

				printf ( "%s", el[ i ].test->commentaire );

				if ( el[ i ].test->bits.exitOnError == 1 )
				{
					printf ( "\n" );
					exit ( 0 );
				}
			}
			else
			{
				printf ( "[%5u/%5u] OK | %6s |",
					el[ i ].test->nbError,
					el[ i ].test->nbLoop ,
					"." );
			}
			printf ( "\n" );
		}
		i++;
	}

	return ( 0 );
}

static void testPrint ( const test_el * const el )
{
	printf ( "-----------------------------\n" );
	printf ( "fonction : %18s \n", el->functionName );
	printf ( "type de tests :    %10s \n", ( el->type == UNIT )?"UNIT":"INTEGRATION" );
	printf ( "fonction adresse : %10p \n", el->function );
	printf ( "arg adresse :      %10p \n", el->arg );
	printf ( "result adresse :   %10p \n", el->result );
	if ( el->result )
	{
		printf ( "       value :     %10d \n", ( uint8_t )*el->result );
	}
	printf ( "flags : loopTest :    %7d \n", el->bits.loopTest );
	printf ( "        exitOnError : %7d \n", el->bits.exitOnError );
	printf ( "        stopOnError : %7d \n", el->bits.stopOnError );
	printf ( "        haveError :   %7d \n", el->bits.haveError );
	printf ( "        quiet  :      %7d \n", el->bits.quietOnLoop );
	printf ( "        onError :     %7d \n", el->bits.isOnError );
	printf ( "commentaire :  %s \n", el->commentaire );
}

static void printAllTests ( const menu_el * el )
{
	uint16_t i;
	for ( i = 0; el[ i ].menu != NULL || el[ i ].test != NULL || el[ i ].comment != NULL; i++ )
	{
    	if ( el[ i ].menu != NULL )
		{
			printf ( "\e[1;31mmenu_in\e[0m:  %s\n", el[i].comment );
			printAllTests ( el[ i ].menu );
			printf ( "\e[1;31mmenu_out\e[0m: %s\n", el[i].comment );
		}
		else if ( el[ i ].test != NULL )
		{
			testPrint ( el[i].test );
		}
    }
}

static void menuAuto ( menu_el * const el, const char * logFile )
{
	char buffer[ 10 ];
	uint32_t choice = 0xffff;
	struct timeval timeout;
	fd_set fdSet;
	int exitValue = 0;

	while ( testLevelTitle[ exitValue ] )
	{
		exitValue++;
	}

	printf ( "#####################################################################\n" );
	#ifdef __REQUEST_H__
		choice = menu ( 0, testLevelTitle, NULL );
	#else
		do
		{ // what type of tests
			for ( int i = 0; i < exitValue; i++ )
			{
				printf ( "    %d - %s\n", i, testLevelTitle[ i ] );
			}
			printf ( "#####################################################################\n-> " );

			do
			{
				scanf ( "%10s", buffer );
				choice = ( uint16_t ) atoi ( buffer );

				// need to explain each case
				if ( choice < exitValue &&
					( buffer[ 0 ] >= '0' ) &&
					( buffer[ 0 ] <= '9' ) )
				{
					break;
				}
				else
				{
					printf ( "\e[A\e[2K-> " );
				}
			}
			while ( 1 );

			if ( choice != 0xffff )
			{
				break;
			}
		}
		while ( 1 );
	#endif

	if ( choice == 5 )
	{
		return;
	}
	else if ( choice )
	{
		choice = 1 << ( choice + 3 );
	}

	printf ( "enter \e[1;32mq\e[0m or \e[1;32mQ\e[0m to quit\n" );
	printf ( "#####################################################################\n" );
	printf ( "%15s | %12s | %16s | %6s | %s\n", "function name", "test type", " ", "result", "comment" );
	printf ( "#####################################################################\n" );

	do
	{
		autoExec ( el, logFile, choice );

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		FD_ZERO ( &fdSet );
		FD_SET ( fileno ( stdin ), &fdSet );

		select ( fileno ( stdin ) + 1, &fdSet, NULL, NULL, &timeout );

		if ( FD_ISSET ( fileno ( stdin ), &fdSet ) )
		{
			scanf ( "%10s", buffer );
			
			if ( ( *buffer == 'q' ) ||
				( *buffer == 'Q' ) )
			{
				break;
			}
		}
	}
	while ( 1 );
}

uint8_t mainMenuTest ( menu_el * const el, const char * logFile )
{
	char buffer[ 10 ];
	uint16_t choice = 0;
	do
	{
		printf ( "#####################################################################\n" );
		#ifdef __REQUEST_H__
			choice = menu ( 0, (char *[]){ "auto test", "manual test", "print tests", "exit", NULL }, NULL );
		#else
			printf ( "    0 - auto test\n" );
			printf ( "    1 - manual test\n" );
			printf ( "    2 - print tests\n" );
			printf ( "    3   exit\n" );
			printf ( "#####################################################################\n-> " );

			do
			{
				scanf ( "%10s", buffer );
				choice = ( uint16_t ) atoi ( buffer );

				// need to explain each case
				if ( ( buffer[ 0 ] >= '0' ) &&
					( buffer[ 0 ] <= '3' ) )
				{
					break;
				}
				else
				{
					printf ( "\e[A\e[2K-> " );
				}
			}
			while ( 1 );
		#endif

		switch ( choice )
		{
			case 0:
			{
				menuAuto ( el, logFile );
				break;
			}
			case 1:
			{
				menuManager ( el );
				break;
			}
			case 2:
			{
				printAllTests ( el );
				break;
			}
			default:
			{
				return ( 0 );
				break;
			}
		}
	}
	while ( 1 );

	return ( 0 );
}


////////////////////////////////////////////////////////////////////////////////
// test part

static int max ( int a, int b )
{
	return ( (a>b)?a:b );
}

#define MAX(a,b) ((a>b)?a:b)

static uint8_t testFnc ( void * arg )
{
	int a = 5;
	int b = 6;
	int ret = max ( a++, --b );

	if ( ( ret != 5 ) ||
		( a != 6 ) ||
		( b != 5 ) )
	{
		return ( 1 );
	}

    return ( 0 );
}

static uint8_t testDef ( void * arg )
{
	int a = 5;
	int b = 6;
	int ret = MAX ( a++, --b );

	if ( ( ret != 5 ) ||
		( a != 6 ) ||
		( b != 5 ) )
	{
		return ( 1 );
	}

    return ( 0 );
}

int __attribute__((weak)) main ( void )
{
	uint8_t result;
	test_el tFnc = {
		.type = UNIT,
		.function = testFnc,
		.arg = NULL,
		.result = NULL,
		.bits = {
			.loopTest = 1,
			.exitOnError = 0,
			.stopOnError = 0,
			.quietOnLoop = 0
		},
		.functionName = "max",
		.commentaire = ""
	};
	test_el tDef = {
		.type = UNIT,
		.function = testDef,
		.arg = NULL,
		.result = NULL,
		.bits = {
			.loopTest = 1,
			.exitOnError = 0,
			.stopOnError = 0,
			.quietOnLoop = 0
		},
		.functionName = "MAX",
		.commentaire = ""
	};
	menu_el testMax[] = {
		{ NULL, &tFnc, "test du max by function"  },
		{ NULL, &tDef, "test du max by define" },
		{ NULL, NULL, NULL }
	};
	mainMenuTest ( testMax, NULL );
	return ( 0 );
}