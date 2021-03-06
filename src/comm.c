/*****************************************************************************/
/*                                                                           */
/*  Copyright (C) 2005 Adrian Gonera                                         */
/*                                                                           */
/*  This file is part of Rhapsody.                                           */
/*                                                                           */
/*  This program is free software; you can redistribute it and/or modify     */
/*  it under the terms of the GNU General Public License as published by     */
/*  the Free Software Foundation; either version 2 of the License, or        */
/*  (at your option) any later version.                                      */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful,          */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*  GNU General Public License for more details.                             */
/*                                                                           */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program; if not, write to the Free Software              */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA  */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "autodefs.h"
#include "defines.h"
#ifdef NCURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif

#include "autodefs.h"
#include "defines.h"

#include "log.h"
#include "ncolor.h"
#include "common.h"
#include "events.h"
#include "cevents.h"
#include "screen.h"
#include "network.h"
#include "parser.h"
#include "ctcp.h"
#include "dcc.h"
#include "main.h"
#include "cmenu.h"
#include "forms.h"
#include "config.h"
#include "option.h"

int connect_to_server (server *S);
int disconnect_from_server (server *S);

extern int screenupdated;

/* communication functions ******************************************************/


int disconnect_from_server (server *S){
	char buffer[MAXDATASIZE];

	if (S->active || S->connect_status){
		sprintf(buffer,"QUIT\n");
		send_all(S->serverfd, buffer, strlen(buffer));
		S->active = 0;
		S->proxyactive = 0;
		if (S->serverfd) close(S->serverfd);
		if (S->proxyfd) close(S->proxyfd);
		vprint_server_attrib(S, MESSAGE_COLOR, "Disconnected from %s\n", S->server);
		set_statusline_update_status(statusline, U_ALL_REFRESH);
		screenupdated = 1;
		return(1);
	}
	return(0);
}	

