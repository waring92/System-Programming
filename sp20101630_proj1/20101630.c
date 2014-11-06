/* 헤더파일 읽기 */
#include "20101630.h"

/* 프로그램의 시작 */
int main (int argc, char *argv[]) {

	/* 변수 설정 */
	FILE *inputFile, *outFile;
	char inputline[MAX];

	/* 입력파일 읽기 */
	if ((inputFile = fopen(argv[1],"r")) == NULL) exit(1);

	/* 출력파일 생성 */
	if ((outFile = fopen("output.txt","w")) == NULL) exit(1);

	/* 1줄씩 읽어들이기 */
	while (fgets(inputline,MAX,inputFile) != NULL) {
		/* 읽어들인 문자열에서 구분작업 시작 */
		checkWord(inputline,outFile);
	}

	/* 파일 닫기 */
	fclose(inputFile);
	fclose(outFile);

	return 0;
}

//--------------------------------------------------//
// 함    수: checkWord
// 목    적: 읽어들인 문자열에 separator가 있는지 확인하고
//	     separator를 기준으로 token을 구분
// 파라미터: 문자열 inputLine, 파일포인터 outFile
// 리 턴 값: 없음
//--------------------------------------------------//

void checkWord (char *inputLine, FILE *outFile) {

	/* 변수 설정 */
	int i, isQuot = 1;

	/* strtok 사용을 위한 변수 설정 */
	char *token;
	char separator[] = " \t,\n\r";

	/* 주석문을 나타내는 '.'가 나올경우 문자열 자름 */
	for (i = 0; i < strlen(inputLine); i++) {

		/* 홑따옴표(')가 나오면 isQuot를 음/양수 변환 */
		if (inputLine[i] == '\'') {
			isQuot = -1*isQuot;
			continue;
		}
		/* isQuot이 양수일 경우(따옴표 안이 아닐경우) */
		if (isQuot > 0) {
			/* 주석문 처리 */
			if (inputLine[i] == '.') {
				inputLine[i++] = '\n';
				inputLine[i++] = '.';
				inputLine[i++] = '\0';
				break;
			}
		}
		/* isQuot이 음수일 경우(따옴표 안쪽일 경우)  */
		if (isQuot < 0) {
			transformChar(&inputLine[i],ENCODE);
		}

	}	

	/* strtok를 통해 token 만들기 */
	token = strtok(inputLine, separator);
	while (token != NULL) {
		/* 잘린 token을 변환하여 출력  */
		printOut(outFile, token);
		token = strtok(NULL, separator);
	}

}

//--------------------------------------------------//
// 함    수: transformChar
// 목    적: 홑따옴표(')안에 들어있는 separator들을
//           특정문자로 치환하여 tokenizing을 방지.
//	     나중에 출력할때는 다시 실제 캐릭터로
//	     변환하여 복원.
// 파라미터: 캐릭터포인터 str, 치환인지 복원인지 판단하는 int code
// 리 턴 값: 없음
//--------------------------------------------------//

void transformChar (char *str,int code) {
	
	/* 변수 설정 */
	int i;

	/* 각각의 separator에 대해 치환할 값 설정 */
	char rea_sptr[] = {' ','\t',',','\0'};
	char sub_sptr[] = {255, 254,253,'\0'};

	/* code별로 치환과 복원 진행 */
	for (i = 0; i < strlen(rea_sptr); i++) {
		if (code == ENCODE) {
			if (*str == rea_sptr[i]) {
				*str = sub_sptr[i];
				return;
			}
		}
		if (code == DECODE) {
			if (*str == sub_sptr[i]) {
				*str = rea_sptr[i];
				return;
			}
		}
	}

	return;

}

//--------------------------------------------------//
// 함    수: printOut
// 목    적: token을 출력하며, 혹시 변환할
//	     문자열이 있다면 변환하여 출력.
// 파라미터: 파일포인터 outFile, 문자열 token, 변환문자열 tstr
// 리 턴 값: 없음
//--------------------------------------------------//

void printOut (FILE *outFile, char *token) {	

	/* 변환할 필요가 없다면, 바로 화면과 output.txt에 출력  */
	if ((token[0] != 'C' && token[0] != 'X') || token[1] != '\'') {
		printf("%s\n", token);
		fprintf(outFile, "%s\n", token);
		return;
	}

	/* 변환을 위한 변수 설정 */
	int i, dec;

	/* transformChar에서 치환 된 문자열을 */
	/* DECODE 파라미터로 호출하여 복원  */
	for (i = 0; i < strlen(token); transformChar(&token[i++],DECODE));

	/* 뒤쪽의 홑따옴표(')를 NULL로 바꿔, 문자열 끝임을 표시 */
	token[i-1] = '\0';

	/* 문자열의 맨앞 글자를 보고 변환 진행 */
	switch (token[0]) {

	/* 문자열인경우 바로 출력, 16진수일경우 sscanf로 변환 후 출력 */
	case 'C': 	printf("C\n%s\n", token+2);
			fprintf(outFile,"C\n%s\n", token+2);
			break;
	case 'X':	sscanf(token+2, "%x", &dec);
			printf("X\n%d\n", dec);
			fprintf(outFile,"X\n%d\n", dec);
			break;
	}

	return;
}
