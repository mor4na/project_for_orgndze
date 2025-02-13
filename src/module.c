#include "module.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Преобразование порядка байтов
uint16_t convert_endian_16(uint16_t value) {
    return (value >> 8) | (value << 8);
}
uint32_t convert_endian_32(uint32_t value) {
    return ((value >> 24) & 0xFF) | ((value >> 8) & 0xFF00) | ((value << 8) & 0xFF0000) | ((value << 24) & 0xFF000000);
}
uint64_t convert_endian_64(const uint8_t bytes[8]) {
    uint64_t res = 0;
    res |= (uint64_t)bytes[0] << 56;
    res |= (uint64_t)bytes[1] << 48;
    res |= (uint64_t)bytes[2] << 40;
    res |= (uint64_t)bytes[3] << 32;
    res |= (uint64_t)bytes[4] << 24;
    res |= (uint64_t)bytes[5] << 16;
    res |= (uint64_t)bytes[6] << 8;
    res |= (uint64_t)bytes[7];
    return res;
}

// Основной парсер
int parse_file(const char *filename, LUHData *data) {
    FILE *file;
    unsigned long file_size;
    if (!data) return 0;
    file = fopen(filename, "rb");
    if (!file) {
        printf("Couldn't open file: %s\n", filename);
        return 0;
    }
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);
    // Чтение заголовка
    if (fread(&data->header, sizeof(LUHHeader), 1, file) != 1) {
        printf("Header reading error.\n");
        fclose(file);
        return 0;
    }
    // Преобразование полей заголовка
    data->header.header_length = convert_endian_32(data->header.header_length);
    data->header.file_format_version = convert_endian_16(data->header.file_format_version);
    data->header.part_flags = convert_endian_16(data->header.part_flags);
    data->header.load_pn_length_ptr = convert_endian_32(data->header.load_pn_length_ptr);
    data->header.target_hw_ids_ptr = convert_endian_32(data->header.target_hw_ids_ptr);
    data->header.data_files_ptr = convert_endian_32(data->header.data_files_ptr);
    data->header.support_files_ptr = convert_endian_32(data->header.support_files_ptr);
    data->header.user_defined_data_ptr = convert_endian_32(data->header.user_defined_data_ptr);
    
    // Инициализация
    data->target_hw_count = 0;
    data->data_files_count = 0;
    data->support_files_count = 0;
    data->total_data_files_size = 0;
    data->total_support_files_size = 0;
    data->dataFiles = NULL;
    data->supportFiles = NULL;
    
    printf("File: %s, size: %lu bytes\n", filename, file_size);
    
    // Разбор Target HW IDs
    if (data->header.target_hw_ids_ptr)
        data->target_hw_count = parse_target_hw_ids(file, data->header.target_hw_ids_ptr, file_size, "Target HW IDs");
    
    // Разбор Data Files
    if (data->header.data_files_ptr) {
        data->dataFiles = parse_data_files(file, data->header.data_files_ptr, file_size,
                                           &data->data_files_count, &data->total_data_files_size);
    }
    // Разбор Support Files
    if (data->header.support_files_ptr) {
        data->supportFiles = parse_support_files(file, data->header.support_files_ptr, file_size,
                                                 &data->support_files_count, &data->total_support_files_size);
    }
    fclose(file);
    return 1;
}

// Разбор Target HW IDs (просто количество)
uint16_t parse_target_hw_ids(FILE *file, uint32_t ptr_in_words, unsigned long file_size, const char *section_name) {
    uint32_t offset = ptr_in_words * 2;
    uint16_t count_raw;
    if (offset + 2 > file_size) {
        printf("%s: Pointer out of range.\n", section_name);
        return 0;
    }
    fseek(file, offset, SEEK_SET);
    if (fread(&count_raw, sizeof(uint16_t), 1, file) != 1) {
        printf("Error reading quantity %s.\n", section_name);
        return 0;
    }
    return convert_endian_16(count_raw);
}

