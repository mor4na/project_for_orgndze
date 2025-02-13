#include <stdio.h>
#include "module.h"

int main() {
    const char *filename = "KAHT.BM21.00111.LUH";
    LUHData parsed_data;
    uint16_t i;
    
    printf("Starting file parsing...\n");
    if (parse_file(filename, &parsed_data)) {
        printf("\nFile parsed successfully.\n");
        printf("Data Files Count: %u\n", parsed_data.data_files_count);
        printf("Support Files Count: %u\n", parsed_data.support_files_count);
        printf("Total Data Files Size: %llu bytes\n", (unsigned long long)parsed_data.total_data_files_size);
        printf("Total Support Files Size: %llu bytes\n", (unsigned long long)parsed_data.total_support_files_size);
        
        // Вывод деталей Data Files
        printf("\nData Files Details:\n");
        for (i = 0; i < parsed_data.data_files_count; i++) {
            printf("Data File %u:\n", i + 1);
            printf("  Name: %s\n", parsed_data.dataFiles[i].name);
            printf("  PN: %s\n", parsed_data.dataFiles[i].pn);
            printf("  Length (words): %u\n", parsed_data.dataFiles[i].lengthWords);
            printf("  Length (bytes): %llu\n", (unsigned long long)parsed_data.dataFiles[i].lengthBytes);
            printf("  CRC: 0x%04X\n", parsed_data.dataFiles[i].crc);
            printf("  Check Value Length: %u\n", parsed_data.dataFiles[i].checkValueLen);
            printf("  Check Value Type: 0x%04X\n", parsed_data.dataFiles[i].checkValueType);
        }
        
        // Вывод деталей Support Files
        printf("\nSupport Files Details:\n");
        for (i = 0; i < parsed_data.support_files_count; i++) {
            printf("Support File %u:\n", i + 1);
            printf("  Name: %s\n", parsed_data.supportFiles[i].name);
            printf("  PN: %s\n", parsed_data.supportFiles[i].pn);
            printf("  Length (bytes): %u\n", parsed_data.supportFiles[i].lengthBytes);
            printf("  CRC: 0x%04X\n", parsed_data.supportFiles[i].crc);
            printf("  Check Value Length: %u\n", parsed_data.supportFiles[i].checkValueLen);
            printf("  Check Value Type: 0x%04X\n", parsed_data.supportFiles[i].checkValueType);
        }
    } else {
        printf("Error parsing file.\n");
    }
    
    free_luh_data(&parsed_data);
    
    printf("Program finished.\n");
    return 0;
}
