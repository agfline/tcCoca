
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>

#include "lib/libTC.h"



static char *TC_FORMAT_STR[] = {
	"unknown",
	"23.976",
	"24",
	"25",
	"29.97NDF",
	"29.97DF",
	"30",
	"47.95",
	"48",
	"50",
	"59.94NDF",
	"59.94DF",
	"60",
    "72",
	"96",
	"100",
	"120",
    ""
};




void show_help()
{
    printf(" \n\
    tcCoca - timecode converter & calculator\n\
    \n\
    Usage :\n\
        tcCoca -F <format> <tc_value> [options]\n\
    \n\
        tc value can be either hh:mm:ss:ff timecode, frame number or any\n\
        value associated with an edit rate.\n\
    \n\n\
    Options :\n\
            --help                  show this help\n\
        -l, --list                  list all supported TC formats\n\
		\n\
        -F, --format      <format>  set the TC value format to <format>\n\
        -R, --rate        <rate>    specify the edit rate of the input TC\n\
                                    value - default is frame rate\n\
		\n\
        -c, --convert-to  <format>  convert TC value to the given <format>\n\
        -a, --add         <value>   add <value> to input TC value\n\
        -s, --sub         <value>   subtract <value> from input TC value\n\
    \n\n\
    Output :\n\
        -h, --hmsf                  output TC as a time value hh:mm:ss:ff only\n\
        -f, --frames                output TC as a frame number only\n\
        -n, --no-rollover           don't rollover if TC is bigger than day limit\n\
    \n\
    \n\
    Examples :\n\
        tcCoca -F 29.97DF 01:02:03:04 -a 02:10:01:7 \n\
        tcCoca -F 29.97DF 4147194251 -R 48000/1\n\
        tcCoca -F 29.97DF 2589407\n\
        tcCoca -F 29.97DF 01:00:00:00 -c 60\n\
    \n");
}




void show_formats()
{
	printf( "\n Supported timecode format options :\n\n" );

    for ( size_t i = 1; TC_FORMAT_STR[i][0] != '\0'; i++ )
    {
        printf( "   %s\n", TC_FORMAT_STR[i] );
    }

	printf( "\n" );
}




static int isNumber( const char *str )
{
    size_t str_sz = strlen(str);

    for ( size_t i = 0; i < str_sz; i++ )
    {
        if ( isdigit( str[i] ) == 0 )
        {
            return 0;
        }
    }

    return 1;
}




static rational_t string_to_rational( const char *str )
{
    int numerator = 0;
    int denominator = 0;

    int rc = sscanf( str, "%i/%i", &numerator, &denominator );

    if ( rc == EPERM && isNumber( str ) )
    {
        rc = sscanf( str, "%i", &numerator );
        denominator = 1;
    }
    else if ( rc == EPERM )
    {
        fprintf( stderr, "Wrong edit rate format.\n" );
    }

    rational_t num = {numerator, denominator};

    return num;
}




static enum TC_FORMAT string_to_format( const char *str )
{
    for ( size_t i = 1; TC_FORMAT_STR[i][0] != '\0'; i++ )
    {
        if ( strcmp( str, TC_FORMAT_STR[i] ) == 0 )
        {
            return i;
        }
    }

    return TC_FORMAT_UNK;
}




static struct timecode * build_timecode_from_value( const char *tc_value, enum TC_FORMAT tc_format, const char *edit_rate )
{

    if ( strlen(tc_value) == 11 &&
         isdigit(tc_value[0])   &&
         isdigit(tc_value[1])   &&
         isdigit(tc_value[3])   &&
         isdigit(tc_value[4])   &&
         isdigit(tc_value[6])   &&
         isdigit(tc_value[7])   &&
         isdigit(tc_value[9])   &&
         isdigit(tc_value[10])
       )
    {
        struct timecode *tc = calloc( 1, sizeof(struct timecode) );

        tc_set_by_string( tc, tc_value, tc_format );

        return tc;
    }
    else if ( edit_rate != NULL && isNumber( tc_value ) )
    {
        uint64_t   unitValue = strtol( tc_value, NULL, 10 );
        rational_t rate      = string_to_rational( edit_rate );

        if ( rate.denominator == 0 )
        {
            return NULL;
        }

        struct timecode *tc = calloc( 1, sizeof(struct timecode) );

        tc_set_by_unitValue( tc, unitValue, &rate, tc_format );

        return tc;
    }
    else if ( isNumber( tc_value ) )
    {
        struct timecode *tc = calloc( 1, sizeof(struct timecode) );

        uint64_t frames = strtol( tc_value, NULL, 10 );

        tc_set_by_frames( tc, frames, tc_format );

        return tc;
    }
    else
    {
        fprintf( stderr, "Wrong timecode value format.\n" );
    }


