#include "20101630.h"

int main (void) {

	int i;

	// Tokenizing에서의 Token과 입력 커맨드의 Token이
	// 혼동되어, 입력어를 [mnemonic]과 [paramter]로 구분했습니다.
	// 예) [mnemonic] [paramter], [parameter] 
	// 예) dump       A0        , A2

	// Opcode, Mnemonic Table을 생성
	hash_make();

	// 입력을 받는다.
	// 입력 'q[uit]'로 exit()명령을 받기 전까지 무한 루프.
	while (1) {
		// 입력값과 토큰(parameter)들을 초기화
		initialize_command();
		initialize_parameter();

		// "sicsim>"을 출력하고 입력을 받는다.
		printSIC();

		// parameter_count는 mnemonic과 parmater의 갯수를
		// 더한 값이므로, 0 보다 크면 command_analyser()를 호출한다.
		// 0 이라면 입력한 값이 없다는 뜻(공백줄인 상태로 엔터).
		if (parameter_count > 0)
			command_analyser();

		// 'q' 혹은 'quit' 명령이 들어오면 오류가 있는지 없는지를
		// 판단하여 0 혹은 1로 변경되는  isError 변수를 2로 변경.
		if (isError == 2) {
			break;
		}
	}

	// 메모리 Free
	history_node *previous;
	history_node *current;

	current = history_head;
	while (current != NULL) {
		previous = current;
		current = current->nextNode;
		free(previous);
	}

	hash_node *previoush;
	hash_node *currenth;

	for (i = 0; i < MAX_HASH_SIZE; i++) {
		currenth = opcode_list[i];
		while (currenth != NULL) {
			previoush = currenth;
			currenth = currenth->nextNode;
			free(previoush);
		}

		currenth = mnemonic_list[i];
		while (currenth != NULL) {
			previoush = currenth;
			currenth = currenth->nextNode;
			free(previoush);
		}
	}

	return 0;
}

//-----------------------------------------------//
// 함수이름: printSIC
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: 전역변수 command에 입력 받는 함수.
//-----------------------------------------------//
void printSIC (void) {

	char c;
	int i;

	// "sicsim>" 출력
	printf("sicsim> ");

	// newLine(\n)이 아닌 문자를 command 전역변수에 저장.
	i = 0;
	while ((c = getchar()) != '\n') {
		command[i++] = c;
	} command[i] = '\0';

	// 입력이 완료 되면 이를 Tokenizing.
	if  (i > 0) command_tokenizing();

	return;
}

