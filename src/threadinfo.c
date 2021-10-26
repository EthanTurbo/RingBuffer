#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <execinfo.h>
#include "threadinfo.h"
 
void out_stack(char *sig);
 
void signal_exit(int dunno) 
{ 
	char* signal_str = "";
	char dunno_str[10] = {0};
	sprintf(dunno_str, "%d", dunno);
	switch (dunno) 
	{
		case 1:
			signal_str = "SIGHUP(1)";
			break;
		case 2:
			signal_str = "SIGINT(2:CTRL_C)"; //CTRL_C
			break;
		case 3:
			signal_str = "SIGQUIT(3)";
			break;
		case 6:
		{
			signal_str = "SIGABRT(6)";
			out_stack(signal_str);
		}
		break;
		case 9:
			signal_str = "SIGKILL(9)";
			break;
		case 15:
			signal_str = "SIGTERM(15 KILL)"; //kill 
			break;
		case 11:
		{
			signal_str = "SIGSEGV(11)"; //SIGSEGV 
			out_stack(signal_str);
		}
		break;	
		default:
			signal_str = "OTHER";
			break;
	}
	exit(0);
}
 
static void output_addrline(char addr[])
{
	char cmd[256];
	char line[256];
	char addrline[32]={0,};
	char *str1, *str2;
	FILE* file;
 
	str1 = strchr(addr,'[');
	str2 = strchr(addr, ']');
	if(str1 == NULL || str2 == NULL)
	{
		return;
	}	
	memcpy(addrline, str1 + 1, str2 -str1);
	snprintf(cmd, sizeof(cmd), "addr2line -e /proc/%d/exe %s ", getpid(), addrline);
	file = popen(cmd, "r");
	if(NULL != fgets(line, 256, file)) 
	{
		printf("%s\n", line);
	}
	pclose(file);	
}
void out_stack(char *sig)
{
	void *array[32];
	size_t size;
	char **strings;
	int i;
	
	printf("%s\n", sig);
	size = backtrace (array, 32);
	strings = backtrace_symbols (array, size);
	if (NULL == strings)
	{
		printf("backtrace_symbols\n");
		return ;
	}
	
	for (i = 0; i < size; i++)
	{
		printf("%s",strings[i]);
		output_addrline(strings[i]);	
	}
	
	free(strings);
}
void test3(int n)
{
	char *str;
	printf("in test3 [%d]\n", n);
	strcpy(str, "123");
}
void test2(int n)
{
	printf("in test2 [%d]\n", n);
	test3(3);
}
void test1(int n)
{
	printf("in test1 [%d]\n", n);
	test2(2);
}
int register_signal()
{
	signal(SIGHUP, signal_exit); 
	signal(SIGINT, signal_exit);
	signal(SIGQUIT, signal_exit);
	signal(SIGABRT, signal_exit);
	signal(SIGKILL, signal_exit);
	signal(SIGTERM, signal_exit);
	signal(SIGSEGV, signal_exit);
	
	//test1(1);
}