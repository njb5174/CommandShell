#ifndef PR6_CTIME_H
#define PR6_CTIME_H

/* CMPSC 311 Project 6 toolkit
 *
 * This file is provided as part of the project description and starter kit.
 * If you modify it, then put your name, email address, and the date, in this
 * comment, and include this file in the materials you turn in.
 */

/*----------------------------------------------------------------------------*/

/* current time as a character string */
char * Ctime(char buf[26]);

void print_msg(char *msg);
void print_msg_1(char *msg, int n);
void print_msg_2(char *msg, int n1, int n2);
void print_msg_error(char *msg, char *errmsg);
void print_msg_abort(char *msg);

/*----------------------------------------------------------------------------*/

#endif

