#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>
#include <stdio.h>

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

uint32_t convert_endian_32(uint32_t value);
uint16_t convert_endian_16(uint16_t value);

void parse_file(const char *filename);
void parse_target_hw_ids(FILE *file, uint32_t ptr_in_words, unsigned long file_size);
void parse_data_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size);
void parse_support_files(FILE *file, uint32_t ptr_in_words, unsigned long file_size);
void parse_load_pn_length(FILE *file, uint32_t load_pn_length_ptr);


#endif // MODULE_H
