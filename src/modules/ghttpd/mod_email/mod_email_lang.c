/*
    NullLogic Groupware - Copyright (C) 2000-2008 Dan Cahill

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
#include "mod_email.h"

typedef struct {
	char *key;
	char *val;
} LANGPAIR;

static LANGPAIR langpair[]={
	/* common strings */
	COMMON_LANG_SYMS
	/* this module */
	{ "TAB_GEN",            lang.tab_gen            },
	{ "TAB_SER",            lang.tab_ser            },
	{ "TAB_ADV",            lang.tab_adv            },
	{ "TAB_SIG",            lang.tab_sig            },
	{ "TAB_FLD",            lang.tab_fld            },
	{ "TAB_FIL",            lang.tab_fil            },
	{ "FOLDER1_NAME",       lang.folder1_name       },
	{ "FOLDER2_NAME",       lang.folder2_name       },
	{ "FOLDER3_NAME",       lang.folder3_name       },
	{ "FOLDER4_NAME",       lang.folder4_name       },
	{ "FOLDER5_NAME",       lang.folder5_name       },
	{ "FOLDER6_NAME",       lang.folder6_name       },
	{ "FOLDER7_NAME",       lang.folder7_name       },
	{ "MAIL_MOVETO",        lang.mail_moveto        },
	{ "MAIL_FROM",          lang.mail_from          },
	{ "MAIL_TO",            lang.mail_to            },
	{ "MAIL_CC",            lang.mail_cc            },
	{ "MAIL_BCC",           lang.mail_bcc           },
	{ "MAIL_SUBJECT",       lang.mail_subject       },
	{ "MAIL_DATE",          lang.mail_date          },
	{ "MAIL_SIZE",          lang.mail_size          },
	{ "MAIL_VIEWSOURCE",    lang.mail_viewsource    },
	{ "MAIL_PREV",          lang.mail_prev          },
	{ "MAIL_NEXT",          lang.mail_next          },
	{ "MAIL_PREVPAGE",      lang.mail_prevpage      },
	{ "MAIL_NEXTPAGE",      lang.mail_nextpage      },
	{ "MAIL_REPLY",         lang.mail_reply         },
	{ "MAIL_REPLYALL",      lang.mail_replyall      },
	{ "MAIL_FORWARD",       lang.mail_forward       },
	{ "MAIL_PRINT",         lang.mail_print         },
	{ "MAIL_DELETE",        lang.mail_delete        },
	{ "MAIL_CHECKING",      lang.mail_checking      },
	{ "MAIL_OK",            lang.mail_ok            },
	{ "MAIL_SENT",          lang.mail_sent          },
	{ "MAIL_ERR_DNS_POP3",  lang.mail_err_dns_pop3  },
	{ "MAIL_ERR_DNS_SMTP",  lang.mail_err_dns_smtp  },
	{ "MAIL_ERR_CON_POP3",  lang.mail_err_con_pop3  },
	{ "MAIL_ERR_CON_SMTP",  lang.mail_err_con_smtp  },
	{ "MAIL_ERR_CONNRESET", lang.mail_err_connreset },
	{ "MAIL_ERR_BOX_LOCK",  lang.mail_err_box_lock  },
	{ "MAIL_ERR_NOMSG",     lang.mail_err_nomsg     },
	{ NULL, NULL }
};

static void lang_callback(char *var, char *val)
{
	int found=0;
	int i;

	for (i=0;;i++) {
		if (langpair[i].key==NULL) break;
		if (strcmp(var, langpair[i].key)==0) {
			if (langpair[i].val!=NULL) {
				strncpy(langpair[i].val, val, STR_LEN-1);
			}
			found=1;
		}
	}
	if (!found) {
		log_error("lang", __FILE__, __LINE__, 1, "unknown language string '%s'", var);
	}
	return;
}

int lang_read()
{
	memset((char *)&lang, 0, sizeof(lang));
	language_read(config->langcode, "mod_email", lang_callback);
	return 0;
}
