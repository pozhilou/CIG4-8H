#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <byteswap.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "arm_pcie_protocol.h"

/* ltoh: little to host */
/* htol: little to host */
/* #if __BYTE_ORDER == __LITTLE_ENDIAN
#define ltohl(x) (x)
#define ltohs(x) (x)
#define htoll(x) (x)
#define htols(x) (x)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define ltohl(x) __bswap_32(x)
#define ltohs(x) __bswap_16(x)
#define htoll(x) __bswap_32(x)
#define htols(x) __bswap_16(x)
#endif

#define PRER 0x0
#define CTR 0x4
#define TXR 0x8
#define RXR 0xc
#define CR 0x10
#define SR 0x14 */

#define RESULT_BUF_LENGTH 4096

extern uint32_t xdma_reg_read(uint32_t addr);

extern uint32_t xdma_reg_write(uint32_t addr, uint32_t value);

extern uint32_t xdma_reg_write_bytes(uint32_t addr, uint8_t value);

extern uint32_t xdma_reg_write_word(uint32_t addr, uint16_t value);

extern uint32_t xdma_memcpy_write(uint32_t addr, unsigned char *value, size_t n);

char cmd_result[RESULT_BUF_LENGTH];

//
uint32_t get_cmd_result()
{
    uint32_t result;
    /*if(cmd_result[0] == '0' && cmd_result[1] == 'x')
    {
            result = atoi(cmd_result[2]);

            if(result ==0)
            {
                result = atoi
            }
    }*/
    result = strtol(cmd_result, NULL,16);
    //printf("get cmd result:0x%x\n",result);
    return result;
}

uint32_t send_cmd_to_a53_sync(const char send_cmd[])
{
    uint32_t cmd_length = strlen(send_cmd);
    uint32_t result_length = 0;
    uint32_t cursor = 0, buf_cursor = 0;
    uint32_t word;
    uint32_t loop_count = 0;
    uint32_t result = 0;

    // pc set flag register as 0x5a5aa5a5
    xdma_reg_write(0, 0x5a5aa5a5);

    // pc write cmd data.
    // start address at 0x8, 4 bytes once write.
    // write data length at 0x4 after cmd date write completely.
    do
    {
        if (cursor + 4 <= cmd_length)
        {
            word = 0;
            memcpy((void *)&word, (const void *)send_cmd + cursor, 4);
            xdma_reg_write(cursor + 8, word);
        }
        else if (cursor < cmd_length)
        {
            word = 0;
            memcpy((void *)&word, (const void *)send_cmd + cursor, cmd_length - cursor);
            xdma_reg_write(cursor + 8, word);
        }
        else // cursor >= cmd_length
        {
            break;
        }
        cursor += 4;
    } while (1);
    xdma_reg_write(0x4, cmd_length);

    // write correspondence type register 0x30304 as CMD_SHELL(0x3)
    xdma_reg_write(0x30304, 0x3);

    // write interrupt register 0x30300 as 1
    xdma_reg_write(0x30300, 0x01);

    // read a53 interrupt status 1000 times
    // 0:interrupt clear
    // !0:interrupt not clear
    // if a53 malfunction, should clear interrupt manually

    while (loop_count < 1000)
    {
        usleep(100);
        result = xdma_reg_read(0x30300);
        if (result == 0)
        {
            break;
        }
        loop_count += 1;
    }
    if (loop_count == 1000 && (result != 0))
    {
        xdma_reg_write(0x30300, 0x0);
        // str_result = "Interrupt state is error, A53 malfunction ";
        printf("Interrupt state is error, A53 malfunction \n");
        return -1;
    }

    // read cmd execute result at 0x30308
    // 0 OK !0 Error
    result = xdma_reg_read(0x30308);
    if (result != 0)
    {
        printf(" read cmd execute result error, A53 malfunction \n");
        return -1;
    }

    // read flag at 0x8000
    // 0x5a5aa5a5 ok
    word = xdma_reg_read(0x8000);
    if (word != 0x5a5aa5a5)
    {
        printf("read flag error, A53 malfunction \n");
        return -1;
    }
    // read cmd execute result data
    // read log length at 0x8004
    // read log data begin with 0x8008
    result_length = xdma_reg_read(0x8004);

    cursor = (result_length < RESULT_BUF_LENGTH) ? 0 : (result_length - result_length % RESULT_BUF_LENGTH);
    buf_cursor = 0;
    word = 0;
    // str_result = "A53 cmd execute:%s\n" % input_str
    // print(str_result)
    // logging.info(str_result)
    memset(cmd_result, 0, RESULT_BUF_LENGTH);

    while (cursor < result_length)
    {
        cursor < result_length;
        word = xdma_reg_read(0x8008 + cursor);

        memcpy(cmd_result + buf_cursor, &word, 4);
        cursor += 4;
        buf_cursor += 4;
    }

    result_length = buf_cursor;

    // str_result = "A53 cmd result (len:%d): %s\n" % (length, output_str)
    // print(str_result)
    // logging.info(str_result)
    printf("cmd result: \n%s\n", cmd_result);
    return 0;
}

