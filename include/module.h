#ifndef MODULE_H
#define MODULE_H

#include <stdint.h> // Для uint32_t, uint16_t

// Структура заголовка файла LUH
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

// Определение функций для конвертации Endian
uint32_t convert_endian_32(uint32_t value);
uint16_t convert_endian_16(uint16_t value);

// Функция для парсинга файла
void parse_file(const char *filename);

// Функция для обработки секции Target HW IDs
void parse_target_hw_ids(FILE *file, uint32_t target_hw_ids_ptr);
// Функция для обработки секции Data files
void parse_data_files(FILE *file, uint32_t data_files_ptr);
// Функция для обработки секции Support files
void parse_support_files(FILE *file, uint32_t support_files_ptr);

#endif // MODULE_H

