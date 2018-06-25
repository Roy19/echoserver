#undef NDEBUG
#include <net.h>

int main(int argc, char * argv[]){
        check(argc == 3, "USAGE: echo_server <host> <port>");
	check(echo_server(argv[1],argv[2]) == 0, 
		"Failed to properly run echo_server.");
error:
	return -1;
}
