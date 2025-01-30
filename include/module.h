#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>
#include <stdio.h>  // Для FILE*

// Структура заголовка файла LUH
typedef struct {
    uint32_t header_length;        // 4 байта, длина заголовка в 16-битных словах
    uint16_t file_format_version;  // 2 байта, версия формата
    uint16_t part_flags;           // 2 байта, флаги
    uint32_t load_pn_length_ptr;   // 4 байта, указатель (в словах) на Load PN Length
    uint32_t target_hw_ids_ptr;    // 4 байта, указатель (в словах) на Number of Target HW IDs
    uint32_t data_files_ptr;       // 4 байта, указатель (в словах) на Number of Data Files
    uint32_t support_files_ptr;    // 4 байта, указатель (в словах) на Number of Support Files
    uint32_t user_defined_data_ptr;// 4 байта, указатель (в словах) на User Defined Data
} LUHHeader;

// Функции для перевода Endian
uint32_t convert_endian_32(uint32_t value);
uint16_t convert_endian_16(uint16_t value);

// Основная функция парсинга
void parse_file(const char *filename);

// Функции для разбора секций
void parse_target_hw_ids(FILE *file, uint32_t target_hw_ids_ptr, unsigned long file_size);
void parse_data_files(FILE *file, uint32_t data_files_ptr, unsigned long file_size);
void parse_support_files(FILE *file, uint32_t support_files_ptr, unsigned long file_size);

#endif // MODULE_H