// Разбор Data Files – сохраняем данные в массив структур
DataFileInfo *parse_data_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size,
                                 uint16_t *count, uint64_t *totalSize) {
    DataFileInfo *files = NULL;
    uint64_t total = 0;
    uint32_t baseOffset = ptr_in_words * 2;
    uint16_t declared_count, countRaw, i;
    if (baseOffset + 2 > file_size) {
        printf("Data Files: Pointer out of range.\n");
        return NULL;
    }
    fseek(file, baseOffset, SEEK_SET);
    if (fread(&countRaw, sizeof(uint16_t), 1, file) != 1) {
        printf("Error reading number of Data Files.\n");
        return NULL;
    }
    declared_count = convert_endian_16(countRaw);
    printf("Declared Data Files Count: %u\n", declared_count);
    *count = 0;
    files = (DataFileInfo *)malloc(declared_count * sizeof(DataFileInfo));
    if (!files) return NULL;
    
    for (i = 0; i < declared_count; i++) {
        long recordStart = ftell(file);
        uint16_t pointerRaw, dfPointer;
        uint16_t nameLenRaw, nameLen;
        char *fileName = NULL;
        uint16_t pnLenRaw, pnLen;
        char *pn = NULL;
        uint32_t lengthWordsRaw, lengthWords;
        uint16_t crcRaw, crc;
        uint8_t lenBytesArr[8];
        uint64_t lengthBytes;
        uint16_t checkValLenRaw, checkValLen;
        uint16_t checkValType = 0, cvTypeRaw;
        
        if (fread(&pointerRaw, sizeof(uint16_t), 1, file) != 1) break;
        dfPointer = convert_endian_16(pointerRaw);
        
        if (fread(&nameLenRaw, sizeof(uint16_t), 1, file) != 1) break;
        nameLen = convert_endian_16(nameLenRaw);
        fileName = (char *)malloc(nameLen + 1);
        if (!fileName) break;
        if (fread(fileName, 1, nameLen, file) != nameLen) { free(fileName); break; }
        fileName[nameLen] = '\0';
        if (nameLen % 2 == 1) fseek(file, 1, SEEK_CUR);
        
        if (fread(&pnLenRaw, sizeof(uint16_t), 1, file) != 1) { free(fileName); break; }
        pnLen = convert_endian_16(pnLenRaw);
        pn = (char *)malloc(pnLen + 1);
        if (!pn) { free(fileName); break; }
        if (fread(pn, 1, pnLen, file) != pnLen) { free(fileName); free(pn); break; }
        pn[pnLen] = '\0';
        if (pnLen % 2 == 1) fseek(file, 1, SEEK_CUR);
        
        if (fread(&lengthWordsRaw, sizeof(uint32_t), 1, file) != 1) { free(fileName); free(pn); break; }
        lengthWords = convert_endian_32(lengthWordsRaw);
        
        if (fread(&crcRaw, sizeof(uint16_t), 1, file) != 1) { free(fileName); free(pn); break; }
        crc = convert_endian_16(crcRaw);
        
        if (fread(lenBytesArr, 1, 8, file) != 8) { free(fileName); free(pn); break; }
        lengthBytes = convert_endian_64(lenBytesArr);
        
        if (fread(&checkValLenRaw, sizeof(uint16_t), 1, file) != 1) { free(fileName); free(pn); break; }
        checkValLen = convert_endian_16(checkValLenRaw);
        if (checkValLen >= 4) {
            if (fread(&cvTypeRaw, sizeof(uint16_t), 1, file) != 1) { free(fileName); free(pn); break; }
            checkValType = convert_endian_16(cvTypeRaw);
            fseek(file, checkValLen - 4, SEEK_CUR);
        } else if (checkValLen > 0) {
            fseek(file, checkValLen, SEEK_CUR);
        }
        
        files[*count].name = fileName;
        files[*count].pn = pn;
        files[*count].lengthWords = lengthWords;
        files[*count].crc = crc;
        files[*count].lengthBytes = lengthBytes;
        files[*count].checkValueLen = checkValLen;
        files[*count].checkValueType = checkValType;
        total += lengthBytes;
        (*count)++;
        
        if (i < declared_count - 1 && dfPointer != 0) {
            long nextRecord = recordStart + (dfPointer * 2);
            if (nextRecord < (long)file_size)
                fseek(file, nextRecord, SEEK_SET);
            else {
                printf("Next Data File out of range.\n");
                break;
            }
        }
    }
    *totalSize = total;
    return files;
}

