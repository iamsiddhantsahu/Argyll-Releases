

/* 
 * Argyll Color Correction System
 * Web Display target patch window
 *
 * Author: Graeme W. Gill
 * Date:   3/4/12
 *
 * Copyright 2012 Graeme W. Gill
 * All rights reserved.
 *
 * This material is licenced under the GNU AFFERO GENERAL PUBLIC LICENSE Version 3 :-
 * see the License.txt file for licencing details.
 */

#include <stdio.h>
#include <string.h>
#ifdef NT
# include <winsock2.h>
#else
# include <sys/types.h>
# include <ifaddrs.h>
# include <netinet/in.h> 
# include <arpa/inet.h>
#endif
#include "copyright.h"
#include "aconfig.h"
#include "icc.h"
#include "numsup.h"
#include "cgats.h"
#include "conv.h"
#include "dispwin.h"
#include "conv.h"
#include "mongoose.h"

#undef DEBUG
//#define STANDALONE_TEST

#ifdef DEBUG
#define errout stderr
# define debug(xx)	fprintf(errout, xx )
# define debug2(xx)	fprintf xx
# define debugr(xx)	fprintf(errout, xx )
# define debugr2(xx)	fprintf xx
# define debugrr(xx)	fprintf(errout, xx )
# define debugrr2(xx)	fprintf xx
# define debugrr2l(lev, xx)	fprintf xx
#else
#define errout stderr
# define debug(xx) 
# define debug2(xx)
# define debugr(xx) if (p->ddebug) fprintf(errout, xx ) 
# define debugr2(xx) if (p->ddebug) fprintf xx
# define debugrr(xx) if (callback_ddebug) fprintf(errout, xx ) 
# define debugrr2(xx) if (callback_ddebug) fprintf xx
# define debugrr2l(lev, xx) if (callback_ddebug >= lev) fprintf xx
#endif

// A handler for the /ajax/get_messages endpoint.
// Return a list of messages with ID greater than requested.
static void ajax_get_messages(struct mg_connection *conn,
                              const struct mg_request_info *request_info) {
char src_addr[20];

	dispwin *p = (dispwin *)(request_info->user_data);

//sockaddr_to_string(src_addr, sizeof(src_addr), &conn->client.rsa);

//	printf("ajax_messages query_string '%s'\n",request_info->query_string);

	p->ccix++;

	while(p->ncix == p->ccix && p->mg_stop == 0) {
		msec_sleep(50);
	}

	mg_printf(conn, "#%02X%02X%02X",
			(int)(p->r_rgb[0] * 255.0 + 0.5),
			(int)(p->r_rgb[1] * 255.0 + 0.5),
			(int)(p->r_rgb[2] * 255.0 + 0.5));
}

static void *webwin_ehandler(enum mg_event event,
                           struct mg_connection *conn,
                           const struct mg_request_info *request_info) {

	if (event != MG_NEW_REQUEST) {
		return NULL;
	}
//	printf("Got event with uri = '%s'\n",request_info->uri);
	if (strcmp(request_info->uri, "/ajax/messages") == 0) {
		ajax_get_messages(conn, request_info);
	} else if (strcmp(request_info->uri, "/webdisp.js") == 0) {
#ifndef NEVER
		char *webdisp_js = 
	"\r\n"
	"if (typeof XMLHttpRequest == \"undefined\") {\r\n"
	"	XMLHttpRequest = function () {\r\n"
	"		try { return new ActiveXObject(\"Msxml2.XMLHTTP.6.0\"); }\r\n"
	"			catch (e) {}\r\n"
	"		try { return new ActiveXObject(\"Msxml2.XMLHTTP.3.0\"); }\r\n"
	"			catch (e) {}\r\n"
	"		try { return new ActiveXObject(\"Microsoft.XMLHTTP\"); }\r\n"
	"			catch (e) {}\r\n"
	"		throw new Error(\"This browser does not support XMLHttpRequest.\");\r\n"
	"	};\r\n"
	"}\r\n"
	"\r\n"
	"var ccolor = \"\";\r\n"
	"var oXHR;\r\n"
	"\r\n"
	"function XHR_response() {\r\n"
	"	if (oXHR.readyState != 4)\r\n"
	"		return;\r\n"
	"\r\n"
	"	if (oXHR.status != 200) {\r\n"
	"		return;\r\n"
	"	}\r\n"
	"	if (ccolor != oXHR.responseText) {\r\n"
	"		ccolor = oXHR.responseText;\r\n"
	"		document.body.style.background = ccolor;\r\n"
	"	}\r\n"
	"	oXHR.open(\"GET\", \"/ajax/messages?\" + document.body.style.background + \" \" + Math.random(), true);\r\n"
	"	oXHR.onreadystatechange = XHR_response;\r\n"
	"	oXHR.send();\r\n"
	"}\r\n"
	"\r\n"
	"window.onload = function() {\r\n"
	"	ccolor = \"#808080\";\r\n"
	"	document.body.style.background = ccolor;\r\n"
	"\r\n"
	"	oXHR = new XMLHttpRequest();\r\n"
	"	oXHR.open(\"GET\", \"/ajax/messages?\" + document.body.style.background, true);\r\n"
	"	oXHR.onreadystatechange = XHR_response;\r\n"
	"	oXHR.send();\r\n"
	"};\r\n";
	    mg_write(conn, webdisp_js, strlen(webdisp_js));
#else
		return NULL;	/* Read webdisp.js */
#endif
	} else {
	    mg_printf(conn, "HTTP/1.1 200 OK\r\n"
		"Cache-Control: no-cache\r\n"
		"Content-Type: text/html\r\n\r\n"
		"<html>\r\n"
		"<head>\r\n"
		"<title>ArgyllCMS Web Display</title>\r\n"
		"<script src=\"webdisp.js\"></script>\r\n"
		"</head>\r\n"
		"</html>\r\n"
		);
	}

	return "yes";
}