    return NULL;
}




int main( int argc, char *argv[] )
{

    char  *c_tc_format  = NULL;
    char  *c_edit_rate  = NULL;
    char  *c_convert_to = NULL;
    char  *c_add_value  = NULL;
    char  *c_sub_value  = NULL;

    int outputHMSF   = 0;
    int outputFrames = 0;
    int noRollover   = 0;



	static struct option long_options[] = {

		{ "help",	     no_argument,		 0,	 0x80  },

		{ "list",        no_argument,        0,   'l'  },
		{ "format",      required_argument,  0,   'F'  },
		{ "rate",        required_argument,  0,   'R'  },
		{ "convert-to",  required_argument,  0,   'c'  },
		{ "add",         required_argument,  0,   'a'  },
		{ "sub",         required_argument,  0,   's'  },

		{ "hmsf",        no_argument,        0,   'h'  },
		{ "frames",      no_argument,        0,   'f'  },
		{ "rollover",    no_argument,        0,   'r'  },

		{ 0,             0,                  0,    0   }
	};


	int c = 0;

	while ( 1 )
	{
		int option_index = 0;

		c = getopt_long( argc, argv, "lF:R:c:a:s:hfn", long_options, &option_index );

		if ( c == -1 )
			break;

		switch ( c )
		{
			case 'l':   show_formats();                   return 0;
			case 'F':   c_tc_format      = optarg;           break;
			case 'R':   c_edit_rate      = optarg;           break;

			case 'c':   c_convert_to     = optarg;           break;
			case 'a':   c_add_value      = optarg;           break;
			case 's':   c_sub_value      = optarg;           break;

			case 'h':   outputHMSF       = 1;                break;
			case 'f':   outputFrames     = 1;                break;
			case 'n':   noRollover       = 1;                break;

			case 0x80:	show_help();                      return 0;

			default:                                         break;
		}
	}



	if ( optind == argc )
	{
		fprintf( stderr, "Missing timecode value.\n" );
		return 1;
	}

	char *c_tc_value = argv[argc-1];



    if ( c_tc_format == NULL )
    {
        fprintf( stderr, "Missing timecode --format.\n" );
        return 1;
    }

    enum TC_FORMAT tc_format = string_to_format( c_tc_format );

    if ( tc_format == TC_FORMAT_UNK )
    {
        fprintf( stderr, "Unsupported timecode format \"%s\"\nSee tcCoca -l for a list of supported formats.\n", c_tc_format );
        return 1;
    }



	struct timecode *tc = NULL;
    struct timecode *sub_value = NULL;
    struct timecode *add_value = NULL;


	tc = build_timecode_from_value( c_tc_value, tc_format, c_edit_rate );

    if ( tc == NULL )
    {
        return 1;
    }

    tc->noRollover = noRollover;



    if ( c_convert_to != NULL )
    {
        enum TC_FORMAT convert_to = string_to_format( c_convert_to );

        if ( convert_to == TC_FORMAT_UNK )
        {
            return 1;
        }

        tc_convert( tc, convert_to );
    }
    else if ( c_add_value != NULL )
    {
        add_value = build_timecode_from_value( c_add_value, tc_format, NULL );

        if ( add_value == NULL )
        {
            return 1;
        }

        add_value->noRollover = noRollover;

        tc_add( tc, add_value );
    }
    else if ( c_sub_value != NULL )
    {
        sub_value = build_timecode_from_value( c_sub_value, tc_format, NULL );

        if ( sub_value == NULL )
        {
            return 1;
        }

        sub_value->noRollover = noRollover;

        tc_sub( tc, sub_value );
    }



    if ( outputHMSF == 1 )
    {
        printf( "%s\n", tc->string );
    }
    else if ( outputFrames )
    {
        printf( "%u\n", tc->frameNumber );
    }
    else
    {
		printf( "format   : %s\n", TC_FORMAT_STR[tc->format] );
        printf( "timecode : %s\n", tc->string );
        printf( "frames   : %u\n", tc->frameNumber );
    }



    if ( tc != NULL )
    {
        free( tc );
    }

    if ( add_value != NULL )
    {
        free( add_value );
    }

    if ( sub_value != NULL )
    {
        free( sub_value );
    }


	return 0;
}
