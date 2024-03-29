/*
 * nlcomm.c 
 * Functions to create netlink socket, send and recieve requests
 * and parse them.
 * 
 * Author: Shubham Tiwari <f2016935@pilani.bits-pilani.ac.in>
 */ 

#include <linux/rtnetlink.h>
#include <linux/gen_stats.h>
#include <linux/pkt_sched.h>
#include <bits/sockaddr.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "nlcomm.h"
#include <stdbool.h>


   /* +----------+
	* | nlmsghdr |
	* |----------|
	* |   len	|
	* |   type   |
	* |   flags  |
	* |   seq	|
	* |   pid	|
	* +----------+
	* 
	* +----------------+
	* |	msghdr	  |
	* +----------------+
	* | msg_name	   |
	* | msg_namelen	|
	* | msg_iov		| [nlmsghdr + payload]
	* | msg_iovlen	 |
	* | msg_control	|
	* | msg_controllen |
	* | msg_flags	  |
	* +----------------+
	* 
	* msghdr is the actual message sent through the sendmsg() function
	* and recieved using recvmsg().
	* Message header name is a bit mis-leading in that its just a header. 
	* This header data actually contains the vector to the whole netlink 
	* message.
	* msg_iov is the pointer to the buffer (created using malloc) containing 
	* the actual netlink message. msg_iovlen is the number of iovecs (I/O vectors) 
	* present in the message.
	* 
	* Example: A request message consists of two iovecs, first one pointing to the netlink 
	* message header (struct nlmsghdr) and the second vector pointing to the request message. 
	* Request message is formed using the struct appropriate to what the programmer wants 
	* to achieve. 
	* nlmsghdr->len is the sum of size of nlmsghdr and the request message(s). Netlink socket 
	* is opened and is bound to the source (in our case its our userspace application, whos pid
	* is fetched using getpid()). msghdr->msg_name is the address of struct sockaddr_nl
	* inititalized with the destinations address (We put 0 in pid to refer to kernel).
	* msghdr->msg_namelen is the size of sockaddr_nl.
	* 
	*/

   /* Netlink message format
	* 
	* +----------+-----+---------------+-----+-------------+-----+--------------+
	* | nlmsghdr | Pad | Family Header | Pad | Attr Header | Pad | Attr payload |
	* +----------+-----+---------------+-----+-------------+-----+--------------+
	* 
	* There can be any number of messages of in this sequence.
	* Moreover, Attr header and Attr payload can also be in sequence together, forming 
	* multiple attributes in the same netlink message.
	*/


#include <sys/time.h>

//Returns the current time with microsecond accuracy
double getMicrotime(){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec  + currentTime.tv_usec* (double)1e-6;
}
//	double time = getMicrotime();
//	printf("%f\n", time);

int nl_sock() {
	int sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (sock_fd < 0) {
		printf("\n Failed to open netlink socket");
		return -1;
	}

	/* 
		struct sockaddr_nl {
		sa_family_t	 nl_family; // AF_NETLINK
		unsigned short  nl_pad;	// zero
		__u32		   nl_pid;	// process pid
		__u32		   nl_groups; // multicast grps mask
		};
	*/

	struct sockaddr_nl src_addr;

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();

	/* Once socket is opened, it has to be bound to local address */
	int rtnl = bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
	if (rtnl < 0) {
		printf("\n Failed to bind local address to the socket");
		return -1;
	}

	return sock_fd;
}

static int filter_ifindex;

