#include "utils.h"

int main(int argc, char const *argv[])
{
	char* v = url_decode("code=3%2B4");
	if(match_float(argv[1]))
		printf("It is a float\n");
	printf("Result is %s\n",v);
	return 0;
}
