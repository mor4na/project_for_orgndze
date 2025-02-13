#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>
#include <stdio.h>

// Заголовок LUH (поля уже в little-endian)
typedef struct {
    uint32_t header_length;       
    uint16_t file_format_version; 
    uint16_t part_flags;          
    uint32_t load_pn_length_ptr;  
    uint32_t target_hw_ids_ptr;   
    uint32_t data_files_ptr;      
    uint32_t support_files_ptr;   
    uint32_t user_defined_data_ptr;
} LUHHeader;

// Информация о Data File
typedef struct {
    char *name;
    char *pn;
    uint32_t lengthWords;       // длина в 16-битных словах
    uint16_t crc;
    uint64_t lengthBytes;       // длина в байтах
    uint16_t checkValueLen;
    uint16_t checkValueType;
} DataFileInfo;

// Информация о Support File
typedef struct {
    char *name;
    char *pn;
    uint32_t lengthBytes;       
    uint16_t crc;
    uint16_t checkValueLen;
    uint16_t checkValueType;
} SupportFileInfo;

// Основная структура данных
typedef struct {
    LUHHeader header;
    uint16_t target_hw_count;
    uint16_t data_files_count;
    uint16_t support_files_count;
    uint64_t total_data_files_size;
    uint64_t total_support_files_size;
    DataFileInfo *dataFiles;
    SupportFileInfo *supportFiles;
} LUHData;

// Функции преобразования порядка байтов
uint16_t convert_endian_16(uint16_t value);
uint32_t convert_endian_32(uint32_t value);
uint64_t convert_endian_64(const uint8_t bytes[8]);

// Функция парсинга файла
int parse_file(const char *filename, LUHData *data);

// Функции разбора секций
uint16_t parse_target_hw_ids(FILE *file, uint32_t ptr_in_words, unsigned long file_size, const char *section_name);
DataFileInfo *parse_data_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size, uint16_t *count, uint64_t *totalSize);
SupportFileInfo *parse_support_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size, uint16_t *count, uint64_t *totalSize);

// Функция освобождения памяти
void free_luh_data(LUHData *data);

#endif
