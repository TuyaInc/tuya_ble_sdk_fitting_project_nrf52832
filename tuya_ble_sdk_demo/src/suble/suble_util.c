#include "suble_common.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/

/*********************************************************
FN: 
*/
uint8_t suble_util_check_sum8(uint8_t *buf, uint32_t size)
{
    uint8_t sum = 0;
    for(uint32_t idx=0; idx<size; idx++) {
        sum += buf[idx];
    }
    return sum;
}

/*********************************************************
FN: 
*/
uint16_t suble_util_check_sum16(uint8_t *buf, uint32_t size)
{
    uint16_t sum = 0;
    for(uint32_t idx=0; idx<size; idx++) {
        sum += buf[idx];
    }
    return sum;
}

/*********************************************************
FN: 
*/
uint16_t suble_util_crc16(uint8_t* buf, uint32_t size, uint16_t* p_crc)
{
    uint16_t poly[2] = {0, 0xa001}; //0x8005 <==> 0xa001
    uint16_t crc = 0xffff;
    int i, j;
    
    for(j=size; j>0; j--)
    {
        uint8_t ds = *buf++;
        for(i=0; i<8; i++)
        {
            crc = (crc >> 1) ^ poly[(crc ^ ds) & 1];
            ds = ds >> 1;
        }
    }
    return crc;
}

/*********************************************************
FN: 
*/
uint32_t suble_util_crc32(uint8_t* buf, uint32_t size, uint32_t* p_crc)
{
    uint32_t crc = (p_crc == NULL) ? 0xFFFFFFFF : ~(*p_crc);
    for (uint32_t i = 0; i < size; i++)
    {
        crc = crc ^ buf[i];
        for (uint32_t j = 8; j > 0; j--)
        {
            crc = (crc >> 1) ^ (0xEDB88320U & ((crc & 1) ? 0xFFFFFFFF : 0));
        }
    }
    return ~crc;
}

/*********************************************************
FN: 
*/
void suble_util_reverse_byte(void* buf, uint32_t size)
{
    uint8_t* p_tmp = buf;
    uint8_t  tmp;
    for(uint32_t idx=0; idx<size/2; idx++)
    {
        tmp = *(p_tmp+idx);
        *(p_tmp+idx) = *(p_tmp+size-1-idx);
        *(p_tmp+size-1-idx) = tmp;
    }
}

/*********************************************************
FN: 
*/
uint32_t suble_util_numarray2int(uint8_t *num_array, uint32_t start_idx, uint32_t size)
{
    if(start_idx >= size) {
        return (uint32_t)-1;
    }
    
	uint32_t tmp_int = 0;
	for (uint32_t idx=start_idx; idx<start_idx+size; idx++) {
		tmp_int = (tmp_int*10) + num_array[idx];
	}
	return tmp_int;
}




/*********************************************************  string  *********************************************************/

/*********************************************************
FN: 字符转int
*/
uint8_t suble_util_str_hexchar2int(uint8_t hexchar)
{
    switch(hexchar){
        case '0':return 0;
        case '1':return 1;
        case '2':return 2;
        case '3':return 3;
        case '4':return 4;
        case '5':return 5;
        case '6':return 6;
        case '7':return 7;
        case '8':return 8;
        case '9':return 9;
        case 'a':
        case 'A':return 10;
        case 'b':
        case 'B':return 11;
        case 'c':
        case 'C':return 12;
        case 'd':
        case 'D':return 13;
        case 'e':
        case 'E':return 14;
        case 'f':
        case 'F':return 15;
        default: return (uint8_t)-1;
    }
}

