#include "print.h"

#include "sbi.h"

/**
 * @brief print a string
 *
 * @param s the string
 */
void puts(char *s) {
  while (*s) {
    sbi_ecall(0x1, 0x0, *s++, 0, 0, 0, 0, 0);  // print *s
  }
}

/**
 * @brief print an integer with newline
 *
 * @param x the integer
 */
void puti(int x) {
  char s[12];  // the string presentation of x
  for (short i = 0; i < 12; i++) s[i] = 0;
  short l = 0, r = 0;
  if (x < 0) {
    x *= -1;
    s[r++] = '-';
    l++;
  }
  do {
    s[r++] = x % 10 + '0';
    x /= 10;
  } while (x);
  r--;
  while (l < r) {  // reverse
    short tmp = s[l];
    s[l++] = s[r];
    s[r--] = tmp;
  }
  puts(s);  // print
}
