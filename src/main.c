#include <stdio.h>
#include "module.h"

// Главная функция программы
int main() {
    // Имя файла, который будем парсить
    const char *filename = "KAHT.BM21.00111.LUH";
    
    // Структура для хранения распарсенных данных
    LUHData parsed_data;

    // Выводим сообщение о начале работы
    printf("Starting file parsing...\n");

    // Проверяем, успешно ли открыт файл и можно ли его парсить
    if (parse_file(filename, &parsed_data)) {
        printf("\nFile parsed successfully.\n");
        printf("Total Data Files Size: %u bytes\n", parsed_data.data_files_size);
        printf("Total Support Files Size: %u bytes\n", parsed_data.support_files_size);
        printf("Total Combined Size: %u bytes\n", parsed_data.data_files_size + parsed_data.support_files_size);
    } else {
        printf("Error parsing file.\n");
    }

    // Завершаем работу программы
    printf("Program finished.\n");
    return 0;
}