//-----------------------------------------------//
// 함수이름: command_tokenizing
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: 입력 받은 전역변수 command를
//	     토큰 단위로 잘라서 전역변수 parameter[]에
//	     하나씩 저장하고, 이 갯수를 parameter_count에
//	     저장한다.
//-----------------------------------------------//
void command_tokenizing (void) {

	// 변수 설정
	char temp_command[MAX];
	int i, j;

	int command_check, comma_check, parameter_check, moreinput_check;

	// 입력 된 command가 손상되지 않도록 temp_command로 복사하고 사용한다.
	strcpy(temp_command,command);

	//
	// 명령어는 [mnemonic] [parameter], [parameter] ... 의 형식이다.
	//
	// 명령어는 몇가지 규칙을 따라야 하는데, 이는 다음과 같다.
	// 1. [mnemonic]이 나온 후에 [parameter]가 나와야 한다.
	// 2. [mnemonic]은 [parameter]와 공백으로 구분된다.
	// 3. [parameter]는 쉼표(,)로 구분된다.
	// 4. [parameter]사이에서 공백이 사용될 수 있다. 예) dump 4, 5/ dump 4,5
	//
	// 이를 변수로 바꾸어 설명해보면 다음과 같다.
	// 1. [mnemonic]이 나오면 command_check = 1 이 되며, 
	//    command_check = 0 일땐, [parameter]를 받지 않는다.
	// 2. command_check = 1 이고, [parameter]가 나오기 전까지 공백을 무시한다.
	// 3. 쉼표(,)가 나오면 comma_check = 1 이 되며, 다음 [parameter]가 올 수 있다.
	//    comma_check가 0 일때, 다음 [parameter]가 오거나 쉼표가 두번 연속 오면
	//    이는 잘못 된 입력이다. 또한, 쉼표(,)가 있는데 문장이 끝나도 잘못 된 입력이다.
	// 4. [parameter] 사이에 공백은 무시한다.
	//
	// 또, [parameter]를 구분 할 수 있는 경우는 다음과 같다.
	// 1. 문자 후에 공백/NULL이 올경우. 예) dump 4 / dump 5
	// 2. 문자 후에 쉼표(,)가 올 경우.  예) dump 4, 5 -> [4]와 [5]가 parameters
	// 
	// parameter_check 변수는 현재 parameter를 입력 받고 있는지를 검사하며,
	// morinput_check 변수는 [mnemonic] 이후에 [parameter]가 나올 때 1이 된다.
	
	// 변수 초기화
	command_check = comma_check = parameter_check = moreinput_check = 0;

	for (i = j = 0; i <= strlen(temp_command); i++) {
		// [mnemonic]를 받을 차례 (반드시 먼저 받아지므로)
		if (command_check == 0) {
			// 공백/NULL 이 아니면 저장한다
			if (temp_command[i] != ' ' && temp_command[i] != '\t' && temp_command[i] != '\0') {
				parameter[parameter_count][j++] = temp_command[i];
	
			// [mnemonic]이 입력 된 후, 공백이 나오면 [mnemonic]을 저장한다.
			// 규칙 2번에 의해 공백이 주어졌을 때, 저장하는 것이다.
			} else {
				if (j > 0) {
					parameter[parameter_count++][j] = '\0';

					// 이제 [parmaeter]를 받을 차례이므로, command_check = 1.
					command_check = 1;

					// 첫 [parameter]는 쉼표 없이 구분되므로 comma_check = 1로 설정.
					comma_check = 1;
					j = 0;
				}
			}
		// [parameter]를 받을 차례
		} else {
			// 쉼표(,)가 나온 상태일 때. 즉, [parameter]를 받을 수 있는 상태.
			if (comma_check == 1) {
				// 공백/NULL 이 아니면 저장한다.
				if (temp_command[i] != ' ' && temp_command[i] != '\t' && temp_command[i] != '\0') {
					// [mnemonic] 외에 [parameter]를 입력 받고 있으므로, moreinput_check = 1.
					moreinput_check = 1;

					// 만약 공백/NULL이 아닌 문자가 쉼표(,)라면 연속으로 쉼표(,)가 나온 것이므로
					// 에러에 해당하므로 에러메세지를 출력하고 리턴한다.  예) dump 4,,6
					if (temp_command[i] == ',') {
						if (parameter_check == 0) {
							ERROR(ERR_PAR);
							initialize_parameter();
							return;
						} else {
							// 단, [parameter]를 입력받자마자 쉼표(,)가 나온 경우에는
							// 쉼표(,)가 연속적으로 나온게 아닌, [parameter]의 입력 완료를
							// 나타내므로([parameter]를 구분하는 2번째 경우) 저장한다.
							parameter[parameter_count++][j] = '\0';

							j = 0;
							// [parameter]의 입력이 끝났으므로, parameter_check = 0.
							parameter_check = 0;
						}
					// 쉼표(,)가 아닐경우, 한 캐릭터씩 저장한다.
					} else {
						// [parameter]를 입력 중이므로, parameter_check = 1.
						parameter_check = 1;
						parameter[parameter_count][j++] = temp_command[i];
					}
				// 공백/NULL이 나왔을 경우,
				} else {
					// [parameter]가 입력 중일 경우, 입력이 완료된 것이므로 저장한다.
					// 그렇지 않으면 아무 경우도 아니므로 아무것도 하지 않는다.
					if (parameter_check == 1) {
						parameter[parameter_count++][j] = '\0';
						
						j = 0;

						// [parameter]의 입력이 끝났으므로, parameter_check = 0
						parameter_check = 0;
						// 이제 다음 [parameter]을 받으려면, 쉼표(,)가 필요하므로
						// comma_check = 0 이 된다.
						comma_check = 0;
					}
				}
			// 쉼표(,)가 나오지 않은 상태
			} else {
				// 공백/NULL이 아닌 문자가 왔을 경우, 쉼표(,) 없이 [parmeter]을 받았으므로
				// 이는 오류에 해당한다. 예) dump 4 6 같은 경우.
				if (temp_command[i] != ' ' && temp_command[i] != '\t' && temp_command[i] != '\0') {
					// 공백/NULL이 아닌 문자가 쉼표(,)라면 comma_check = 1 로 설정한다.
					if (temp_command[i] == ',') {
						comma_check = 1;
					// 쉼표(,)가 아니라면 명백한 오류이므로, 오류를 출력하고 리턴.
					} else {
						ERROR(ERR_PAR);
						initialize_parameter();
						return;
					}
				}
			}
		}
	}

	// 쉼표(,)가 있는데 [parameter]를 받지않고, 입력이 끝났다면 이는 오류에 해당하므로,
	// 오류를 출력하고 리턴한다.  예) dump 4, 
	if (comma_check == 1 && moreinput_check == 1) {
		ERROR(ERR_PAR);
		initialize_parameter();
		return;
	}

	return;
	
}

