#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "types.h"
#include "vhost.h"

int main(int argc, char **argv)
{
	assert(argc == 2);

	vhost_user_start(argv[1]);

	return 0;
}