/* ----------------------------------------------- */

/* Get RAMDAC values. ->del() when finished. */
/* Return NULL if not possible */
static ramdac *webwin_get_ramdac(dispwin *p) {
	debugr("webdisp doesn't have a RAMDAC\n"); 
	return NULL;
}

/* Set the RAMDAC values. */
/* Return nz if not possible */
static int webwin_set_ramdac(dispwin *p, ramdac *r, int persist) {
	debugr("webdisp doesn't have a RAMDAC\n"); 
	return 1;
}

/* ----------------------------------------------- */
/* Install a display profile and make */
/* it the default for this display. */
/* Return nz if failed */
int webwin_install_profile(dispwin *p, char *fname, ramdac *r, p_scope scope) {
	debugr("webdisp doesn't support installing profiles\n"); 
	return 1;
}

/* Un-Install a display profile */
/* Return nz if failed, */
int webwin_uninstall_profile(dispwin *p, char *fname, p_scope scope) {
	debugr("webdisp doesn't support uninstalling profiles\n"); 
	return 1;
}

/* Get the currently installed display profile. */
/* Return NULL if failed. */
icmFile *webwin_get_profile(dispwin *p, char *name, int mxlen) {
	debugr("webdisp doesn't support getting the current profile\n"); 
	return NULL;
}

/* ----------------------------------------------- */

/* Change the window color. */
/* Return 1 on error, 2 on window being closed */
static int webwin_set_color(
dispwin *p,
double r, double g, double b	/* Color values 0.0 - 1.0 */
) {
	int j;

	debugr("webwin_set_color called\n");

	if (p->nowin)
		return 1;

	p->rgb[0] = r;
	p->rgb[1] = g;
	p->rgb[2] = b;

	for (j = 0; j < 3; j++) {
		if (p->rgb[j] < 0.0)
			p->rgb[j] = 0.0;
		else if (p->rgb[j] > 1.0)
			p->rgb[j] = 1.0;
		p->r_rgb[j] = p->rgb[j];
	}

	/* This is probably not actually thread safe... */
	p->ncix++;

	while(p->ncix != p->ccix) {
		msec_sleep(50);
	}

	/* Allow some time for the display to update before */
	/* a measurement can take place. This allows time for */
	/* the browser to update the background color, the CRT */
	/* refresh or LCD processing/update time, + */
	/* display settling time (quite long for smaller LCD changes). */
	msec_sleep(200);

	return 0;
}

/* ----------------------------------------------- */
/* Set the shell set color callout */
void webwin_set_callout(
dispwin *p,
char *callout
) {
	debugr2((errout,"webwin_set_callout called with '%s'\n",callout));

	p->callout = strdup(callout);
}

/* ----------------------------------------------- */
/* Destroy ourselves */
static void webwin_del(
dispwin *p
) {

	debugr("webwin_del called\n");

	if (p == NULL)
		return;

	p->mg_stop = 1;
	mg_stop((struct mg_context *)p->pcntx);

	if (p->name != NULL)
		free(p->name);
	if (p->description != NULL)
		free(p->description);
	if (p->callout != NULL)
		free(p->callout);

	free(p);
}

/* ----------------------------------------------- */

