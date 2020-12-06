#include "lib_tar.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#define BSIZE 512

/**
 * free allocated memory, print error to stderr and exit.
 *
 */
void exit_err()
{
    for (int i = 0; i < count; i++) {
        free(entries[i]);
    }

    fprintf(stderr, "Program error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
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
    if (entries == NULL)
        build_entries(tar_fd);

    // int hdr_nbr = 0;
    // int read_ret;
    // uint8_t *buffer = malloc(sizeof(uint8_t) * BSIZE);

    // while (read_ret = read_file(tar_fd, ???, 0, buffer, BSIZE) > 0)
    // {

    // }

    //if (invalid_magic_value) return -1;
    //if (invalid_version_value) return -2;
    //if (invalid_checksum_value) return -3;
    //free(buffer);
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
    if (entries == NULL) build_entries(tar_fd);

    //for all entries check if path corresponds
    for (int i = 0; i < count; i++)
    {
        if (strcmp(path, entries[i]) == 0) {
            return i;
        }
    }
    return 0;
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
    // int index = exists(tar_fd, path);
    // if (index) {
        get buffer from index
        // uint8_t* buffer = get_buffer(tar_fd, index);
        // if (is_dir_header(buffer)) {
            count number of files in dir checking string contains on entries
            // int filecount = 0;
            // while (strstr(entries[index + filecount], path) != NULL) filecount++;
            // return filecount;
        // } else {
            // return 0;
        // }
    // } else {
        // return 0;
    // }
}

uint8_t* get_buffer(int tar_fd, int* path, int nb) {
    FILE* tar_fp = fdopen(tar_fd, "r");
    uint8_t* buffer = (uint8_t *)malloc(sizeof(uint8_t) * BSIZE * nb);

    while (fread(buffer, BSIZE, 1, tar_fp) > 0){
        if (strcmp(path, buffer) == 0) {
            if (nb > 1)
                fread(buffer, BSIZE, nb - 1, tar_fp);
            return buffer;
        }
    }

    return NULL;
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
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         index of linked file otherwise.
 */
int is_symlink(int tar_fd, char *path)
{
    return 0;
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
    if (is_dir(tar_fd, path)) {
        // print all directories
    }
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
    //read(tar_fd, );
}

/**
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

bool is_file_header(uint8_t *buffer)
{
    if (buffer == NULL) return false;

    char typeflag = buffer[156];

    switch (typeflag)
    {
    case '0':
        return true;
    case '\0':
        return true;
    case '3':
        return true;
    case '4':
        return true;
    case '6':
        return true;
    case '7':
        return true;
    default:
        return false;
    }
}

bool is_dir_header(uint8_t *buffer)
{
    if (buffer == NULL) return false;

    char typeflag = buffer[156];

    if (typeflag == '5')
        return true;
    else
        return false;
}

bool is_link_header(uint8_t *buffer)
{
    if (buffer == NULL) return false;

    char typeflag = buffer[156];

    if (typeflag == '1' || typeflag == '2')
        return true;
    else
        return false;
}
