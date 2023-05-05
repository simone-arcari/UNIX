/*

this program cuts all the emails it finds in the file provided as input and writes them to the cutted.txt file

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_PARAM   2
#define MAX_LEN     1024
#define CHIOCCIOLA  '@'

int main(int argc, char** argv) {
    char buffer[MAX_LEN];
    char* filename;
    char* output_filename = "cutted.txt";
    FILE* file;
    FILE* output;
    int len;

    if(argc != NUM_PARAM) {
        printf("error number of param\n");
        printf("usage: ./prog_name filename\n");
        exit(EXIT_FAILURE);
    
    }

    filename = argv[1];


    file = fopen(filename, "r+");
    output = fopen(output_filename, "w+");
    if(file == NULL || output == NULL) {
        perror("error fopen()");
        exit(EXIT_FAILURE);

    }

    while(fscanf(file, "%s", buffer ) != EOF) {

        len = strlen(buffer);
        for(int i=0; i<len; i++) {
            if(buffer[i] == CHIOCCIOLA){
                fprintf(output, "%s\n", buffer);
                fflush(output);
                break;
            }
        }
    }
    


    return EXIT_SUCCESS;

}
