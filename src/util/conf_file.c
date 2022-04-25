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

static
bool valid_ip(const char* ip)
{
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

char* get_near_ric_ip(void)
{
  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  char filename[256] = {0};
  const char* dir = SERVICE_MODEL_DIR_PATH;

  int const n = snprintf(filename, 255, "%sflexric.conf", dir );
  assert(n < 255 && "Overflow in the path");


  FILE * fp = fopen(filename, "r");
  if (fp == NULL){
    printf("/usr/lib/flexric/flexric.conf not found. Did you forget to sudo make install?");
    exit(EXIT_FAILURE);
  }

  char ip_addr[24] = {0};
  while ((read = getline(&line, &len, fp)) != -1) {
    const char* needle = "NEAR_RIC_IP =";
    char* ans = strstr(line, needle);
    if(ans != NULL){
      ans += strlen(needle); 
      ans = ltrim(ans);
      ans = rtrim(ans);
      memcpy(ip_addr, ans , strlen(ans)); // \n character         

      if(valid_ip(ip_addr) == false){
        printf("Invalid IP address = %s \n",ip_addr);
        fclose(fp);
        exit(EXIT_FAILURE);
      }
    }
  }

  fclose(fp);
  if (line)
    free(line);

  return strdup(ip_addr);
}

