#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "module.h"

// Функция для конвертации 32-битного числа из Big Endian в Little Endian
uint32_t convert_endian_32(uint32_t value) {
    return ((value >> 24) & 0xFF) |
           ((value >> 8) & 0xFF00) |
           ((value << 8) & 0xFF0000) |
           ((value << 24) & 0xFF000000);
}

// Функция для конвертации 16-битного числа
uint16_t convert_endian_16(uint16_t value) {
    return (value >> 8) | (value << 8);
}

// Функция для вывода данных в текстовом или HEX формате
void print_data_as_text(uint8_t *data, size_t length) {
    size_t text_start = 0;
    
    // Пропускаем неотображаемые символы в начале (служебные байты)
    while (text_start < length && (data[text_start] < 32 || data[text_start] > 126)) {
        text_start++;
    }
    
    size_t text_length = 0;
    for (size_t i = text_start; i < length; i++) {
        if (data[i] >= 32 && data[i] <= 126) {
            text_length++;
        }
    }
    
    // Если 90% оставшихся символов - текст, выводим строку
    if ((float)text_length / (length - text_start) > 0.9) {
        printf("  Data (text): \"");
        fwrite(data + text_start, 1, length - text_start, stdout);
        printf("\"\n");
    } else {
        printf("  Data (hex): ");
        for (size_t i = 0; i < length; i++) {
            printf("%02X ", data[i]);
        }
        printf("\n");
    }
}

// Функция для чтения количества записей в разделе
uint16_t parse_section(FILE *file, uint32_t ptr_in_words, unsigned long file_size, const char *section_name) {
    uint32_t offset = ptr_in_words * 2; // Переводим слова в байты
    if (offset + sizeof(uint16_t) > file_size) {
        printf("%s pointer out of bounds.\n", section_name);
        return 0;
    }

    fseek(file, offset, SEEK_SET);
    uint16_t count_raw;
    if (fread(&count_raw, sizeof(uint16_t), 1, file) != 1) {
        printf("Error reading %s count.\n", section_name);
        return 0;
    }
    uint16_t count = convert_endian_16(count_raw);

    printf("%s Count: %u\n", section_name, count);
    return count;
}

// Функция парсинга Data Files
uint16_t parse_data_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size) {
    uint32_t offset = ptr_in_words * 2;
    if (offset + 16 > file_size) return 0;

    fseek(file, offset, SEEK_SET);
    uint8_t data[16];
    fread(data, 1, 16, file);
    print_data_as_text(data, 16);
    return 1;
}

// Функция парсинга Support Files (использует ту же логику, что Data Files)
uint16_t parse_support_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size) {
    return parse_data_files(file, ptr_in_words, file_size);
}

// Главная функция парсинга файла
int parse_file(const char *filename, LUHData *parsed_data) {
    if (!parsed_data) {
        printf("Invalid output data structure.\n");
        return 0;
    }

    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Could not open file: %s\n", filename);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    unsigned long file_size = ftell(file);
    rewind(file);

    printf("File: %s, size: %lu bytes\n", filename, file_size);

    LUHHeader header;
    if (fread(&header, sizeof(LUHHeader), 1, file) != 1) {
        printf("Error reading file header.\n");
        fclose(file);
        return 0;
    }

    // Конвертируем заголовок в правильный порядок байтов
    header.header_length = convert_endian_32(header.header_length);
    header.file_format_version = convert_endian_16(header.file_format_version);
    header.part_flags = convert_endian_16(header.part_flags);
    header.load_pn_length_ptr = convert_endian_32(header.load_pn_length_ptr);
    header.target_hw_ids_ptr = convert_endian_32(header.target_hw_ids_ptr);
    header.data_files_ptr = convert_endian_32(header.data_files_ptr);
    header.support_files_ptr = convert_endian_32(header.support_files_ptr);
    header.user_defined_data_ptr = convert_endian_32(header.user_defined_data_ptr);

    parsed_data->header = header;

    printf("\nHeader Info:\n");
    printf("  Header Length (words): %u\n", header.header_length);
    printf("  File Format Version: 0x%X\n", header.file_format_version);
    printf("  Part Flags: 0x%X\n", header.part_flags);

    // Парсим секции файла
    if (header.target_hw_ids_ptr) {
        parsed_data->target_hw_count = parse_section(file, header.target_hw_ids_ptr, file_size, "Target HW IDs");
    }

    if (header.data_files_ptr) {
        parsed_data->data_files_count = parse_data_files(file, header.data_files_ptr, file_size);
    }

    if (header.support_files_ptr) {
        parsed_data->support_files_count = parse_support_files(file, header.support_files_ptr, file_size);
    }

    fclose(file);
    return 1;
}
