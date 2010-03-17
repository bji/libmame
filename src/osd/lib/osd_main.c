/** **************************************************************************
 * osd_main.c
 *
 * Copyright Bryan Ischo and the MAME Team.
 * Visit http://mamedev.org for licensing and usage restrictions.
 *
 ************************************************************************** **/

#include "clifront.h"

int main(int argc, char **argv)
{
	return cli_execute(argc, argv, 0);
}
