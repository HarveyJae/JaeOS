#include "../include/printf.h"
#include <stdarg.h>
#include "uart.h"

#ifdef USE_QEMU_VIRT
void putchar(int8_t ch)
{
    uart_putchar((int64_t)ch);
}
#endif // USE_QEMU_VIRT

/* 刷新缓冲区 */
static void flush_buffer(int8_t *buf, int32_t *pos)
{
    for (int32_t i = 0; i < *pos; i++)
    {
        putchar(buf[i]);
    }
    *pos = 0;
}
/**
 * 强化数字打印
 * 输出缓冲区
 * num:待打印数字
 * base:进制
 * sign:有符号1 or 无符号0
 * min_width:宽度，最低不会低于该宽度
 * 宽度检测待补充：超出最大指定宽度发生错误
 * 目前该函数默认数字宽度不会越界
 */
uint32_t print_number(int8_t *output_buffer, uint32_t *buffer_pos, uint64_t num, int8_t base, int8_t sign, int8_t min_width)
{
    static int8_t digits[] = "0123456789ABCDEF";
    int8_t buffer[NUM_MAX_WIDTH];
    uint8_t cnt = 0;           /* 对字符进行计数 */
    uint8_t negative_flag = 0; /* 正负数标志 */
    /* 有符号数负数 */
    if (sign && (int64_t)num < 0)
    {
        negative_flag = 1;
        /* 将负数转换为正数，最后额外补充负号 */
        num = -(int64_t)num;
    }
    /* 将数字转换为字符串(逆序) */
    while (num > 0)
    {
        buffer[cnt++] = digits[num % base];
        num / base;
    }
    /* 添加负号 */
    if (negative_flag)
    {
        buffer[cnt++] = '-';
    }
    /* 计算需要填充的空格数 */
    int8_t padding = (int8_t)cnt - min_width - (negative_flag ? (int8_t)1 : (int8_t)0);
    while (padding-- > 0)
    {
        buffer[cnt++] = ' ';
    }

    /* 写入输出缓冲区 */
    while (--cnt >= 0)
    {
        if (buffer_pos >= BUFFER_SIZE)
        {
            flush_buffer(output_buffer, buffer_pos);
        }
        output_buffer[*buffer_pos] = buffer[cnt];
    }
}
/**
 * vprintf支持缓冲区，缓冲区满刷新缓冲区
 * 暂不支持遇到换行符刷新缓冲区
 * 将缓冲区定义在栈中，后续可以优化到全局缓冲区
 * 后续实现printf函数的返回值应用
 */
static void vprintf(const char *fmt, va_list args)
{

    int8_t output_buffer[BUFFER_SIZE]; /* 缓冲区 */
    int32_t buffer_pos = 0;            /* 缓冲区结尾指针的下一位 */
    uint8_t width = 0;                 /* 宽度修饰符 */
    uint8_t long_flag = 0;             /* 是否使用L后缀 */
    /* 遍历字符串 */
    for (; *fmt; fmt++)
    {
        if (*fmt != '%')
        {
            if (buffer_pos >= BUFFER_SIZE)
            {
                /* 检查缓缓冲区是否满 */
                /* 当buffer_pos = BUFFER_SIZE的时候，缓冲区满 */
                flush_buffer(output_buffer, &buffer_pos);
            }
            output_buffer[buffer_pos++] = *fmt;
            continue;
        }
        else /* *fmt == '%'*/
        {
            fmt++; /* 跳过% */
            /* 解析格式修饰符 */
            /* 解析宽度:%5d */
            while (*fmt >= '0' && *fmt <= '9')
            {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }
            /* 解析L标志 */
            if (*fmt == 'L' || *fmt == 'l')
            {
                long_flag = 1;
                fmt++;
                if (*fmt == 'L' || *fmt == 'l')
                {
                    long_flag = 2;
                    fmt++;
                }
            }
            /* 处理格式字符 */
            switch (*fmt)
            {
            case 's':
            {
                int8_t *s = va_arg(args, int8_t *);
                while (*s && buffer_pos < BUFFER_SIZE)
                {
                    if (buffer_pos >= BUFFER_SIZE)
                    {
                        /* 检查缓缓冲区是否满 */
                        flush_buffer(output_buffer, &buffer_pos);
                    }
                    output_buffer[buffer_pos++] = *s++;
                }
                break;
            }
            case 'c':
            {
                int8_t ch = va_arg(args, int32_t); /* 这里是否会有问题 */
                if (buffer_pos >= BUFFER_SIZE)
                {
                    /* 检查缓缓冲区是否满 */
                    flush_buffer(output_buffer, &buffer_pos);
                }
                output_buffer[buffer_pos++] = ch;
                break;
            }
            case 'd':
            {
                /* 无论是L还是LL均以64位输出 */
                int64_t num = long_flag ? va_arg(args, int64_t) : va_arg(args, int32_t);
                print_number(output_buffer, &buffer_pos, num, 10, 1, width); /* 十进制有符号数 */
                break;
            }

            case 'x':
            case 'X':
            {
                uint64_t num = long_flag ? va_arg(args, uint64_t) : va_arg(args, uint32_t);
                print_number(output_buffer, &buffer_pos, num, 16, 0, width); /* 十六进制无符号数 */
                break;
            }
            case 'p':
            {
                void *p = va_arg(args, void *);
                if (buffer_pos >= BUFFER_SIZE)
                {
                    /* 检查缓缓冲区是否满 */
                    flush_buffer(output_buffer, &buffer_pos);
                }
                output_buffer[buffer_pos++] = '0';
                if (buffer_pos >= BUFFER_SIZE)
                {
                    /* 检查缓缓冲区是否满 */
                    flush_buffer(output_buffer, &buffer_pos);
                }
                output_buffer[buffer_pos++] = 'x';
                print_number(output_buffer, &buffer_pos, p, 16, 0, 8); /* 固定长度为8的十六进制数 */
                break;
            }
            case '%':
            {
                if (buffer_pos >= BUFFER_SIZE)
                {
                    /* 检查缓缓冲区是否满 */
                    flush_buffer(output_buffer, &buffer_pos);
                }
                output_buffer[buffer_pos++] = '%';
                break;
            }
            default: /* 该处应添加报错逻辑，目前 nothing to do */
                break;
            }
        }
    }
    /* 刷新剩下的缓冲区 */
    if (buffer_pos > 0)
    {
        flush_buffer(output_buffer, &buffer_pos);
    }
}
/**
 * 内核初始化时的printf函数
 * 不支持线程安全
 */
void early_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    /* va_list va_start va_end是GCC编译器在stdarg.h中提供的处理可变参数的宏 */
    /* va_list类型用于跟踪可变参数的位置，可变参数可能存储在整数寄存器/浮点数寄存器/栈中，因此需要三个位置的指针 */
    vprintf(fmt, args);
    va_end(args);
}