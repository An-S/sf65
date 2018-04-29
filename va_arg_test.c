
/*
 * test_strcspn.c
 *
 * Copyright 2018 mc78 <mc78@mc78-TP300LA>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void printArgs(char *str, ...){
    va_list va;

    va_start(va, str);
    while (str != NULL){
        puts(str);
        str = va_arg(va, char*);
    }
    va_end(va);
}

int main(int argc, char **argv)
{
    printArgs("Eins", "Zwei", "Drei", NULL);
    printArgs("Vier", "Fuenf", NULL);
    puts("--- Ende! ---");
    return 0;
}
