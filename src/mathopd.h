/*
 *   Copyright 1996, 1997, 1998 Michiel Boland.
 *
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *
 *   1. Redistributions of source code must retain the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 *   3. All advertising materials mentioning features or use of
 *      this software must display the following acknowledgement:
 *
 *   This product includes software developed by Michiel Boland.
 *
 *   4. The name of the author may not be used to endorse or promote
 *      products derived from this software without specific prior
 *      written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 *   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR
 *   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _mathopd_h
#define _mathopd_h

#define CGI_MAGIC_TYPE "CGI"
#define IMAP_MAGIC_TYPE "Imagemap"
#define REDIRECT_MAGIC_TYPE "Redirect"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pwd.h>
#include <grp.h>

#ifdef NEED_MEMORY_H
#include <memory.h>
#endif

#ifdef POLL
#include <stropts.h>
#include <poll.h>
#endif

#ifndef NO_GETRLIMIT
#include <sys/resource.h>
#endif

#ifndef SA_RESTART
#define SA_RESTART 0
#endif

#ifndef SA_INTERRUPT
#define SA_INTERRUPT 0
#endif

#define DEFAULT_BUF_SIZE 12288
#define INPUT_BUF_SIZE 2048
#define DEFAULT_NUM_CONNECTIONS 64
#define DEFAULT_TIMEOUT 60
#define DEFAULT_PORT 80
#define DEFAULT_FILEMODE 0666
#define DEFAULT_UMASK 022

#define STRLEN 400
#define PATHLEN (2 * STRLEN)

enum {
	ALLOW,
	DENY,
	APPLY
};

enum {
	CLASS_FILE = 1,
	CLASS_SPECIAL,
	CLASS_EXTERNAL
};

enum {
	M_UNKNOWN,
	M_HEAD,
	M_GET,
	M_POST
};

enum {
	HC_FREE,
	HC_ACTIVE
};

enum {
	HC_READING,
	HC_WRITING,
	HC_WAITING,
	HC_CLOSING
};

enum {
	L_LOG,
	L_PANIC,
	L_ERROR,
	L_WARNING,
	L_TRANS,
	L_AGENT,
	L_DEBUG
};

struct pool {
	char *floor;
	char *ceiling;
	char *start;
	char *end;
	char state;
};

struct access {
	int type;
	unsigned long mask;
	unsigned long addr;
	struct access *next;
};

struct mime {
	int class;
	char *ext;
	char *name;
	struct mime *next;
};

struct simple_list {
	char *name;
	struct simple_list *next;
};

struct control {
	char *alias;
	int symlinksok;
	int loglevel;
	int path_args_ok;
	struct simple_list *index_names;
	struct access *accesses;
	struct mime *mimes;
	struct control *next;
	struct simple_list *locations;
	struct access *clients;
	char *admin;
	int refresh;
	char *realm;
	char *userfile;
	char *error_401_file;
	char *error_403_file;
	char *error_404_file;
};

struct virtual {
	char *host;
	char *fullname;
	struct server *parent;
	struct control *controls;
	unsigned long nrequests;
	unsigned long nread;
	unsigned long nwritten;
	struct virtual *next;
};

struct server {
	int fd;
	int port;
	struct in_addr addr;
	char *name;
	struct virtual *children;
	struct control *controls;
	struct server *next;
#ifdef POLL
	int pollno;
#endif
	unsigned long naccepts;
	unsigned long nhandled;
};

struct request {
	struct connection *cn;
	struct virtual *vs;
	char *user_agent;
	char *referer;
	char *from;
	char *authorization;
	char *cookie;
	char *host;
	char *in_content_type;
	char *in_content_length;
	char path[PATHLEN];
	char path_translated[PATHLEN];
	char path_args[PATHLEN];
	const char *content_type;
	int num_content;
	int class;
	long content_length;
	time_t last_modified;
	time_t ims;
	char *location;
	const char *status_line;
	const char *error;
	const char *method_s;
	char *url;
	char *args;
	char *params;
	int protocol_major;
	int protocol_minor;
	int method;
	int status;
	struct control *c;
	struct stat finfo;
	int isindex;
	const char *error_file;
	char user[16];
	char iphost[16];
	char *servername;
};

struct connection {
	struct request *r;
	int state;
	struct server *s;
	int fd;
	int rfd;
	struct sockaddr_in peer;
	struct sockaddr_in sock;
	char ip[16];
	time_t t;
	time_t it;
	struct pool *input;
	struct pool *output;
	int assbackwards;
	int keepalive;
	int action;
	struct connection *next;
#ifdef POLL
	int pollno;
#endif
};

struct tuning {
	int buf_size;
	int input_buf_size;
	int num_connections;
	int timeout;
	int accept_multi;
};

/* main */

extern const char server_version[];
extern volatile int gotsigterm;
extern volatile int gotsighup;
extern volatile int gotsigusr1;
extern volatile int gotsigusr2;
extern volatile int gotsigwinch;
extern volatile int numchildren;
extern time_t startuptime;
extern int debug;
extern int fcm;
extern int my_pid;
extern void die(const char *, const char *, ...);
extern int fork_request(struct request *, int (*)(struct request *));

/* config */

extern struct tuning tuning;
extern struct simple_list *exports;
extern char *pid_filename;
extern char *log_filename;
extern char *error_filename;
extern char *agent_filename;
extern char *child_filename;

extern char *admin;
extern char *rootdir;
extern char *coredir;
extern struct connection *connections;
extern struct server *servers;
extern char *user_name;
#ifdef POLL
extern struct pollfd *pollfds;
#endif
extern void config(void);

/* core */

extern int nconnections;
extern int maxconnections;
extern time_t current_time;

extern void log(int, const char *, ...);
extern void lerror(const char *);
extern void httpd_main(void);

/* request */

extern int prepare_reply(struct request *);
extern int process_request(struct request *);
extern struct control *faketoreal(char *, char *, struct request *, int);
extern void construct_url(char *, char *, struct request *);
extern void escape_url(char *);
extern int unescape_url(char *, char *);

/* imap */

extern int process_imap(struct request *);

/* cgi */

extern int process_cgi(struct request *);

/* dump */

extern void dump(void);

/* base64 */

extern void base64initialize(void);
extern int webuserok(const char *, const char *, char *, int);

/* redirect */

extern int process_redirect(struct request *);

#ifdef NEED_STRERROR
extern const char *strerror(int);
#endif

#ifdef NEED_STRDUP
extern char *strdup(const char *);
#endif

#ifdef NEED_PROTOTYPES
int accept(int, struct sockaddr *, int *);
int bind(int, struct sockaddr *, int);
void bzero(char *, int);
int ftruncate(int, off_t);
int getopt(int, char * const *, const char *);
int initgroups(const char *, gid_t);
int listen(int, int);
int lstat(const char *, struct stat *);
int recv(int, char *, int, int);
int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
int send(int, const char *, int, int);
int setsockopt(int, int, int, const char *, int);
int socket(int, int , int);
int strcasecmp(const char *, const char *);

#ifndef NO_GETRLIMIT
int getrlimit(int, struct rlimit *);
int setrlimit(int, const struct rlimit *);
#endif

#endif

#endif /* NEED_PROTOTYPES */