// Разбор Support Files – аналогично Data Files
SupportFileInfo *parse_support_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size,
                                       uint16_t *count, uint64_t *totalSize) {
    SupportFileInfo *files = NULL;
    uint64_t total = 0;
    uint32_t baseOffset = ptr_in_words * 2;
    uint16_t declared_count, countRaw, i;
    if (baseOffset + 2 > file_size) {
        printf("Support Files: Pointer out of range.\n");
        return NULL;
    }
    fseek(file, baseOffset, SEEK_SET);
    if (fread(&countRaw, sizeof(uint16_t), 1, file) != 1) {
        printf("Error reading number of Support Files.\n");
        return NULL;
    }
    declared_count = convert_endian_16(countRaw);
    printf("Declared Support Files Count: %u\n", declared_count);
    *count = 0;
    files = (SupportFileInfo *)malloc(declared_count * sizeof(SupportFileInfo));
    if (!files) return NULL;
    
    for (i = 0; i < declared_count; i++) {
        long recordStart = ftell(file);
        uint16_t pointerRaw, sfPointer;
        uint16_t nameLenRaw, nameLen;
        char *fileName = NULL;
        uint16_t pnLenRaw, pnLen;
        char *pn = NULL;
        uint32_t lengthRaw, sfLength;
        uint16_t crcRaw, crc;
        uint16_t checkValLenRaw, checkValLen;
        uint16_t checkValType = 0, cvTypeRaw;
        
        if (fread(&pointerRaw, sizeof(uint16_t), 1, file) != 1) break;
        sfPointer = convert_endian_16(pointerRaw);
        
        if (fread(&nameLenRaw, sizeof(uint16_t), 1, file) != 1) break;
        nameLen = convert_endian_16(nameLenRaw);
        fileName = (char *)malloc(nameLen + 1);
        if (!fileName) break;
        if (fread(fileName, 1, nameLen, file) != nameLen) { free(fileName); break; }
        fileName[nameLen] = '\0';
        if (nameLen % 2 == 1) fseek(file, 1, SEEK_CUR);
        
        if (fread(&pnLenRaw, sizeof(uint16_t), 1, file) != 1) { free(fileName); break; }
        pnLen = convert_endian_16(pnLenRaw);
        pn = (char *)malloc(pnLen + 1);
        if (!pn) { free(fileName); break; }
        if (fread(pn, 1, pnLen, file) != pnLen) { free(fileName); free(pn); break; }
        pn[pnLen] = '\0';
        if (pnLen % 2 == 1) fseek(file, 1, SEEK_CUR);
        
        if (fread(&lengthRaw, sizeof(uint32_t), 1, file) != 1) { free(fileName); free(pn); break; }
        sfLength = convert_endian_32(lengthRaw);
        
        if (fread(&crcRaw, sizeof(uint16_t), 1, file) != 1) { free(fileName); free(pn); break; }
        crc = convert_endian_16(crcRaw);
        
        if (fread(&checkValLenRaw, sizeof(uint16_t), 1, file) != 1) { free(fileName); free(pn); break; }
        checkValLen = convert_endian_16(checkValLenRaw);
        if (checkValLen >= 4) {
            if (fread(&cvTypeRaw, sizeof(uint16_t), 1, file) != 1) { free(fileName); free(pn); break; }
            checkValType = convert_endian_16(cvTypeRaw);
            fseek(file, checkValLen - 4, SEEK_CUR);
        } else if (checkValLen > 0) {
            fseek(file, checkValLen, SEEK_CUR);
        }
        
        files[*count].name = fileName;
        files[*count].pn = pn;
        files[*count].lengthBytes = sfLength;
        files[*count].crc = crc;
        files[*count].checkValueLen = checkValLen;
        files[*count].checkValueType = checkValType;
        total += sfLength;
        (*count)++;
        
        if (i < declared_count - 1 && sfPointer != 0) {
            long nextRecord = recordStart + (sfPointer * 2);
            if (nextRecord < (long)file_size)
                fseek(file, nextRecord, SEEK_SET);
            else {
                printf("Next Support File out of range.\n");
                break;
            }
        }
    }
    *totalSize = total;
    return files;
}

// Освобождение памяти
void free_luh_data(LUHData *data) {
    uint16_t i;
    if (data->dataFiles) {
        for (i = 0; i < data->data_files_count; i++) {
            free(data->dataFiles[i].name);
            free(data->dataFiles[i].pn);
        }
        free(data->dataFiles);
        data->dataFiles = NULL;
    }
    if (data->supportFiles) {
        for (i = 0; i < data->support_files_count; i++) {
            free(data->supportFiles[i].name);
            free(data->supportFiles[i].pn);
        }
        free(data->supportFiles);
        data->supportFiles = NULL;
    }
}
