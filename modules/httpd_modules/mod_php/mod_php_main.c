/*
    NullLogic Groupware - Copyright (C) 2000-2004 Dan Cahill

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#define SRVMOD_MAIN 1
#include "http_mod.h"
#include "mod_php.h"

static CONN *get_sid()
{
	int sid;

	if (http_proc->conn==NULL) return NULL;
	for (sid=0;sid<http_proc->config.http_maxconn;sid++) {
		if (http_proc->conn[sid].id==pthread_self()) break;
	}
	if ((sid<0)||(sid>=http_proc->config.http_maxconn)) return NULL;
	return &http_proc->conn[sid];
}

static char *php_embed_read_cookies(TSRMLS_D)
{
	CONN *sid=get_sid();

	if (sid==NULL) return NULL;
	return sid->dat->in_Cookie;
}

static int php_embed_read_post(char *buffer, uint count_bytes TSRMLS_DC)
{
	CONN *sid=get_sid();
	unsigned int i;

	if (sid==NULL) return 0;
	for (i=0;(i<(unsigned int)sid->dat->in_ContentLength)&&(i<count_bytes);i++) {
		buffer[i]=sid->PostData[i];
	}
	return i;
}

static int php_embed_deactivate(TSRMLS_D)
{
	CONN *sid=get_sid();

	if (sid==NULL) return SUCCESS;
	flushbuffer(sid);
	return SUCCESS;
}

static inline size_t php_embed_single_write(const char *str, uint str_length)
{
	CONN *sid=get_sid();
	int ret;

	if (sid==NULL) return 0;
	flushbuffer(sid);
	ret=tcp_send(&sid->socket, str, str_length, 0);
	if (ret<=0) return 0;
	return ret;
}

static int php_embed_ub_write(const char *str, uint str_length TSRMLS_DC)
{
	const char *ptr = str;
	uint remaining = str_length;
	size_t ret;

	while (remaining > 0) {
		ret = php_embed_single_write(ptr, remaining);
		if (!ret) {
			php_handle_aborted_connection();
		}
		ptr += ret;
		remaining -= ret;
	}
	return str_length;
}

static void php_embed_flush(void *server_context)
{
	CONN *sid=get_sid();

	if (sid==NULL) return;
	flushbuffer(sid);
	return;
}

static void php_embed_send_header(sapi_header_struct *sapi_header, void *server_context TSRMLS_DC)
{
}

static void php_embed_log_message(char *message)
{
	log_error("mod_php", __FILE__, __LINE__, 1, "%s", message);
	return;
}

static void php_embed_register_variables(zval *track_vars_array TSRMLS_DC)
{
	CONN *sid=get_sid();
	char tmpbuf[80];

	if (sid==NULL) return;
	snprintf(tmpbuf, sizeof(tmpbuf)-1, "%d", sid->dat->in_ContentLength);
	php_register_variable("CONTENT_LENGTH", tmpbuf, track_vars_array TSRMLS_CC);
	if (!strlen(sid->dat->in_ContentType)) {
		php_register_variable("CONTENT_TYPE", "application/x-www-form-urlencoded", track_vars_array TSRMLS_CC);
	} else {
		php_register_variable("CONTENT_TYPE", sid->dat->in_ContentType, track_vars_array TSRMLS_CC);
	}
	php_register_variable("GATEWAY_INTERFACE", "CGI/1.1", track_vars_array TSRMLS_CC);
	php_register_variable("HTTP_COOKIE",       sid->dat->in_Cookie, track_vars_array TSRMLS_CC);
	php_register_variable("HTTP_USER_AGENT",   sid->dat->in_UserAgent, track_vars_array TSRMLS_CC);
	php_register_variable("PATH_TRANSLATED",   "/home/null/www/tools/n-shell.php", track_vars_array TSRMLS_CC);
	php_register_variable("PHP_SELF",          sid->dat->in_CGIScriptName, track_vars_array TSRMLS_CC);
	php_register_variable("QUERY_STRING",      sid->dat->in_QueryString, track_vars_array TSRMLS_CC);
	php_register_variable("REMOTE_ADDR",       sid->dat->in_RemoteAddr, track_vars_array TSRMLS_CC);
	snprintf(tmpbuf, sizeof(tmpbuf)-1, "%d", sid->dat->in_RemotePort);
	php_register_variable("REMOTE_PORT",       tmpbuf, track_vars_array TSRMLS_CC);
	php_register_variable("REMOTE_USER",       sid->dat->user_username, track_vars_array TSRMLS_CC);
	php_register_variable("REQUEST_METHOD",    sid->dat->in_RequestMethod, track_vars_array TSRMLS_CC);
	php_register_variable("REQUEST_URI",       sid->dat->in_RequestURI, track_vars_array TSRMLS_CC);
	php_register_variable("SCRIPT_NAME",       sid->dat->in_CGIScriptName, track_vars_array TSRMLS_CC);
	php_register_variable("SERVER_PROTOCOL",   "HTTP/1.1", track_vars_array TSRMLS_CC);
	php_register_variable("SERVER_SOFTWARE",   PACKAGE_NAME, track_vars_array TSRMLS_CC);
//	php_import_environment_variables(track_vars_array TSRMLS_CC);
	return;
}

static int php_embed_startup(sapi_module_struct *sapi_module)
{
	if (php_module_startup(sapi_module, NULL, 0)==FAILURE) {
		return FAILURE;
	}
	return SUCCESS;
}

sapi_module_struct php_embed_module = {
	"embed",			/* name */
	"NullLogic Groupware mod_php",	/* pretty name */
	
	php_embed_startup,		/* startup */
	php_module_shutdown_wrapper,	/* shutdown */
  
	NULL,				/* activate */
	php_embed_deactivate,		/* deactivate */
  
	php_embed_ub_write,		/* unbuffered write */
	php_embed_flush,		/* flush */
	NULL,				/* get uid */
	NULL,				/* getenv */
  
	php_error,			/* error handler */
  
	NULL,				/* header handler */
	NULL,				/* send headers handler */
	php_embed_send_header,		/* send header handler */
	
	php_embed_read_post,		/* read POST data */
	php_embed_read_cookies,		/* read Cookies */
  
	php_embed_register_variables,	/* register server variables */
	php_embed_log_message,		/* Log message */
  
	NULL,				/* Block interruptions */
	NULL,				/* Unblock interruptions */

	STANDARD_SAPI_MODULE_PROPERTIES
};

