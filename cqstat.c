#include <linux/rtnetlink.h>
#include <unistd.h>
#include <stdio.h>
#include <net/if.h>

#include "nlcomm.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void usage(void) {
	printf("Options:\n\n\
\t-c,\t\tGet stats per Class\n\
\t-q,\t\tGet stats per Qdisc\n\
\t-w [FILE],\t\tProvide file name\n\
\tdev [if_name],\t\tInterface to monitor. Can be used multiple times\n");
}

int run = 1;
void  INThandler(int sig)
{
	run = 0;	
}

int main(int argc, char *argv[]) {

	// parsing arguments
	char r_type; // 'c' or 'q' (class or queue) statistics
	char *ints[argc/2];
	char base_file_name[16];
	char file_name[40];
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
			strcpy(base_file_name, *argv);
		}
		argc--; argv++;
	}
	if(base_file_name[0] == '\0')
		strcpy(base_file_name, "cqdata");


	// get system time
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
//	printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	sprintf(file_name,"%s_%d-%d-%d_%d:%d:%d.csv", base_file_name, tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	//due to bug with the -c option: override r_type to 'q'
	r_type = 'q';

	printf("filename: %s\n\n", file_name);

	if(ints_index == 0) {
		printf("Provide interface name\n");
		usage();
		exit(1);
}
	if (!r_type)
		r_type = 'c';

// Opening file for writting

	FILE *fp;
	fp = fopen(file_name, "w+");
	fprintf(fp, "time, dev, qdisc, handle, parent, bytes, packets, qlen, drops");
	
	while(run) {
		int sock_fd = nl_sock();
		nl_dump_class_qdisc_request(sock_fd, r_type);
		nl_print_qdisc_stats_new(sock_fd, ints, ints_index, file_name, fp); 
//		nl_print_qdisc_stats(
		close(sock_fd);
		usleep(5000); //5 miliseconds
	}
	/* Close netlink socket */ 
	// Close file
	fclose(fp);
// end Main part 

	printf("\n");
}  


/*
int myfunction(char* fileName, FILE** readFile) // pointer pointer to allow pointer to be changed 
{
    if(( *readFile = fopen(fileName,"r")) == NULL)
    {
        return FILE_ERROR;
    }
    return FILE_NO_ERROR;
}

int main(int argc, char **argv)
{
    FILE* openReadFile; // This needs to be a pointer. 
    if(myfunction(argv[1], &openReadFile) != FILE_NO_ERROR) // allow address to be updated 
    {
        printf("\n %s : ERROR opening file. \n", __FUNCTION__);
    }
}
*/
