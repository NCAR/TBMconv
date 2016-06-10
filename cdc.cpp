
/**
 * Author: Nicholas DeCicco <nsd.cicco@gmail.com>
 */

#include <stdlib.h>
#include "cdc.hpp"

void cdc_decode(char *const str, const size_t len)
{
	size_t i;
	char ascii[65 /* add one for the null terminator */] =
		" " /* "display code 0 has no associated graphic in the 63-character set" */
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"0123456789"
		"+-*/()$= ,.#[]%\"_!&'?<>@\\^;";

	for (i = 0; i < len; i++) {
		str[i] = ascii[(int) str[i]];
	}
}
