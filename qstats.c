/*
 */ 

#include <linux/rtnetlink.h>
#include <unistd.h>
#include <stdio.h>
#include <net/if.h>

#include "nlcomm.h"
#include <time.h>
#include <string.h>


int main(int argc, char *argv[]) {

	char r_type; // 'c' or 'q' (class or queue) statistics
	// parsing arguments

	char ints[argc/2][IFNAMSIZ];	// list containing interface names passed as arguments
	int ints_index = 0;
	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			argc--; argv++;
			strcpy(ints[ints_index], *argv);
			ints_index++;
			// TO DO: set add to list of interfaces
		} else if(strcmp(*argv, "-c") == 0) {
			r_type = 'c';
		} else if(strcmp(*argv, "-q") == 0) {
			r_type = 'q';
		}
		argc--; argv++;
	}
	if (!r_type)
		r_type = 'c';

	clock_t start_t, end_t; 
	start_t = clock(); 

// Main part
	/* Open a netlink socket */ 
	int sock_fd = nl_sock();
	/* Using nl_print_qdisc_stats as the callback function to parse netlink message */
	nl_dump_class_qdisc_request(sock_fd, nl_print_qdisc_stats_new, r_type);

	/* Close netlink socket */ 
	close(sock_fd);
// end Main part 

	end_t = clock();
	double total_t = (double)(end_t - start_t)/CLOCKS_PER_SEC;
	printf("\ntime taken: %f s", total_t);

	printf("\n");
}  

