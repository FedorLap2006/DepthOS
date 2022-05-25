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

void exit(long num) { _syscall0(0x1); }
long strlen(const char *s) {
  long i = 0;
  while (*s++)
    ++i;
  return i;
}

void print(const char *data) { _syscall2(0x2, strlen(data), (long)data); }

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
  print("Hello from another C thread!\n");
  _syscall1(0x6, 2);
}

void test_thread() {
  print("Hello from C thread!\n");
  long thid = spawn_thread(test_thread2, 0x9FFFD000);
  /*print("Created a new thread (child) [");
        thid += '0';
  _syscall2(0x2, 1, &thid);
  print("]\n");*/

  // print("Hello from first C thread!");
  _syscall1(0x6, 1);
}

void _start() {
  print("Hello from C!\n");
  long thid = spawn_thread(test_thread, 0x9FFFF000);
  // print("Created a new thread (main)");
  // thid += '0';
  //_syscall2(0x2, 1, &thid);
  // print("]");
  for (;;) {
  }
  // exit(0);
}