int php_embed_init(int argc, char **argv PTSRMLS_DC)
{
	zend_llist global_vars;
#ifdef ZTS
	zend_compiler_globals *compiler_globals;
	zend_executor_globals *executor_globals;
	php_core_globals *core_globals;
	sapi_globals_struct *sapi_globals;
	void ***tsrm_ls;
#endif

#ifdef ZTS
	tsrm_startup(http_proc->config.http_maxconn, 1, 0, NULL);
#endif
#ifdef ZTS
	compiler_globals = ts_resource(compiler_globals_id);
	executor_globals = ts_resource(executor_globals_id);
	core_globals = ts_resource(core_globals_id);
	sapi_globals = ts_resource(sapi_globals_id);
	tsrm_ls = ts_resource(0);
	*ptsrm_ls = tsrm_ls;
#endif
	sapi_startup(&php_embed_module);
	if (php_embed_module.startup(&php_embed_module)==FAILURE) {
		return FAILURE;
	}
	zend_llist_init(&global_vars, sizeof(char *), NULL, 0);  
	/* Set some Embedded PHP defaults */
	SG(options)|=SAPI_OPTION_NO_CHDIR;
	SG(server_context)=(void *)1;
//	zend_alter_ini_entry("html_errors", 12, "0", 1, PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
//	zend_alter_ini_entry("implicit_flush", 15, "1", 1, PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
//	zend_alter_ini_entry("max_execution_time", 19, "0", 1, PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
	zend_alter_ini_entry("always_populate_raw_post_data", strlen("always_populate_raw_post_data")+1, "1", 1, PHP_INI_SYSTEM, PHP_INI_STAGE_ACTIVATE);
	return SUCCESS;
}

