#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

// #include "user/ls.c"
char* fmtname(char *path) {
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), '\0', DIRSIZ-strlen(p));
  return buf;
}

void find(char *path, char *findFile) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    fd = open(path, 0);

    fstat(fd, &st);

    switch (st.type) {
        case T_DEVICE:
        case T_FILE:

        case T_DIR:
            strcpy(buf, path);
            p = buf+strlen(buf);
            *p++ = '/';
            while(read(fd, &de, sizeof(de)) == sizeof(de)){
                if(de.inum == 0)
                    continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                stat(buf, &st);

                if (strcmp(fmtname(buf), findFile) == 0) {
                    printf("%s\n", buf);
                }
                if (st.type == T_DIR && !(strcmp(fmtname(buf), ".") == 0 || strcmp(fmtname(buf), "..") == 0)) {
                    find(buf, findFile);
                }
            }
            break;
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc <= 2)
    {
        fprintf(2, "usage: find [dirname] [filename]\n");
        exit(1);
    }

    find(argv[1], argv[2]);

    exit(0);
}