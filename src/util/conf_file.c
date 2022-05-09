#include "conf_file.h"

#include <assert.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "alg_ds/alg/defer.h"

const char* conf_file = "/usr/local/flexric/flexric.conf";
const char* libs_dir = "/usr/local/flexric/";

static
bool valid_ip(const char* ip)
{
  assert(ip != NULL);
  if (strlen(ip) < 7) // 8.8.8.8, at least we need 7 characters  
    return false;

  struct sockaddr_in sa;
  int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
  return result != 0;
}


static
char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

static
char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}
/*
static
char *trim(char *s)
{
    return rtrim(ltrim(s)); 
}
*/

char* get_near_ric_ip(args_t args)
{
  char* line = NULL;
  defer({free(line);});
  size_t len = 0;
  ssize_t read;

  FILE * fp = fopen(args.conf_file, "r");
  
  if (fp == NULL){
    printf("%s not found. Did you forget to sudo make install?\n", args.conf_file);
    exit(EXIT_FAILURE);
  }
  
  defer({fclose(fp); } );
  
  char ip_addr[24] = {0};
  while ((read = getline(&line, &len, fp)) != -1) {
    const char* needle = "NEAR_RIC_IP =";
    char* ans = strstr(line, needle);
    if(ans != NULL){
      ans += strlen(needle); 
      ans = ltrim(ans);
      ans = rtrim(ans);
      memcpy(ip_addr, ans , strlen(ans)); // \n character
      break;
    }    
  }
    
  if(valid_ip(ip_addr) == false){
    printf("IP address string invalid = %s Check the config file\n",ip_addr);
    exit(EXIT_FAILURE);
  }
  
  return strdup(ip_addr);
}

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void init_args(args_t* args)
{
  strcpy(args->conf_file,conf_file);
  strcpy(args->libs_dir, libs_dir);
   
}
void print_usage(const char* prog)
{
  printf("Usage: %s [options]\n", prog);
  printf("\n");
  printf("    General options:\n");
  printf("  -h         : print usage\n");
  printf("  -c         : path to the config file\n");
  printf("  -p         : path to the shared libs \n");
}  

int parse_args(int argc, char** argv, args_t* args)
{
  int opt;
  init_args(args);
  
  while((opt = getopt(argc, argv, "hc:p:")) != -1) {
    switch(opt) {
    case 'h':
      return 1;
      
    case 'c':
      strcpy(args->conf_file,optarg);
      break;

    case 'p':
      strcpy(args->libs_dir,optarg);
      break;
    default:
      return 1;
    }
  }
  printf("setting the config file to %s\n",args->conf_file);
  printf("setting path for the shared libraries to %s\n",args->libs_dir);
 return 0;
}