//typedef struct bram_cmd_file
//{
//    uint32_t uiFlag;          // file flag CMD_FILE_FLAG 4
//    char acFilePath[96];      // file path, max 96 chars 1
//    uint32_t uiTotalLen;      // file total length 4
//    uint16_t uwWrittenLen;    // file already write length, write from here 2
//    uint16_t uwDataLen;       // data size in this cmd 2
//    uint8_t ucIsFileSync;     // last set 1, use fsync(fileno(fp)), makesure wrtie emmc succ 1
//    uint8_t ucCheckSum;       // aucData byte sum 1
//    uint8_t ucReserved[2];     // 1
//    uint8_t aucData[0xFF80];  // cmd data, max 0xFF80
//} BRAM_CMD_FILE_S;

uint32_t send_file_to_a53_sync(const char *file_path)
{
    uint32_t file_size;
    uint32_t result_length = 0;
    uint32_t cursor = 0;
    uint16_t buf_cursor = 0;
    uint32_t word;
    uint32_t loop_count = 0;
    uint32_t result = 0;
    FILE * pInFile = NULL;
    char ps_path[96] = {0};
    int ps_path_lenth = 0;
    int a = 0, b = 0;
    unsigned char* buffer;
    size_t read_size = 0;
    long long cur_size;
    int ret = 0;
    uint32_t checksum = 0;
    uint8_t last_byte = 0;
    uint8_t result_value;

    printf("%s file name:%s\n", __func__, file_path);
    pInFile = fopen(file_path, "rb");
    if(NULL == pInFile)
    {
        perror("open error");
        return -1;
    }
    //将文件指针移到文件末尾,获取文件大小
    fseek(pInFile, 0, SEEK_END);
    file_size = ftell(pInFile);
    rewind(pInFile); //将文件指计重新定位到文件开头
    printf("%s file size: %d bytes\n", file_path, file_size);

    buffer = (unsigned char *)malloc(file_size);
    if (buffer == NULL)
        printf("malloc buf failed\n");

    #if 0
    read_size = fread(buffer, 1, file_size, pInFile);
    if(read_size != file_size)
    {
        perror("Error reading file");
        free(buffer);
        fclose(pInFile);
        return 1;
    }
    #endif

    //for(size_t i=0; i < file_size; i++)
    //{
    //    printf("0x%02X", buffer[i]);
    //}
    printf("\n");
    sprintf(ps_path, "/etc/common/%s", file_path);
    printf("ps_path: %s, size:%ld\n", ps_path, strlen(ps_path));
    // pc set flag register as 0x5B5BB5B5
    xdma_reg_write(0, 0x5B5BB5B5);
    //pc write file path.
    //start address at 0x4, 4 bytes once write.
    ps_path_lenth = strlen(ps_path);
    do
    {
		printf("%s(%d) cursor:%d, ps_path_lenth:%d\n", __func__, __LINE__, cursor, ps_path_lenth);
        if (cursor + 4 <= ps_path_lenth)
        {
            word = 0;
            memcpy((void *)&word, (const void *)ps_path + cursor, 4);
			printf("%s(%d) word:%s\n", __func__, __LINE__, (void *)&word);
            xdma_reg_write(cursor + 4, word);
        }
        else if (cursor < ps_path_lenth)
        {
            word = 0;
            memcpy((void *)&word, (const void *)ps_path + cursor, ps_path_lenth - cursor);
            printf("%s(%d) word:%s\n", __func__, __LINE__, (void *)&word);
			xdma_reg_write(cursor + 4, word);
        }
        else // cursor >= path_length
        {
            break;
        }
        cursor += 4;
    } while (1);

    xdma_reg_write(100, file_size); // uiTotalLen
    uint32_t a1 = xdma_reg_read(100);
    printf("a1: %d\n", a1);

    uint32_t b1 = 0;
    int i;
    #define UDATALEN 0xF600
    a = file_size / UDATALEN;
    b = file_size % UDATALEN;
    printf("%s(%d) a:%d, b:%d\n", __func__, __LINE__, a, b);
    for(i = 0; i < a; i++)
    {
        memset(buffer, 0, UDATALEN);
        ret = fseek(pInFile, i * UDATALEN, SEEK_SET);
        if(!ret)
        {
            cur_size = ftell(pInFile); //计算往后偏移了几帧
            read_size = fread(buffer, 1, UDATALEN, pInFile);
            if(read_size != UDATALEN)
            {
                perror("Error reading file");
                free(buffer);
                fclose(pInFile);
                return 1;
            }
            printf("read_size:%lld, i:%d, cur_size:%lld\n", read_size, i, cur_size);
        }

        xdma_reg_write(104, i * UDATALEN); // uiWrittenLen offset
        xdma_reg_write(108, UDATALEN); // uwDataLen
        b1 = xdma_reg_read(108);
        printf("b1: 0x%04x\n", b1);
        xdma_reg_write(110, 0);
        checksum = 0;
        for(size_t i = 0; i < read_size; i++)
        {
            checksum = checksum + buffer[i];
        }
        last_byte = checksum & 0xFF;

        printf("%s(%d) last_byte:0x%02x\n", __func__, __LINE__, last_byte);
        //xdma_reg_write(cursor + 14, last_byte);
        xdma_reg_write_bytes(111, last_byte);
        result_value = xdma_reg_read(111);
        printf("%s(%d) result_value:0x%02x\n", __func__, __LINE__, result_value);
        xdma_memcpy_write(120, buffer, UDATALEN);

        // write correspondence type register 0x30304 as CMD_FILE(0x4)
        xdma_reg_write(0x30304, 0x4);

        // write interrupt register 0x30300 as 1
        xdma_reg_write(0x30300, 0x01);

        // read a53 interrupt status 1000 times
        // 0:interrupt clear
        // !0:interrupt not clear
        // if a53 malfunction, should clear interrupt manually
        while (loop_count < 100000)
        {
            usleep(100);
            result = xdma_reg_read(0x30300);
            if (result == 0)
            {
                break;
            }
            loop_count += 1;
        }
        if (loop_count == 100000 && (result != 0))
        {
            xdma_reg_write(0x30300, 0x0);
            // str_result = "Interrupt state is error, A53 malfunction ";
            printf("%s Interrupt state is error, A53 malfunction \n", __FILE__);
            return -1;
        }
        // read cmd execute result at 0x30308
        // 0 OK !0 Error
        result = xdma_reg_read(0x30308);
        if (result != 0)
        {
            printf("%s(%d) read cmd execute result error:%d, A53 malfunction \n", __func__, __LINE__, result);
            return -1;
        }
    }

    memset(buffer, 0, b);
    printf("%s(%d) i:%d\n", __func__, __LINE__, i);
    ret = fseek(pInFile, i * UDATALEN, SEEK_SET);
    if(!ret)
    {
        cur_size = ftell(pInFile); //计算往后偏移了几帧
        read_size = fread(buffer, 1, b, pInFile);
        printf("(%d) read_size:%d, b:%d\n", __LINE__, read_size, b);
        if(read_size != b)
        {
            perror("Error reading file");
            free(buffer);
            fclose(pInFile);
            return 1;
        }
        printf("read_size:%lld, cur_size:%lld\n", read_size, cur_size);
    }

    printf("(%d) read_size:%d, b:%d\n", __LINE__, read_size, b);
    xdma_reg_write(104, i * UDATALEN); // uiWrittenLen offset
    xdma_reg_write(108, b); // uwDataLen
    b1 = xdma_reg_read(108);
    printf("b1: %d\n", b1);
    xdma_reg_write(110, 1); // ucIsFileSync
    checksum = 0;
    for(size_t i = 0; i < read_size; i++)
    {
        checksum = checksum + buffer[i];
    }
    last_byte = checksum & 0xFF;

    printf("%s(%d) last_byte:0x%02x\n", __func__, __LINE__, last_byte);
    //xdma_reg_write(cursor + 14, last_byte);
    xdma_reg_write_bytes(111, last_byte); // ucCheckSum
    result_value = xdma_reg_read(111);
    printf("%s(%d) result_value:0x%02x\n", __func__, __LINE__, result_value);
    xdma_memcpy_write(120, buffer, b); // aucData

    // write correspondence type register 0x30304 as CMD_FILE(0x4)
    xdma_reg_write(0x30304, 0x4);
    // write interrupt register 0x30300 as 1
    xdma_reg_write(0x30300, 0x01);

    while (loop_count < 100000)
    {
        usleep(100);
        result = xdma_reg_read(0x30300);
        if (result == 0)
        {
            break;
        }
        loop_count += 1;
    }
    if (loop_count == 100000 && (result != 0))
    {
        xdma_reg_write(0x30300, 0x0);
        // str_result = "Interrupt state is error, A53 malfunction ";
        printf("%s Interrupt state is error, A53 malfunction \n", __FILE__);
        return -1;
    }
    // read cmd execute result at 0x30308
    // 0 OK !0 Error
    result = xdma_reg_read(0x30308);
    if (result != 0)
    {
        printf("%s(%d) read cmd execute result error:%d, A53 malfunction \n", __func__, __LINE__, result);
        return -1;
    }

#if 0
    a = file_size / 0xFF80;
    b = file_size % 0xFF80;
    printf("%s(%d) a:0x%02x, b:0x%04x\n", __func__, __LINE__, a, b);

    for(int i = 0; i < a; i++)
    {
        for (size_t j = 0; j < 0xff80/4; j++)
        {
            word = 0;
            memset(buffer, 0, 4);
            ret = fseek(pInFile, 4 * (j+a), SEEK_SET);
            if(!ret)
            {
                cur_size = ftell(pInFile); //计算往后偏移了几帧
                read_size = fread(buffer, 1, 4, pInFile);
                printf("read_size:%lld, cur_size:%lld, 0x%04x\n", read_size, cur_size, buffer);
                memcpy((void *)&word, buffer, 4);
            }

            printf("%s(%d) j:%d, word:0x%04x\n", __func__, __LINE__, j, (void *)&word);
            xdma_reg_write(cursor + 9, buf_cursor);
            buf_cursor += 4;
            xdma_reg_write(cursor + 11, 4);
            xdma_reg_write(cursor + 13, 0);
            //xdma_reg_write(cursor + 14, 0);
            xdma_reg_write(cursor + 16, word);
        }
    }
    xdma_reg_write(cursor + 13, 1);
#endif

    // pc write file data.
    // start address at 0xFF, 4 bytes once write.
    // write data length at 0x4 after cmd date write completely.

    //释放内存并关闲文件
    free(buffer);
    fclose(pInFile);
    return 0; 
}
