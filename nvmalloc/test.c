#include <stdio.h>
#include <stdlib.h>
#include "nv_map.h"

int main(){

	void *ptr = NULL;
	size_t bytes;
	struct rqst_struct rqst;

	rqst.bytes = 100;
	rqst.pid = 99;
	rqst.id = 1;
	//ptr = pnv_malloc(bytes, &rqst);
	ptr = pnv_read(bytes, &rqst);

	

return 0;
}
