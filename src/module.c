#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "module.h"

// Конвертация 32-битного значения из Big Endian в Little Endian
uint32_t convert_endian_32(uint32_t value) {
    return ((value >> 24) & 0xFF) |
           ((value >> 8) & 0xFF00) |
           ((value << 8) & 0xFF0000) |
           ((value << 24) & 0xFF000000);
}

// Конвертация 16-битного значения из Big Endian в Little Endian
uint16_t convert_endian_16(uint16_t value) {
    return (value >> 8) | (value << 8);
}

void parse_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        return;
    }

    printf("Parsing file: %s\n", filename);

    // Переместимся в конец файла, чтобы узнать размер
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    printf("File size: %ld bytes\n", file_size);

    // Прочитаем заголовок
    LUHHeader header;
    fread(&header, sizeof(LUHHeader), 1, file);

    // Конвертируем значения из Big Endian в Little Endian
    header.header_length = convert_endian_32(header.header_length);
    header.file_format_version = convert_endian_16(header.file_format_version);
    header.part_flags = convert_endian_16(header.part_flags);
    header.load_pn_length_ptr = convert_endian_32(header.load_pn_length_ptr);
    header.target_hw_ids_ptr = convert_endian_32(header.target_hw_ids_ptr);
    header.data_files_ptr = convert_endian_32(header.data_files_ptr);
    header.support_files_ptr = convert_endian_32(header.support_files_ptr);
    header.user_defined_data_ptr = convert_endian_32(header.user_defined_data_ptr);

    // Вывод информации о заголовке
    printf("Header Info:\n");
    printf("  Header Length: %u bytes\n", header.header_length);
    printf("  File Format Version: %u\n", header.file_format_version);
    printf("  Part Flags: 0x%04X\n", header.part_flags);
    printf("  Pointer to Load PN Length: %u\n", header.load_pn_length_ptr);
    printf("  Pointer to Target HW IDs: %u\n", header.target_hw_ids_ptr);
    printf("  Pointer to Data Files: %u\n", header.data_files_ptr);
    printf("  Pointer to Support Files: %u\n", header.support_files_ptr);
    printf("  Pointer to User Defined Data: %u\n", header.user_defined_data_ptr);

    // Обработка секции Target HW IDs
    printf("\nProcessing Target HW IDs Section:\n");
    parse_target_hw_ids(file, header.target_hw_ids_ptr);
    // Обработка секции Data Files
    printf("\nProcessing Data Files Section:\n");
    parse_data_files(file, header.data_files_ptr);
    // Обработка секции Support Files
    printf("\nProcessing Support Files Section:\n");
    parse_support_files(file, header.support_files_ptr);


    fclose(file);
}

    // Target HW IDs
void parse_target_hw_ids(FILE *file, uint32_t target_hw_ids_ptr) {
    printf("Entering parse_target_hw_ids function...\n");

    if (target_hw_ids_ptr == 0) {
        printf("No Target HW IDs section available.\n");
        return;
    }

    printf("Seeking to Target_HW_IDs section at offset: %u\n", target_hw_ids_ptr);

    // Перейти к секции Target HW IDs
    fseek(file, target_hw_ids_ptr, SEEK_SET);

    // Прочитать количество идентификаторов
    uint16_t hw_id_count;
    fread(&hw_id_count, sizeof(uint16_t), 1, file);
    printf("HW ID Count (raw): %u\n", hw_id_count);
    hw_id_count = convert_endian_16(hw_id_count);

    printf("Target HW IDs Count: %u\n", hw_id_count);

    if (hw_id_count == 0) {
        printf("No Target HW IDs found in this section.\n");
        return;
    }

    // Ограничение количества идентификаторов для вывода
    if (hw_id_count > 20) {
        printf("Limiting output to 20.\n");
        hw_id_count = 20;
    }

    // Прочитать сами идентификаторы (каждый по 2 байта)
    printf("Starting to read HW IDs...\n");
    for (int i = 0; i < hw_id_count; i++) {
        uint16_t hw_id;
        fread(&hw_id, sizeof(uint16_t), 1, file);
        hw_id = convert_endian_16(hw_id);
        printf("  HW ID %d: 0x%04X\n", i + 1, hw_id);
    }

    // Сохранение результатов в папку program_results
    FILE *output_hw_ids = fopen("./program_results/target_hw_ids.txt", "w");
    if (!output_hw_ids) {
    perror("Error creating target_hw_ids.txt");
    return;
    }

    fprintf(output_hw_ids, "Target HW IDs Count: %u\n", hw_id_count);

    for (int i = 0; i < hw_id_count && i < 20; i++) { // Лимит на 20 ID
    uint16_t hw_id;
    fread(&hw_id, sizeof(uint16_t), 1, file);
    hw_id = convert_endian_16(hw_id);
    fprintf(output_hw_ids, "  HW ID %d: 0x%04X\n", i + 1, hw_id);
    }

    fclose(output_hw_ids);
    printf("Target HW IDs saved to target_hw_ids.txt\n");   
}

    // Data Files