/*********************************************************
FN: int转字符
*/
uint8_t suble_util_str_int2hexchar(uint8_t int_num)
{
    switch(int_num){
        case 0:return '0';
        case 1:return '1';
        case 2:return '2';
        case 3:return '3';
        case 4:return '4';
        case 5:return '5';
        case 6:return '6';
        case 7:return '7';
        case 8:return '8';
        case 9:return '9';
        case 10:return 'a';
        case 11:return 'b';
        case 12:return 'c';
        case 13:return 'd';
        case 14:return 'e';
        case 15:return 'f';
//        case 10:return 'A';
//        case 11:return 'B';
//        case 12:return 'C';
//        case 13:return 'D';
//        case 14:return 'E';
//        case 15:return 'F';
        default:return (uint8_t)-1;
    }
}

uint32_t suble_util_str_hexstr2int(uint8_t* hexstr, uint32_t size, int* sum)
{
    *sum = 0;
    for(uint32_t idx=0; idx<size; idx++)
    {
        uint8_t tmp = suble_util_str_hexchar2int(hexstr[idx]);
        if(tmp == (uint8_t)-1) {
            return SUBLE_ERROR_COMMON;
        }
        
        (*sum) = (*sum)<<4;
        (*sum) += tmp;
    }
    return SUBLE_SUCCESS;
}

uint32_t suble_util_str_intstr2int(uint8_t* intstr, uint32_t size, int* sum)
{
    *sum = 0;
    for(uint32_t idx=0; idx<size; idx++)
    {
        uint8_t tmp = suble_util_str_hexchar2int(intstr[idx]);
        if(tmp == (uint8_t)-1 || tmp > 9) {
            return SUBLE_ERROR_COMMON;
        }
        
        (*sum) = (*sum)*10;
        (*sum) += tmp;
    }
    return SUBLE_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t suble_util_str_hexstr2hexarray(uint8_t* hexstr, uint32_t size, uint8_t* hexarray)
{
    uint8_t hex_num = 0;
	for(uint32_t idx=0; idx<size; idx++)
    {
        uint8_t tmp = suble_util_str_hexchar2int(hexstr[idx]);
        if(tmp == (uint8_t)-1) {
            return SUBLE_ERROR_COMMON;
        }
        
        hex_num <<= 4;
        hex_num |= tmp;
        
        if((idx & 1) == 1){
            hexarray[idx>>1] = hex_num;
            hex_num = 0;
        }
	}
	return SUBLE_SUCCESS;
}

/*********************************************************
FN: 
*/
uint32_t suble_util_str_hexarray2hexstr(uint8_t* hexarray, uint32_t size, uint8_t* hexstr)
{
    uint32_t idx;
	for(idx=0; idx<size; idx++)
    {
        uint8_t high = hexarray[idx]>>4;
        uint8_t low  = hexarray[idx]&0x0F;
        
        hexstr[idx*2] = suble_util_str_int2hexchar(high);
        hexstr[idx*2+1] = suble_util_str_int2hexchar(low);
	}
//    hexstr[idx*2] = '0';
	return SUBLE_SUCCESS;
}




/*********************************************************
FN: 
*/
long source_to_decimal(uint8_t temp[], int source)
{
    long decimal_num = 0;  /*存储展开之后的和*/
    int length;
    int i;
    for( i=0; temp[i]!='\0'; i++ );
    length=i;
    for( i=0; i<=length-1; i++ )  /*累加*/
        decimal_num = (decimal_num*source) + suble_util_str_hexchar2int(temp[i]);
    return decimal_num;
}

/*********************************************************
FN: 
*/
int decimal_to_object(uint8_t temp[], long decimal_num, int object)
{
    int i=0;
    while(decimal_num)
    {
        temp[i] = suble_util_str_int2hexchar(decimal_num % object);  /*求出余数并转换为字符*/
        decimal_num = decimal_num / object;  /*用十进制数除以基数*/
        i++;
    }
    temp[i]='\0';
    return i;
}

/*********************************************************
FN: 
*/
void source_to_object(int source_notation, uint8_t source_array[], int object_notation, uint8_t object_array[])
{
    long decimal_num = source_to_decimal(source_array, source_notation);
    int length = decimal_to_object(object_array, decimal_num, object_notation);
}























