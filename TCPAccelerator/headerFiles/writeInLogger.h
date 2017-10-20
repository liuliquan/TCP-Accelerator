//
// Created by root on 10/19/17.
//
#ifndef headerIncluded
#include "headers.h"
#endif

#ifndef structIncluded
#include "types.h"
#endif

/*define variables*/
struct sockaddr_in source,dest;

void print_tcp_packet(unsigned char* Buffer, int Size, FILE *logfile);


void print_ip_header(unsigned char* Buffer, int Size,FILE *logfile);


void print_ethernet_header(unsigned char* Buffer, int Size,FILE *logfile);


void PrintData (unsigned char* data , int Size,FILE *logfile);
