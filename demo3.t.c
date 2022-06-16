/* Custom test functions and macros. */

/* To create custom assertion macro use ASSERT or __ASSERT macro.
 *
 *	ASSERT(boolean, message_string)
 *	__ASSERT(boolean, code_executed_on_error)
 */

#include <string.h>
#include "walter.h"

/*
 * Use ASSERT for simple assertions that can be written inline and
 * prints single string on error.
 */
#define CLAMP(min,x,max) \
	ASSERT((min <= x && x <= max), "CLAMP("#min", "#x", "#max")")

TEST("Custom CLAMP macro")
{
	CLAMP(1, 2, 10);
	CLAMP(0.0, 0.003, 1.0);
	CLAMP(-100, -3, 0);
	CLAMP(-1, 2, 1);	/* Fail */
}

/*
 * Pass assertion when in given ints array ARR of size SIZ number
 * X can be found.  In this example external function is used that
 * return value of true (1) or false (0).
 */
#define HAS_INT(arr,siz,x) \
	ASSERT(__has_int(arr,siz,x), "HAS_INT("#arr", "#siz", "#x")")

int __has_int(int *arr, size_t siz, int x);

int
__has_int(int *arr, size_t siz, int x)
{
	size_t i;

	for (i = 0; i < siz; i++) {
		if (arr[i] == x)
			return 1;
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

/*
 * Use __ASSERT if you want to produce complex error message.  In
 * theory you can execute any code as second __ASSERT argument.  It
 * will be executed after error message file path with line number is
 * printed and before terminated new line character.  It was designed
 * to execute fprintf(stderr, ...) tho.
 *
 * In this example custom functions are created.  First for testing
 * and second for printing error message.  It's not required and you
 * can type everything directly in macro but might be helpful when
 * logic gets bigger.
 *
 * This macro tests if given STR string starts with X string.
 *
 * Note that arguments of STR and X are passed with # prefix so those
 * will be exact strings of arguments passed to macro.  With that you
 * can print name of function or variable passed to macro instead of
 * string value.
 */
#define STARTS_WITH(str,x) \
	__ASSERT(__starts_with(str, x), \
		 __starts_with_print(#str, #x, str, x))

int __starts_with(char *str, char *x);
void __starts_with_print(char *arg_str, char *arg_x, char *str, char *x);

int
__starts_with(char *str, char *x)
{
	return strncmp(str, x, strlen(x)) == 0;
}

void
__starts_with_print(char *arg_str, char *arg_x, char *str, char *x)
{
	fprintf(stderr, "STARTS_WITH(%s, %s)\n\t%s\n\t%s",
		arg_str, arg_x, str, x);
}

TEST("Custom STARTS_WITH macro")
{
	char *str = "Lorem ipsum";

	STARTS_WITH(str, "Lor");
	STARTS_WITH(str, "Lorem ip");
	STARTS_WITH("abcdef", "abc");
	STARTS_WITH(str, "abc"); /* Fail */
}
