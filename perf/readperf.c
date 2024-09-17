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

static void syntax(FILE* stream)
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
int read_file_with_char(const char* filename)
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

int multi_read_with_buffer(const char* filename)
{
    int i;
    for (i=0; i<ITERATIONS; ++i) {
        tot += read_file_with_buffer(filename);
    }
    return tot;
}

int multi_read_with_char(const char* filename)
{
    int i;
    for (i=0; i<ITERATIONS; ++i) {
        tot += read_file_with_char(filename);
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