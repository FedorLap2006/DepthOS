long _syscall0(long no) {
  long val;
  __asm__ volatile("int $0x64" : "=r"(val) : "a"(no));
  return val;
}

long _syscall1(long no, long a0) {
  long val;
  __asm__ volatile("int $0x64" : "=r"(val) : "a"(no), "b"(a0));
  return val;
}
long _syscall2(long no, long a0, long a1) {
  long val;
  __asm__ volatile("int $0x64" : "=r"(val) : "a"(no), "b"(a0), "c"(a1));
  return val;
}
long _syscall3(long no, long a0, long a1, long a2) {
  long val;
  __asm__ volatile("int $0x64"
                   : "=r"(val)
                   : "a"(no), "b"(a0), "c"(a1), "d"(a2));
  return val;
}

void exit(long num) { _syscall0(0x1); }
long strlen(const char *s) {
  long i = 0;
  while (*s++)
    ++i;
  return i;
}

int write(int fd, const char *data, long n) {
  return _syscall3(0x2, fd, data, n);
}
int read(int fd, const char *data, long n) {
  return _syscall3(0x8, fd, data, n);
}

int fork() { return _syscall0(0x3); }

// void print(const char *data) { _syscall2(0x2, strlen(data), (long)data); }
void print(const char *data) { write(1, data, strlen(data)); }

struct thcreate_params {
  int flags;
  unsigned char join;
  void *stack;
  void *entry;
};

long spawn_thread(void *function, void *stack) {
  struct thcreate_params params = {
      .entry = function, .join = 0, .stack = stack};
  return _syscall1(0x5, &params);
}

void test_thread2() {
  print("Hello from another C thread!");
  _syscall1(0x6, 2);
}

void test_thread() {
  print("Hello from C thread!!!!!!");
  long thid = spawn_thread(test_thread2, 0x9FFFD000);
  /*print("Created a new thread (child) [");
        thid += '0';
  _syscall2(0x2, 1, &thid);
  print("]");*/

  // print("Hello from first C thread!");
  _syscall1(0x6, 1);
  exit(0);
}

int getc() {
  char buf;
  read(0, &buf, 1);
  return buf;
}

void putc(int c) { write(1, &c, 1); }

struct cmd_h {
  char *cmd;
  void (*handler)(int argc, char *argv[128]);
  int nofork;
};

void uname(int argc, char *argv[128]) {
  print("DEPTHOS_ARCTIC");

  if (argc == 2) {
    print(" i686");
  }
  print("\n");
}

void cat(int argc, char *argv[128]) {
  if (argc < 2) {
    print("NOPE");
    return;
  }
  int fd = _syscall1(0x9, argv[1]);
  if (fd < 0) {
    print("NOPE");
    return;
  }
  char buffer[256];
  buffer[read(fd, buffer, 256)] = 0;
  print(buffer);
  _syscall1(0xA, fd);
}

void exit_cmd(int argc, char *argv[128]) {
  *(unsigned long long *)(0x0) = 0xC0FFEE;
}

void clear(int argc, char *argv[128]) {}

struct cmd_h commands[] = {
    {"uname", uname, 0},
    {"cat", cat, 0},
    {"exit", exit_cmd, 1},
};

void dispatch_command(char cmd[256]) {
  char *argv[128];
  char *current = cmd;
  int argc = 0;
  for (int i = 0; i < strlen(cmd); i++) {
    if (cmd[i] == ' ') {
      cmd[i] = 0;
      argv[argc++] = current;
      current = cmd + strlen(argv[argc - 1]) + 1;
    }
  }
  argv[argc++] = current;

  for (int i = 0; i < sizeof(commands) / sizeof(*commands); i++) {
    if (strlen(argv[0]) != strlen(commands[i].cmd))
      continue;
    for (int j = 0; j < strlen(cmd); j++) {
      if (argv[0][j] != commands[i].cmd[j]) {
        goto next;
      }
    }
    if (!commands[i].nofork) {
      if (fork() == 0) {
        commands[i].handler(argc, argv);
        exit(0);
      }
    } else
      commands[i].handler(argc, argv);
    return;
  next:;
  }
}

void shell() {
  char cmd[256];
  char *ptr = cmd;
  print("$> ");
  while (1) {
    int c = getc();
    if (c == '\n') {
      *ptr = 0;
      print("\n");
      dispatch_command(cmd);
      ptr = cmd;
      *ptr = 0;
      print("$> ");
    } else if (c == '\b') {
      ptr--;
      putc(c);
      putc(' ');
      putc(c);
    } else {
      *ptr = c;
      putc(c);
      ptr++;
    }
  }
}

void _start() {
  // print("Hello from C!\n");
  print("Welcome to dash shell!\n");
  // int fd = _syscall1(0x9, "/etc/depthos.cfg");
  // char buf_cfg[40];
  // int n = read(fd, buf_cfg, 40);
  // buf_cfg[n] = 0;
  // print("Buffer contains=====================");
  // print(buf_cfg);
  // print("=====================");

  // long thid = spawn_thread(test_thread, 0x9FFFF000);
  // print("Created a new thread (main)");
  // thid += '0';
  //_syscall2(0x2, 1, &thid);
  // print("]");
  while (1) {
    shell();
  }
  // char buf;
  // for (;;) {
  //   read(0, &buf, 1);
  //   // print("EYO");
  //   write(1, &buf, 1);
  // }
  // exit(0);
}
