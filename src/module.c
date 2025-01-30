#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "module.h"

    // Конвертация 32-битного Big Endian в Little Endian
uint32_t convert_endian_32(uint32_t value) {
    return ((value >> 24) & 0xFF) |
           ((value >> 8) & 0xFF00) |
           ((value << 8) & 0xFF0000) |
           ((value << 24) & 0xFF000000);
}

    // Конвертация 16-битного Big Endian в Little Endian
uint16_t convert_endian_16(uint16_t value) {
    return (value >> 8) | (value << 8);
}

void parse_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Could not open file: %s\n", filename);
        return;
    }

    // Узнаем размер файла
    fseek(file, 0, SEEK_END);
    unsigned long file_size = ftell(file);
    rewind(file);
    printf("File: %s, size: %lu bytes\n", filename, file_size);

    // Считываем заголовок
    LUHHeader header;
    fread(&header, sizeof(LUHHeader), 1, file);

    // Переводим поля заголовка
    header.header_length = convert_endian_32(header.header_length);
    header.file_format_version = convert_endian_16(header.file_format_version);
    header.part_flags = convert_endian_16(header.part_flags);
    header.load_pn_length_ptr = convert_endian_32(header.load_pn_length_ptr);
    header.target_hw_ids_ptr = convert_endian_32(header.target_hw_ids_ptr);
    header.data_files_ptr = convert_endian_32(header.data_files_ptr);
    header.support_files_ptr = convert_endian_32(header.support_files_ptr);
    header.user_defined_data_ptr = convert_endian_32(header.user_defined_data_ptr);

    // Вывод основных полей заголовка
    printf("\nHeader Info:\n");
    printf("  Header Length (words): %u\n", header.header_length);
    printf("  File Format Version: 0x%X\n", header.file_format_version);
    printf("  Part Flags: 0x%X\n", header.part_flags);
    printf("  Pointer to Target HW IDs: %u\n", header.target_hw_ids_ptr);
    printf("  Pointer to Data Files: %u\n", header.data_files_ptr);
    printf("  Pointer to Support Files: %u\n", header.support_files_ptr);
    printf("\nProcessing Load PN Length Section:\n");
    parse_load_pn_length(file, header.load_pn_length_ptr);
    printf("\n");

    // Переходим к нужным секциям, если указатели не ноль
    if (header.target_hw_ids_ptr) {
        parse_target_hw_ids(file, header.target_hw_ids_ptr, file_size);
    }
    if (header.data_files_ptr) {
        parse_data_files(file, header.data_files_ptr, file_size);
    }
    if (header.support_files_ptr) {
        parse_support_files(file, header.support_files_ptr, file_size);
    }

    fclose(file);
}

void parse_target_hw_ids(FILE *file, uint32_t ptr_in_words, unsigned long file_size) {
    // Смещение в байтах = ptr_in_words * 2
    uint32_t offset = ptr_in_words * 2;
    if (offset + sizeof(uint16_t) > file_size) {
        printf("Target HW IDs pointer out of bounds.\n");
        return;
    }

    fseek(file, offset, SEEK_SET);

    // Считываем 2 байта (кол-во HW ID)
    uint16_t count_raw;
    fread(&count_raw, sizeof(uint16_t), 1, file);
    uint16_t count = convert_endian_16(count_raw);

    printf("Target HW IDs Count: %u\n", count);
}

void parse_data_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size) {
    uint32_t offset = ptr_in_words * 2;
    if (offset + sizeof(uint16_t) > file_size) {
        printf("Data Files pointer out of bounds.\n");
        return;
    }

    fseek(file, offset, SEEK_SET);

    uint16_t count_raw;
    fread(&count_raw, sizeof(uint16_t), 1, file);
    uint16_t count = convert_endian_16(count_raw);

    printf("Data Files Count: %u\n", count);
}

void parse_support_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size) {
    uint32_t offset = ptr_in_words * 2;
    if (offset + sizeof(uint16_t) > file_size) {
        printf("Support Files pointer out of bounds.\n");
        return;
    }

    fseek(file, offset, SEEK_SET);

    uint16_t count_raw;
    fread(&count_raw, sizeof(uint16_t), 1, file);
    uint16_t count = convert_endian_16(count_raw);

    printf("Support Files Count: %u\n", count);
}

void parse_load_pn_length(FILE *file, uint32_t load_pn_length_ptr) {
    if (load_pn_length_ptr == 0) {
        printf("No Load PN Length section available.\n");
        return;
    }

    printf("\nProcessing Load PN Length Section:\n");
    printf("Seeking to Load PN Length section at offset: %u\n", load_pn_length_ptr);

    // Перейти к нужному смещению
    fseek(file, load_pn_length_ptr * 2, SEEK_SET); // Переводим из слов в байты

    // Читаем 16-битное значение
    uint16_t load_pn_length;
    fread(&load_pn_length, sizeof(uint16_t), 1, file);
    load_pn_length = convert_endian_16(load_pn_length);

    printf("Load PN Length: %u characters\n", load_pn_length);
}
