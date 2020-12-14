#include "lib_tar.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define BSIZE 512


char *int2str(int nb) {
    int i = 0;
    int div = 1;
    int cmp = nb;
    char *nbr = malloc(sizeof(char) * 12);
    if (!nbr)
        return (NULL);
    if (nb < 0)
        nbr[i++] = '-';
    while ((cmp /= 10) != 0)
        div = div * 10;
    while (div > 0) {
        nbr[i++] = abs(nb / div) + 48;
        nb = nb % div;
        div /= 10;
    }
    nbr[i] = '\0';
    return (nbr);
}

/**
 * @brief Get buffer from path in file at
 *
 * @param tar_fd File descriptor pointing to start of file
 * @param path to search for in file descriptor
 * @param nb number of blocks to store in buffer
 * @return Buffer storing 512 bytes * nb if path match, NULL otherwise
 */
uint8_t* get_buffer(int tar_fd, char* path, int nb) {
    FILE* tar_fp = fdopen(tar_fd, "r");
    uint8_t* buffer = (uint8_t *)malloc(sizeof(uint8_t) * BSIZE * nb);
    printf("get_buffer|file opened and buffer malloc\n");
    if (path==NULL){
        fread(buffer, BSIZE, 1, tar_fp);
        return buffer;
    } 

    while (fread(buffer, BSIZE, 1, tar_fp) > 0){
        //printf("get_buffer|buffer:%s\n",(char*) buffer);
        if (strcmp(path, (char*)buffer) == 0) {
            if (nb > 1)
                fread(buffer + 512, BSIZE, nb - 1, tar_fp);
            return buffer;
        }
    }
    printf("get_buffer|no header with path:%s\n",path);
    free(buffer);
    return NULL;
}

tar_header_t* get_buffer_at_offset(int tar_fd, int nbBlockOffset){
    FILE* tar_fp = fdopen(tar_fd, "r");
    tar_header_t* header = (tar_header_t*) malloc(sizeof(tar_header_t));
    lseek(tar_fd,BSIZE * nbBlockOffset,SEEK_SET);
    if(fread(header, BSIZE, 1, tar_fp)<0) return NULL;
    return header;
}

/*
 * @brief copy slice of array to
 *
 * @param src
 * @param offset
 * @param len
 *
 * @return allocated char* containing the slice
 */
uint8_t* buf_slice(uint8_t *src, size_t offset, size_t len)
{
    uint8_t* dest = (uint8_t *)malloc(sizeof(uint8_t) * len);
    memcpy(dest, src + offset, len);

    return dest;
}

/*
* functions used in check_archive
*
*/
int check_sum(uint8_t* buffer){
    size_t checksum = 0;
    int SizeOfArray = BSIZE;
    for(int x = 0; x < 148; x++)
    {
        checksum += buffer[x];
    }
    checksum += 8 * ' ';
    for(int x = 156; x < SizeOfArray; x++)
    {
        checksum += buffer[x];
    }
    size_t real_check_sum = TAR_INT((char*) buffer+148);
    printf("check_sum|calculated:%li vs header_val:%li\n", checksum, real_check_sum);
    return checksum == real_check_sum;
}

int check_header(tar_header_t* buffer){
    
    if (strcmp((char*) buffer+257,TMAGIC) != 0) return -1;
    
    char ver[3];
    memcpy(ver,buffer->version,2);
    ver[2] = '\0';
    //printf("check_sum|buffer->version:%s\n",ver);
    if (strcmp(ver,TVERSION)!= 0) return -2;

    if (check_sum((uint8_t*) buffer)!=1) return -3;
    
    return 0;
}

int is_padding(uint8_t* buffer){
    int i = 0;
    while(i<512){
        if(*(buffer + i++) != '\0'){
            return 0;
        }
    }
    return 1;
}




/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         index of entry in archive otherwise.
 */
int exists(int tar_fd, char *path)
{
    uint8_t* buffer = get_buffer(tar_fd, path, 1);

    if (buffer == NULL){
        return 0;
    } else {
        free(buffer);
        return 1;
    }
}

/**
 * @brief return type of block
 *
 * @param buffer containing block
 * @return 0 = NULL buffer or not a header or unknown typeflag
 *         1 = file
 *         2 = directory
 *         3 = link
 */