int nl_dump_class_qdisc_request(int sock_fd, char r_type) {

	struct tcmsg t = { .tcm_family = AF_UNSPEC };
/*
	char d[IFNAMSIZ] = {};
//	strncpy(d, "enp6s0", sizeof(d)-1);
	strncpy(d, "veth1", sizeof(d)-1);

	if (d[0]) {
		t.tcm_ifindex = if_nametoindex(d);
		filter_ifindex = t.tcm_ifindex;
	}
*/
	/* rtnl_dump_request(&rth, RTM_GETQDISC, &t, sizeof(t)) < 0) */
	void *req = (void *)&t;
	int type;
	if (r_type == 'c') { 
		type = RTM_GETTCLASS;
	} else { 
		type = RTM_GETQDISC;
	}
	int len = sizeof(t);

	/* 
		struct nlmsghdr {
			__u32  nlmsg_len;   //Length of msg incl. hdr
			__u16  nlmsg_type;  //Message content
			__u16  nlmsg_flags; //Additional flags
			__u32  nlmsg_seq;   //Sequence number
			__u32  nlmsg_pid;   //Sending process PID
		}
	*/

	struct nlmsghdr nlh = {
		.nlmsg_len = NLMSG_LENGTH(len),
		.nlmsg_type = type,
		.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST,
//		.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP, 
		.nlmsg_seq = 0,
	};

	/* vector of data to send */
	/*  
		struct iovec {
			void __user *iov_base;	// BSD uses caddr_t (1003.1g requires void *) 
			__kernel_size_t iov_len; // Must be size_t (1003.1g) 
		};
	*/
	struct iovec iov[2] = {
		{ .iov_base = &nlh, .iov_len = sizeof(nlh) },
		{ .iov_base = req, .iov_len = len }
	};

	/* 
		struct sockaddr_nl {
			sa_family_t	 nl_family; // AF_NETLINK
			unsigned short  nl_pad;	// zero
			__u32		   nl_pid;	// process pid
			__u32		   nl_groups; // multicast grps mask
		};
	*/

	struct sockaddr_nl dst_addr;

	memset(&dst_addr, 0, sizeof(dst_addr));
	dst_addr.nl_family = AF_NETLINK;
	dst_addr.nl_pid = 0; /* For linux kernel */
	dst_addr.nl_groups = 0; /* unicast */

   /* struct msghdr {
		void *msg_name;		//Address to send to
		socklen_t msg_namelen; //Length of address data

		struct iovec *msg_iov; //Vector of data to send
		size_t msg_iovlen;	 //Number of iovec entries

		void *msg_control;	 //Ancillary data
		size_t msg_controllen; //Ancillary data buf len

		int msg_flags;		 //Flags on received msg
	};
*/

	struct msghdr msg = {
		.msg_name = &dst_addr,
		.msg_namelen = sizeof(dst_addr),
		.msg_iov = iov,
		.msg_iovlen = 2,
	};

	/* message has been framed, now send the message out */ 
	sendmsg(sock_fd, &msg, 0);
//	printf("\nWaiting for message from the kernel!");

}

void nl_parse_attr(struct rtattr *rta, int len, struct rtattr *tb[], int max) {
	memset(tb, 0, sizeof(struct rtattr *)*(max+1));
	unsigned short type;
	while (RTA_OK(rta, len)) {
		type = rta->rta_type;
		if ((type <= max) && (!tb[type])) {
			tb[type] = rta;	
		}

		rta = RTA_NEXT(rta, len);
	}
}