int connect_to_server (server *S){
	char buffer[MAXDATASIZE];
	static struct sockaddr_in their_addr;
	static struct hostent *host;
	int userlen, passlen;

	// get the name of the host 

	alarm_occured=0;
	alarm(CONNECTTIMEOUT);

	S->proxy = configuration.proxy;

	if (S->proxy == PROXY_DIRECT){
		// connect to a server one step at a time returning to report messages after each step
		if (S->connect_status == 0){

			S->active = 0;
			S->proxyactive = 0;
			S->serverfd = -1;
			S->proxyfd = -1;

			strcpy(S->user, configuration.user);
			strcpy(S->host, configuration.hostname);
			strcpy(S->domain, configuration.domain);
			strcpy(S->name, configuration.userdesc);
			strcpy(S->nick, configuration.nick);
			strcpy(S->lastnick, configuration.nick);

			if ((S->serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
				vprint_server_attrib(S, ERROR_COLOR, "Error: Cannot create a socket\n");
				S->connect_status = -1;
				return(0);
			}
			else{
				vprint_server_attrib(S, MESSAGE_COLOR, "Looking up %s ...\n", S->server);
				S->connect_status = 1;
				return(1);
			}
		}

		if (S->connect_status == 1){
			if ((host = gethostbyname(S->server)) == NULL) {
				vprint_server_attrib(S, ERROR_COLOR, "Error: Cannot find host %s\n", S->server);
				S->connect_status = -1;
				return(0);
			}
			else {
				vprint_server_attrib(S, MESSAGE_COLOR, "Connecting to %s (%s) on port %d ...\n", 
					host->h_name, inet_ntoa(*((struct in_addr*)host->h_addr)), S->port);
				S->connect_status = 2;
				return(1);
			}
		}

		if (S->connect_status == 2){
			their_addr.sin_family = AF_INET;                                            
			their_addr.sin_port = htons(S->port);                              
			their_addr.sin_addr = *((struct in_addr *)host->h_addr);
			bzero(&(their_addr.sin_zero), 8);
		        if (connect (S->serverfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
				vprint_server_attrib(S, ERROR_COLOR, "Failed to connect to host %s (%s) on port %d ...\n", 
					host->h_name, inet_ntoa(*((struct in_addr *)host->h_addr)), S->port);
				S->connect_status = -1;
				return(0);
			}
			vprint_server_attrib(S, MESSAGE_COLOR, "Connected to %s (%s) on port %d ...\n", 
				host->h_name, inet_ntoa(*((struct in_addr*)host->h_addr)), S->port);
			S->connect_status = 3;
			return(1);
		}

		if (S->connect_status == 3){
			vprint_server_attrib(S, MESSAGE_COLOR, "Logging on as %s ...\n", S->nick);
	
			S->nickinuse = 1;
			sprintf(buffer,"USER %s %s %s :%s\n",S->user, S->host, S->domain, S->name);
			send_all(S->serverfd, buffer, strlen(buffer));
			if (strlen(S->password)){
				sprintf(buffer,"PASS %s\n", S->password);
				send_all(S->serverfd, buffer, strlen(buffer));
			}
			sprintf(buffer,"NICK :%s\n",S->nick);
			send_all(S->serverfd, buffer, strlen(buffer));
			sprintf(buffer,"MODE %s +%s\n",S->nick, configuration.mode);
			send_all(S->serverfd, buffer, strlen(buffer));
			S->connect_status = -1;
        		if (alarm_occured) return (0);
			else {
				fcntl(S->serverfd, F_SETFL, O_NONBLOCK);
				S->active=1;
				set_statusline_update_status(statusline, U_ALL_REFRESH);
				screenupdated = 1;
				return(1);
			}
		}	
		return(0);
	}

	else if (S->proxy == PROXY_SOCKS4){
	        // use the SOCKS4 proxy rather than a direct connection 

		if (S->connect_status == 0){
			S->active = 0;
			S->proxyactive = 0;
			S->serverfd = -1;
			S->proxyfd = -1;

			strcpy(S->user, configuration.user);
			strcpy(S->host, configuration.hostname);
			strcpy(S->domain, configuration.domain);
			strcpy(S->name, configuration.userdesc);
			strcpy(S->nick, configuration.nick);
			strcpy(S->lastnick, configuration.nick);

			if ((S->proxyfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
				vprint_server_attrib(S, ERROR_COLOR, "Error: Cannot create a socket\n");
				S->connect_status = -1;
				return(0);
			}
			else{
				vprint_server_attrib(S, MESSAGE_COLOR, "Looking up SOCKS4 proxy %s ...\n", 
					configuration.proxyhostname);
				S->connect_status = 1;
				return(1);
			}
		}

		else if (S->connect_status == 1){
			if ((host = gethostbyname(configuration.proxyhostname)) == NULL) {
				vprint_server_attrib(S, ERROR_COLOR, "Error: Cannot find SOCKS4 proxy %s\n", 
					configuration.proxyhostname);
				S->connect_status = -1;
				return(0);
			}
			else {
				vprint_server_attrib(S, MESSAGE_COLOR, "Connecting to SOCKS4 proxy %s (%s) port %d ...\n", 
					host->h_name, inet_ntoa(*((struct in_addr*)host->h_addr)), 
					configuration.proxyport);
				S->connect_status = 2;
				return(1);
			}
		}

		else if (S->connect_status == 2){
			their_addr.sin_family = AF_INET;                                            
			their_addr.sin_port = htons(configuration.proxyport); 
			their_addr.sin_addr = *((struct in_addr *)host->h_addr);
			bzero(&(their_addr.sin_zero), 8);
		        if (connect (S->proxyfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
				vprint_server_attrib(S, ERROR_COLOR, "Failed to connect to proxy %s (%s) on port %d ...\n", 
					host->h_name, inet_ntoa(*((struct in_addr *)host->h_addr)), 
					configuration.proxyport);
				S->connect_status = -1;
				return(0);
			}
			vprint_server_attrib(S, MESSAGE_COLOR, "Connected to proxy %s (%s) on port %d ...\n", 
				host->h_name, inet_ntoa(*((struct in_addr*)host->h_addr)), configuration.proxyport);
			vprint_server_attrib(S, MESSAGE_COLOR, "Looking up IRC server %s ...\n", S->server);
			S->connect_status = 3;
			S->proxyactive = 1;
			return(1);
		}

		else if (S->connect_status == 3){
			if ((host = gethostbyname(S->server)) == NULL) {
				vprint_server_attrib(S, ERROR_COLOR, "Error: Cannot find server %s\n", S->server);
				S->connect_status = -1;
				return(0);
			}
			else {
				vprint_server_attrib(S, MESSAGE_COLOR, "Requesting proxy connection to %s (%s) port %d ...\n", 
					host->h_name, inet_ntoa(*((struct in_addr*)host->h_addr)), S->port);
				S->connect_status = 4;
				return(1);
			}
		}

		else if (S->connect_status == 4){
			/* create SOCK4 connect message */
			buffer[0] = 4;
			buffer[1] = 1; /* CONNECT */
			buffer[2] = (S->port & 0xff00) >> 8;
			buffer[3] = (S->port & 0xff);
			memcpy(&buffer[4], (struct in_addr*)host->h_addr, 4);
			strcpy(&buffer[8], configuration.proxyusername);
			send_ball(S->proxyfd, buffer, 8 + strlen(buffer + 8) + 1);

			/* at this point, wait until the proxy replies */
			S->connect_status = -1;
		}

		if (S->connect_status == 5){
			vprint_server_attrib(S, MESSAGE_COLOR, "Logging on as %s ...\n", S->nick);

			S->nickinuse = 1;
			sprintf(buffer,"USER %s %s %s :%s\n",S->user, S->host, S->domain, S->name);
			send_all(S->serverfd, buffer, strlen(buffer));
			if (strlen(S->password)){
				sprintf(buffer,"PASS %s\n", S->password);
				send_all(S->serverfd, buffer, strlen(buffer));
			}
			sprintf(buffer,"NICK :%s\n",S->nick);
			send_all(S->serverfd, buffer, strlen(buffer));
			sprintf(buffer,"MODE %s +%s\n",S->nick, configuration.mode);
			send_all(S->serverfd, buffer, strlen(buffer));
			S->connect_status = -1;
			if (alarm_occured) return (0);
			else {
				fcntl(S->serverfd, F_SETFL, O_NONBLOCK);
				S->active=1;
				set_statusline_update_status(statusline, U_ALL_REFRESH);
				screenupdated = 1;
				return(1);
			}
		}
		return(0);
	}	

	else if (S->proxy == PROXY_SOCKS5){
	        // use the SOCKS5 proxy rather than a direct connection 

		if (S->connect_status == 0){
			S->active = 0;
			S->proxyactive = 0;
			S->serverfd = -1;
			S->proxyfd = -1;

			strcpy(S->user, configuration.user);
			strcpy(S->host, configuration.hostname);
			strcpy(S->domain, configuration.domain);
			strcpy(S->name, configuration.userdesc);
			strcpy(S->nick, configuration.nick);
			strcpy(S->lastnick, configuration.nick);

			if ((S->proxyfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
				vprint_server_attrib(S, ERROR_COLOR, "Error: Cannot create a socket\n");
				S->connect_status = -1;
				return(0);
			}
			else{
				vprint_server_attrib(S, MESSAGE_COLOR, "Looking up SOCKS5 proxy %s ...\n", 
					configuration.proxyhostname);
				S->connect_status = 1;
				return(1);
			}
		}

		else if (S->connect_status == 1){
			if ((host = gethostbyname(configuration.proxyhostname)) == NULL) {
				vprint_server_attrib(S, ERROR_COLOR, "Error: Cannot find SOCKS5 proxy %s\n", 
					configuration.proxyhostname);
				S->connect_status = -1;
				return(0);
			}
			else {
				vprint_server_attrib(S, MESSAGE_COLOR, "Connecting to SOCKS5 proxy %s (%s) port %d ...\n", 
					host->h_name, inet_ntoa(*((struct in_addr*)host->h_addr)), 
					configuration.proxyport);
				S->connect_status = 2;
				return(1);
			}
		}

		else if (S->connect_status == 2){
			their_addr.sin_family = AF_INET;                                            
			their_addr.sin_port = htons(configuration.proxyport); 
			their_addr.sin_addr = *((struct in_addr *)host->h_addr);
			bzero(&(their_addr.sin_zero), 8);
		        if (connect (S->proxyfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
				vprint_server_attrib(S, ERROR_COLOR, "Failed to connect to proxy %s (%s) on port %d ...\n", 
					host->h_name, inet_ntoa(*((struct in_addr *)host->h_addr)), 
					configuration.proxyport);
				S->connect_status = -1;
				return(0);
			}
			vprint_server_attrib(S, MESSAGE_COLOR, "Connected to proxy %s (%s) on port %d ...\n", 
				host->h_name, inet_ntoa(*((struct in_addr*)host->h_addr)), configuration.proxyport);
			S->connect_status = 3;
			S->proxyactive = 1;
			return(1);
		}

		else if (S->connect_status == 3){
			vprint_server_attrib(S, MESSAGE_COLOR, "Negotiating authentication methods ...\n");

			/* create SOCK5 connect message */
			buffer[0] = 5;
			buffer[1] = 2; /* NUMBER OF METHODS */
			buffer[2] = 0; /* NO AUTHENTICATION */
			buffer[3] = 2; /* USERNAME / PASSWORD */
			buffer[4] = 1; /* GSS API */
			send_ball(S->proxyfd, buffer, 4);

			/* at this point, wait until the proxy replies and advances status */
			S->connect_status = 4;
			return(1);
		}

		else if (S->connect_status == 8){
			vprint_server_attrib(S, MESSAGE_COLOR, "Sending username and password ...\n");

			/* create username / password message */
			userlen = (unsigned char) strlen(configuration.proxyusername);
			passlen = (unsigned char) strlen(configuration.proxypassword);

			buffer[0] = 1; /* METHOD VERSION */
			buffer[1] = userlen; /* USERLEN */
			buffer[2 + userlen] = passlen; /* PASSLEN */
			memcpy(&buffer[2], configuration.proxyusername, userlen); /* USERNAME */
			memcpy(&buffer[3 + userlen], configuration.proxypassword, passlen); /* PASSWORD */
			send_ball(S->proxyfd, buffer, 3 + userlen + passlen);

			/* at this point, wait until the proxy replies and advances status */
			S->connect_status = 9;
			return(1);
		}

		else if (S->connect_status == 10){
			vprint_server_attrib(S, MESSAGE_COLOR, "Looking up IRC server %s ...\n", S->server);
			S->connect_status = 11;
			return(1);
		}

		else if (S->connect_status == 11){
			if ((host = gethostbyname(S->server)) == NULL) {
				vprint_server_attrib(S, ERROR_COLOR, "Error: Cannot find server %s\n", S->server);
				S->connect_status = -1;
				return(0);
			}
			else {
				vprint_server_attrib(S, MESSAGE_COLOR, "Requesting proxy connection to %s (%s) port %d ...\n", 
					host->h_name, inet_ntoa(*((struct in_addr*)host->h_addr)), S->port);
				S->connect_status = 12;
				return(1);
			}
		}

		else if (S->connect_status == 12){
			/* create SOCK5 connect message */
			buffer[0] = 5;
			buffer[1] = 1; /* CONNECT */
			buffer[2] = 0; /* RESERVED */
			buffer[3] = 1; /* ADDRESS TYPE - IPv4 */
			memcpy(&buffer[4], (struct in_addr*)host->h_addr, 4);
			buffer[8] = (S->port & 0xff00) >> 8;
			buffer[9] = (S->port & 0xff);
			send_ball(S->proxyfd, buffer, 10);

			/* at this point, wait until the proxy replies */
			S->connect_status = 13;
		}

		if (S->connect_status == 15){
			vprint_server_attrib(S, MESSAGE_COLOR, "Logging on as %s ...\n", S->nick);

			S->nickinuse = 1;
			sprintf(buffer,"USER %s %s %s :%s\n",S->user, S->host, S->domain, S->name);
			send_all(S->serverfd, buffer, strlen(buffer));
			if (strlen(S->password)){
				sprintf(buffer,"PASS %s\n", S->password);
				send_all(S->serverfd, buffer, strlen(buffer));
			}
			sprintf(buffer,"NICK :%s\n",S->nick);
			send_all(S->serverfd, buffer, strlen(buffer));
			sprintf(buffer,"MODE %s +%s\n",S->nick, configuration.mode);
			send_all(S->serverfd, buffer, strlen(buffer));
			S->connect_status = -1;
			if (alarm_occured) return (0);
			else {
				fcntl(S->serverfd, F_SETFL, O_NONBLOCK);
				S->active=1;
				set_statusline_update_status(statusline, U_ALL_REFRESH);
				screenupdated = 1;
				return(1);
			}
		}
		return(0);
	}	
	return(0);
}	

void sendcmd_server(server *S, char *command, char *args, char *dest, char *nick){
	char scratch[MAXDATASIZE];

	if (S == NULL) return;	
	if (strlen(nick)==0 && strlen(dest)==0 && strlen(args)==0){
		sprintf(scratch, "%s\n", command); 
	}
	else if (strlen(nick)==0 && strlen(dest)==0){
		sprintf(scratch, "%s :%s\n", command, args); 
	}
	else if (strlen(args)==0 && strlen(dest)>0){
		sprintf(scratch, ":%s %s %s\n", nick, command, dest); 
	}
	else {
		sprintf(scratch, ":%s %s %s :%s\n", nick, command, dest, args); 
	}
	// print_server((char *)S, scratch);
	send_all(S->serverfd, scratch, strlen(scratch));

}

void sendmsg_channel(channel *C, char *message){
	if (C!=NULL && message!=NULL){
		 sendcmd_server(C->server, "PRIVMSG", message, C->channel, (C->server)->nick);
	}
}

void sendmsg_chat(chat *C, char *message){
	if (C!=NULL && message!=NULL){
		 sendcmd_server(C->server, "PRIVMSG", message, C->nick, (C->server)->nick);
	}
}

void sendmsg_dcc_chat(dcc_chat *C, char *message){
	char scratch[MAXDATASIZE];
	
	if (C!=NULL && message!=NULL){
		sprintf(scratch, "%s\n", message); 
		send_all(C->dccfd, scratch, strlen(scratch));
	}
}

void send_server(server *S, char *template, ...){
	char message[MAXDATASIZE];
	char scratch[MAXDATASIZE];
        va_list ap;

        va_start(ap, template);
        vsprintf(message, template, ap);
        va_end(ap);
	sprintf(scratch, "%s\n", message);
	send_all(S->serverfd, scratch, strlen(scratch));
}

