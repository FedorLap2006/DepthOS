/*
 * mdinit - Mini DepthOS init
 *
 * Copyright Fedor Lapshin 2023
 */

#include "bits/posix/posix_stdlib.h"
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MIN(a, b) ((a) > (b) ? b : a)
#define SERVICE_DIR "/etc/init.d/"
#define DEFAULT_SYSTEM_SHELL "/usr/bin/bash"
#define BUILTIN_SYSTEM_SHELL "/usr/bin/bash"

#define BASH_TESTING_BIN "/usr/bin/bash"
#define BASH_TESTING_ARGS                                                      \
  { "/usr/bin/bash", "-i", NULL }


#define _GENERIC_TESTING_ARGS                                                  \
  { TESTING_BIN, NULL }

#define TESTING_BIN BASH_TESTING_BIN
#define TESTING_ARGS BASH_TESTING_ARGS

static inline void debuglog(const char *fmt, ...) {
  char buf[4096];
  va_list l;
  va_start(l, fmt);
  vsnprintf(buf, 4096, fmt, l);
  va_end(l);
#if defined(DEBUG)
  printf("debug: ");
  puts(buf);
#endif
}

struct service {
  char *name;
  char *sh;

  char *bin;
  char **eargs;
  int eargs_len;
};

int parse_systemd_service(FILE *f, struct service *svc) {}
int parse_service(FILE *f, struct service *svc) {
  char buffer[4096];
  char name[256];
#define value (buffer + namelen + 1)
  int buflen, namelen;

  do {
    if (!fgets(buffer, 4096, f))
      break;
    debuglog("buffer: '%s'", buffer);
    buflen = strlen(buffer);
    if (buffer[0] == '\n' || buffer[0] == '=')
      continue;
    else if (!buflen)
      break;
    if (buffer[buflen - 1] == '\n')
      buffer[--buflen] = 0;

    namelen = MIN(strstr(buffer, "=") - buffer, 255);
    memcpy(name, buffer, namelen);
    name[namelen] = 0;
    debuglog("startup service config: name: %s %p\n", name, name);
    debuglog("startup service config: value: %s %p\n", value, value);
#define scase(str) if (strcmp(name, str) == 0)
    scase("sh") svc->sh = strdup(value);
    scase ("eargs[]") {
      if (!svc->eargs) {
        svc->eargs = malloc(sizeof(char*) * 1025);
      }
      assert(svc->eargs_len <= 1024);
      debuglog("eargs[%d++] = %s", svc->eargs_len, value);
      svc->eargs[svc->eargs_len++] = strdup(value);
    }
    scase("bin") svc->bin = strdup(value);
#undef scase
  } while (!feof(f)); // TODO: does fgets set eof?
  
  // if (svc->sh)
  //   printf("Shell command: %s\n", svc->sh);

  if (svc->eargs)
    svc->eargs[svc->eargs_len] = NULL;
  return svc->sh != NULL || svc->bin != NULL || svc->eargs_len > 0; // TODO: others when they will be added
  // return (struct service){.sh = "echo hello world"};
}

__attribute__((noreturn)) void service_entry(struct service service) {
  printf("Running service: %s\n", service.name);
  debuglog("service entry\n");
  debuglog("attempting to run %s (%p)\n", service.sh, service.sh);
  if (service.sh)
    execve("/bin/bash", (char *[]){"/bin/bash", "-c", service.sh, NULL}, environ);
  else if (service.bin && service.eargs_len == 0)
    execve(service.bin, (char *[]){service.bin, NULL}, environ);
  else if (service.bin) {
    for (int i = 0; i < service.eargs_len; i++) {
      debuglog("service arg: %s", service.eargs[i]);
    }
    execve("/bin/bash", service.eargs, environ);
  }

  __builtin_unreachable();
}

void spawn_services(DIR *autorundir) {
  struct dirent *unit;
  while ((unit = readdir(autorundir))) {
    // printf("%s %d %d\n", unit->d_name, unit->d_type, strcmp(".",
    // unit->d_name));
    if (unit->d_type != DT_REG ||
        strcmp("..", unit->d_name) >= 0) // TODO: DT_LNK
      continue;
    char *ext = strrchr(unit->d_name, '.');
    char path[sizeof(unit->d_name) /* 1024 */ + sizeof(SERVICE_DIR)];
    strcpy(path, SERVICE_DIR);
    strcat(path, unit->d_name);

    struct service svc = {0};
    debuglog("fopen: %s\n", path);
    svc.name = (char*)malloc(ext - unit->d_name + 1);
    memcpy(svc.name, unit->d_name, ext - unit->d_name);
    svc.name[ext - unit->d_name] = 0;
    FILE *file = fopen(path, "r");
    if (!ext || !strcmp(ext, ".dsvc")) {
      if (!parse_service(file, &svc))
        printf("Error: could not parse nconfiguration file\n");
    } else if (!strcmp(ext, ".service")) {
      if (!parse_systemd_service(file, &svc))
        printf("Error: could not parse systemd configuration file\n");
    } else {
      debuglog("unknown service file extension: %s\n", ext);
      continue;
    }
    fclose(file);

    if (fork() == 0)
      service_entry(svc);
  }
}

int main(int argc, char *argv[]) {
  // FILE *tty = fopen("/dev/tty", "r+");
  // fwrite("hello", 1, 4, tty);
  printf("DepthOS init system 0.1.0\n");
  printf("Copyright 2023-2024 Fedor Lapshin.\n\n");


  setenv("PATH", "/bin:/usr/bin", 1);
  setenv("HOME", "/root", 1);
  // setenv("PWD", "/", 1);
  setenv("LC_ALL", "C", 1);

  // write(1, "TEST TEST", sizeof("TEST TEST") - 1);
#if defined(TESTING_BIN) && defined(TESTING_ARGS)
  if (fork() == 0) {
    int e = execve(TESTING_BIN, (char *[])TESTING_ARGS, environ);
    if (e) {
      printf("Could not spawn the application: %d %s\n", errno,
             strerror(errno));
      return 1;
    }
  }
#else

  DIR *svcdir = opendir(SERVICE_DIR);
  if (svcdir) {
    // printf("Spawning services...\n");
    spawn_services(svcdir);
  } else {
    printf("Service directory " SERVICE_DIR
           " does not exist. Attempting to run default "
           "shell: " DEFAULT_SYSTEM_SHELL
           "\n"); // TODO: fetch system shell from services somehow, but not
                  // from a hardcoded macro.
    int e = execve(DEFAULT_SYSTEM_SHELL, (char *[]){DEFAULT_SYSTEM_SHELL, NULL},
                   environ);
    if (e < 0) {
      printf("Could not run default shell (errno=%d). Falling back to builtin "
             "(" BUILTIN_SYSTEM_SHELL ")\n",
             -errno);
      e = execve(BUILTIN_SYSTEM_SHELL, (char *[]){BUILTIN_SYSTEM_SHELL, NULL},
                 environ);
      if (e < 0) {
        printf("Could not run builtin shell. You're on your own.\n");
        // exit(1);
      }
    }
  }
#endif
  while (1) {
    // sleep(1);
    // printf("Still alive!\n");
    // __asm__ volatile("int $0x30");
  }
  return 0;
}
