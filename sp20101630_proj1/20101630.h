/* 포함되는 파일 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 상수 정의 */
#define MAX 85
#define ENCODE 1001
#define DECODE 1002

/* 함수 원형 */
void checkWord (char*,FILE*);
void transformChar (char*,int);
void printOut (FILE*,char*);