void nl_print_qdisc_stats(char *buf, int recvlen) {

	struct nlmsghdr *h = (struct nlmsghdr *)buf;
	int msglen = recvlen;

	printf("\nnlmsg_type: %d", h->nlmsg_type);

	struct tcmsg *tcrecv = NLMSG_DATA(h);


	while (NLMSG_OK(h, msglen)) {

		printf ("\n -------------- \n");
		if (h->nlmsg_type == NLMSG_DONE) {
			printf("\nDone iterating through the netlink message");
			break;
		}

		if (h->nlmsg_type == NLMSG_ERROR) {
			printf("\nError message encountered!");
			break;
		}

		/* Parse this message */
		struct tcmsg *tcrecv = NLMSG_DATA(h);

		struct rtattr *tb[TCA_MAX+1];
		struct qdisc_util *q;

		/* TODO: ignore this message if not of RTM_NEWQDISC || RTM_DELQDISC type 
		   NOTE: reply of RTM_QDISCGET is of type RTM_NEWQDISC.
		*/


		int len = h->nlmsg_len;

		len -= NLMSG_LENGTH(sizeof(*tcrecv));

		if (len <0) {
			printf("Wrong len %d\n", len);
			exit(0);
		}

		/* Parse attributes */
		struct rtattr *rta = TCA_RTA(tcrecv);

		nl_parse_attr(rta, len, tb, TCA_MAX);
		
		if (tb[TCA_KIND] == NULL) {
			printf("\nNULL KIND!");
			exit(0);
		}

		/* convert to string -> (const char *)RTA_DATA(rta); */

		printf("\n qdisc %s", (const char *)RTA_DATA(tb[TCA_KIND]));
		printf("\n handle: %x", tcrecv->tcm_handle >> 16);

		printf("[%08x]", tcrecv->tcm_handle);

		/* Print dev name using if_indextoname */
		char d[IFNAMSIZ] = {};
		printf("dev %s", if_indextoname(tcrecv->tcm_ifindex, d));

		if (tcrecv->tcm_parent == TC_H_ROOT)
			printf(" root ");

		if (strcmp("pfifo_fast", RTA_DATA(tb[TCA_KIND])) == 0) {
			/* get prio qdisc kind */

		} else {
			/* get tb[TCA_KIND] qdsic kind */
		}

		/* Print queue stats */
		struct rtattr *tbs[TCA_STATS_MAX + 1];

		/* Parse nested attr using TCA_STATS_MAX */
		rta = RTA_DATA(tb[TCA_STATS2]);
		len = RTA_PAYLOAD(tb[TCA_STATS2]);

		nl_parse_attr(rta, len, tbs, TCA_STATS_MAX);

		/* tc stats structs present in linux/gen_stats.h 
		   They have been pasted below for reference 
		   enum {
			TCA_STATS_UNSPEC,
			TCA_STATS_BASIC,
			TCA_STATS_RATE_EST,
			TCA_STATS_QUEUE,
			TCA_STATS_APP,
			TCA_STATS_RATE_EST64,
			TCA_STATS_PAD,
			TCA_STATS_BASIC_HW,
			__TCA_STATS_MAX,
			};
			#define TCA_STATS_MAX (__TCA_STATS_MAX - 1)

			struct gnet_stats_basic {
				__u64	bytes;
				__u32	packets;
			};

			struct gnet_stats_rate_est {
				__u32	bps;
				__u32	pps;
			};

			struct gnet_stats_rate_est64 {
				__u64	bps;
				__u64	pps;
			};

			struct gnet_stats_queue {
				__u32	qlen;
				__u32	backlog;
				__u32	drops;
				__u32	requeues;
				__u32	overlimits;
			};
		*/

		if (tbs[TCA_STATS_BASIC]) {
			struct gnet_stats_basic bs = {0};

			memcpy(&bs, RTA_DATA(tbs[TCA_STATS_BASIC]), MIN(RTA_PAYLOAD(tbs[TCA_STATS_BASIC]), sizeof(bs)));
			printf("bytes: %llu packets %u", bs.bytes, bs.packets);
			
		}

		if (tbs[TCA_STATS_QUEUE]) {
			struct gnet_stats_queue q = {0};

			memcpy(&q, RTA_DATA(tbs[TCA_STATS_QUEUE]), MIN(RTA_PAYLOAD(tbs[TCA_STATS_QUEUE]), sizeof(q)));
			/* From here we can print all the data in the struct q */

			printf(" qlen: %d drops: %d", q.qlen, q.drops);
		}

		/* Print the rate, try est64 followed by est */ 
		if (tbs[TCA_STATS_RATE_EST64]) {
			struct gnet_stats_rate_est64 re = {0};
			memcpy(&re, RTA_DATA(tbs[TCA_STATS_RATE_EST64]),
			   MIN(RTA_PAYLOAD(tbs[TCA_STATS_RATE_EST64]),
					sizeof(re)));

			printf(" rate (bps): %llu rate (pps): %llu", re.bps, re.pps);		
		}
		else if (tbs[TCA_STATS_RATE_EST]) {
			struct gnet_stats_rate_est re = {0};
			memcpy(&re, RTA_DATA(tbs[TCA_STATS_RATE_EST]),
			   MIN(RTA_PAYLOAD(tbs[TCA_STATS_RATE_EST]),
					sizeof(re)));

			printf(" rate (bps): %u rate (pps): %u", re.bps, re.pps);		

		}

		/* For backward compatibility. In the newer versions of kernel, struct tc_stats has been broken down 
		   into more particular structs (listed above). The previous versions of kernel encapsulated all the 
		   stats in struct tc_stats itself.
		 */

		if (tb[TCA_STATS]) {
			struct tc_stats st = {};
			memcpy(&st, RTA_DATA(tb[TCA_STATS]), MIN(RTA_PAYLOAD(tb[TCA_STATS]), sizeof(st)));

			/* bps and pps were not showing up in the rate estimator above, so instead printed it 
			   from here. All the stats which have been printed above can be printed from here too. 
			   I just tried out bps and pps because they were not showing up above */

			printf(" rate (bps): %u rate (pps): %u", st.bps, st.pps);

			/* can print any of the stats present in struct tc_stats here */

		}

		h = NLMSG_NEXT(h, msglen);
	}
	free(buf);
}


