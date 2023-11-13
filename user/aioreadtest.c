#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "kernel/fcntl.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "kernel/param.h"
#include "user/user.h"

#define fail(msg) do {printf("FAILURE: " msg "\n"); failed = 1; goto done;} while (0);
static int failed = 0;

static void cleanup(void);
static void clearcache(void);
static void test20(void);
static void test1000(void);

int
main(int argc, char *argv[])
{
  cleanup();
  test20();
  test1000();
  exit(failed);
}

static int trashfile = 0;

static void
clearcache()
{
  char buf[BSIZE];
  int fd, blocks;

  if (!trashfile) {
    fd = open("trash.file", O_CREATE | O_WRONLY);
    if(fd < 0){
      printf("aiotest: cannot open trash.file for writing\n");
      exit(-1);
    }

    blocks = 0;
    while(blocks < NBUF){
      *(int*)buf = blocks;
      int cc = write(fd, buf, sizeof(buf));
      if(cc <= 0)
        break;
      blocks++;
    }
    trashfile = 1;
  } else {
    fd = open("trash.file", O_RDONLY);
    if(fd < 0){
      printf("aiotest: cannot open trash.file for reading\n");
      exit(-1);
    }

    blocks = 0;
    while(blocks < NBUF){
      int cc = read(fd, buf, sizeof(buf));
      if(cc <= 0)
        break;
      blocks++;
    }
  }
}

static void
cleanup(void)
{
  unlink("/small.file");
  unlink("/many.file.0");
  unlink("/many.file.1");
  unlink("/many.file.2");
  unlink("/many.file.3");
  unlink("/many.file.4");
}

static char* smallfile = "small.file";

static void
test20()
{
  char block[BSIZE];
  int fd, i, n;
  int buf[20];
  int status[20];

  fd = open(smallfile, O_CREATE | O_WRONLY);
  if(fd < 0){
    printf("aiotest: cannot open %s for writing\n", smallfile);
    exit(-1);
  }

  for(i = 0; i < 20; i++){
      *(int*)block = i;
      int cc = write(fd, block, sizeof(block));
      if(cc <= 0) {
        printf("aiotest: write error at block %d\n", i);
        exit(-1);
      }
  }

  close(fd);
  clearcache();

  fd = open(smallfile, O_RDONLY);
  if(fd < 0){
    printf("aiotest: cannot re-open %s for reading\n", smallfile);
    exit(-1);
  }

  n = 0;
  for(i = 0; i < 20; i++){
    status[i] = 0;
    int cc = aio_read(fd, buf+i, sizeof(int), i*BSIZE, status+i);
    if(cc < 0){
      printf("bigfile: async read error at block %d\n", i);
      exit(-1);
    }
    if (status[i] != 0)
      n++;
  }

  if (n == 20) {
    printf("aiotest: aio_read is not asynchronous\n");
    exit(-1);
  }

  while(1){
    sleep(1);
    n = 0;
    for (i = 0; i < 20; i++){
      if (status[i] != 0)
        n++;
    }
    if (n == 20)
        break;
  }

  for (i = 0; i < 20; i++){
    if(buf[i] != i){
      printf("aiotest: read the wrong data (%d) for block %d\n",
             buf[i], i);
      exit(-1);
    }
  }

  printf("test20: ok\n");
}

static void
test1000()
{
  char block[BSIZE];
  char manyfile[] = "many.file.N";
  int i, j, n, fds[5];
  int status[200];
  int buf[200];

  for (i = 0; i < 5; i++) {
    manyfile[10] = '0' + i;
    fds[i] = open(manyfile, O_CREATE | O_WRONLY);
    if(fds[i] < 0){
      printf("aiotest: cannot open %s for writing\n", manyfile);
      exit(-1);
    }
  }

  for (i = 0; i < 5; i++) {
    if (fork() == 0) {
      n = 0;
      for (j = 0; j < 200; j++){
        *(int*)block = j;
        int cc = write(fds[i], block, sizeof(block));
        if(cc <= 0) {
          printf("aiotest: write error for many.file.%d at block %d\n", i, j);
          exit(-1);
        }
      }
      close(fds[i]);
      exit(0);
    }
  }

  for (i = 0; i < 5; i++) {
    int st;
    wait(&st);
    if (st != 0) {
      exit(-1);
    }
  }

  for (i = 0; i < 5; i++)
    close(fds[i]);

  clearcache();

  for (i = 0; i < 5; i++) {
    manyfile[10] = '0' + i;
    fds[i] = open(manyfile, O_RDONLY);
    if(fds[i] < 0){
      printf("aiotest: cannot open %s for reading\n", manyfile);
      exit(-1);
    }
  }

  for (i = 0; i < 5; i++) {
    if (fork() == 0) {
      n = 0;
      for (j = 0; j < 200; j++){
        status[j] = 0;
        int cc = aio_read(fds[i], buf+j, sizeof(int), j*BSIZE, status+j);
        if(cc < 0){
          printf("aiotest: async read error at block %d\n", j);
          exit(-1);
        }
        if (status[j] != 0)
          n++;
      }

      if (n == 200) {
        printf("aiotest: aio_read is not asynchronous\n");
        exit(-1);
      }

      while(1){
        sleep(1);
        n = 0;
        for (j = 0; j < 200; j++){
          if (status[j] != 0)
            n++;
        }
        if (n == 200)
          break;
      }

      for (j = 0; j < 200; j++)
        if (buf[j] != j) {
          printf("aiotest: read the wrong data (%d) for block %d\n",
                 buf[j], j);
          exit(-1);
        }

      exit(0);
    }
  }

  for (i = 0; i < 5; i++) {
    int st;
    wait(&st);
    if (st != 0) {
      exit(-1);
    }
  }

  for (i = 0; i < 5; i++)
    close(fds[i]);

  printf("test1000: ok\n");
}

