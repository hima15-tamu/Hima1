//
// test program for the alarm lab.
// you can modify this file for testing,
// but please make sure your kernel
// modifications pass the original
// versions of these tests.
//

#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "user/user.h"

void testthreadrun();
void testthreadmore();
void testthreadsbrk();

int
main(int argc, char *argv[])
{
  testthreadrun();
  testthreadmore();
  testthreadsbrk();
  exit(0);
}

volatile int count1 = 0;
volatile int count2 = 0;

void
thread1()
{
  for(int i = 0; i < 100; i++) count1++;
  exit(0);
}

void
thread2()
{
  for(int i = 0; i < 100; i++) count2++;
  exit(0);
}

void
testthreadrun()
{
  void* stack1 = malloc(PGSIZE);
  void* stack2 = malloc(PGSIZE);
  int status;

  if (clone(&thread1, stack1) == -1)
    exit(1);
  if (clone(&thread2, stack2) == -1)
    exit(1);

  wait(&status);
  wait(&status);

  if (count1 == 100 && count2 == 100) {
    printf("testthreadrun passed\n");
  }
}

volatile int count3 = 0;

void
thread3()
{
  count3++;
  if (count3 < 8) {
    void* stack = malloc(PGSIZE);
    int status;
    if (clone(thread3, stack) == -1)
      exit(1);
    wait(&status);
    if (status < 0)
      exit(1);
  }
  exit(0);
}

void
testthreadmore()
{
  void* stack = malloc(PGSIZE);
  int status;

  if (clone(thread3, stack) == -1)
    exit(1);
  wait(&status);

  if (status == 0 && count3 == 8) {
    printf("testthreadmore passed\n");
  }
}


void
thread4()
{
  char* c = sbrk(PGSIZE);
  if(c == (char*)0xffffffffffffffffL){
    printf("%s: failed sbrk in thread\n", c);
    exit(1);
  }

  *c = 255;
  exit(0);
}

void
testthreadsbrk()
{
  void* stack1 = malloc(PGSIZE);
  void* stack2 = malloc(PGSIZE);
  int status;
  char* s = sbrk(0);
  char* e;

  clone(&thread4, stack1);
  clone(&thread4, stack2);

  wait(&status);
  wait(&status);

  e = sbrk(0);

  if (e == s + PGSIZE * 2 && s[0] == 255 && s[PGSIZE] == 255) {
    printf("testthreadsbrk passed\n");
  }
}
