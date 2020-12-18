#include "lib_tar.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define BSIZE 512

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_hex(const uint8_t *bytes, size_t len) {
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
    fseek(tar_fp,0,SEEK_SET);
    printf("get_buffer|begin with path:%s\n",path);
    if (path==NULL){
        fread(buffer, BSIZE, 1, tar_fp);
        return buffer;
    }

    while (fread(buffer, BSIZE, 1, tar_fp) > 0){

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

size_t nb_fileblock(tar_header_t* file_header)
{
    char* size = (char *)malloc(sizeof(char) * 12);
    memcpy(size, file_header->size, 12);
    size_t int_repr = TAR_INT(size);
    size_t nb = (int_repr + 512 - 1) / 512;
    printf("nb_fileblock|int_repr:%lu nb_bloc:%lu\n",int_repr,nb);
    free(size);
    return nb;
}

long get_offset_from_path(int tar_fd, char* path){
    FILE* tar_fp = fdopen(tar_fd, "r");
    tar_header_t* header = (tar_header_t*) malloc(sizeof(tar_header_t));
    long offset = 0;
    while(fread(header,BSIZE,1,tar_fp)>0){
        printf("get_offset_from_path|header_path:%s\n",header->name);
        if(strcmp(path,header->name)){
            free(header);
            return offset * BSIZE;
        }
        offset+=1;
    }
    printf("get_offset_from_path|ERROR has not found the path:%s\n",path);
    free(header);
    return -1;
}

/*
* return 1 si path est de la forme dir/_*
* return 0 sinon
*/
int is_path_of_dir(char* path, char* dir){
    if (strstr(path,dir) == NULL) return 0;
    int c_path = 0; int c_dir = 0;
    for (int i=0;i< strlen(dir);i++) {
        if (dir[i] == '/'){
            c_dir++;
        }
    }

    for (int i=0;i< strlen(path);i++) {
        if (c_path > c_dir){
            return 0;
        }
        if (path[i] == '/'){
            c_path++;
        }
    }
    return 1;
}

char* cut_path(char* path, char* dir){
    int i = 0;
    for (;i< strlen(dir);i++) {}
    return path+i;
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
    //prepare variables
    FILE* tar_fp = fdopen(tar_fd, "r");
    tar_header_t* header = (tar_header_t*) malloc(sizeof(tar_header_t));
    size_t nb_listed_entries = 0;
    printf("list|launched with no_entries:%li and path_header:%s\n",*no_entries,path);

    //find header of guiven path
    int offset = get_offset_from_path(tar_fd,path);
    if(offset<0){//path n'exitse pas dans le header
        printf("list|ERROR offset of path header:%i meaning no such path in the archive\n",offset);
        free(header);
        *no_entries = 0;
        return 0;
    }
    fseek(tar_fp, offset, SEEK_SET);
    fread(header, BSIZE, 1, tar_fp);
    debug_hex((uint8_t*) header,BSIZE);

    //check if path is dir of symlink(resolved if symlink)
    int type = blocktype((uint8_t*) header);
    if(type==3){//symlink
        printf("list|path is symlink pointing to:%s\n",header->name);
        fseek(tar_fp, get_offset_from_path(tar_fd,header->name), SEEK_SET);

    }else if(type==2){//directory
        printf("list|path is dir can launch while\n");
    }else{
        free(header); //maybe erreur sur ingi
        *no_entries = 0;
        return 0;
    }
    char* dir = malloc(sizeof(char)*100);
    memcpy(dir,header->name,100);
    //begin listing th entries
    while (fread(header, BSIZE, 1, tar_fp)>0){
        //debug_hex((uint8_t*) header,BSIZE);
        printf("list|checking header of %s nb_listed_entries:%li\n",header->name,nb_listed_entries);
        if(header == NULL){printf("list|NULL BUFFER\n");fflush(stdout);}


        //check if 2 padding blocks to end archive file
        if(is_padding((uint8_t*) header)){
            printf("listpadding_block\n");
            if(fread(header, BSIZE, 1, tar_fp)<0) {printf("list|fread EOF in padding\n"); break;};
            if((!is_padding((uint8_t*) header))) return -4;
            else break;
        }


        //check kind of file
        int type = blocktype((uint8_t*) header);

        if(type==1){//file
            if (is_path_of_dir(header->name,dir) && nb_listed_entries < *no_entries) {
                char* cut = cut_path(header->name,dir);
                memcpy(entries[nb_listed_entries],cut,strlen(cut));
                nb_listed_entries++;
                printf("list|added cut:%s to entries\n",cut);
            }
            size_t nb_block = nb_fileblock(header);
            printf("list|file header checked of %s with length: %lu blocks\n",header->name,nb_block);
            fseek(tar_fp, BSIZE* nb_block, SEEK_CUR); //move pointer by the nb of blocks the file is
        }else if(type==2){//directory
            if (is_path_of_dir(header->name,dir) && nb_listed_entries < *no_entries) {
                char* cut = cut_path(header->name,dir);
                memcpy(entries[nb_listed_entries],cut,strlen(cut));
                nb_listed_entries++;
            }
            printf("list|dir header checked %s\n",header->name);
        }else if(type==3){ //symlink
            if (is_path_of_dir(header->name,dir) && nb_listed_entries < *no_entries) {
                char* cut = cut_path(header->name,dir);
                memcpy(entries[nb_listed_entries],cut,strlen(cut));
                nb_listed_entries++;
            }
            printf("list|symlink header checked %s\n",header->name);
        }
    }
    *no_entries = nb_listed_entries;
    free(header);
    free(dir);
    return 0;
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
    int nb_headers = 0;


    while (fread(header, BSIZE, 1, tar_fp)>0){

        debug_hex((uint8_t*) header,BSIZE);
        printf("check_archive|checking header of %s\n",header->name);
        if(header == NULL){printf("check_archive|NULL BUFFER\n");fflush(stdout);}


        //check if 2 padding blocks to end archive file
        if(is_padding((uint8_t*) header)){
            printf("check_archive|padding_block\n");
            if(fread(header, BSIZE, 1, tar_fp)<0) {printf("check_archive|fread EOF in padding\n"); break;};
            if((!is_padding((uint8_t*) header))) return nb_headers;
            else break;
        }

        nb_headers ++;

        //check kind of file
        int type = blocktype((uint8_t*) header);

        if(type==1){//file
            int check = check_header(header);
            if (check < 0) return check;
            size_t nb_block = nb_fileblock(header);
            printf("check_archive|file header checked of %s with length: %lu blocks\n",header->name,nb_block);
            fseek(tar_fp, BSIZE* nb_block, SEEK_CUR); //move pointer by the nb of blocks the file is
        }else if(type==2){//directory
            int check = check_header(header);
            if (check < 0) return check;
            printf("check_archive|dir header checked %s\n",header->name);
        }else if(type==3){ //symlink ACHANGER FAUT CHECK VERS QUOI CA POINTE
            int check = check_header(header);
            if(check<0) return check;
            printf("check_archive|symlink header checked %s\n",header->name);
        }
    }

    free(header);
    //fclose(tar_fp);
    return nb_headers;
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
            printf("read_file|link of the symlink:%s\n", link);
            ret = read_file(tar_fd, link, offset, dest, len); // recursive call to link
            free(buffer);
            free(link);
          } break;
        case 1: {//file
            size_t nb_blocks = nb_fileblock((tar_header_t*) buffer); // get number of blocks to read
            free(buffer);
            if (offset > nb_blocks * BSIZE) {
                ret = -2;
            } else {
                buffer = get_buffer(tar_fd, path, 1 + nb_blocks); // read header and file to buffer
                size_t toread_bytes = (*len < (nb_blocks * BSIZE) - offset) ? *len : (nb_blocks * BSIZE) - offset; // number of bytes to read in file
                memcpy(dest, buffer + 512 + offset, toread_bytes);
                size_t unread_bytes = (nb_blocks * BSIZE) - offset - toread_bytes; // number of bytes still unread in file
                printf("read_file|to_read_bytes:%lu  unread_bytes:%lu\n",toread_bytes,unread_bytes);
                ret = (ssize_t) unread_bytes;
                *len= toread_bytes;
                free(buffer);
            }
          } break;
    }
    return ret;
}
