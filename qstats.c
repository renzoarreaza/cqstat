/*
 * qstats.c 
 * Uses netlink to fetch qdisc stats from kernel.
 * 
 * Author: Shubham Tiwari <f2016935@pilani.bits-pilani.ac.in>
 */ 

#include <linux/rtnetlink.h>
#include <unistd.h>
#include <stdio.h>

#include "nlcomm.h"
#include <time.h>
#include <string.h>


int main(int argc, char *argv[]) {

	char r_type;
	// parsing arguments
	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
//			NEXT_ARG();
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