/* Create a web display test window, default grey */
dispwin *new_webwin(
int webdisp,					/* Port number */
double width, double height,	/* Width and height in mm */
double hoff, double voff,		/* Offset from center in fraction of screen, range -1.0 .. 1.0 */
int nowin,						/* NZ if no window should be created - RAMDAC access only */
int blackbg,					/* NZ if whole screen should be filled with black */
int verb,						/* NZ for verbose prompts */
int ddebug						/* >0 to print debug statements to stderr */
) {
	dispwin *p = NULL;
	struct mg_context *mg;
	const char *options[3];
	char port[50];

	debug("new_webwin called\n");

	if ((p = (dispwin *)calloc(sizeof(dispwin), 1)) == NULL) {
		if (ddebug) fprintf(stderr,"new_webwin failed because malloc failed\n");
		return NULL;
	}

	p->name = strdup("Web Window");
	p->nowin = nowin;
	p->native = 0;
	p->blackbg = blackbg;
	p->ddebug = ddebug;
	p->get_ramdac      = webwin_get_ramdac;
	p->set_ramdac      = webwin_set_ramdac;
	p->install_profile = webwin_install_profile;
	p->uninstall_profile = webwin_uninstall_profile;
	p->get_profile     = webwin_get_profile;
	p->set_color       = webwin_set_color;
	p->set_callout     = webwin_set_callout;
	p->del             = webwin_del;

	p->rgb[0] = p->rgb[1] = p->rgb[2] = 0.5;	/* Set Grey as the initial test color */

	p->ncix = 1;

	/* Basic object is initialised, so create a web server */

	options[0] = "listening_ports";
	sprintf(port,"%d", webdisp);
	options[1] = port;
	options[2] = NULL;

	mg = mg_start(&webwin_ehandler, (void *)p, options);
	p->pcntx = (void *)mg;

//printf("Domain = %s'\n",mg_get_option(mg, "authentication_domain"));

	/* Create a suitable description */
#if NT
	{
		char szHostName[255];
		struct hostent *host_entry;
		char *localIP;
		char buf[1000];

		/* We assume WinSock has been started by mongoose */

		// Get the local hostname
		gethostname(szHostName, 255);
		host_entry=gethostbyname(szHostName);
		/* Get first entry */
		localIP = inet_ntoa(*(struct in_addr *)*host_entry->h_addr_list);

		sprintf(buf,"Web Window at http://%s:%d",localIP,webdisp);
		p->description = strdup(buf);

		if (verb)
			printf("Created web server at 'http://%s:%d', now waiting for browser to connect\n",localIP,webdisp);
	}
#else
	{
		struct ifaddrs * ifAddrStruct=NULL;
		struct ifaddrs * ifa=NULL;
		void *tmpAddrPtr=NULL;
		char abuf[INET_ADDRSTRLEN] = "";
		char abuf6[INET6_ADDRSTRLEN] = "";
		char *addr = abuf;
		char buf[1000];
	
		getifaddrs(&ifAddrStruct);
	
		/* Stop at the first non local adderss */
		for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
#ifdef AF_INET6
			if (ifa->ifa_addr->sa_family==AF_INET) { /* IP4 ? */
#endif
				if (strncmp(ifa->ifa_name, "lo",2) == 0 || abuf[0] != '\000')
					continue;
				tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
				inet_ntop(AF_INET, tmpAddrPtr, abuf, INET_ADDRSTRLEN);
//				printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
#ifdef AF_INET6
			} else if (ifa->ifa_addr->sa_family==AF_INET6) { /* IP6 ? */
				if (strncmp(ifa->ifa_name, "lo",2) == 0 || abuf6[0] != '\000')
					continue;
				tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
				inet_ntop(AF_INET6, tmpAddrPtr, abuf6, INET6_ADDRSTRLEN);
//				printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
			} 
#endif
		}
		if (ifAddrStruct!=NULL)
			freeifaddrs(ifAddrStruct);
		if (addr[0] == '\000')
			addr = abuf6;
		if (addr[0] == '\000')
			addr = "Unknown";

		sprintf(buf,"Web Window at http://%s:%d",addr,webdisp);
		p->description = strdup(buf);

		if (verb)
			printf("Created web server at 'http://%s:%d', now waiting for browser to connect\n",addr,webdisp);
	}
#endif

	/* Wait for the web server to connect */
	debugr("new_webwin: waiting for web browser to connect\n");
	while(p->ccix == 0) {
		msec_sleep(50);
	}

	debugr("new_webwin: return sucessfully\n");

	return p;
}
