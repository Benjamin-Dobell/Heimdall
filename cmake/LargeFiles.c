#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#define LARGE_OFF_T (((off_t) 1 << 62) - 1 + ((off_t) 1 << 62))
#define VALID_ARRAY_LENGTH 1
#define INVALID_ARRAY_LENGTH -1

int main(int argc, const char **argv)
{
	int a[(LARGE_OFF_T % 2147483629 == 721 && LARGE_OFF_T % 2147483647 == 1) ? VALID_ARRAY_LENGTH : INVALID_ARRAY_LENGTH];
	off_t offset = ftello(NULL);
	fseeko(NULL, offset, SEEK_SET);
	return 0;
}
