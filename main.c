#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "lzw.h"

#define BUF_SIZE        512

void lzw_writebuf(void *stream, char *buf, unsigned size)
{
    fwrite(buf, size, 1, (FILE*)stream);
}

unsigned lzw_readbuf(void *stream, char *buf, unsigned size)
{
    return fread(buf, 1, size, (FILE*)stream);
}


#define  DICTIONARY_SIZE  (1 << 18)
node_lzw_t dictionary[DICTIONARY_SIZE];
int        hash_table[DICTIONARY_SIZE];

lzw_enc_t lzwe;

/******************************************************************************
**  main
**  --------------------------------------------------------------------------
**  Decodes input LZW code stream into byte stream.
**
**  Arguments:
**      argv[1] - input file name;
**      argv[2] - output file name;
**
**  Return: error code
******************************************************************************/

void read_file_to_buffer(void * buf, size_t elem_size, size_t max_len, FILE *file )
{
    fseek(file, 0, SEEK_END);
    long fsize = max_len > (ftell(file) / elem_size) ? (ftell(file) / elem_size) : max_len;
    fseek(file, 0, SEEK_SET);
    fread(buf, elem_size, fsize, file);
}

int main (int argc, char* argv[])
{
    FILE       *fin;
    FILE       *fout;
    FILE       *fdic;
    FILE       *fhash;
    lzw_enc_t  *ctx = &lzwe;
    unsigned   len;
    char       buf[256];

    if (argc < 3) {
        printf("Usage: lzw-enc <input file> <output file>\n");
        return -1;
    }

    if (!(fin = fopen(argv[1], "rb"))) {
        fprintf(stderr, "Cannot open %s\n", argv[1]);
        return -2;
    }

    if (!(fout = fopen(argv[2], "w+b"))) {
        fprintf(stderr, "Cannot open %s\n", argv[2]);
        return -3;
    }

#ifndef DISABLE_ADD_NEW_NODE
    if (!(fdic = fopen(argv[3], "w+b"))) {
        fprintf(stderr, "Cannot open %s\n", argv[3]);
        return -3;
    }

    if (!(fhash = fopen(argv[4], "w+b"))) {
        fprintf(stderr, "Cannot open %s\n", argv[4]);
        return -3;
    }
#else
    if (!(fdic = fopen(argv[3], "rb"))) {
        fprintf(stderr, "Cannot open %s\n", argv[3]);
        return -3;
    }

    if (!(fhash = fopen(argv[4], "rb"))) {
        fprintf(stderr, "Cannot open %s\n", argv[4]);
        return -3;
    }
#endif

#ifndef DISABLE_ADD_NEW_NODE

    lzw_enc_init(ctx, fout, NULL, 0, &dictionary[0], &hash_table[0], DICT_SIZE);
#else
    lzw_enc_restore(ctx, fout, NULL, 0, dictionary, hash_table, DICTIONARY_SIZE);
    read_file_to_buffer(ctx->dict, sizeof(node_lzw_t), DICTIONARY_SIZE, fdic);
    read_file_to_buffer(ctx->hash, sizeof(int), DICTIONARY_SIZE, fhash);
#endif
    for (int i = 0; i < 1; i++) {
        fseek(fin, 0, SEEK_SET);
        while (len = lzw_readbuf(fin, buf, sizeof(buf))) {
            lzw_encode(ctx, buf, len);
        }
    }
    //lzw_encode(ctx, input_buf,BUF_SIZE);
    lzw_enc_end(ctx);

#ifndef DISABLE_ADD_NEW_NODE

    fwrite(ctx->dict, sizeof(node_lzw_t), ctx->max + 1, fdic);
    fwrite(ctx->hash, sizeof(int), ctx->max + 1, fhash);

    //fwrite(ctx->dict, sizeof(node_lzw_t), DICT_SIZE, fdic);
    //fwrite(ctx->hash, sizeof(int), HASH_SIZE, fhash);
#endif
    int max_value = 0;
    int max_index = 0;
    for (int i = 0; i < ctx->max; i++)
    {
        if (ctx->dict[i].prev > max_value)
        {
            max_value = ctx->dict[i].prev;
            max_index = i;
        }
    }

    printf("max_index %d, max_value %d\n", max_index, max_value);

    fclose(fdic);
    fclose(fhash);

    fclose(fin);
    fclose(fout);


    return 0;
}