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
    } else {
        printf("Error parsing file.\n");
    }

    // Завершаем работу программы
    printf("Program finished.\n");
    return 0;
}