int blocktype(uint8_t *buffer)
{
    if (buffer == NULL) return 0;

    char typeflag = buffer[156];

    switch (typeflag)
    {
    case '0':
        return 1;
    case '\0':
        return 1;
    case '1':
        return 3;
    case '2':
        return 3;
    case '3':
        return 1;
    case '4':
        return 1;
    case '5':
        return 2;
    case '6':
        return 1;
    case '7':
        return 1;
    default:
        return 0;
    }
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         number of 512 bytes blocks otherwise.
 */
int is_file(int tar_fd, char *path)
{
    uint8_t* buffer = get_buffer(tar_fd, path, 1);

    if (buffer == NULL) return 0;
    else
    {
       int ret = (blocktype(buffer) == 1) ? 1 : 0;
       free(buffer);
       return ret;
    }
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         number of entries in the directory otherwise.
 */
int is_dir(int tar_fd, char *path)
{
    uint8_t* buffer = get_buffer(tar_fd, path, 1);

    if (buffer == NULL) return 0;
    else
    {
      int ret = (blocktype(buffer) == 2) ? 1 : 0;
      free(buffer);
      return ret;
    }
}
/**
 * @brief get path of symlink file
 *
 * @param link_header buffer containing symlink header
 * @return char* path of linked file
 */
char* symlink_path(uint8_t* link_header)
{
    char* link = (char *)malloc(sizeof(char) * 100);
    memcpy(link, link_header + 157, 100);
    return link;
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         whatever otherwise.
 */
int is_symlink(int tar_fd, char *path)
{
    uint8_t* buffer = get_buffer(tar_fd, path, 1);

    if (buffer == NULL) return 0;
    else
    {
      int ret = (blocktype(buffer) == 3) ? 1 : 0;
      free(buffer);
      return ret;
    }
}

/**
 * Lists the entries at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entry in entries.
 *                   The callee set it to the number of entry listed.
 *
 * @return zero if no directory at the given path exists in the archive
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries)
{
    return 0;
}


size_t nb_fileblock(uint8_t* file_header)
{
    char* size = (char *)malloc(sizeof(char) * 12);
    memcpy(size, file_header + 124, 12);
    size_t int_repr = TAR_INT(size);
    size_t nb = (int_repr + 512 - 1) / 512;
    printf("nb_fileblock|int_repr:%lu nb_bloc:%lu\n",int_repr,nb);
    free(size);
    return nb;
}


/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd)
{
    FILE* tar_fp = fdopen(tar_fd, "r");
    tar_header_t* header = (tar_header_t*) malloc(sizeof(tar_header_t));


    while (fread(header, BSIZE, 1, tar_fp)>0){
        debug_dump((uint8_t*) header,BSIZE);
        printf("check_archive|checking header of %s\n",header->name);
        if(header == NULL){printf("check_archive|NULL BUFFER\n");fflush(stdout);}

        int type = blocktype((uint8_t*) header);

        if(type==1){//file
            int check = check_header(header);
            if (check < 0) return check;
            size_t nb_block = nb_fileblock((uint8_t*) header);
            printf("check_archive|file header checked of %s with length: %lu blocks\n",header->name,nb_block);
            fseek(tar_fp, BSIZE* nb_block, SEEK_CUR); //move pointer by the nb of blocks the file is
        }else if(type==2){//directory
            int check = check_header(header);
            if (check < 0) return -1;
            printf("check_archive|dir header checked %s\n",header->name);
        }else if(type==3){ //symlink
            long old_offset = ftell(tar_fp);
            printf("check_archive|oldoffset i: %ld",old_offset);
            int offset_block = 0;//funcion qui prend header et rend offset//TODO funct qui retrouve le header du symlink
            fseek(tar_fp, BSIZE * offset_block, SEEK_SET);
            int check = check_header(header);
            if(check<0) return check;
            fseek(tar_fp, old_offset, SEEK_SET);
        }
        if(is_padding((uint8_t*) header)){
            printf("check_archive|padding_block");
            if(fread(header, BSIZE, 1, tar_fp)<0) {printf("check_archive|fread EOF in padding\n"); break;};
            if((!is_padding((uint8_t*) header))) return -4;
            else break;
        }

    }
    free(header);
    return 0;
}




/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len)
{
    uint8_t* buffer = get_buffer(tar_fd, path, 1);

    int type = blocktype(buffer);
    ssize_t ret;

    switch (type) {
        case 0: {//NULL
            ret = -1;
          } break;
        case 2: { //directory
            free(buffer);
            ret = -1;
          } break;
        case 3: {//symlink
            char* link = symlink_path(buffer); // find linked file
            ret = read_file(tar_fd, link, offset, dest, len); // recursive call to link
            free(buffer);
            free(link);
          } break;
        case 1: {//file
            uint32_t nb_blocks = nb_fileblock(buffer); // get number of blocks to read
            free(buffer);
            if (offset > nb_blocks * BSIZE) {
                ret = -2;
            } else {
                buffer = get_buffer(tar_fd, path, 1 + nb_blocks); // read header and file to buffer
                size_t toread_bytes = (*len < (nb_blocks * BSIZE) - offset) ? *len : (nb_blocks * BSIZE) - offset; // number of bytes to read in file
                memcpy(dest, buffer + 512 + offset, toread_bytes);
                size_t unread_bytes = (nb_blocks * BSIZE) - offset - toread_bytes; // number of bytes still unread in file
                ret = (ssize_t) unread_bytes;
                free(buffer);
            }
          } break;
    }
    return ret;
}


 
/*
int check_archive_avec_get_buffer_perso(int tar_fd)
{
    
    tar_header_t* header = get_buffer_at_offset(tar_fd,0);
    printf("header:\n");
    debug_dump((uint8_t*) header, BSIZE);
    int file_to_end = 0;
    int nb_block_offset = 0;
    while (1){

        if(header == NULL){printf("check_archive|NULL BUFFER\n");fflush(stdout);}

        int type = blocktype(header);
        switch (type)
        {
            case 1:{//file
                int check = check_header(header);
                if (check < 0) return check;
                free(header);
                nb_block_offset += nb_fileblock(header);
                header = get_buffer_at_offset(tar_fd, nb_block_offset);
            }break;
            case 2:{//directory
                int check = check_header(header);
                if (check < 0) return check;
                file_to_end +=1;
                nb_block_offset += 1;
                header = get_buffer_at_offset(tar_fd, nb_block_offset);
            }break;
            case 3:{ //symlink
                char* path = symlink_path(header); // find linked file
                header = (tar_header_t*) get_buffer(tar_fd,path,1);
                continue; //jump to next iteration
            }break;
            default:{
                if(is_padding(header)){
                    file_to_end --;
                    header = get_buffer_at_offset(tar_fd, ++nb_block_offset);
                }else 
                    return -1;
            }break;
        }
    }
    return 0;
}
*/