#ifdef _WIN32
#include <io.h>    // for _commit
#define fsync(fd) _commit(fd)
#endif
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<stddef.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<errno.h>

#pragma pack(push, 1)
typedef struct Record{
     uint32_t id;
     double value;
     char name[32];
}Record;
#pragma pack(pop)

static ssize_t read_all(int fd, void *buffer, size_t count)
{
    uint8_t *pointer = buffer;
    size_t left = count;

    while (left > 0)
    {
        ssize_t bytes_read = read(fd, pointer, left);

        if(bytes_read < 0)
        {
            if(errno == EINTR) continue;
            return -1;
        }

        if(bytes_read == 0)
        {
            break; //EndOfFile
        }

        left -= (size_t)bytes_read;
        pointer += bytes_read;
    }

    return (ssize_t)(count - left);
}

static ssize_t write_all(int fd, const void *buffer, size_t count){
     const uint8_t *pointer = buffer;
     size_t left = count;

     while(left>0)
     {
          ssize_t bytes_written = write(fd, pointer, left);
          if(bytes_written<0){
               if(errno==EINTR) continue;
               return 1;
          }
          left -= bytes_written;
          pointer += bytes_written;
     }
     return (ssize_t)count;
}

int main(int argc, char *argv[]){
     if(argc!=3)
     {
          fprintf(stderr, "%s <src.txt> <dest.txt>", argv[0]);
          return 1;
     }

     const char *srcfilePath = argv[1];
     const char *destfilePath = argv[2];

     int fd_read = open(srcfilePath, O_RDONLY);
     if(fd_read<0)
     {
          perror("src open");
          return 1;
     }

     int fd_write = open(destfilePath, O_WRONLY | O_CREAT | O_TRUNC, 0664);
     if(fd_write < 0)
     {
          perror("open destination file");
          return 1;
     }

     const int buffer_size = 4096;
     uint8_t buffer[buffer_size];
     ssize_t read_bytes;
     while((read_bytes = read(fd_read, buffer, sizeof(buffer)))>0)
     {
          if(write_all(fd_write, buffer, read_bytes)!=read_bytes)
          {
               perror("write to dest");
               close(fd_read);
               close(fd_write);
               return 1;
          }
     }

     if(read_bytes<0)
     {
          perror("read");
          close(fd_read);
          close(fd_write);
          return 1;
     }

     close(fd_read);
     close(fd_write);

     printf("file %s is copied to file %s\n", srcfilePath, destfilePath);
     return 0;
}