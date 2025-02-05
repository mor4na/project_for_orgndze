#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "module.h"

// Функция для преобразования 32-битного числа из Big Endian в Little Endian
uint32_t convert_endian_32(uint32_t value) {
    return ((value >> 24) & 0xFF) |
           ((value >> 8) & 0xFF00) |
           ((value << 8) & 0xFF0000) |
           ((value << 24) & 0xFF000000);
}

// Функция для преобразования 16-битного числа из Big Endian в Little Endian
uint16_t convert_endian_16(uint16_t value) {
    return (value >> 8) | (value << 8);
}

// Основная функция парсинга файла
int parse_file(const char *filename, LUHData *parsed_data) {
    if (!parsed_data) {
        printf("Invalid output data structure.\n");
        return 0;
    }
    // Открываем файл в бинарном режиме
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Could not open file: %s\n", filename);
        return 0;
    }

    // Определяем размер файла
    fseek(file, 0, SEEK_END);
    unsigned long file_size = ftell(file);
    rewind(file);

    printf("File: %s, size: %lu bytes\n", filename, file_size);

    // Читаем заголовок файла
    LUHHeader header;
    if (fread(&header, sizeof(LUHHeader), 1, file) != 1) {
        printf("Error reading file header.\n");
        fclose(file);
        return 0;
    }

    // Меняем порядок байтов в заголовке (если нужно)
    header.header_length = convert_endian_32(header.header_length);
    header.file_format_version = convert_endian_16(header.file_format_version);
    header.part_flags = convert_endian_16(header.part_flags);
    header.load_pn_length_ptr = convert_endian_32(header.load_pn_length_ptr);
    header.target_hw_ids_ptr = convert_endian_32(header.target_hw_ids_ptr);
    header.data_files_ptr = convert_endian_32(header.data_files_ptr);
    header.support_files_ptr = convert_endian_32(header.support_files_ptr);
    header.user_defined_data_ptr = convert_endian_32(header.user_defined_data_ptr);

    // Записываем заголовок в структуру
    parsed_data->header = header;

    // Вывод информации о заголовке
    printf("\nHeader Info:\n");
    printf("  Header Length (words): %u\n", header.header_length);
    printf("  File Format Version: 0x%X\n", header.file_format_version);
    printf("  Part Flags: 0x%X\n", header.part_flags);

    // Если есть указатели на секции, вызываем их парсинг
    if (header.target_hw_ids_ptr) {
        parsed_data->target_hw_count = parse_section(file, header.target_hw_ids_ptr, file_size, "Target HW IDs");
    }

    if (header.data_files_ptr) {
        parsed_data->data_files_count = parse_data_files(file, header.data_files_ptr, file_size);
    }

    if (header.support_files_ptr) {
        parsed_data->support_files_count = parse_support_files(file, header.support_files_ptr, file_size);
    }

    // Закрыть файл
    fclose(file);
    return 1;
}

// Функция для парсинга секции с указателем 
uint16_t parse_section(FILE *file, uint32_t ptr_in_words, unsigned long file_size, const char *section_name) {
    uint32_t offset = ptr_in_words * 2;
    if (offset + sizeof(uint16_t) > file_size) {
        printf("%s pointer out of bounds.\n", section_name);
        return 0;
    }

    // Переходим к нужной позиции в файле
    fseek(file, offset, SEEK_SET);

    // Количество элементов в секции
    uint16_t count_raw;
    if (fread(&count_raw, sizeof(uint16_t), 1, file) != 1) {
        printf("Error reading %s count.\n", section_name);
        return 0;
    }
    uint16_t count = convert_endian_16(count_raw);

    printf("%s Count: %u\n", section_name, count);
    
    return count;
}

// Функция для парсинга 
uint16_t parse_data_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size) {
    uint32_t offset = ptr_in_words * 2;
    if (offset + sizeof(uint16_t) > file_size) {
        printf("Data Files pointer out of bounds.\n");
        return 0;
    }

    fseek(file, offset, SEEK_SET);

    uint16_t count;
    fread(&count, sizeof(uint16_t), 1, file);
    count = convert_endian_16(count);

    printf("Data Files Count: %u\n", count);
    
    for (uint16_t i = 0; i < count; i++) {
        uint8_t data[8];  // 8 байт
        if (fread(data, 1, 8, file) != 8) {
            printf("Error reading Data File entry %u.\n", i);
            break;
        }

        printf("  Data File %u: ", i);
        for (int j = 0; j < 8; j++) {
            printf("%02X ", data[j]);
        }
        printf("\n");
    }

    return count;
}

// Функция для парсинга 
uint16_t parse_support_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size) {
    uint32_t offset = ptr_in_words * 2;
    if (offset + sizeof(uint16_t) > file_size) {
        printf("Support Files pointer out of bounds.\n");
        return 0;
    }

    fseek(file, offset, SEEK_SET);

    uint16_t count;
    fread(&count, sizeof(uint16_t), 1, file);
    count = convert_endian_16(count);

    printf("Support Files Count: %u\n", count);
    
    for (uint16_t i = 0; i < count; i++) {
        uint8_t header[2];  // Заголовок записи (2 байта)
        uint8_t body[6];    // Основные данные (6 байт)

        if (fread(header, 1, 2, file) != 2) {
            printf("Error reading Support File header %u.\n", i);
            break;
        }

        if (fread(body, 1, 6, file) != 6) {
            printf("Error reading Support File body %u.\n", i);
            break;
        }

        printf("  Support File %u: Header %02X%02X | Data: ", i, header[0], header[1]);
        for (int j = 0; j < 6; j++) {
            printf("%02X ", body[j]);
        }
        printf("\n");
    }

    return count;
}
