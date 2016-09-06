#ifndef LIGHTVIBRANT_H
#define LIGHTVIBRANT_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
//#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#include <cstdio>
#include <pthread.h>

/* assert(/tmp mounted to tmpfs, i.e. resides in RAM) */
/* just use any file in /tmp */
static const char* mmapFilePath = "/tmp/arduino";


typedef struct mmapData_TAG {
  int light;   // led on IO8
  int vibrant;  // built-in led
  pthread_mutex_t mutex;
  pthread_cond_t cond;
}MMAPDATA;

typedef MMAPDATA* MMAPDATA_HANDLE;

#endif

