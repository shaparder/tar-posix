#include "lib_tar.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define BSIZE 512

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

    while (fread(buffer, BSIZE, 1, tar_fp) > 0){
        if (strcmp(path, (char*)buffer) == 0) {
            if (nb > 1)
                fread(buffer + 512, BSIZE, nb - 1, tar_fp);
            return buffer;
        }
    }

    free(buffer);
    return NULL;
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
    //if (invalid_magic_value) return -1;
    //if (invalid_version_value) return -2;
    //if (invalid_checksum_value) return -3;
    return 0;
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
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries)
{
    return 0;
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

// EN CONSTRUCTION JE SUIS DEAD
uint32_t nb_fileblock(uint8_t* file_header)
{
    char* size = (char *)malloc(sizeof(char) * 12);
    memcpy(size, file_header + 124, 12);
    // TO-DO: convertir size (uint8_t aka bytes) en integer (uint32_t), si tu veux le faire hesite pas j'ai trop la flemme
    uint32_t int_repr = 0;// TO-DO
    uint32_t nb = (int_repr + 512 - 1) / 512;
    free(size);
    return nb;
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

/**
 * @brief copy slice of array to
 *
 * @param src
 * @param offset
 * @param len
 *
 * @return allocated char* containing the slice
uint8_t* buf_slice(uint8_t *src, size_t offset, size_t len)
{
    uint8_t* dest = (uint8_t *)malloc(sizeof(uint8_t) * len);
    memcpy(dest, src + offset, len);

    return dest;
}
 */
