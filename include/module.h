#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>
#include <stdio.h>

// Структура заголовка файла LUH
typedef struct {
    uint32_t header_length;       // Длина заголовка в словах
    uint16_t file_format_version; // Версия формата файла
    uint16_t part_flags;          // Флаги части
    uint32_t load_pn_length_ptr;  // Указатель на длину PN загрузки
    uint32_t target_hw_ids_ptr;   // Указатель на идентификаторы аппаратного обеспечения
    uint32_t data_files_ptr;      // Указатель на файлы данных
    uint32_t support_files_ptr;   // Указатель на вспомогательные файлы
    uint32_t user_defined_data_ptr;// Указатель на пользовательские данные
} LUHHeader;

// Структура для хранения распарсенных данных
typedef struct {
    LUHHeader header;             // Заголовок файла
    uint16_t target_hw_count;     // Количество целевых HW ID
    uint16_t data_files_count;    // Количество файлов данных
    uint16_t support_files_count; // Количество вспомогательных файлов
    uint32_t data_files_size;     // Общий размер файлов данных
    uint32_t support_files_size;  // Общий размер вспомогательных файлов
} LUHData;

// Функции для преобразования Endian
uint32_t convert_endian_32(uint32_t value);
uint16_t convert_endian_16(uint16_t value);

// Основная функция парсинга файла
int parse_file(const char *filename, LUHData *parsed_data);

// Функции для парсинга различных секций файла
uint16_t parse_section(FILE *file, uint32_t ptr_in_words, unsigned long file_size, const char *section_name);
uint16_t parse_data_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size, uint32_t *total_size);
uint16_t parse_support_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size, uint32_t *total_size);

#endif // MODULE_H