#pragma once

// Конфигурация системы (можно редактировать)

// Разрядность процессора: 32 или 64 бит
#define XLEN			64

// Размер физической памяти в мегабайтах
// В этой версии до 1 ГБ в 32 и 64 бит режимах
#define RAM_SIZE_MB		256

// Адрес начала физической памяти (в адресном пространстве CPU)
#define RAM_START		0x40000000u


// Имена файлов
#if XLEN == 64
#define IMAGE_FILE		"linux/Image64"
#define DTB_FILE		"linux/64.dtb"
#else
#define IMAGE_FILE		"linux/Image"
#define DTB_FILE		"linux/32.dtb"
#endif



// Не редактируйте всё, что ниже!

#include <stdint.h>

#define RAM_SIZE		(RAM_SIZE_MB * 1048576)
