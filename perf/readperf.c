#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

typedef enum {
    FDBufferMethod=1,
    FDCharMethod=2,
    FPBufferMethod=3,
    FPCharMethod=4
} ReadMethod;

#define ITERATIONS 1000

static int tot = 0;

static void syntax(FILE* stream)
{
    fprintf(stream, "usage: readperf [fd-char|fd-buffer|fp-char|fp-buffer] file\n");
    fprintf(stream, " where 'fd-buffer' reads a buffer and scans chars\n");
    fprintf(stream, " where 'fd-char' reads a character at a time\n");
    fprintf(stream, " where 'fp-buffer' freads a buffer and scans chars\n");
    fprintf(stream, " where 'fp-char' freads a character at a time\n");
    fprintf(stream, " and 'file' is the file to read\n");
}

int fd_read_file_with_buffer(const char* filename)
{
    char buffer[32760];
    int rc, i;
    int fd = open(filename, O_RDONLY);

    while (1) {
        rc = read(fd, buffer, sizeof(buffer));
        if (rc <= 0) {
            break;
        }
        for (i=0; i < rc; ++i) {
            tot += buffer[i];
        }
    }
    close(fd);
    return tot;
}

int fd_read_file_with_char(const char* filename)
{
    int rc, c, fd; 
    fd = open(filename, O_RDONLY);
    while (1) {
       read(fd, &c, 1);
       if (c < 0) {
        break;
       }
       tot += c;
    }  
    close(fd);
    return tot;
}

int fd_multi_read_with_buffer(const char* filename)
{
    int i;
    for (i=0; i<ITERATIONS; ++i) {
        tot += fd_read_file_with_buffer(filename);
    }
    return tot;
}

int fd_multi_read_with_char(const char* filename)
{
    int i;
    for (i=0; i<ITERATIONS; ++i) {
        tot += fd_read_file_with_char(filename);
    }    
    return tot;
}

int fp_read_file_with_buffer(const char* filename)
{
    char buffer[32760];
    int rc, i;
    FILE* fp = fopen(filename, "rb");

    while (1) {
        rc = fread(buffer, 1, sizeof(buffer), fp);
        if (rc <= 0) {
            break;
        }
        for (i=0; i < rc; ++i) {
            tot += buffer[i];
        }
    }
    fclose(fp);
    return tot;
}

int fp_read_file_with_char(const char* filename)
{
    int rc, c; 
    FILE* fp = fopen(filename, "rb");
    while (1) {
       c = getc(fp);
       if (c < 0) {
        break;
       }
       tot += c;
    }  
    fclose(fp);
    return tot;
}

int fp_multi_read_with_buffer(const char* filename)
{
    int i;
    for (i=0; i<ITERATIONS; ++i) {
        tot += fp_read_file_with_buffer(filename);
    }
    return tot;
}

int fp_multi_read_with_char(const char* filename)
{
    int i;
    for (i=0; i<ITERATIONS; ++i) {
        tot += fp_read_file_with_char(filename);
    }    
    return tot;
}

int main(int argc, char* argv[]) 
{
    const char* method_name = argv[1];
    const char* file_name = argv[2];
    ReadMethod method;

    if (argc < 3) {
        syntax(stdout);
        return 0;
   }
    if (!strcmp(method_name, "fd-buffer")) {
        method = FDBufferMethod;
    } else if (!strcmp(method_name, "fd-char")) {
        method = FDCharMethod;
    } else if (!strcmp(method_name, "fp-buffer")) {
        method = FPBufferMethod;
    } else if (!strcmp(method_name, "fp-char")) {
        method = FPCharMethod;
    } else {
        syntax(stderr);
        return 4;
    }

    switch(method) {
        case FDBufferMethod: return fd_multi_read_with_buffer(file_name);
        case FDCharMethod: return fd_multi_read_with_char(file_name);
        case FPBufferMethod: return fp_multi_read_with_buffer(file_name);
        case FPCharMethod: return fp_multi_read_with_char(file_name);
    }
    printf("Character summation (to avoid premature optimization): %d\n", tot);
    return 4;
}
