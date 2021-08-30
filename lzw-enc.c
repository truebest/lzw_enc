#include <stdio.h>
#include "lzw.h"

/**
 * \brief Запись битов в битовый буфер. Количество бит не должно превышать 24.
 * \param ctx - указатель на контекст LZW
 * \param bits - биты для записи
 * \param nbits - количество бит для записи, 0-24
 */
static void lzw_enc_writebits(lzw_enc_t *const ctx, unsigned bits, unsigned nbits)
{
    // shift old bits to the left, add new to the right
    ctx->bb.buf = (ctx->bb.buf << nbits) | (bits & ((1 << nbits)-1));

    nbits += ctx->bb.n;

    // flush whole bytes
    while (nbits >= 8)
    {
        nbits -= 8;
        ctx->buff[ctx->lzwn++] = ctx->bb.buf >> nbits;

        if (ctx->lzwn == sizeof(ctx->buff)) {
            ctx->lzwn = 0;
#ifdef USE_OUTPUT_BUFFER
            for (int i = 0; i < sizeof(ctx->buff); i++) {
                if (ctx->e_pos < ctx->e_size) {
                    ctx->e_buf[ctx->e_pos++] = ctx->buff[i];
                }
            }
#endif
#ifdef USE_OUTPUT_FILE
            lzw_writebuf(ctx->stream, ctx->buff, sizeof(ctx->buff));
#endif
        }
    }

    ctx->bb.n = nbits;
}

/**
 * \brief Функция хеширования используется для поиска комбинации <префикс> + <символ>
 * \param code - код префикса
 * \param c - символ
 * \return - хеш-код
 */
__inline static int lzw_hash(const int dh_size, const int code, const unsigned char c)
{
    return (code ^ ((int)c << 6)) & (dh_size-1);
}

void lzw_enc_restore(lzw_enc_t *ctx, void *stream, char * buf, unsigned buf_size, void * p_dic, void * p_hash, int dh_size)
{
    ctx->code     = CODE_NULL; // non-existent code
    ctx->max      = 0;
    ctx->codesize = 17;
    ctx->e_buf    = buf;
    ctx->e_size   = buf_size;
    ctx->e_pos    = 0;
    ctx->bb.n     = 0; // bit-buffer init
    ctx->lzwn     = 0; // output code-buffer init
    ctx->stream   = stream;
    ctx->en_dic   = 0;
    ctx->dict     = p_dic;
    ctx->hash     = p_hash;
    ctx->dh_size  = dh_size;
}

/**
 * \brief Инициализирует контекст кодировщика LZW.
 * \param ctx - контекст LZW
 * \param stream - Указатель на объект потока ввода / вывода;
 */
void lzw_enc_init(lzw_enc_t *ctx, void *stream, char * buf, unsigned buf_size, node_lzw_t * p_dic, int * p_hash, int dh_size)
{
    unsigned i;

    ctx->code     = CODE_NULL; // non-existent code
    ctx->max      = 255;
    ctx->codesize = 8;
    ctx->e_buf    = buf;
    ctx->e_size   = buf_size;
    ctx->e_pos    = 0;
    ctx->bb.n     = 0; // bit-buffer init
    ctx->lzwn     = 0; // output code-buffer init
    ctx->stream   = stream;
    ctx->en_dic   = 1;
    ctx->dict     = p_dic;
    ctx->hash     = p_hash;
    ctx->dh_size  = dh_size;

    // clear hash table
    for (i = 0; i < ctx->dh_size; i++)
        ctx->hash[i] = CODE_NULL;

    for (i = 0; i < 256; i++)
    {
        int hash = lzw_hash(ctx->dh_size, CODE_NULL, i);

        ctx->dict[i].prev  = CODE_NULL;
        ctx->dict[i].next  = ctx->hash[hash];
        ctx->dict[i].ch    = i;
        ctx->hash[hash]    = i;
    }
}

/**
 * \brief Сбросить контекст кодировщика LZW. Используется при переполнении словаря. Размер кода установлен на 8 бит.
 * \param ctx - контекст LZW
 */
static void lzw_enc_reset(lzw_enc_t *const ctx)
{
    unsigned i;

#if DEBUG
    printf("reset\n");
#endif

    ctx->max      = 255;
    ctx->codesize = 8;

    for (i = 0; i < ctx->dh_size; i++)
        ctx->hash[i] = CODE_NULL;

    for (i = 0; i < 256; i++)
    {
        int hash = lzw_hash(ctx->dh_size, CODE_NULL, i);

        ctx->dict[i].next  = ctx->hash[hash];
        ctx->hash[hash]    = i;
    }
}

