/* Copyright (C) 2000 MySQL AB & MySQL Finland AB & TCX DataKonsult AB
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA */

/* defines for the libmysql library */

#ifndef _mysql_h
#define _mysql_h

#ifdef __CYGWIN__     /* CYGWIN implements a UNIX API */
#undef WIN
#undef _WIN
#undef _WIN32
#undef _WIN64
#undef __WIN__
#endif

#ifndef MYSQL_SERVER
#ifdef	__cplusplus
extern "C" {
#endif
#endif
  
#ifndef _global_h				/* If not standard header */
#include <sys/types.h>
#ifdef __LCC__
#include <winsock.h>				/* For windows */
#endif
typedef char my_bool;
#if (defined(_WIN32) || defined(_WIN64)) && !defined(__WIN__)
#define __WIN__
#endif
#if !defined(__WIN__)
#define STDCALL
#else
#define STDCALL __stdcall
#endif
typedef char * gptr;

#ifndef ST_USED_MEM_DEFINED
#define ST_USED_MEM_DEFINED
typedef struct st_used_mem {			/* struct for once_alloc */
  struct st_used_mem *next;			/* Next block in use */
  unsigned int	left;				/* memory left in block  */
  unsigned int	size;				/* size of block */
} USED_MEM;
typedef struct st_mem_root {
  USED_MEM *free;
  USED_MEM *used;
  USED_MEM *pre_alloc;
  unsigned int	min_malloc;
  unsigned int	block_size;

  void (*error_handler)(void);
} MEM_ROOT;
#endif

#ifndef my_socket_defined
#ifdef __WIN__
#define my_socket SOCKET
#else
typedef int my_socket;
#endif
#endif
#endif
//#include "mysql_com.h"
#ifndef _mysql_com_h
#define _mysql_com_h

#define NAME_LEN	64		/* Field/table name length */
#define HOSTNAME_LENGTH 60
#define USERNAME_LENGTH 16
#define SERVER_VERSION_LENGTH 60

#define LOCAL_HOST	"localhost"
#define LOCAL_HOST_NAMEDPIPE "."

#if defined(__WIN__) && !defined( _CUSTOMCONFIG_)
#define MYSQL_NAMEDPIPE "MySQL"
#define MYSQL_SERVICENAME "MySql"
#endif /* __WIN__ */

enum enum_server_command {COM_SLEEP,COM_QUIT,COM_INIT_DB,COM_QUERY,
			  COM_FIELD_LIST,COM_CREATE_DB,COM_DROP_DB,COM_REFRESH,
			  COM_SHUTDOWN,COM_STATISTICS,
			  COM_PROCESS_INFO,COM_CONNECT,COM_PROCESS_KILL,
			  COM_DEBUG,COM_PING,COM_TIME,COM_DELAYED_INSERT,
			  COM_CHANGE_USER, COM_BINLOG_DUMP,
                          COM_TABLE_DUMP, COM_CONNECT_OUT};

#define NOT_NULL_FLAG	1		/* Field can't be NULL */
#define PRI_KEY_FLAG	2		/* Field is part of a primary key */
#define UNIQUE_KEY_FLAG 4		/* Field is part of a unique key */
#define MULTIPLE_KEY_FLAG 8		/* Field is part of a key */
#define BLOB_FLAG	16		/* Field is a blob */
#define UNSIGNED_FLAG	32		/* Field is unsigned */
#define ZEROFILL_FLAG	64		/* Field is zerofill */
#define BINARY_FLAG	128
/* The following are only sent to new clients */
#define ENUM_FLAG	256		/* field is an enum */
#define AUTO_INCREMENT_FLAG 512		/* field is a autoincrement field */
#define TIMESTAMP_FLAG	1024		/* Field is a timestamp */
#define SET_FLAG	2048		/* field is a set */
#define NUM_FLAG	32768		/* Field is num (for clients) */
#define PART_KEY_FLAG	16384		/* Intern; Part of some key */
#define GROUP_FLAG	32768		/* Intern: Group field */
#define UNIQUE_FLAG	65536		/* Intern: Used by sql_yacc */

#define REFRESH_GRANT		1	/* Refresh grant tables */
#define REFRESH_LOG		2	/* Start on new log file */
#define REFRESH_TABLES		4	/* close all tables */
#define REFRESH_HOSTS		8	/* Flush host cache */
#define REFRESH_STATUS		16	/* Flush status variables */
#define REFRESH_THREADS		32	/* Flush status variables */
#define REFRESH_SLAVE           64      /* Reset master info and restart slave
					   thread */
#define REFRESH_MASTER          128     /* Remove all bin logs in the index
					   and truncate the index */

/* The following can't be set with mysql_refresh() */
#define REFRESH_READ_LOCK	16384	/* Lock tables for read */
#define REFRESH_FAST		32768	/* Intern flag */

#define CLIENT_LONG_PASSWORD	1	/* new more secure passwords */
#define CLIENT_FOUND_ROWS	2	/* Found instead of affected rows */
#define CLIENT_LONG_FLAG	4	/* Get all column flags */
#define CLIENT_CONNECT_WITH_DB	8	/* One can specify db on connect */
#define CLIENT_NO_SCHEMA	16	/* Don't allow database.table.column */
#define CLIENT_COMPRESS		32	/* Can use compression protocol */
#define CLIENT_ODBC		64	/* Odbc client */
#define CLIENT_LOCAL_FILES	128	/* Can use LOAD DATA LOCAL */
#define CLIENT_IGNORE_SPACE	256	/* Ignore spaces before '(' */
#define CLIENT_CHANGE_USER	512	/* Support the mysql_change_user() */
#define CLIENT_INTERACTIVE	1024	/* This is an interactive client */
#define CLIENT_SSL              2048     /* Switch to SSL after handshake */
#define CLIENT_IGNORE_SIGPIPE   4096     /* IGNORE sigpipes */
#define CLIENT_TRANSACTIONS	8192	/* Client knows about transactions */

#define SERVER_STATUS_IN_TRANS  1	/* Transaction has started */
#define SERVER_STATUS_AUTOCOMMIT 2	/* Server in auto_commit mode */

#define MYSQL_ERRMSG_SIZE	200
#define NET_READ_TIMEOUT	30		/* Timeout on read */
#define NET_WRITE_TIMEOUT	60		/* Timeout on write */
#define NET_WAIT_TIMEOUT	8*60*60		/* Wait for new query */

#ifndef Vio_defined
#define Vio_defined
#ifdef HAVE_VIO
class Vio;					/* Fill Vio class in C++ */
#else
struct st_vio;					/* Only C */
typedef struct st_vio Vio;
#endif
#endif

typedef struct st_net {
  Vio* vio;
  my_socket fd;					/* For Perl DBI/dbd */
  int fcntl;
  unsigned char *buff,*buff_end,*write_pos,*read_pos;
  char last_error[MYSQL_ERRMSG_SIZE];
  unsigned int last_errno,max_packet,timeout,pkt_nr;
  unsigned char error;
  my_bool return_errno,compress;
  my_bool no_send_ok; /* needed if we are doing several
   queries in one command ( as in LOAD TABLE ... FROM MASTER ),
   and do not want to confuse the client with OK at the wrong time
		      */
  unsigned long remain_in_buf,length, buf_length, where_b;
  unsigned int *return_status;
  unsigned char reading_or_writing;
  char save_char;
} NET;

#define packet_error ((unsigned int) -1)

enum enum_field_types { FIELD_TYPE_DECIMAL, FIELD_TYPE_TINY,
			FIELD_TYPE_SHORT,  FIELD_TYPE_LONG,
			FIELD_TYPE_FLOAT,  FIELD_TYPE_DOUBLE,
			FIELD_TYPE_NULL,   FIELD_TYPE_TIMESTAMP,
			FIELD_TYPE_LONGLONG,FIELD_TYPE_INT24,
			FIELD_TYPE_DATE,   FIELD_TYPE_TIME,
			FIELD_TYPE_DATETIME, FIELD_TYPE_YEAR,
			FIELD_TYPE_NEWDATE,
			FIELD_TYPE_ENUM=247,
			FIELD_TYPE_SET=248,
			FIELD_TYPE_TINY_BLOB=249,
			FIELD_TYPE_MEDIUM_BLOB=250,
			FIELD_TYPE_LONG_BLOB=251,
			FIELD_TYPE_BLOB=252,
			FIELD_TYPE_VAR_STRING=253,
			FIELD_TYPE_STRING=254
};

#define FIELD_TYPE_CHAR FIELD_TYPE_TINY		/* For compability */
#define FIELD_TYPE_INTERVAL FIELD_TYPE_ENUM	/* For compability */

extern unsigned long max_allowed_packet;
extern unsigned long net_buffer_length;

#define net_new_transaction(net) ((net)->pkt_nr=0)

int	my_net_init(NET *net, Vio* vio);
void	net_end(NET *net);
void	net_clear(NET *net);
int	net_flush(NET *net);
int	my_net_write(NET *net,const char *packet,unsigned long len);
int	net_write_command(NET *net,unsigned char command,const char *packet,
			  unsigned long len);
int	net_real_write(NET *net,const char *packet,unsigned long len);
unsigned int	my_net_read(NET *net);

struct rand_struct {
  unsigned long seed1,seed2,max_value;
  double max_value_dbl;
};

  /* The following is for user defined functions */

enum Item_result {STRING_RESULT,REAL_RESULT,INT_RESULT};

typedef struct st_udf_args
{
  unsigned int arg_count;		/* Number of arguments */
  enum Item_result *arg_type;		/* Pointer to item_results */
  char **args;				/* Pointer to argument */
  unsigned long *lengths;		/* Length of string arguments */
  char *maybe_null;			/* Set to 1 for all maybe_null args */
} UDF_ARGS;

  /* This holds information about the result */

typedef struct st_udf_init
{
  my_bool maybe_null;			/* 1 if function can return NULL */
  unsigned int decimals;		/* for real functions */
  unsigned int max_length;		/* For string functions */
  char	  *ptr;				/* free pointer for function data */
  my_bool const_item;			/* 0 if result is independent of arguments */
} UDF_INIT;

  /* Constants when using compression */
#define NET_HEADER_SIZE 4		/* standard header size */
#define COMP_HEADER_SIZE 3		/* compression header extra size */

  /* Prototypes to password functions */

#ifdef __cplusplus
extern "C" {
#endif
  
void randominit(struct rand_struct *,unsigned long seed1,
		unsigned long seed2);
double rnd(struct rand_struct *);
void make_scrambled_password(char *to,const char *password);
void get_salt_from_password(unsigned long *res,const char *password);
void make_password_from_salt(char *to, unsigned long *hash_res);
char *scramble(char *to,const char *message,const char *password,
	       my_bool old_ver);
my_bool check_scramble(const char *, const char *message,
		       unsigned long *salt,my_bool old_ver);
char *get_tty_password(char *opt_message);
void hash_password(unsigned long *result, const char *password);

/* Some other useful functions */

void my_init(void);
void load_defaults(const char *conf_file, const char **groups,
		   int *argc, char ***argv);
my_bool my_thread_init(void);
void my_thread_end(void);

#ifdef __cplusplus
}
#endif

#define NULL_LENGTH ((unsigned long) ~0) /* For net_store_length */

#endif

//#include "mysql_version.h"
/* Copyright Abandoned 1996, 1999, 2001 MySQL AB
   This file is public domain and comes with NO WARRANTY of any kind */

/* Version numbers for protocol & mysqld */

#ifdef _CUSTOMCONFIG_
#include <custom_conf.h>
#else
#define PROTOCOL_VERSION		10
#define MYSQL_SERVER_VERSION		"3.23.46"
#define MYSQL_SERVER_SUFFIX		""
#define FRM_VER				6
#define MYSQL_VERSION_ID		32346
#define MYSQL_PORT			3306
#define MYSQL_UNIX_ADDR			"/tmp/mysql.sock"

/* mysqld compile time options */
#ifndef MYSQL_CHARSET
#define MYSQL_CHARSET			"latin1"
#endif
#endif

extern unsigned int mysql_port;
extern char *mysql_unix_port;

#define IS_PRI_KEY(n)	((n) & PRI_KEY_FLAG)
#define IS_NOT_NULL(n)	((n) & NOT_NULL_FLAG)
#define IS_BLOB(n)	((n) & BLOB_FLAG)
#define IS_NUM(t)	((t) <= FIELD_TYPE_INT24 || (t) == FIELD_TYPE_YEAR)
#define IS_NUM_FIELD(f)	 ((f)->flags & NUM_FLAG)
#define INTERNAL_NUM_FIELD(f) (((f)->type <= FIELD_TYPE_INT24 && ((f)->type != FIELD_TYPE_TIMESTAMP || (f)->length == 14 || (f)->length == 8)) || (f)->type == FIELD_TYPE_YEAR)

typedef struct st_mysql_field {
  char *name;			/* Name of column */
  char *table;			/* Table of column if column was a field */
  char *def;			/* Default value (set by mysql_list_fields) */
  enum enum_field_types type;	/* Type of field. Se mysql_com.h for types */
  unsigned int length;		/* Width of column */
  unsigned int max_length;	/* Max width of selected set */
  unsigned int flags;		/* Div flags */
  unsigned int decimals;	/* Number of decimals in field */
} MYSQL_FIELD;

typedef char **MYSQL_ROW;		/* return data as array of strings */
typedef unsigned int MYSQL_FIELD_OFFSET; /* offset to current field */

#if defined(NO_CLIENT_LONG_LONG)
typedef unsigned long my_ulonglong;
#elif defined (__WIN__)
typedef unsigned __int64 my_ulonglong;
#else
typedef unsigned long long my_ulonglong;
#endif

#define MYSQL_COUNT_ERROR (~(my_ulonglong) 0)

typedef struct st_mysql_rows {
  struct st_mysql_rows *next;		/* list of rows */
  MYSQL_ROW data;
} MYSQL_ROWS;

typedef MYSQL_ROWS *MYSQL_ROW_OFFSET;	/* offset to current row */

typedef struct st_mysql_data {
  my_ulonglong rows;
  unsigned int fields;
  MYSQL_ROWS *data;
  MEM_ROOT alloc;
} MYSQL_DATA;

struct st_mysql_options {
  unsigned int connect_timeout,client_flag;
  my_bool compress,named_pipe;
  unsigned int port;
  char *host,*init_command,*user,*password,*unix_socket,*db;
  char *my_cnf_file,*my_cnf_group, *charset_dir, *charset_name;
  my_bool use_ssl;				/* if to use SSL or not */
  char *ssl_key;				/* PEM key file */
  char *ssl_cert;				/* PEM cert file */
  char *ssl_ca;					/* PEM CA file */
  char *ssl_capath;				/* PEM directory of CA-s? */
};

enum mysql_option { MYSQL_OPT_CONNECT_TIMEOUT, MYSQL_OPT_COMPRESS,
		    MYSQL_OPT_NAMED_PIPE, MYSQL_INIT_COMMAND,
		    MYSQL_READ_DEFAULT_FILE, MYSQL_READ_DEFAULT_GROUP,
		    MYSQL_SET_CHARSET_DIR, MYSQL_SET_CHARSET_NAME};

enum mysql_status { MYSQL_STATUS_READY,MYSQL_STATUS_GET_RESULT,
		    MYSQL_STATUS_USE_RESULT};

typedef struct st_mysql {
  NET		net;			/* Communication parameters */
  gptr		connector_fd;		/* ConnectorFd for SSL */
  char		*host,*user,*passwd,*unix_socket,*server_version,*host_info,
		*info,*db;
  unsigned int	port,client_flag,server_capabilities;
  unsigned int	protocol_version;
  unsigned int	field_count;
  unsigned int 	server_status;
  unsigned long thread_id;		/* Id for connection in server */
  my_ulonglong affected_rows;
  my_ulonglong insert_id;		/* id if insert on table with NEXTNR */
  my_ulonglong extra_info;		/* Used by mysqlshow */
  unsigned long packet_length;
  enum mysql_status status;
  MYSQL_FIELD	*fields;
  MEM_ROOT	field_alloc;
  my_bool	free_me;		/* If free in mysql_close */
  my_bool	reconnect;		/* set to 1 if automatic reconnect */
  struct st_mysql_options options;
  char	        scramble_buff[9];
  struct charset_info_st *charset;
  unsigned int  server_language;
} MYSQL;


typedef struct st_mysql_res {
  my_ulonglong row_count;
  unsigned int	field_count, current_field;
  MYSQL_FIELD	*fields;
  MYSQL_DATA	*data;
  MYSQL_ROWS	*data_cursor;
  MEM_ROOT	field_alloc;
  MYSQL_ROW	row;			/* If unbuffered read */
  MYSQL_ROW	current_row;		/* buffer to current row */
  unsigned long *lengths;		/* column lengths of current row */
  MYSQL		*handle;		/* for unbuffered reads */
  my_bool	eof;			/* Used my mysql_fetch_row */
} MYSQL_RES;

/* Functions to get information from the MYSQL and MYSQL_RES structures */
/* Should definitely be used if one uses shared libraries */

my_ulonglong STDCALL mysql_num_rows(MYSQL_RES *res);
unsigned int STDCALL mysql_num_fields(MYSQL_RES *res);
my_bool STDCALL mysql_eof(MYSQL_RES *res);
MYSQL_FIELD *STDCALL mysql_fetch_field_direct(MYSQL_RES *res,
					      unsigned int fieldnr);
MYSQL_FIELD * STDCALL mysql_fetch_fields(MYSQL_RES *res);
MYSQL_ROWS * STDCALL mysql_row_tell(MYSQL_RES *res);
unsigned int STDCALL mysql_field_tell(MYSQL_RES *res);

unsigned int STDCALL mysql_field_count(MYSQL *mysql);
my_ulonglong STDCALL mysql_affected_rows(MYSQL *mysql);
my_ulonglong STDCALL mysql_insert_id(MYSQL *mysql);
unsigned int STDCALL mysql_errno(MYSQL *mysql);
char * STDCALL mysql_error(MYSQL *mysql);
char * STDCALL mysql_info(MYSQL *mysql);
unsigned long STDCALL mysql_thread_id(MYSQL *mysql);
const char * STDCALL mysql_character_set_name(MYSQL *mysql);

MYSQL *		STDCALL mysql_init(MYSQL *mysql);
#ifdef HAVE_OPENSSL
int		STDCALL mysql_ssl_set(MYSQL *mysql, const char *key,
				      const char *cert, const char *ca,
				      const char *capath);
char *		STDCALL mysql_ssl_cipher(MYSQL *mysql);
int		STDCALL mysql_ssl_clear(MYSQL *mysql);
#endif /* HAVE_OPENSSL */
MYSQL *		STDCALL mysql_connect(MYSQL *mysql, const char *host,
				      const char *user, const char *passwd);
my_bool		STDCALL mysql_change_user(MYSQL *mysql, const char *user, 
					  const char *passwd, const char *db);
#if MYSQL_VERSION_ID >= 32200
MYSQL *		STDCALL mysql_real_connect(MYSQL *mysql, const char *host,
					   const char *user,
					   const char *passwd,
					   const char *db,
					   unsigned int port,
					   const char *unix_socket,
					   unsigned int clientflag);
#else
MYSQL *		STDCALL mysql_real_connect(MYSQL *mysql, const char *host,
					   const char *user,
					   const char *passwd,
					   unsigned int port,
					   const char *unix_socket,
					   unsigned int clientflag);
#endif
void		STDCALL mysql_close(MYSQL *sock);
int		STDCALL mysql_select_db(MYSQL *mysql, const char *db);
int		STDCALL mysql_query(MYSQL *mysql, const char *q);
int		STDCALL mysql_send_query(MYSQL *mysql, const char *q,
					 unsigned int length);
int		STDCALL mysql_read_query_result(MYSQL *mysql);
int		STDCALL mysql_real_query(MYSQL *mysql, const char *q,
					unsigned int length);
int		STDCALL mysql_create_db(MYSQL *mysql, const char *DB);
int		STDCALL mysql_drop_db(MYSQL *mysql, const char *DB);
int		STDCALL mysql_shutdown(MYSQL *mysql);
int		STDCALL mysql_dump_debug_info(MYSQL *mysql);
int		STDCALL mysql_refresh(MYSQL *mysql,
				     unsigned int refresh_options);
int		STDCALL mysql_kill(MYSQL *mysql,unsigned long pid);
int		STDCALL mysql_ping(MYSQL *mysql);
char *		STDCALL mysql_stat(MYSQL *mysql);
char *		STDCALL mysql_get_server_info(MYSQL *mysql);
char *		STDCALL mysql_get_client_info(void);
char *		STDCALL mysql_get_host_info(MYSQL *mysql);
unsigned int	STDCALL mysql_get_proto_info(MYSQL *mysql);
MYSQL_RES *	STDCALL mysql_list_dbs(MYSQL *mysql,const char *wild);
MYSQL_RES *	STDCALL mysql_list_tables(MYSQL *mysql,const char *wild);
MYSQL_RES *	STDCALL mysql_list_fields(MYSQL *mysql, const char *table,
					 const char *wild);
MYSQL_RES *	STDCALL mysql_list_processes(MYSQL *mysql);
MYSQL_RES *	STDCALL mysql_store_result(MYSQL *mysql);
MYSQL_RES *	STDCALL mysql_use_result(MYSQL *mysql);
int		STDCALL mysql_options(MYSQL *mysql,enum mysql_option option,
				      const char *arg);
void		STDCALL mysql_free_result(MYSQL_RES *result);
void		STDCALL mysql_data_seek(MYSQL_RES *result,
					my_ulonglong offset);
MYSQL_ROW_OFFSET STDCALL mysql_row_seek(MYSQL_RES *result, MYSQL_ROW_OFFSET);
MYSQL_FIELD_OFFSET STDCALL mysql_field_seek(MYSQL_RES *result,
					   MYSQL_FIELD_OFFSET offset);
MYSQL_ROW	STDCALL mysql_fetch_row(MYSQL_RES *result);
unsigned long * STDCALL mysql_fetch_lengths(MYSQL_RES *result);
MYSQL_FIELD *	STDCALL mysql_fetch_field(MYSQL_RES *result);
unsigned long	STDCALL mysql_escape_string(char *to,const char *from,
					    unsigned long from_length);
unsigned long STDCALL mysql_real_escape_string(MYSQL *mysql,
					       char *to,const char *from,
					       unsigned long length);
void		STDCALL mysql_debug(const char *debug);
char *		STDCALL mysql_odbc_escape_string(MYSQL *mysql,
						 char *to,
						 unsigned long to_length,
						 const char *from,
						 unsigned long from_length,
						 void *param,
						 char *
						 (*extend_buffer)
						 (void *, char *to,
						  unsigned long *length));
void 		STDCALL myodbc_remove_escape(MYSQL *mysql,char *name);
unsigned int	STDCALL mysql_thread_safe(void);

  
#define mysql_reload(mysql) mysql_refresh((mysql),REFRESH_GRANT)

/* new api functions */

#define HAVE_MYSQL_REAL_CONNECT

#ifndef MYSQL_SERVER  
#ifdef	__cplusplus
}
#endif
#endif

#endif
