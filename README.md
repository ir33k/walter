Walter
======

Single header library for writing unit tests in C with fewer
complications by avoiding boilerplate.

	walter.h        Library with full documentation and licence
	demo/           Working demonstration test programs AKA examples
	tests.c         Unit tests for this library
	snap/           Snapshots for tests.c library tests
	build           Script to build and run tests

Expected to work on POSIX systems and NOT on Windows.


Example
-------

```c
// File: example.t.c
#include <string.h>             // Include your code
#include "walter.h"             // Include Walter

TEST("Test description")        // Define test with assertions
{
	OK(strlen("abc") == 3); // Fail when not true
	SAME("abb", "abc", 3);  // Fail when buffers of size 3 are different
}

// No main() function as it is already defined in walter.h
```

Compile and run:

```sh
$ cc -o example.t example.t.c
$ ./example.t
	First incorrect byte at index: 2
	"abb"
	"abc"
example.t.c:8:	SAME("abb", "abc", 3)
example.t.c:5:	TEST Test description
example.t.c	1 fail
```

This is example of default program output when second assertion failed
at third character (index 2).  First you get the details about what
went wrong, then path to failed assertion, then path to failed test
with that assertion.  Lastly there is a summery of how many tests
failed in total.  By default when all tests pass there is no output.
Program exit code is a number of failed tests.

Full documentation with longer example is in `walter.h`.  More working
examples can be found in `demo` directory.
