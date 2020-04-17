#ifndef STRING_H
#define STRING_H

#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "mem.h"

typedef char string;

typedef struct string_head string_head;

struct __attribute__((__packed__)) string_head {
    size_t length;
    size_t capacity;
    char **chars;
};

string *string_init_with_length(char *init, size_t length);
string *string_init(char *init);

size_t string_len_size(string *s);
int string_len(string *s);
size_t string_cap_size(string *s);
int string_cap(string *s);
void string_free(string *s);

string *concat(string *a, string *b);
string *concat_list(string **list, int size);
string *concat_varg(int size, ...);

string *substring(string *s, size_t start, size_t end);

string *string_append(string *a, char *b);

int string_compare(string *a, string *b);
bool string_equal(string *a, string *b);

string *char_to_string(char ch);
string *int_to_string(int number);
string *int8_to_string(int8_t number);
string *int16_to_string(int16_t number);
string *int32_to_string(int32_t number);
string *int64_to_string(int64_t number);
string *size_t_to_string(int64_t number);

string *uint_to_string(unsigned int number);
string *uint8_to_string(uint8_t number);
string *uint16_to_string(uint16_t number);
string *uint32_to_string(uint32_t number);
string *uint64_to_string(uint64_t number);

string *float_to_string(float number);
string *float32_to_string(float number);
string *float64_to_string(double number);

int string_to_int(string *str);
int8_t string_to_int8(string *str);
int16_t string_to_int16(string *str);
int32_t string_to_int32(string *str);
int64_t string_to_int64(string *str);
size_t string_to_size_t(string *str);

unsigned int string_to_uint(string *str);
uint8_t string_to_uint8(string *str);
uint16_t string_to_uint16(string *str);
uint32_t string_to_uint32(string *str);
uint64_t string_to_uint64(string *str);

float string_to_float(string *str);
float string_to_float32(string *str);
double string_to_float64(string *str);

string *format(string *f, ...);

#endif