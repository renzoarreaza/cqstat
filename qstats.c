#include <linux/rtnetlink.h>
#include <unistd.h>
#include <stdio.h>
#include <net/if.h>

#include "nlcomm.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>

void usage(void) {
	printf("Options:\n\n\
\t-c,\t\tGet stats per Class\n\
\t-q,\t\tGet stats per Qdisc\n\
\t-w [FILE],\t\tProvide file name. Will append interface name to file\n\
\tdev [if_name],\t\tInterface to monitor. Can be used multiple times\n");
}


int main(int argc, char *argv[]) {

	// parsing arguments
	char r_type; // 'c' or 'q' (class or queue) statistics
//	char ints[argc/2][IFNAMSIZ];	// list containing interface names passed as arguments
	char *ints[argc/2];
	char file_name[16];
	int ints_index = 0;
	while (argc > 0) {
		if (strcmp(*argv, "dev") == 0) {
			argc--; argv++;
//			strcpy(ints[ints_index], *argv);
			ints[ints_index]= *argv;
			ints_index++;
		} else if(strcmp(*argv, "-c") == 0) {
			r_type = 'c';
		} else if(strcmp(*argv, "-q") == 0) {
			r_type = 'q';
		} else if(strcmp(*argv, "-w") == 0) {
			argc--; argv++;
			strcpy(file_name, *argv);
		}
		argc--; argv++;
	}
	if(file_name[0] == '\0') {
		strcpy(file_name, "stats");
		// or have a default filename....
		printf("Using default filename, stats_<int>\n\n");
	}
	if(ints_index == 0) {
		printf("Provide interface name\n");
		usage();
		exit(1);
}
	if (!r_type)
		r_type = 'c';

	clock_t start_t, end_t; 
	start_t = clock(); 

// Main part
	/* Open a netlink socket */ 
	int sock_fd = nl_sock();
	/* Using nl_print_qdisc_stats as the callback function to parse netlink message */
	nl_dump_class_qdisc_request(sock_fd, nl_print_qdisc_stats_new, r_type, ints, ints_index);

	/* Close netlink socket */ 
	close(sock_fd);
// end Main part 

	end_t = clock();
	double total_t = (double)(end_t - start_t)/CLOCKS_PER_SEC;
	printf("\ntime taken: %f s", total_t);

	printf("\n");
}  

