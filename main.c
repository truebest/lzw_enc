#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "lzw.h"
#include "dictionary_lzw.h"
#include "hash_lzw.h"
#include "string.h"

#define BUF_SIZE        512

void lzw_writebuf(void *stream, char *buf, unsigned size)
{
    fwrite(buf, size, 1, (FILE*)stream);
}

unsigned lzw_readbuf(void *stream, char *buf, unsigned size)
{
    return fread(buf, 1, size, (FILE*)stream);
}


#define  DICTIONARY_SIZE  (0x10400)
#define LZW_TEST_ARRAY_SIZE     512

node_lzw_t dictionary[DICTIONARY_SIZE];
int        hash_table[DICTIONARY_SIZE];
uint8_t encoded_array[LZW_TEST_ARRAY_SIZE];

const char * str = "1619616162.063Dimd Init\n"
                   "1619616162.063IimdInit Done\n"
                   "1619616162.063InbfInit\n"
                   "1619616162.063InbfInit Done\n"
                   "1619616162.143Iserniu_certificates_check: Certificate https_onboard_cert (/spinor/ssl/certs/https_onboard_cert) verified succesfully\n"
                   "1619616162.153Iserniu_certificates_check: Certificate elux_root_ca_cert_dev (/spinor/ssl/ca_lists/elux_root_ca_cert_dev) verified succesfully\n"
                   "1619616162.163Iserniu_certificates_check: Certificate elux_root_ca_cert_prod (/spinor/ssl/ca_lists/elux_root_ca_cert_prod) verified succesfully\n"
                   "1619616162.243Iserniu_certificates_check: Certificate niu5_client_cert (/spinor/ssl/certs/niu5_client_cert) verified succesfully\n"
                   "1619616162.343Dwif Scan queue creation ok\n"
                   "1619616162.343Dimd {stm} registering\n"
                   "1619616162.343Dimd {stm} look if already registered\n"
                   "1619616162.343Dimd Look for a free table location\n"
                   "1619616162.343Iimd{stm} registered, 21 free\n"
                   "1619616162.453IstmSTA MAC = 00:F4:8D:90:B0:5D\n"
                   "1619616162.453Dimd {led} registering\n"
                   "1619616162.453Dimd {led} look if already registered\n"
                   "1619616162.453Dimd Look for a free table location\n"
                   "1619616162.453Iimd{led} registered, 20 free\n"
                   "1619616162.453Dimd {ifm} registering\n"
                   "1619616162.453Dimd {ifm} look if already registered\n"
                   "1619616162.453Dimd Look for a free table location\n"
                   "1619616162.453Iimd{ifm} registered, 19 free\n"
                   "1619616162.453Dimd {ifmP} registering\n"
                   "1619616162.453Dimd {ifmP} look if already registered\n"
                   "1619616162.453Dimd Look for a free table location\n"
                   "1619616162.453Iimd{ifmP} registered, 18 free\n"
                   "1619616162.453IrmoInit Done\n"
                   "1619616162.453Dimd {snt} registering\n"
                   "1619616162.453Dimd {snt} look if already registered\n"
                   "1619616162.453Dimd Look for a free table location\n"
                   "1619616162.453Iimd{snt} registered, 17 free\n"
                   "1619616182.453Ddba> fs_scan: ufs/appl/baud | 4\n"
                   "1619616162.453Ddba> fs_scan: ufs/sec/pvtkey | 0\n"
                   "1619616162.453Ddba> fs_scan: ufs/sec/provpwd | 32\n"
                   "1619616162.453Ddba> fs_scan: ufs/sec/rootpwd | 4\n"
                   "1619616162.453Ddba> fs_scan: ufs/ser20/disws | 0\n"
                   "1619616162.453Ddba> fs_scan: ufs/h4/APL/0007 | 9\n"
                   "1619616162.463Ddba> fs_scan: ufs/h4/APL/000A | 2\n"
                   "1619616162.463Ddba> fs_scan: ufs/h4/APL/0002 | 8\n"
                   "1619616162.463Ddba> fs_scan: ufs/ECP/env | 3\n"
                   "1619616162.463Ddba> fs_scan: ufs/h4/APL/0001 | 4\n"
                   "1619616162.463Ddba> fs_scan: ufs/ota/fail_cnt | 1\n"
                   "1619616162.463Ddba> fs_scan: ufs/ECP/inst | 7\n"
                   "1619616162.463Ddba> fs_scan: ufs/wifi/ssid | 11\n"
                   "1619616162.463Ddba> fs_scan: ufs/wifi/pwd | 8\n"
                   "1619616162.463Ddba> fs_scan: ufs/wifi/stype | 7\n"
                   "1619616162.463Ddba> fs_scan: ufs/wifi/ccode | 2\n"
                   "1619616162.463Ddba> fs_scan: ufs/cert_exp | 158\n"
                   "1619616162.463Ddba> fs_scan: ufs/mqtt/cliid | 43\n"
                   "1619616162.463Ddba> fs_scan: ufs/mqtt/token | 18\n"
                   "1619616162.463Ddba> fs_scan: ufs/mqtt/url | 46\n"
                   "1619616162.463Ddba> fs_scan: ufs/mqtt/port | 2\n"
                   "1619616162.463Dimd {hd} registering\n"
                   "1619616162.463Dimd {hd} look if already registered\n"
                   "1619616162.463Dimd Look for a free table location\n"
                   "1619616162.463Iimd{hd} registered, 12 free\n"
                   "1619616162.463Dimd {mtx} registering\n"
                   "1619616162.463Dimd {mtx} look if already registered\n"
                   "1619616162.463Dimd Look for a free table location\n"
                   "1619616162.463Iimd{mtx} registered, 11 free\n"
                   "1619616162.463Dimd {ser} registering\n"
                   "1619616162.463Dimd {ser} look if already registered\n"
                   "1619616162.463Dimd Look for a free table location\n"
                   "1619616162.463Iimdhdbc} registered, 8 free\n"
                   "1619616162.463Dimd {ata} registering\n"
                   "1619616162.463Dimd {ata} look if already registered\n"
                   "1619616162.463Dimd Look for a free table location\n"
                   "1619616162.463Iimd{ata} registered, 7 free\n"
                   "1619616162.473IstaBOOT|PON Cold\n"
                   "1619616162.473Irmo{app_main} registered, 19 free\n"
                   "1619616402.973Irmo{led} registered, 18 free\n"
                   "1619616162.973IstmStart stm thread\n"
                   "1619616162.973Irmo{stm} registered, 17 free\n"
                   "1619616162.973Irmo{ifm} registered, 16 free\n"
                   "1619616162.973Irmo{sntp} registered, 15 free\n"
                   "1619616162.973Irmo{ota_trg} registered, 14 free\n"
                   "1619616162.973Irmo{epsv} registered, 13 free\n"
                   "1619616162.973Irmo{http} registered, 12 free\n"
                   "1619616162.973Irmo{niu_inet} registered, 11 free\n"
                   "1619616162.973Irmo{db_agent} registered, 10 free\n"
                   "1619616162.973Irmo{hacl_disp} registered, 9 free\n"
                   "1619616162.973Irmo{niu_cli} registered, 8 free\n"
                   "1619616162.973Iwiftx_power_control_get_all: No data in DB\n"
                   "1619616162.973Iwiftx_power_control_get: No data in DB or DB error, return the default (0)\n"
                   "1619616162.983IdbaREAD /spinor/db01/ufs/wifi/ssid\n"
                   "1619616162.983IdbaREAD /spinor/db01/ufs/wifi/pwd\n"
                   "1619616162.983IdbaREAD /spinor/db01/ufs/wifi/stype\n"
                   "1619616162.983IdbaREAD /spinor/db01/ufs/wifi/ccode\n"
                   "1619616162.983IonbPRCC Get OK: GB\n"
                   "1619616162.983IstmCredential present but wait for command to start STA \n"
                   "1619616162.993IdbaREAD /spinor/db01/ufs/ota/fail_cnt\n"
                   "1619616162.993IotaOTA cnt = now\n"
                   "1619616162.993IotaTry OTA in 0s (3000 ticks)\n"
                   "1619616162.993IstmSTM_WIFI_OFF ENTRY\n"
                   "1619616162.993IstmWiFi is enabled.\n"
                   "1619616162.993IhmnOta State = 0\n"
                   "1619616163.003IdbaREAD /spinor/db01/ufs/ECP/env\n"
                   "1619616163.003Iserget_cloud_env DEV\n"
                   "1619616163.003ImtxMTX PUBLISH INIT -> IDLE\n"
                   "1619616163.003IhmnSpecialMsg - HACL4_NAME_NIU_ALERT_EVENT INF\n"
                   "1619616163.003IhacHACL_DB Fwd\n"
                   "1619616163.003Irmo{icm} registered, 7 free\n"
                   "1619616163.003Irmo{eap_agent} registered, 6 free\n"
                   "1619616163.003Decp Queue for ECP HACL ARRAY was created\n"
                   "1619616163.003Iimd{epsW} registered, 6 free\n"
                   "1619616163.003Dprh prov_handler: Inited!\n"
                   "1619616163.003IepsModule started\n"
                   "1619616163.003Dprh kPROV_HANDLER_STATE_INIT => kPROV_HANDLER_STATE_IDLE\n"
                   "1619616163.003IdbaREAD /spinor/db01/ufs/mqtt/url\n"
                   "1619616163.003IstaHEAP|Total heap = 302336\0";

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
int read_file_to_buffer(void * buf, size_t elem_size, FILE *file )
{
    fseek(file, 0, SEEK_SET);
    int data_size = 0;
    fread(&data_size, sizeof(int), 1, file);
    fread(buf, elem_size, data_size, file);
    return data_size;
}

