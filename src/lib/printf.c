#include <stdarg.h>
#include "common/types.h"
#include "dev/uart.h"
#include "lock/mutex.h"
static void print_char(char ch, int32_t length, int32_t ladjust)
{
    int32_t i;

    if (length < 1)
    {
        length = 1;
    }
    const char space = ' ';
    if (ladjust)
    {
        uart_putchar(ch);
        for (i = 1; i < length; i++)
        {
            uart_putchar(space);
        }
    }
    else
    {
        for (i = 0; i < length - 1; i++)
        {
            uart_putchar(space);
        }
        uart_putchar(ch);
    }
}
static void print_num(uint64_t u, int32_t base, int32_t neg_flag, int32_t length,
                      int32_t ladjust, uint8_t padc, int32_t upcase)
{
    /* algorithm :
     *  1. prints the number from left to right in reverse form.
     *  2. fill the remaining spaces with padc if length is longer than
     *     the actual length
     *     TRICKY : if left adjusted, no "0" padding.
     *		    if negtive, insert  "0" padding between "0" and number.
     *  3. if (!ladjust) we reverse the whole string including paddings
     *  4. otherwise we only reverse the actual string representing the num.
     */

    int32_t actualLength = 0;
    char buf[length + 70];
    char *p = buf;
    int32_t i;

    do
    {
        int32_t tmp = u % base;
        if (tmp <= 9)
        {
            *p++ = '0' + tmp;
        }
        else if (upcase)
        {
            *p++ = 'A' + tmp - 10;
        }
        else
        {
            *p++ = 'a' + tmp - 10;
        }
        u /= base;
    } while (u != 0);

    if (neg_flag)
    {
        *p++ = '-';
    }

    /* figure out actual length and adjust the maximum length */
    actualLength = p - buf;
    if (length < actualLength)
    {
        length = actualLength;
    }

    /* add padding */
    if (ladjust)
    {
        padc = ' ';
    }
    if (neg_flag && !ladjust && (padc == '0'))
    {
        for (i = actualLength - 1; i < length - 1; i++)
        {
            buf[i] = padc;
        }
        buf[length - 1] = '-';
    }
    else
    {
        for (i = actualLength; i < length; i++)
        {
            buf[i] = padc;
        }
    }

    /* prepare to reverse the string */
    int32_t begin = 0;
    int32_t end;
    if (ladjust)
    {
        end = actualLength - 1;
    }
    else
    {
        end = length - 1;
    }

    /* adjust the string pointer */
    while (end > begin)
    {
        char tmp = buf[begin];
        buf[begin] = buf[end];
        buf[end] = tmp;
        begin++;
        end--;
    }
    for (i = 0; i < length; i++)
    {
        uart_putchar(buf[i]);
    }
}
void print_str(const char *s, int32_t length, int32_t ladjust)
{
    int32_t i;
    int32_t len = 0;
    const char *s1 = s;
    while (*s1++)
    {
        len++;
    }
    if (length < len)
    {
        length = len;
    }

    if (ladjust)
    {
        for (i = 0; i < len; i++)
        {
            uart_putchar(s[i]);
        }
        for (i = len; i < length; i++)
        {
            uart_putchar(' ');
        }
    }
    else
    {
        for (i = 0; i < length - len; i++)
        {
            uart_putchar(' ');
        }
        for (i = 0; i < len; i++)
        {
            uart_putchar(s[i]);
        }
    }
}
static void vprintfmt(const char *fmt, va_list ap)
{
    char c;
    const char *s;
    int64_t num;
    int32_t i;

    int32_t width;
    int32_t long_flag; // output is long (rather than int)
    int32_t neg_flag;  // output is negative
    int32_t ladjust;   // output is left-aligned
    uint8_t padc;      // padding char

    /* ----- MOS EXERCISE 1 vprintfmt AFTER boot BEGIN ----- */
    for (;;)
    {
        /* scan for the next '%' */
        // ----- MOS BLANK BEGIN -----
        int32_t length = 0;
        s = fmt;
        for (; *fmt != '\0'; fmt++)
        {
            if (*fmt != '%')
            {
                length++;
            }
            else
            {
                for (i = 0; i < length; i++)
                {
                    uart_putchar(s[i]);
                }
                length = 0;
                fmt++;
                break;
            }
        }
        // ----- MOS BLANK END -----

        /* flush the string found so far */
        // ----- MOS BLANK BEGIN -----
        for (i = 0; i < length; i++)
        {
            uart_putchar(s[i]);
        }
        // ----- MOS BLANK END -----

        /* check "are we hitting the end?" */
        // ----- MOS BLANK BEGIN -----
        if (!*fmt)
        {
            break;
        }
        // ----- MOS BLANK END -----

        /* we found a '%' */
        // ----- MOS BLANK BEGIN -----
        ladjust = 0;
        padc = ' ';
        // ----- MOS BLANK END -----

        /* check format flag */
        // ----- MOS BLANK BEGIN -----
        if (*fmt == '-')
        {
            ladjust = 1;
            padc = ' ';
            fmt++;
        }
        else if (*fmt == '0')
        {
            ladjust = 0;
            padc = '0';
            fmt++;
        }
        // ----- MOS BLANK END -----

        /* get width */
        // ----- MOS BLANK BEGIN -----
        width = 0;
        while ((*fmt >= '0') && (*fmt <= '9'))
        {
            width = width * 10 + (*fmt) - '0';
            fmt++;
        }
        // ----- MOS BLANK END -----

        /* check for long */
        // ----- MOS BLANK BEGIN -----
        long_flag = 0;
        /* 支持long long*/
        while (*fmt == 'l')
        {
            long_flag = 1;
            fmt++;
        }
        // ----- MOS BLANK END -----

        neg_flag = 0;
        switch (*fmt)
        {
        case 'b':
            if (long_flag)
            {
                num = va_arg(ap, int64_t);
            }
            else
            {
                num = va_arg(ap, int32_t);
            }
            print_num(num, 2, 0, width, ladjust, padc, 0);
            break;

        case 'd':
        case 'D':
            if (long_flag)
            {
                num = va_arg(ap, int64_t);
            }
            else
            {
                num = va_arg(ap, int32_t);
            }

            /*
             * Refer to other parts (case 'b', case 'o', etc.) and func 'print_num' to
             * complete this part. Think the differences between case 'd' and the
             * others. (hint: 'neg_flag').
             */
            // ----- MOS BLANK BEGIN -----
            neg_flag = num < 0;
            num = neg_flag ? -num : num;
            print_num(num, 10, neg_flag, width, ladjust, padc, 0);
            // ----- MOS BLANK END -----

            break;

        case 'o':
        case 'O':
            if (long_flag)
            {
                num = va_arg(ap, uint64_t);
            }
            else
            {
                num = va_arg(ap, uint32_t);
            }
            print_num(num, 8, 0, width, ladjust, padc, 0);
            break;

        case 'u':
        case 'U':
            if (long_flag)
            {
                num = va_arg(ap, uint64_t);
            }
            else
            {
                num = va_arg(ap, uint32_t);
            }
            print_num(num, 10, 0, width, ladjust, padc, 0);
            break;

        case 'x':
        case 'p':
            if (long_flag)
            {
                num = va_arg(ap, uint64_t);
            }
            else
            {
                num = va_arg(ap, uint32_t);
            }
            print_num(num, 16, 0, width, ladjust, padc, 0);
            break;

        case 'X':
            if (long_flag)
            {
                num = va_arg(ap, uint64_t);
            }
            else
            {
                num = va_arg(ap, uint32_t);
            }
            print_num(num, 16, 0, width, ladjust, padc, 1);
            break;

        case 'c':
            c = (char)va_arg(ap, int32_t);
            print_char(c, width, ladjust);
            break;

        case 's':
            s = (char *)va_arg(ap, char *);
            print_str(s, width, ladjust);
            break;

        case '\0':
            fmt--;
            break;

        default:
            /* output this char as it is */
            uart_putchar(*fmt);
        }
        fmt++;
    }
    /* ----- MOS EXERCISE END ----- */
}
/**
 * @brief 用于内核启动阶段主核hart0初始化打印，调用stdarg标准库函数，不存在动态链接
 *
 * @param fmt
 * @param ...
 */