/**
 * \brief Ищет строку в LZW словаре. Используется только в кодировщике.
 * \param ctx - контекст LZW
 * \param code - код начала строки (уже в словаре)
 * \param c - последний символ
 * \return код, представляющий строку или CODE_NULL, если словарь заполнен
 */
static int lzw_enc_findstr(lzw_enc_t *const ctx, int code, unsigned char c)
{
    int nc;

    // hash search
    for (nc = ctx->hash[lzw_hash(ctx->dh_size, code,c)]; nc != CODE_NULL; nc = ctx->dict[nc].next)
    {
        if (ctx->dict[nc].prev == code && ctx->dict[nc].ch == c) {
            break;
        }
    }

    return nc;
}

/**
 * \brief Добавляет строку в словарь LZW.
 * \param ctx - контекст LZW
 * \param code - код начала строки (уже в словаре)
 * \param c - последний символ
 * \return код, представляющий строку, или CODE_NULL, если словарь заполнен
 */
static int lzw_enc_addstr(lzw_enc_t *const ctx, int code, unsigned char c)
{
    int hash;

    if (ctx->max + 1 == CODE_NULL) {
        return CODE_NULL;
    }
    else {
        ctx->max++;
    }

    hash = lzw_hash(ctx->dh_size, code, c);

    // добавить новый код
    ctx->dict[ctx->max].prev  = code;
    ctx->dict[ctx->max].next  = ctx->hash[hash];
    ctx->dict[ctx->max].ch    = c;
    // добавляем новый код в хеш-таблицу
    ctx->hash[hash] = ctx->max;
#if DEBUG
    printf("add code %x = %x + %c\n", ctx->max, code, c);
#endif

    return ctx->max;
}

/**
 * \brief ** Кодировать буфер по алгоритму LZW. Выходные данные записываются приложением
 * конкретный обратный вызов для потока, определенного приложением внутри этой функции.
 * \param ctx - контекст LZW
 * \param buf - входной байтовый буфер;
 * \param size - размер буфера
 * \return количество обработанных байтов.
 */
int lzw_encode(lzw_enc_t *ctx, char * buf, unsigned size)
{
    unsigned i;

    if (!size) return 0;

    for (i = 0; i < size; i++)
    {
        unsigned char c = buf[i];
        int           nc = lzw_enc_findstr(ctx, ctx->code, c);

        if (nc == CODE_NULL)
        {
            // строка не найдена - пишем <префикс>
            lzw_enc_writebits(ctx, ctx->code, ctx->codesize);
#ifndef DISABLE_ADD_NEW_NODE
#if DEBUG
            printf("code %x (%d)\n", ctx->code, ctx->codesize);
#endif
            if (ctx->en_dic) {
                // при необходимости увеличиваем размер кода (количество бит)
                if (ctx->max+1 == (1 << ctx->codesize)) {
                    ctx->codesize++;
                }

                // добавляем в словарь <префикс> + <текущий символ>
                if (lzw_enc_addstr(ctx, ctx->code, c) == CODE_NULL) {
                    // словарь заполнен - сбросить кодировщик
                    //lzw_enc_reset(ctx);
                    lzw_disable_update_dictionary(ctx);
                }
            }
#endif

            ctx->code = c;
        }
        else
        {
            ctx->code = nc;
        }
    }

    return size;
}


void lzw_disable_update_dictionary(lzw_enc_t *ctx)
{
    ctx->en_dic = 0;
}

/**
 * \brief Завершить процесс кодирования LZW. Поскольку выходные данные записываются в выходной поток
 * через битовый буфер он может содержать несохраненные данные. Эта функция сбрасывает битовый буфер
 * и заполняет последний байт нулевыми битами
 * \param ctx - контекст LZW
 */
void lzw_enc_end(lzw_enc_t *ctx)
{
#if DEBUG
    printf("code %x (%d)\n", ctx->code, ctx->codesize);
#endif
    // write last code
    lzw_enc_writebits(ctx, ctx->code, ctx->codesize);
    // flush bits in the bit-buffer
    if (ctx->bb.n)
        lzw_enc_writebits(ctx, 0, 8 - ctx->bb.n);
#ifdef USE_OUTPUT_FILE
    lzw_writebuf(ctx->stream, ctx->buff, ctx->lzwn);
#endif
#ifdef OUTPUT_BUFFER
    for (int i = 0; i < ctx->lzwn; i++) {
        if (ctx->e_pos < ctx->e_size) {
            ctx->e_buf[ctx->e_pos++] = ctx->buff[i];
        }
    }
#endif
}