void initialize_parameter (void) {

	int i;

	// parameter_count와 paramter들을 초기화
	parameter_count = 0; isError = 0;
	for (i = 0; i < MAX_PARAMETER; i++) {
		parameter[i][0] = '\0';
	} 

	return;

}

void initialize_command (void) {

	int i;
	
	// command 변수를 초기화
	for (i = 0; i < MAX; command[i++] = '\0');

	return;
}

//-----------------------------------------------//
// 함수이름: command_analyser
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: parameter_analyser() 함수를 통해
//           입력한 command가 어떤 명령인지 판단하여
//           알맞은 함수를 호출.
//-----------------------------------------------//
void command_analyser (void) {

	switch (parameter_analyser()) {
	case -1: isError = 1;	break;		//없는 명령어
	case  0: help(); 	break;		//help 명령어
	case  1: dir(); 	break; 		//dir  명령어
	case  2: isError = 2;	break; 		//quit 명령어
	case  3: history(LIST_HISTORY); break;	//history 명령어
	case  4: dump(); 	break; 		//dump 명령어
	case  5: edit();	break; 		//edit 명령어
	case  6: fill();	break; 		//fill 명령어
	case  7: reset();	break; 		//reset 명령어
	case  8: OpcodeMnemonic(MneToOp); break; //opcode 명령어
	case  9: OpcodeMnemonic(OpToMne); break; //mnemonic 명령어
	case 10: hash_list(MneToOp); break; //opcodelist 명령어
	case 11: hash_list(OpToMne); break; //mnemoniclist 명령어
	}

	// 에러가 없어야 history Linked-List에 저장한다.
	if (isError == 0) {
		history(SAVE_HISTORY);
	}

	return;

}

//-----------------------------------------------//
// 함수이름: parameter_analyser
// 리 턴 값: commandMenu[]의 index, 없으면 -1
// 파라미터: 없음
// 목    표: 입력받은 command중에서 [mnemonic]에
//	     해당하는 parameter[0]과 메뉴들의 집합인
//	     commandMenu[] 배열과 비교하여 일치하는
//	     commandMenu[]의 index값을 반환.
//-----------------------------------------------//
int parameter_analyser (void) {

	int i;
	
	// 전역변수로 설정된 commancMenu[]와 parameter[0]를 비교
	for (i = 0; i < MENU_COUNT; i++) {
		if (strcmp(parameter[0],commandMenu[i][0]) == 0 ||
		    strcmp(parameter[0],commandMenu[i][1]) == 0) {
			return i;
		}
	}

	return -1;

}

//-----------------------------------------------//
// 함수이름: history
// 리 턴 값: 없음
// 파라미터: 저장인지 출력인지 결정하는 integer.
// 목    표: 명령어를 입력하면 history(SAVE_HISTORY)를
//	     통해 저장하고, 출력할 때는 LIST_HISTORY를
//	     통해 출력한다.
//-----------------------------------------------//
void history (int operation) {

	// 예외 처리, [parameter]가 없어야 한다.
	if (operation != SAVE_HISTORY && 
	    parameter_count != 1) {
		ERROR(ERR_PAR);
		return;
	}

	// Linked-List 를 위해 포인터 설정
	history_node *current = NULL;
	history_node *previous = NULL;

	// SAVE_HISTORY 일 경우, List 끝에 저장.
	if (operation == SAVE_HISTORY) {
		current = history_head;
		
		// Linked-List 의 끝까지 이동
		while (current != NULL) {
			previous = current;
			current = current->nextNode;
		}

		// malloc을 통해 메모리를 할당하고, 값을 저장.
		current = (history_node *)malloc(sizeof(history_node));
		strcpy(current->command,command);
		current->nextNode = NULL;		

		if (history_head != NULL)	{ previous->nextNode = current;  }
		else		  		{ history_head = current; }

		return;

	// LIST_HISTORY 일 경우, List 를 출력.
	} else {
		// 방금 입력한 hi[story]를 저장하고 출력해야한다.
		history(SAVE_HISTORY);
		// 중복 처리 되는 걸 방지하기 위해 isError = 1.
		isError = 1;

		// 갯수를 Count할 변수 설정
		int xIn = 1;

		current = history_head;
		
		// Linked-List를 끝까지 이동하면서 출력.
		while (current != NULL) {
			printf("%d\t%s\n",xIn++,current->command);
			current = current->nextNode;
		}

		return;

	}

}