void parse_data_files(FILE *file, uint32_t data_files_ptr) {
    printf("Entering parse_data_files function...\n");

    if (data_files_ptr == 0) {
        printf("No Data Files section available.\n");
        return;
    }

    printf("Seeking to Data_Files section at offset: %u\n", data_files_ptr);

    // Перейти к секции Data Files
    fseek(file, data_files_ptr, SEEK_SET);

    // Прочитать количество файлов
    uint16_t file_count;
    fread(&file_count, sizeof(uint16_t), 1, file);
    file_count = convert_endian_16(file_count);

    printf("Data Files Count: %u\n", file_count);

    if (file_count == 0) {
        printf("No Data Files found in this section.\n");
        return;
    }

    // Ограничение количества идентификаторов для вывода
    if (file_count > 20) {
        printf("Limiting output to 20.\n");
        file_count = 20;
    }

    // Прочитать информацию о файлах
    for (int i = 0; i < file_count; i++) {
        uint32_t file_offset;
        fread(&file_offset, sizeof(uint32_t), 1, file);
        file_offset = convert_endian_32(file_offset);
        printf("  File %d Offset: %u\n", i + 1, file_offset);
    }

    // Сохранение результатов в папку program_results   
    FILE *output_data_files = fopen("./program_results/data_files.txt", "w");
    if (!output_data_files) {
    perror("Error creating data_files.txt");
    return;
    }

    fprintf(output_data_files, "Data Files Count: %u\n", file_count);

    for (int i = 0; i < file_count && i < 20; i++) { // Лимит на 20 файлов
    uint32_t file_offset;
    fread(&file_offset, sizeof(uint32_t), 1, file);
    file_offset = convert_endian_32(file_offset);
    fprintf(output_data_files, "  File %d Offset: %u\n", i + 1, file_offset);
    }

    fclose(output_data_files);
    printf("Data Files saved to data_files.txt\n");
}

    // Support Files
void parse_support_files(FILE *file, uint32_t support_files_ptr) {
    printf("Entering parse_support_files function...\n");

    if (support_files_ptr == 0) {
        printf("No Support Files section available.\n");
        return;
    }

    printf("Seeking to Support_Files section at offset: %u\n", support_files_ptr);

    // Перейти к секции Support Files
    fseek(file, support_files_ptr, SEEK_SET);

    // Прочитать количество файлов поддержки
    uint16_t support_file_count;
    fread(&support_file_count, sizeof(uint16_t), 1, file);
    support_file_count = convert_endian_16(support_file_count);

    printf("Support Files Count: %u\n", support_file_count);

    if (support_file_count == 0) {
        printf("No Support Files found in this section.\n");
        return;
    }

    // Ограничение количества идентификаторов для вывода
    if (support_file_count > 20) {
        printf("Limiting output to 20.\n");
        support_file_count = 20;
    }

    // Прочитать информацию о файлах поддержки
    for (int i = 0; i < support_file_count; i++) {
        uint32_t file_offset;
        fread(&file_offset, sizeof(uint32_t), 1, file);
        file_offset = convert_endian_32(file_offset);
        printf("  Support File %d Offset: %u\n", i + 1, file_offset);
    }

    // Сохранение результатов в папку program_results    
    FILE *output_support_files = fopen("./program_results/support_files.txt", "w");
    if (!output_support_files) {
    perror("Error creating support_files.txt");
    return;
    }

    fprintf(output_support_files, "Support Files Count: %u\n", support_file_count);

    for (int i = 0; i < support_file_count && i < 20; i++) { // Лимит на 20 файлов
    uint32_t file_offset;
    fread(&file_offset, sizeof(uint32_t), 1, file);
    file_offset = convert_endian_32(file_offset);
    fprintf(output_support_files, "  Support File %d Offset: %u\n", i + 1, file_offset);
    }

    fclose(output_support_files);
    printf("Support Files saved to support_files.txt\n");

}







