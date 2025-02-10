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
uint16_t parse_data_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size) {
    uint32_t offset = ptr_in_words * 2;
    if (offset + 8 > file_size) return 0;

    fseek(file, offset, SEEK_SET);
    uint32_t data_file_length_raw;
    if (fread(&data_file_length_raw, sizeof(uint32_t), 1, file) != 1) {    
        printf("Error reading Data File Length.\n");
        return 0;
    }
    uint32_t data_file_length = convert_endian_32(data_file_length_raw) & 0xFFFF; // Берём только 2 байта

    // Вывод "сырых" байтов перед интерпретацией
    printf("  Raw Data Length Bytes (hex): ");
    uint8_t *raw_length_bytes = (uint8_t*)&data_file_length_raw;
    for (size_t i = 0; i < sizeof(uint64_t); i++) {
        printf("%02X ", raw_length_bytes[i]);
    }
    printf("\n");

    printf("  Data File Length (Bytes): %llu\n", (unsigned long long) data_file_length);

    if (offset + 8 + data_file_length > file_size) {
        printf("Error: Data File Length exceeds file size.\n");
        return 0;
    }

    uint8_t *data = malloc(data_file_length);
    if (!data) {
        printf("Memory allocation error.\n");
        return 0;
    }
    fread(data, 1, data_file_length, file);
    
    printf("  Data (hex): ");
    for (size_t i = 0; i < data_file_length; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");

    // Вывод интерпретированных данных в десятичном формате
    uint32_t interpreted_value = 0;
    for (size_t i = 0; i < data_file_length; i++) {
        interpreted_value = (interpreted_value << 8) | data[i];
    }

    printf("  Interpreted Data: ");
    for (size_t i = 0; i < data_file_length; i++) {
        if (data[i] >= 32 && data[i] <= 126) { // ASCII символы, проверить функцию если будет ошибка ввывода !!!ВАЖНО!!!
            printf("%c", data[i]);  // Вывод как текст
        } else {
            printf("%02X ", data[i]);  // Если не текст, вывод как HEX
        }
    }
    printf("\n");    

    free(data);
    return 1;
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
    header.data_files_ptr = convert_endian_32(header.data_files_ptr);

    parsed_data->header = header;

    printf("\nHeader Info:\n");
    printf("  Header Length (words): %u\n", header.header_length);
    printf("  File Format Version: 0x%X\n", header.file_format_version);
    printf("  Part Flags: 0x%X\n", header.part_flags);

    if (header.data_files_ptr) {
        parsed_data->data_files_count = parse_data_files(file, header.data_files_ptr, file_size);
    }

    fclose(file);
    return 1;
}