int test_compare(FILE *fdic)
{
    node_lzw_t * p_node_1 = (node_lzw_t *)dictionary_lzw_bin;
    node_lzw_t * p_node_2 = (node_lzw_t *)&dictionary[0];
    read_file_to_buffer(p_node_2, sizeof(node_lzw_t), fdic);

    for (int i = 0; i < DICT_LZW_LEN / sizeof(node_lzw_t); i++) {
        if (memcmp(p_node_1, p_node_2, sizeof(node_lzw_t)) != 0) {
            printf("compare error at %d \n", i);
            printf("p_node_1: prev 0x%X \t next 0x%X \t char 0x%X \n", p_node_1->prev, p_node_1->next, p_node_1->ch);
            printf("p_node_2: prev 0x%X \t next 0x%X \t char 0x%X \n", p_node_2->prev, p_node_2->next, p_node_2->ch);
            return -1;
        }
    }

    return 0;
}


int encodeToFile(lzw_enc_t * ctx,  char* argv[], char * out_file_name, int num_lines)
{
    FILE       *fin;
    FILE       *fout;
    FILE       *fdic;
    FILE       *fhash;
    char       buf[256];
    int        encoded_bytes;
    int        read_num_lines;

    if (!(fin = fopen(argv[1], "rb"))) {
        fprintf(stderr, "Cannot open %s\n", argv[1]);
        return -2;
    }

    if (!(fout = fopen(out_file_name, "w+b"))) {
        fprintf(stderr, "Cannot open %s\n", out_file_name);
        return -3;
    }

    if (!(fdic = fopen(argv[3], "rb"))) {
        fprintf(stderr, "Cannot open %s\n", argv[3]);
        return -3;
    }

    if (!(fhash = fopen(argv[4], "rb"))) {
        fprintf(stderr, "Cannot open %s\n", argv[4]);
        return -3;
    }
#ifndef READ_DICT_FROM_ARRAY
    lzw_enc_restore(ctx, fout, NULL, 0, dictionary, hash_table, DICTIONARY_SIZE);
    read_file_to_buffer(ctx->dict, sizeof(node_lzw_t), fdic);
    read_file_to_buffer(ctx->hash, sizeof(int), fhash);
#else
    lzw_enc_restore(&lzwe, fout, NULL, 0, (node_lzw_t *)&dictionary_lzw_bin[4], (int *)&hash_lzw_bin[4], *(int *)dictionary_lzw_bin);
#endif
    fseek(fin, 0, SEEK_SET);
    int c,  len = 0;
    read_num_lines = 0;
    encoded_bytes = 0;

    while (len = lzw_readbuf(fin, buf, sizeof(buf))) {
        for (int i = 0; i < len; i++) {
            if (buf[i] == '\n') {
                read_num_lines++;
                if (read_num_lines >= num_lines) {
                    len = i;
                    break;
                }
            }
        }
        encoded_bytes += lzw_encode(ctx, buf, len);
        if (read_num_lines >= num_lines) {
            break;
        }
    }

    lzw_enc_end(ctx);

    fclose(fdic);
    fclose(fhash);
    fclose(fin);
    fclose(fout);

    return encoded_bytes;
}


