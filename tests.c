#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: ./%s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }
    
    
    if(0){//CHECK_ARCHIVE
        printf("TEST CHECK_ARCHIVE----------------------------------------------------------------------------------------------------------------------------\n");
        printf("check_archive returned:%i \n",check_archive(fd));
   
    }

    if(0){//GET_BUFFER
        printf("TEST GET_BUFFER  normal file and symlink----------------------------------------------------------------------------------------------------------------------------\n");
        uint8_t* buffer = get_buffer(fd, "debug/folder1/file1.txt", 2);
        if (buffer == NULL) printf("NULL BUFFER from get_buffer\n");
        else
        {
            debug_dump(buffer, 512 * 2);
            blocktype(buffer);
            free(buffer);
        }
        buffer = get_buffer(fd, "debug/folder2/symlink_file1txt", 2);
        if (buffer == NULL) printf("NULL BUFFER from get_buffer\n");
        else
        {
            debug_dump(buffer, 512 * 2);
            blocktype(buffer);
            free(buffer);
        }
    }

    if(1){//READ_FILE
        printf("TEST READ_FILE  normal txt and symlink----------------------------------------------------------------------------------------------------------------------------\n");
        uint8_t* read_buffer = malloc(sizeof(uint8_t)* 512 *2);
        size_t* len = malloc(sizeof(size_t));
        *len = 512*2;
        ssize_t ret_readfile = read_file(fd, "debug/folder1/file1.txt", 1, read_buffer, len);
        debug_dump(read_buffer,*len);
        printf("read_file returned:%ld\n",ret_readfile);
            ret_readfile = read_file(fd, "debug/folder2/symlink_file1txt", 3, read_buffer, len);
        debug_dump(read_buffer,*len);
        printf("read_file returned:%ld\n",ret_readfile);
        free(read_buffer); free(len); 
    }

    if(0){//LIST
        printf("TEST LIST----------------------------------------------------------------------------------------------------------------------------------------------\n");
        fd = open(argv[1] , O_RDONLY);//si jenleve cette ligne list fonctionne plus 
        size_t* nb_entries = malloc(sizeof(size_t));
        *nb_entries = 5;
        char** entries = malloc(sizeof(char**)* (*nb_entries));
        for (size_t i = 0; i < *nb_entries; i++) 
            entries[i] = malloc(sizeof(char) * 100);

        int ret_list = list(fd,"debug/folder1/subfolder1/symlinkfolder2",entries,nb_entries);
        printf("list returned:%d with nb_entries:%li \nentries: \n",ret_list,*nb_entries);
        for (size_t i = 0; i < *nb_entries; i++)
            printf("%s\n",entries[i]);

        free(nb_entries); free(entries);
    }

    return 0;
}