//-----------------------------------------------//
// 함수이름: help
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: 이 프로그램에서 사용 가능한 명령어를
//	     출력한다.
//-----------------------------------------------//
void help (void) {

	// 예외 처리, [parameter]가 없어야 한다.
	// 예) help 1 / help 2, 3
	if (parameter_count != 1) {
		ERROR(ERR_PAR);
		return;
	}

	printf("       h[elp]\n");
	printf("       d[ir]\n");
	printf("       q[uit]\n");
	printf("       hi[story]\n");
	printf("       du[mp] [start, end]\n");
	printf("       e[dit] address, value\n");
	printf("       f[ill] start, end, value\n");
	printf("       reset\n");
	printf("       opcode mnemonic\n");
	printf("       mnemonic opcode\n");
	printf("       opcodelist\n");
	printf("       mnemoniclist\n");

	return;
}

//-----------------------------------------------//
// 함수이름: dir
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: 폴더 내의 파일과 폴더를 순차적으로 출력.
//-----------------------------------------------//
void dir (void) {

	// 예외 처리, [parameter] 가 없어야 한다.
	if (parameter_count != 1) {
		ERROR(ERR_PAR);
		return;
	}

	// 변수 설정
	DIR            	*execute_folder;
   	struct dirent  	*current = NULL;

	struct stat	file;
	int    count = 1;

	// 현재 폴더 "."를 연다.
	execute_folder = opendir(".");
   	if (execute_folder != NULL) {
		// readdir 함수를 통해 파일 하나씩 읽음.
      		while((current = readdir(execute_folder)) != NULL) {

			// d_name 변수를 통해 파일의 이름을 출력.
        		printf("%16s", current->d_name);

			// lstat 함수를 통해 해당 파일의 type을 읽음.
			if (lstat(current->d_name,&file) < 0) {
				printf("ERROR: Can not access File State.\n");
				return;
			}

			// S_ISDIR() 함수를 통해 디렉토리인지 판단.
			if      (S_ISDIR(file.st_mode)) printf("/");

			// S_IXUSR 과 &(AND) 연산을 통해 Executable 인지 판단.
			else if (file.st_mode&S_IXUSR)  printf("*");

			// 가독성 좋은 출력을 위해 3개씩 출력
			if (count++ % 3 == 0) printf("\n");
			else 	 	  printf("\t");
      		}
      		closedir(execute_folder);
   	} else {
		printf("ERROR: Can not Access Folder.\n");
		return;
	} 
	printf("\n"); 

	return; 
}

