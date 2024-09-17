#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include <fcntl.h>

typedef enum {
    BufferMethod=1,
    CharMethod=2
} ReadMethod;

#define ITERATIONS 1000

static int tot = 0;

int syntax(FILE* stream)
{
    fprintf(stream, "usage: readperf [char|buffer] file\n");
    fprintf(stream, " where 'buffer' reads a buffer and scans chars\n");
    fprintf(stream, " where 'char' reads a character at a time\n");
    fprintf(stream, " and 'file' is the file to read\n");
}

int read_file_with_buffer(const char* filename)
{
    char buffer[32760];
    int rc, i;
    int fd = open(filename, O_RDONLY);

    while ((rc = read(fd, buffer, sizeof(buffer)) > 0)) {
        for (i=0; i < rc; ++i) {
            tot += buffer[i];
        }
    }
    return tot;
}
int read_file_with_char(const char* filename)
{
    int rc, c; 
    int fd = open(filename, O_RDONLY);

    while ((rc = read(fd, &c, 1) > 0)) {
        tot += c;
    }  
    return tot;
}

int multi_read_with_buffer(const char* filename)
{
    int i;
    for (i=0; i<ITERATIONS; ++i) {
        tot += read_file_with_buffer(filename);
    }
    return 0;
}

int multi_read_with_char(const char* filename)
{
    int i;
    for (i=0; i<ITERATIONS; ++i) {
        tot += read_file_with_char(filename);
    }    
    return 0;
}

int main(int argc, char* argv[]) 
{
    const char* method_name = argv[1];
    const char* file_name = argv[2];
    ReadMethod method;

    if (argc < 2) {
        syntax(stdout);
    }
    if (!strcmp(method_name, "buffer")) {
        method = BufferMethod;
    } else if (!strcmp(method_name, "char")) {
        method = CharMethod;
    } else {
        syntax(stderr);
        return 4;
    }

    switch(method) {
        case BufferMethod: return multi_read_with_buffer(file_name);
        case CharMethod: return multi_read_with_char(file_name);
    }
    printf("Character summation (to avoid premature optimization): %d\n", tot);
    return 4;
}