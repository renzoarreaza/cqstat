/*
 * nlcomm.h 
 * 
 * Author: Shubham Tiwari <f2016935@pilani.bits-pilani.ac.in>
 */ 

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

int nl_sock();
int nl_dump_qdisc_request(int sock_fd, void (*cb)(char *, int));
int nl_dump_class_request(int sock_fd, void (*cb)(char *, int));
int nl_dump_class_qdisc_request(int sock_fd, void (*cb)(char *, int), char r_type);
void nl_print_qdisc_stats(char *buf, int recvlen);
void nl_print_qdisc_stats_new(char *buf, int recvlen);
void nl_print_qdisc_stats_start(char *buf, int recvlen);
void nl_parse_attr(struct rtattr *rta, int len, struct rtattr *tb[], int max);
