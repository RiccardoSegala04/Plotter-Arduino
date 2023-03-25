#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#define MAX_LINE 256

void print_help() {
    printf("Plotter:\n");
    printf("    Use -f <path> to specify gcode input file to draw\n");
    printf("    Use -i <path> to specify the serial interface\n");
}

int serial_open(const char* path) {
    int fd = open(pathname, O_RDWR | O_NOCTTY);

    if(fd<0) {
        perror("Could not open serial port");
        exit(1);
    }

    struct termios settings;

    cfmakeraw(&settings);
    settings.c_cflag |= (CLOCAL | CREAD);
    settings.c_iflag &= ~(IXOFF | IXANY);

    settings.c_cc[VMIN] = 0; 
    settings.c_cc[VTIME] = 5;

    cfsetispeed(&settings, B9600);
    cfsetospeed(&settings, B9600);

    tcsetattr(fd, TCSANOW, &settings);

    return fd;
}

void plotter_print(const char* gcode, const char* interface) {

    int sfd = serial_open(interface);

    FILE* fp = fopen(gcode, "r");

    printf("Sending %s to the plotter in 3..", gcode);
    fflush(stdout);
    sleep(1);
    printf("2..");
    fflush(stdout);
    sleep(1);
    printf("1..");
    fflush(stdout);
    sleep(1);

    putchar('\n');

    char line[MAX_LINE];
    while(fgets(line, MAX_LINE, fp)!=NULL) {
        printf("%s", line);
        write(sfd, line, strlen(line));

        bzero(line, MAX_LINE);
        do {
            int n_read = read(sfd, line, MAX_LINE);
            puts(line);
        }while(strcmp(line, "OK\n"));
    }

}

int main(int argc, const char* argv[]) {

    char i_path[256];
    char f_path[256];

    bzero(i_path, 256);
    bzero(f_path, 256);

    int option;
    while((option = getopt(argc, (char* const*)argv, ":i:f:h")) != -1) {
        switch(option) {
            case 'i':
                if(optarg!=NULL)
                    strcat(i_path, optarg);
                else {
                    printf("Option -i needs an argument!\n");
                    print_help();
                    exit(-1);
                }
                break;
            case 'f':
                if(optarg!=NULL)
                    strcat(f_path, optarg);
                else {
                    printf("Option -f needs an argument!\n");
                    print_help();
                    exit(-1);
                }
                break;
            case 'h':
                print_help();
                exit(0);
                break;
            case ':':
                return -1;
            default:
                printf("Invalid option.\n");
                print_help();
                exit(-1);
        }
    }

    if(strlen(i_path)==0 || strlen(f_path)==0) {
        fprintf(stderr, "Error!\n");
        print_help();
        exit(-1);
    }

    if(access(f_path, R_OK) || access(i_path, R_OK | W_OK)) {
        fprintf(
            stderr, "Could not read from %s: %s\n", argv[1], strerror(errno));
    } 

    plotter_print(f_path);
    
    return 0;
}