void nl_print_qdisc_stats_new(int sock_fd, char **ints, int ints_index, char* file_name, FILE *dataFile) {
	
//	Writting to file
//	fprintf(dataFile, "writting from nlcomm.c\n");

	double time = getMicrotime();
//	printf("%f\n", time);
	
	/* Read message from the kernel */
	struct sockaddr_nl nladdr;
	struct iovec iovrecv;
	iovrecv.iov_base = NULL;
	iovrecv.iov_len = 0;

	
	struct msghdr msg_recv = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iovrecv,
		.msg_iovlen = 1,
	};

	char *buf;

	/* Determine buffer length for recieving the message */

	int recvlen = recvmsg(sock_fd, &msg_recv, MSG_PEEK | MSG_TRUNC);
	if (recvlen < 32768)
		recvlen = 32768;

	/* Allocate buffer for recieving the message */
	buf = malloc(recvlen);

	/* Reset the I/O vector */
	iovrecv.iov_base = buf;
	iovrecv.iov_len = recvlen;

	/* Now recieve the message */
	recvlen = recvmsg(sock_fd, &msg_recv, 0);

	if (recvlen <0) {
		free(buf);
		printf("\n Error during netlink msg recv: len < 0");
		exit(1);
	}

	/* At this point of time, buf contains the message */

//	printf("\nRecieved msg len: %d", recvlen);
//	printf("\nRecieved message payload: %s", (char *)buf);

/*	
	int i;
	i = 0;
	printf("\nnew stats function:\n");
    for (i=0; i<ints_index; i++) {
        printf("dev %d: %s\n", i, ints[i]);
    }
*/
/*
	i = 0;
	printf("\nSelected interfaces: ");
    for (i=0; i<ints_index; i++) {
		if (i==0)
			printf("%s", ints[i]);
		if (i!=0)
			printf(", %s", ints[i]);
    }
*/

	struct nlmsghdr *h = (struct nlmsghdr *)buf;
	int msglen = recvlen;

//	printf("\nnlmsg_type: %d", h->nlmsg_type);

	struct tcmsg *tcrecv = NLMSG_DATA(h);

