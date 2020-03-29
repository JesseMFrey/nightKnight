/*
 * terminal.h
 *
 *  Created on: Aug 24, 2017
 *      Author: jmf6
 */

#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdlib.h>

#define TERM_MAX_LINE   80

//structure for terminal status
typedef struct{
    //line buffer
    char linebuf[TERM_MAX_LINE];
    unsigned int cIdx;
}TERM_DAT;

//structure for describing commands
 typedef struct{
   const char* name;
   const char* helpStr;
   //function pointer to command
   int (*cmd)(int argc,char **argv);
 }CMD_SPEC;

 //table of commands with help
 extern const CMD_SPEC cmd_tbl[];

//process a single char for terminal
void terminal_proc_char(char c,TERM_DAT *data);

//process a block of data can run multiple commands
void terminal_proc_block(const char *inbuf,size_t num,TERM_DAT *data);

//help command
int helpCmd(int argc,char **argv);

//initialize terminal structure
void terminal_init(TERM_DAT *data);

#endif