//-----------------------------------------------//
// 함수이름: dump
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: 가상메모리 memory변수에 저장 된 값을
//           출력하며, [parameter]에 따라 range를
//	     설정하여 출력.
//-----------------------------------------------//
void dump (void) {

	int i, j, adx;
	int p, q, line;

	// 예외처리와 함께 [parameter]가 있을 때,
	// Hexadecimal로 변환작업을 실행.
	if (parameter_count > 1) {
		// isHex(parameter,variable)을 통해 16진수 입력
		if (!isHex(parameter[1],&p)) {
			ERROR(ERR_HEX);
			return;
		}
	// [parameter]가 없으면 방금 전 마지막 출력주소 +1 을 불러들임.
	} else p = dump_head;

	// 2번째 [parameter]가 있으면, 16진수로 입력
	if (parameter_count > 2) {
		if (!isHex(parameter[2],&q)) {
			ERROR(ERR_HEX);
			return;
		}
	// [parameter]가 없으면, 10줄을 출력하기 위해 마지막값 계산.
	} else q = p + 160 - 1;
	if (parameter_count > 3) {
		ERROR(ERR_PAR);
		return;
	}

	// 예외처리, Out of Range 에러를 출력한다.
	if (p < 0 ||  q < 0 || p > q || p >= MAX_MEMORY ||
	    (p < MAX_MEMORY && q >= MAX_MEMORY)) {
		ERROR(ERR_OUT);
		return;
	}

	// 시작 위치(p)와 끝날 위치(q)를 출력하는데,
	// 총 몇 줄이 출력되는지를 미리 계산.
	line = ((q - q%0x10) - (p - p%0x10)) / 0x10 + 1;

	// 출력되는 줄의 첫 주소값을 adx에 저장.
	adx = p - p%0x10;
	for (i = 0; i < line; i++, adx+=0x10) {

		// 해당 줄의 첫 메모리 주소값 출력
		printf("%05X ",adx);

		// 총 16개의 메모리 값을 출력
		for (j = 0; j < 0x10; j++) {
			if (adx + j >= p && adx + j <= q) {
				printf("%02X ",memory[adx+j]);
			} else {
			// 단, range내에 들어오지 않는 것은 빈칸 출력.
				printf("   ");
			}
		}
		printf("; ");

		// 총 16개의 ASCII 코드 출력.
		for (j = 0; j < 16; j++) {
			// Range내에 들어오며, 0x20과 0x7E사이를 출력.
			if (adx + j >= p && adx + j <= q) {
				if (memory[adx + j] >= 0x20 &&
				    memory[adx + j] <= 0x7E) {
					printf("%c",memory[adx + j]);
					continue;
				}
			}
			printf(".");
		}
		printf("\n");
	}

	// 마지막 출력 주소값에 1을 더함.
	dump_head = q + 1;

	return;
}

//-----------------------------------------------//
// 함수이름: reset
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: 모든 가상메모리 mememory 배열을 초기화.
//-----------------------------------------------//
void reset (void) {

	// 예외 처리.
	if (parameter_count != 1) {
		ERROR(ERR_PAR);
		return;
	}

	int i;

	// 모든 메모리를 0x00으로 초기화
	for (i = 0; i < MAX_MEMORY; i++) {
		memory[i] = 0x00;
	}

}

//-----------------------------------------------//
// 함수이름: edit
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: [parameter]에 지정된 위치에 특정 값을
//	     저장한다.
//-----------------------------------------------//
void edit (void) {

	// [mnemonic] + [parameter]의 갯수가 3개이여야 한다.
	if (parameter_count != 3) { 
		ERROR(ERR_PAR);
		return;
	}

	int adx, val;

	// [parameter]로 입력 받은 값을 Hexadecimal로 변환.
	if (!isHex(parameter[1],&adx) ||
	    !isHex(parameter[2],&val)) {
		ERROR(ERR_HEX);
		return;
	}

	// 입력값이 0xFF보다 크면 에러
	if (val > 0xFF) {
		ERROR(ERR_SO_BIG);
		return;
	}

	// 입력하는 주소가 메모리 Range 밖이면 에러
	if (adx < 0 || adx >= MAX_MEMORY) {
		ERROR(ERR_OUT);
		return;
	}

	// 값 입력
	memory[adx] = val;

	return;

}

//-----------------------------------------------//
// 함수이름: fill
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: 입력 시작 위치부터 끝 위치까지
//           입력 된 값으로 설정한다.
//-----------------------------------------------//
void fill (void) {

	// 총 3개의 [parameter]가 들어와야한다.
	if (parameter_count != 4) {
		ERROR(ERR_PAR);
		return;
	}

	int bgn, end, val;

	// [parameter]들을 16진수로 입력
	if (!isHex(parameter[1],&bgn) ||
	    !isHex(parameter[2],&end) ||
	    !isHex(parameter[3],&val)) {
		ERROR(ERR_HEX);
		return;
	}

	// 입력값이 0xFF보다 크면 에러
	if (val > 0xFF) {
		ERROR(ERR_SO_BIG);
		return;
	}

	// 주소값이 Out Range일 경우 예외 처리
	if (bgn < 0 || end < 0 || bgn > end ||
	    bgn >= MAX_MEMORY  || end >= MAX_MEMORY) {
		ERROR(ERR_OUT);
		return;
	}

	// 시작위치(bgn)부터 끝위치(end)까지 설정
	for (;bgn<=end;bgn++) {
		memory[bgn] = val;
	}

	return;

}