void early_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    /* 不加锁*/
    vprintfmt(fmt, ap);
    /* 不解锁*/

    va_end(ap);
}
/**
 * @brief printf函数初始化
 * 
 */
void printf_init(void)
{
    /* 不可重入*/
    mutex_init(&pr_lock, "printf_mutex", MUTEX_TYPE_SPIN);
}
/**
 * @brief printf函数(支持互斥锁)
 * 
 * @param fmt 
 * @param ... 
 */
void printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    mutex_lock(&pr_lock);
    vprintfmt(fmt, ap);
    mutex_unlock(&pr_lock);

    va_end(ap);
}
/**
 * @brief 打印系统logo，内核启动完毕后打印
 * 
 */
void logo_init(void)
{
    printf("\n"); 
	printf("        JJJ         AAAAA  EEEEEEEEEE        .\"OOOOOOO\".    .SSSSSSSS.\n");
	printf("        JJJ        AA  AA  EEE              OOO\"     \"OOO  SSSS    SSSS\n");
	printf("        JJJ       AA   AA  EEE              OOO       OOO  SSSS.\n");
	printf("        JJJ      AAA   AA  EEEEEEEEEE       OOO       OOO   \"SSSSS.\n");
	printf("        JJJ     AAA    AA  EEEEEEEEEE       OOO       OOO      \"SSSS.\n");
	printf(" JJ     JJJ    AAAAAAAAAA  EEE              OOO       OOO        \"SSS\n");
	printf(" JJJJJJJJJJ   AAAA    AAA  EEE              OOO\"     \"OOO  SSSS    SSSS\n");
	printf(" JJJJJJJJJJ  AAAAA    AAA  EEEEEEEEEE        \".OOOOOOO.\"     \"SSSSSSSS\"\n");
	printf("\n");
}