//	printf("msglen: %d", msglen);

	while (NLMSG_OK(h, msglen)) {

		if (h->nlmsg_type == NLMSG_DONE) {
//			printf("\nDone iterating through the netlink message");
			break;
		}

		if (h->nlmsg_type == NLMSG_ERROR) {
			printf("\nError message encountered!");
			break;
		}

		/* Parse this message */
		struct tcmsg *tcrecv = NLMSG_DATA(h);

		struct rtattr *tb[TCA_MAX+1];
		struct qdisc_util *q;

		/* TODO: ignore this message if not of RTM_NEWQDISC || RTM_DELQDISC type 
		   NOTE: reply of RTM_QDISCGET is of type RTM_NEWQDISC.
		*/


		int len = h->nlmsg_len;

		len -= NLMSG_LENGTH(sizeof(*tcrecv));

		if (len <0) {
			printf("Wrong len %d\n", len);
			exit(0);
		}

		/* Parse attributes */
		struct rtattr *rta = TCA_RTA(tcrecv);

		nl_parse_attr(rta, len, tb, TCA_MAX);

		//ignore superfluous information 
//		if (filter_ifindex && filter_ifindex != tcrecv->tcm_ifindex) {
//			h = NLMSG_NEXT(h, msglen);
//			continue;
//		}

		{
			char e[IFNAMSIZ] = {};
			if_indextoname(tcrecv->tcm_ifindex, e);
			int i;
			bool selected = false;
			for (i=0; i<ints_index; i++) {
				if (!strcmp(ints[i], e)) {	
					selected = true;
				}
			}
			if (!selected) {
				h = NLMSG_NEXT(h, msglen);
				continue;
			}
		}


//		printf ("\n -------------- \n");
//		printf("saved index: %d \nreceived index: %d \n", filter_ifindex, tcrecv->tcm_ifindex);
		
		if (tb[TCA_KIND] == NULL) {
			printf("\nNULL KIND!");
			exit(0);
		}

		// Start writting info to file

		fprintf(dataFile, "\n%f", time);
		
		/* convert to string -> (const char *)RTA_DATA(rta); */
		/* Print dev name using if_indextoname */
		char d[IFNAMSIZ] = {};
//		printf("dev %s", if_indextoname(tcrecv->tcm_ifindex, d));
		fprintf(dataFile, ", %s", if_indextoname(tcrecv->tcm_ifindex, d));

//		printf("\n qdisc %s", (const char *)RTA_DATA(tb[TCA_KIND]));
		fprintf(dataFile, ", %s", (const char *)RTA_DATA(tb[TCA_KIND]));
		//handle
		__u32 handle = tcrecv->tcm_handle;
		if (handle == TC_H_ROOT) {
//			printf(" root ");
			fprintf(dataFile, ", root");
		}
		else if (handle == TC_H_UNSPEC) {
//			printf(" handle none ");
			fprintf(dataFile, ", none");
		}
		else if (TC_H_MAJ(handle) == 0) {
//			printf(" handle :%x ", TC_H_MIN(handle));
			fprintf(dataFile, ", :%x", TC_H_MIN(handle));
		}
		else if (TC_H_MIN(handle) == 0) {
//			printf(" handle %x: ", TC_H_MAJ(handle) >> 16);
			fprintf(dataFile, ", %x:", TC_H_MAJ(handle) >> 16);
		}
		else {
//			printf(" handle %x:%x ", TC_H_MAJ(handle) >> 16, TC_H_MIN(handle));
			fprintf(dataFile, ", %x:%x", TC_H_MAJ(handle) >> 16, TC_H_MIN(handle));
		}

//		printf("[%08x]", tcrecv->tcm_handle);
	

		//parent
		__u32 parent = tcrecv->tcm_parent;
		if (parent == TC_H_ROOT) {
//			printf(" root ");
			fprintf(dataFile, ", root");
		}
		else if (parent == TC_H_UNSPEC) {
//			printf(" parent none ");
			fprintf(dataFile, ", none");
		}
		else if (TC_H_MAJ(parent) == 0) {
//			printf(" parent :%x ", TC_H_MIN(parent));
			fprintf(dataFile, ", :%x", TC_H_MIN(parent));
		}
		else if (TC_H_MIN(parent) == 0) {
//			printf(" parent %x: ", TC_H_MAJ(parent) >> 16);
			fprintf(dataFile, ", %x:", TC_H_MAJ(parent) >> 16);
		}
		else {
//			printf(" parent %x:%x ", TC_H_MAJ(parent) >> 16, TC_H_MIN(parent));
			fprintf(dataFile, ", %x:%x", TC_H_MAJ(parent) >> 16, TC_H_MIN(parent));
		}

		if (strcmp("pfifo_fast", RTA_DATA(tb[TCA_KIND])) == 0) {
			/* get prio qdisc kind */

		} else {
			/* get tb[TCA_KIND] qdsic kind */
		}
/*
		if (tcrecv->tcm_info) // leaf info only for classes, returns 0 for queues. 
			printf(" leaf %x: ", tcrecv->tcm_info>>16);
*/

		/* Print queue stats */
		struct rtattr *tbs[TCA_STATS_MAX + 1];

		/* Parse nested attr using TCA_STATS_MAX */
		rta = RTA_DATA(tb[TCA_STATS2]);
		len = RTA_PAYLOAD(tb[TCA_STATS2]);

		nl_parse_attr(rta, len, tbs, TCA_STATS_MAX);

		/* tc stats structs present in linux/gen_stats.h 
		   They have been pasted below for reference 
		   enum {
			TCA_STATS_UNSPEC,
			TCA_STATS_BASIC,
			TCA_STATS_RATE_EST,
			TCA_STATS_QUEUE,
			TCA_STATS_APP,
			TCA_STATS_RATE_EST64,
			TCA_STATS_PAD,
			TCA_STATS_BASIC_HW,
			__TCA_STATS_MAX,
			};
			#define TCA_STATS_MAX (__TCA_STATS_MAX - 1)

			struct gnet_stats_basic {
				__u64	bytes;
				__u32	packets;
			};

			struct gnet_stats_rate_est {
				__u32	bps;
				__u32	pps;
			};

			struct gnet_stats_rate_est64 {
				__u64	bps;
				__u64	pps;
			};

			struct gnet_stats_queue {
				__u32	qlen;
				__u32	backlog;
				__u32	drops;
				__u32	requeues;
				__u32	overlimits;
			};
		*/

		struct gnet_stats_basic bs = {0};
		if (tbs[TCA_STATS_BASIC]) {

			memcpy(&bs, RTA_DATA(tbs[TCA_STATS_BASIC]), MIN(RTA_PAYLOAD(tbs[TCA_STATS_BASIC]), sizeof(bs)));
//			printf("bytes: %llu packets %u", bs.bytes, bs.packets);
			fprintf(dataFile, ", %llu, %u", bs.bytes, bs.packets);
			
		}

		if (tbs[TCA_STATS_QUEUE]) {
			struct gnet_stats_queue q = {0};

			memcpy(&q, RTA_DATA(tbs[TCA_STATS_QUEUE]), MIN(RTA_PAYLOAD(tbs[TCA_STATS_QUEUE]), sizeof(q)));
			/* From here we can print all the data in the struct q */

//			printf(" qlen: %d drops: %d", q.qlen, q.drops);
			fprintf(dataFile, ", %d, %d, %d", q.qlen, q.backlog, q.drops);
		}
// DATA to be save to file

		h = NLMSG_NEXT(h, msglen);
	}
	free(buf);
}