//-----------------------------------------------//
// 함수이름: OpcodeMnemonic
// 리 턴 값: 없음
// 파라미터: MneToOp인지, OpToMne인지를 확인
// 목    표: Hash Table로 만들어진 Opcode Table과
//	     Mnemonic Table을 탐색하면서, Opcode 혹은
//           Mnemonic 을 출력함.
//-----------------------------------------------//
void OpcodeMnemonic (int operation) {

	// [parameter]가 1개 있어야 한다.
	if (parameter_count != 2) {
		ERROR(ERR_PAR);
		return;
	}

	int StrToInt;

	hash_node *current;

	// Opcode Mnemonic 입력을 받았을 때
	if (operation == MneToOp) {

		// opcode_list 배열에서 hasu_function 을 통해 첫 header를 지정
		current = opcode_list[hash_function(operation,parameter[1],0)];

	// Mnemonic Opcode 입력을 받았을 때,
	} else {

		// 입력 받은 Opcode를 16진수로 변환
		if (!isHex(parameter[1],&StrToInt)) {
			ERROR(ERR_HEX);
			return;
		}

		// mnemonic_list 배열에서 hash_function 을 통해 첫 header를 지정
		current = mnemonic_list[hash_function(operation,NULL,StrToInt)];
	}

	// Linked-List를 따라가면서 알맞은 Opcode 혹은 Mnemonic 확인
	while (current != NULL) {
		if (operation == MneToOp) {
			// strcmp를 통해 동일한 mnemonic이 있는지 검사
			if (strcmp(current->mnemonic,parameter[1]) == 0) {
				printf("opcode is %02X\n",current->opcode);
				return;
			}
		} else {
			// opcode와 같은 값이 있는지 확인.
			if (current->opcode == StrToInt) {
				printf("mnemonic is %s\n",current->mnemonic);
				return;
			}
		}
		current = current->nextNode;
	}

	// while문을 빠져나왔다면, 없다는 뜻이므로 에러 출력
	ERROR(ERR_NOT_IN);

	return;

}

//-----------------------------------------------//
// 함수이름: isHex
// 리 턴 값: 16진수가 맞으면 1, 아니면 0
// 파라미터: 16진수인지 맞는지 확인할 문자열 str
//	     16진수가 맞다면 수를 저장한 int dst
// 목    표: 입력받은 문자열이 16진수가 맞는지 판단하고
//	     16진수가 맞다면 저장 후, 리턴
//-----------------------------------------------//
int isHex (char *str,int *dst) {

	int i;

	for (i = 0; i < strlen(str); i++) {
		if ((str[i] >= '0' && str[i] <= '9') ||
		    (str[i] >= 'A' && str[i] <= 'F') ||
		    (str[i] >= 'a' && str[i] <= 'f')) {
			continue;
		} else {
			return 0;
		}
	}

	sscanf(str,"%x",dst);

	return 1;

}

//-----------------------------------------------//
// 함수이름: ERROR
// 리 턴 값: 없음
// 파라미터: 어떤 에러인지를 나타내는 int idx
// 목    표: 특정 예외처리에서 특정 문구를 출력.
//-----------------------------------------------//
void ERROR (int idx) {

	// isError 를 1로 변경하여, history에
	// 저장되지 않게 한다.
	isError = 1;
	printf("%s\n",errorMenu[idx]);

	return;

}