void php_embed_shutdown(TSRMLS_D)
{
	php_request_shutdown((void *) 0);
	php_module_shutdown(TSRMLS_C);
#ifdef ZTS
	tsrm_shutdown();
#endif
	return;
}

DllExport int mod_main(CONN *sid)
{
	zend_file_handle file_handle;
	char filename[512];
	char *ptemp;

	if (strncmp(sid->dat->in_CGIScriptName, "/php/", 5)!=0) { return -1; }
	memset(filename, 0, sizeof(filename));
	ptemp=sid->dat->in_CGIScriptName;
	snprintf(filename, sizeof(filename)-1, "%s/%04d/htdocs%s", config->dir_var_domains, sid->dat->user_did, ptemp);
	fixslashes(filename);

	SG(request_info).request_method       = sid->dat->in_RequestMethod;
	SG(request_info).query_string         = sid->dat->in_QueryString;
//	SG(request_info).post_data            = sid->PostData;
//	SG(request_info).raw_post_data        = sid->PostData;
	SG(request_info).cookie_data          = sid->dat->in_Cookie;
//	SG(request_info).content_length       = sid->dat->in_ContentLength;
//	SG(request_info).post_data_length     = 0;
//	SG(request_info).raw_post_data_length = 0;
	SG(request_info).path_translated      = filename;
	SG(request_info).request_uri          = sid->dat->in_RequestURI;
	if (!strlen(sid->dat->in_ContentType)) {
		SG(request_info).content_type = "application/x-www-form-urlencoded";
	} else {
		SG(request_info).content_type = sid->dat->in_ContentType;
	}
	SG(request_info).no_headers           = 0;
	SG(headers_sent)                      = 0;
	file_handle.type = ZEND_HANDLE_FILENAME;
//	file_handle.filename = SG(request_info).path_translated;
	file_handle.filename = filename;
	file_handle.opened_path = NULL;
	file_handle.handle.fd = 0;
	file_handle.free_filename = 0;

	send_header(sid, 0, 200, "1", "text/html", -1, -1);

	if (php_request_startup(TSRMLS_C)==SUCCESS) {
		zend_eval_string("foreach ($HTTP_SERVER_VARS as $x => $a) { echo \"<font color=green>[$x][$a]</font><br>\"; }", NULL, "nullgroupare-sapi");
		zend_eval_string("foreach ($_COOKIE as $x => $a) { echo \"<font color=red>[$x][$a]</font><br>\"; }", NULL, "nullgroupare-sapi");
		zend_eval_string("foreach ($HTTP_GET_VARS as $x => $a) { echo \"<font color=black>[$x][$a]</font><br>\"; }", NULL, "nullgroupare-sapi");
		zend_eval_string("foreach ($HTTP_POST_VARS as $x => $a) { echo \"<font color=blue>[$x][$a]</font><br>\"; }", NULL, "nullgroupare-sapi");
		php_execute_script(&file_handle TSRMLS_CC);
		php_request_shutdown(NULL);
		return 0;
	}
	return 0;
}

DllExport int mod_exit()
{
	return 0;
}

DllExport int mod_init(_PROC *_proc, HTTP_PROC *_http_proc, FUNCTION *_functions)
{
	MODULE_MENU newmod = {
		"mod_php",		// mod_name
		0,			// mod_submenu
		"",			// mod_menuname
		"",			// mod_menuuri
		"",			// mod_menuperm
		"mod_main",		// fn_name
		"/php/",		// fn_uri
		mod_init,		// fn_init
		mod_main,		// fn_main
		mod_exit		// fn_exit
	};

	proc=_proc;
	http_proc=_http_proc;
	config=&proc->config;
	functions=_functions;
	if (mod_import()!=0) return -1;
	if (mod_export_main(&newmod)!=0) return -1;
	php_embed_init(0, NULL);
	return 0;
}
