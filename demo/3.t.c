/* Custom test functions and macros.

To create custom assertion macro use ASSERT macro as a base.

	ASSERT(BOOL, MSG)

BOOL can be anything results in true/false value, that is 0 value
as false and non 0 value as true.  MSG is a string printed when
BOOL value is 0.
*/

#include <string.h>
#include "../walter.h"

/* Example 1
 * ===================================================================
 * 
 * Simple assertions that can be written inline.  To stay true to
 * library convention you should define second argument in ASSERT to
 * be string representation of this assertion macro.
 */
#define CLAMP(min,x,max) \
	ASSERT(((min) <= (x) && (x) <= (max)), "CLAMP("#min", "#x", "#max")")

TEST("Custom CLAMP macro")
{
	CLAMP(1, 2, 10);
	CLAMP(0.0, 0.003, 1.0);
	CLAMP(-100, -3, 0);
	CLAMP(-1, 2, 1);	/* Fail */
}

/* Example 2
 * ===================================================================
 * 
 * Pass assertion when in array of ints ARR of size SIZ int X can be
 * found.  In this example external function is used that return value
 * of true (non 0) or false (0).
 */
#define HAS_INT(arr,siz,x) \
	ASSERT(__has_int(arr,siz,x), "HAS_INT("#arr", "#siz", "#x")")

/* Return non 0 value when ARR of SIZ size contains vlue of X. */
int __has_int(int *arr, size_t siz, int x);

int
__has_int(int *arr, size_t siz, int x)
{
	size_t i;
	for (i = 0; i < siz; i++) {
		if (arr[i] == x) {
			return 1;
		}
	}
	return 0;
}

TEST("Custom HAS_INT macro")
{
	int arr[5] = {0, 1, 2, 3, 4};

	HAS_INT(arr, 5, 2);
	HAS_INT(arr, 5, 3);
	HAS_INT(arr, 5, 4);
	HAS_INT(arr, 5, 5);	/* Fail */
}

/* Example 3
 * ===================================================================
 * 
 * You might want to print more details when assertion fails.  This
 * can be done inside test function, the first ASSERT macro argument.
 */
#define STARTS_WITH(str,x) \
	ASSERT(__starts_with(str, x), "STARTS_WITH("#str", "#x")")

/* Return non 0 value when STR string starts with X string. */
int __starts_with(char *str, char *x);

int
__starts_with(char *str, char *x)
{
	if (strncmp(str, x, strlen(x)) == 0) {
		return 1;
	}
	/* Print strings so the difference is clearly visible.
	 * Using tab as prefix helps separate from next error.*/
	printf("\t'%s'\n"
	       "\t'%s'\n",
	       str, x);
	return 0;
}

TEST("Custom STARTS_WITH macro")
{
	char *str = "Lorem ipsum";

	STARTS_WITH(str, "Lor");
	STARTS_WITH(str, "Lorem ip");
	STARTS_WITH("abcdef", "abc");
	STARTS_WITH(str, "abc"); /* Fail */
}