//-----------------------------------------------//
// 함수이름: hash_make
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: opcode.txt 를 읽어들여서, hash table을
//           hash_function을 통해 Linked-List를 만든다.
//-----------------------------------------------//
void hash_make (void) {

	// 파일을 읽어들임.
	FILE *fp = fopen("opcode.txt","r");
	if (fp == NULL) {
		printf("ERROR: 'opcode.txt' access error.\n");
		return;
	}

	int count;

	// opcode_list와 mnemonic_list를 초기화
	for (count = 0; count < MAX_HASH_SIZE; count++) {
		opcode_list[count] = mnemonic_list[count] = NULL;
	}

	// 변수 설정
	char inputline[MAX];
	char separator[] = " \t\n";
	char *token;

	char mnemonic[MAX_OPCODE];
	int  opcode;

	// 파일로부터 한줄씩 입력 받음.
	while (fgets(inputline,MAX,fp) != NULL) {
		count = 0;

		// separator을 통해 tokenizing을 한다.
		token = strtok(inputline,separator);
		while (token != NULL) {
			// 나머지 Token은 필요 없으므로 배제.
			if (count == 2) {
				count = 0;
				break;
			}
			// 두번째 Token일 경우, mnemonic에 저장
			if (count == 1) {
				count++;
				strcpy(mnemonic,token);
			}
			// 첫 Token일 경우, Opcode 이므로 16진수로 변환
			// integer opocde 변수에 저장
			if (count == 0) {
				count++;
				if (!isHex(token,&opcode)) {
					ERROR(ERR_HEX);
					return;
				}
			}
			token = strtok(NULL,separator);
		}

		// hash_function을 통해 index를 구함
		// MneToOp를 파라미터로 넘겨주어 Opcode Table의 hash_function 계산.
		count = hash_function(MneToOp,mnemonic,0);

		// 해당 Index에 Linked-List 에 저장
		hash_add(&opcode_list[count],mnemonic,opcode);	

		// hash_function을 통해 index를 구함
		// OpToMne를 파라미터로 넘겨주어 Mnemonic Table의 hash_function 계산.
		count = hash_function(OpToMne,NULL,opcode);

		// 해당 Index에 Linked-List 에 저장.
		hash_add(&mnemonic_list[count],mnemonic,opcode);
	}

}

//-----------------------------------------------//
// 함수이름: hash_list
// 리 턴 값: 없음
// 파라미터: MneToOp 인지, OpToMne인지 확인하는 int operation.
// 목    표: Mnemonic Table 혹은 Opcode Table을 출력.
//-----------------------------------------------//
void hash_list (int operation) {

	// 예외처리, [parameter]가 있으면 안된다.
	if (parameter_count != 1) {
		ERROR(ERR_PAR);
		return;
	}

	int i;
	hash_node *current;
	hash_node **head;

	// 파라미터에 따른 head 포인터 설정.
	if (operation == MneToOp) {
		head = opcode_list;
	} else {
		head = mnemonic_list;
	}

	// 0 ~ 19 까지의 배열을 탐색
	for (i = 0; i < MAX_HASH_SIZE; i++) {
		printf("%2d : ",i);
		current = head[i];

		// Linked-List를 탐색하며 출력
		while (current != NULL) {
			printf("[%s,%02X]",current->mnemonic,current->opcode);
			if (current->nextNode != NULL) {
				printf(" -> ");
			}
			current = current->nextNode;
		}
		printf("\n");
	}

	return;
}

//-----------------------------------------------//
// 함수이름: hash_add
// 리 턴 값: 없음
// 파라미터: hash_function에서 받은 인덱스 head.
//	     입력 될 mnemonic 과 opcode.
// 목    표: Linked-List를 만들고 추가한다.
//-----------------------------------------------//
void hash_add (hash_node **head, char *mnemonic, int opcode) {

	// 변수 설정
	hash_node *current;
	hash_node *previous;

	// 마지막 Linked-List를 찾는다.
	current = *head;
	while (current != NULL) {
		previous = current;
		current = current->nextNode;
	}

	// malloc을 통해 메모리 할당
	current = (hash_node *)malloc(sizeof(hash_node));
	
	// 값을 저장
	strcpy(current->mnemonic,mnemonic);
	current->opcode = opcode;
	current->nextNode = NULL;

	if (*head == NULL) *head = current;
	else previous->nextNode = current;

	return;
}

//-----------------------------------------------//
// 함수이름: hash_function
// 리 턴 값: 해쉬값을 return.
// 파라미터: OpToMne 혹은 MneToOP인지를 보는 int operation.
//	     hash_function을 계산할 mnemonic과 opcode.
// 목    표: 해쉬테이블을 만들기 위한 해쉬 함수.
//	     계산 되어진 index 값을 리턴한다.
//-----------------------------------------------//
int hash_function (int operation,char *mnemonic,int opcode) {

	if (operation == OpToMne) {
		// Mnemonic Table의 Hash값을 리턴
		return (opcode / 0x14 + opcode % 0x14) % MAX_HASH_SIZE;
	} else {
		// Opcode Table의 Hash값을 리턴
		int sum = 0, i;

		for (i = 0; i < strlen(mnemonic); i++) {
			sum += (int)(mnemonic[i] - 'A') + (i + 1);
		}

		return sum % MAX_HASH_SIZE;
	}

}