//#define RUN_ENCODER_STRING_NUM

int main (int argc, char* argv[])
{
    FILE       *fin;
    FILE       *fout;
    FILE       *fdic;
    FILE       *fhash;
    lzw_enc_t  *ctx = &lzwe;
    unsigned   len;
    char       buf[256];

#ifdef TEST_ENCODER_STRING_NUM

    for (int i = 500; i <= 12500; i+= 500 ) {
        //    Haffman_myHeaderEncode("kodavr.bit", "kodavr.bht", "bonch.bbt", i);
        sprintf(buf, "bonch_lzw_%i.bin\0", i);
        encodeToFile(ctx, argv, buf, i);
    }
    return 0;
#endif

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

    //return test_compare(fdic);

#ifndef DISABLE_ADD_NEW_NODE

    lzw_enc_init(ctx, fout, NULL, 0, &dictionary[0], &hash_table[0], DICTIONARY_SIZE);
#else
#ifndef READ_DICT_FROM_ARRAY
    lzw_enc_restore(ctx, fout, NULL, 0, dictionary, hash_table, DICTIONARY_SIZE);
    read_file_to_buffer(ctx->dict, sizeof(node_lzw_t), fdic);
    read_file_to_buffer(ctx->hash, sizeof(int), fhash);
#else
    int * p_size = ( int *) dictionary;
    //lzw_enc_restore(ctx, fout, NULL, 0, &dictionary_lzw_bin[4], &hash_lzw_bin[4], *(int *)dictionary_lzw_bin);
    lzw_enc_restore(&lzwe, fout, encoded_array, LZW_TEST_ARRAY_SIZE, (node_lzw_t *)&dictionary_lzw_bin[4], (int *)&hash_lzw_bin[4], *(int *)dictionary_lzw_bin);
#endif
#endif
#ifndef USE_OUTPUT_BUFFER
    fseek(fin, 0, SEEK_SET);
    while (len = lzw_readbuf(fin, buf, sizeof(buf))) {
        lzw_encode(ctx, buf, len);
    }
#else
    for (int i = 0; i < 1; i++) {
        lzw_encode(ctx, str, strlen(str));
    }

#endif
    lzw_enc_end(ctx);

#ifdef USE_OUTPUT_BUFFER
    printf("encoded buf len %d:  ", ctx->e_pos);
    for (int i = 0; i < LZW_TEST_ARRAY_SIZE; i++)
    {
        printf("0x%X ", encoded_array[i]);
    }
    printf("\n");
#endif

#ifndef DISABLE_ADD_NEW_NODE
    int dic_size = DICTIONARY_SIZE;
    printf("DICTIONARY_SIZE 0x%x \n", DICTIONARY_SIZE);
    fwrite(&dic_size, sizeof(int), 1, fdic);
    fwrite(ctx->dict, sizeof(node_lzw_t), DICTIONARY_SIZE, fdic);
    fwrite(&dic_size, sizeof(int), 1, fhash);
    fwrite(ctx->hash, sizeof(int), DICTIONARY_SIZE, fhash);

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

    printf("max_index %d, max_value %d, ctx->max %d\n", max_index, max_value, ctx->max);

    fclose(fdic);
    fclose(fhash);

    fclose(fin);
    fclose(fout);


    return 0;
}