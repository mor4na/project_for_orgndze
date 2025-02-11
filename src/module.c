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

// Функция парсинга Data Files
uint16_t parse_data_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size, uint32_t *total_size, uint16_t *file_count) {
    uint32_t offset = ptr_in_words * 2;
    if (offset + 8 > file_size) return 0;

    fseek(file, offset, SEEK_SET);
    uint32_t data_file_length_raw;
    *file_count = 0;
    while (offset + 8 <= file_size) {
        (*file_count)++;
        printf("Processing Data File Directory Entry %u at offset: %u\n", *file_count, offset);
        
        if (fread(&data_file_length_raw, sizeof(uint32_t), 1, file) != 1) return 0;
        uint32_t data_file_length = convert_endian_32(data_file_length_raw) & 0xFFFF;
        
        *total_size += data_file_length;
        offset += data_file_length + 8;
        fseek(file, offset, SEEK_SET);
    }
    return 1;
}

// Функция парсинга Support Files
uint16_t parse_support_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size, uint32_t *total_size, uint16_t *file_count) {
    uint32_t offset = ptr_in_words * 2;
    if (offset + 8 > file_size) return 0;

    fseek(file, offset, SEEK_SET);
    uint32_t support_file_length_raw;
    *file_count = 0;
    while (offset + 8 <= file_size) {
        (*file_count)++;
        printf("Processing Support File Directory Entry %u at offset: %u\n", *file_count, offset);
        
        if (fread(&support_file_length_raw, sizeof(uint32_t), 1, file) != 1) return 0;
        uint32_t support_file_length = convert_endian_32(support_file_length_raw) & 0xFFFF;
        
        *total_size += support_file_length;
        offset += support_file_length + 8;
        fseek(file, offset, SEEK_SET);
    }
    return 1;
}

// Главная функция парсинга файла
int parse_file(const char *filename, LUHData *parsed_data) {
    if (!parsed_data) return 0;

    FILE *file = fopen(filename, "rb");
    if (!file) return 0;

    fseek(file, 0, SEEK_END);
    unsigned long file_size = ftell(file);
    rewind(file);

    LUHHeader header;
    if (fread(&header, sizeof(LUHHeader), 1, file) != 1) {
        fclose(file);
        return 0;
    }

    header.header_length = convert_endian_32(header.header_length);
    header.data_files_ptr = convert_endian_32(header.data_files_ptr);
    header.support_files_ptr = convert_endian_32(header.support_files_ptr);

    parsed_data->header = header;
    parsed_data->data_files_size = 0;
    parsed_data->support_files_size = 0;
    parsed_data->data_files_count = 0;
    parsed_data->support_files_count = 0;

    if (header.data_files_ptr) {
        parse_data_files(file, header.data_files_ptr, file_size, &parsed_data->data_files_size, &parsed_data->data_files_count);
    }
    if (header.support_files_ptr) {
        parse_support_files(file, header.support_files_ptr, file_size, &parsed_data->support_files_size, &parsed_data->support_files_count);
    }

    printf("Total Data Files Processed: %u\n", parsed_data->data_files_count);
    printf("Total Support Files Processed: %u\n", parsed_data->support_files_count);

    fclose(file);
    return 1;
}