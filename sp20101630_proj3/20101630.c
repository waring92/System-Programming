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

	// Symtab 메모리 Free
	for (i = 0; i < MAX_HASH_SIZE; i++) {
		currenth = symbol_list[i];
		while (currenth != NULL) {
			previoush = currenth;
			currenth = currenth->nextNode;
			free(previoush);
		}
		currenth = symbol_temp[i];
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
	case  1: dir(DIR_DIRECTORY); 	break; 	//dir  명령어
	case  2: isError = 2;	break; 		//quit 명령어
	case  3: history(LIST_HISTORY); break;	//history 명령어
	case  4: dump(); 	break; 		//dump 명령어
	case  5: edit();	break; 		//edit 명령어
	case  6: fill();	break; 		//fill 명령어
	case  7: reset();	break; 		//reset 명령어
	case  8: OpcodeMnemonic(MneToOp); break;//opcode 명령어
	case  9: OpcodeMnemonic(OpToMne); break;//mnemonic 명령어
	case 10: hash_list(MneToOp); break; 	//opcodelist 명령어
	case 11: hash_list(OpToMne); break; 	//mnemoniclist 명령어
	case 12: assemble_run(); break; 	//assemble 명령어
	case 13: dir(DIR_TYPE);		break; 	//type 명령어
	case 14: symtab_list();	break; 		//symbol 명령어
	case 15: disassemble_run();	break; 	//disassemble 명령어

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
	printf("       assemble filename\n");
	printf("       type filename\n");
	printf("       symbol\n");
	printf("       disassemble filename\n");

	return;
}

//-----------------------------------------------//
// 함수이름: dir
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: 폴더 내의 파일과 폴더를 순차적으로 출력.
//-----------------------------------------------//
void dir (int operation) {

	// 예외 처리, [parameter] 가 없어야 한다.
	if (operation == DIR_DIRECTORY && parameter_count != 1) {
		ERROR(ERR_PAR);
		return;
	// 만약 'TYPE' 명령을 통해 접근 했다면 [parameter]가 있어야한다.
	} else if (operation == DIR_TYPE && parameter_count != 2) {
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

			// 'TYPE' 명령을 통해 접근 했으면, 동일한 이름의 파일을 읽고 출력
			if (operation == DIR_TYPE) {
				// d_name과 입력 받은 fileame과 비교
				if (strcmp(current->d_name,parameter[1]) == 0) {
					FILE *op = fopen(current->d_name,"r");
					char type_print[MAX];

					// 한줄씩 읽으며 그대로 화면에 출력
					while (fgets(type_print,MAX,op) != NULL) {
						printf("%s",type_print);
					}
					
					return;
				}
			} else if (operation == DIR_DIRECTORY) {	
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
		}

		// 모든 파일을 읽을 때까지, 출력을 못한다면 그런 파일이 없다는 에러를 출력.
		if (operation == DIR_TYPE) {
			printf("ERROR: There isn't such a file\n");
			return;
		}

      		closedir(execute_folder);
  	} else {
		printf("ERROR: Can not Access Folder.\n");
		return;
	} 
	printf("\n"); 
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
		symbol_list[count] = symbol_temp[count] = 
		opcode_list[count] = mnemonic_list[count] = NULL;
	}
	symbol_count = 0;

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

		return abs(sum) % MAX_HASH_SIZE;
	}

}

//-----------------------------------------------//
// 함수이름: assemble
// 리 턴 값: 오류가 있으면 -1, 없으면 0
// 파라미터: 없음
// 목    표: 입력 된 .asm파일을 읽어들여 Pass1과
//	     Pass2 알고리즘을 실행하여 결과값 생성.
//	     오류가 있다면 오류를 화면상에 표시.
//-----------------------------------------------//
int assemble (void) {

	// Pass 1, Pass 2 알고리즘에 도움을 주게 될
	// intermediate 파일을 생성.
	FILE *intermediate = fopen("intermediate","w");

	// object listing과 object program 을 생성하기 전에
	// 임시로 값들을 저장해둘 파일 생성.
	// 값들에 오류가 있을 경우, [obj]와 [lst]를 생성하지 않기 위한 장치.
	FILE *lst = fopen("temp.lst","w");
	FILE *obj = fopen("temp.obj","w");

	// 입력 된 명령에 문제가 있으면 예외처리
	if (parameter_count != 2) {
		ERROR(ERR_PAR);
		return -2;
	}

	// 변수 선언 및 초기화
	int LOCCTR = 0x00000;
	int CURCTR = 0x00000;
	int ROWCTR = 0;

	int start_address = 0;
	int initial_address = -1;
	int program_length = 0;

	// .asm 파일을 읽어들임.
	FILE *op = fopen(parameter[1],"r");
	if (op == NULL) {
		printf("ERROR: There isn't such a file.\n");
		return -2;
	}

	char *extension;
	extension = parameter[1] + strlen(parameter[1]) - 4;
	if (strcmp(extension,".asm") != 0) {
		printf("ERROR: Input File is not [.asm] file.\n");
		return -2;
	}

	char inputLine[MAX];
	char preserve[MAX];

	char separator[] = ",\t \n\r";
	char *token;

	char label[MAX];
	char opcode[MAX];
	char operand[MAX];
	char exerand[MAX];
	
	int opcode_num;
	int operand_num;

	int objectcode;

	int token_counter = 0;

	int i, count, isError = 0;

	// Pass1 알고리즘에서 생성할 Symbol Table 초기화.
	hash_node *current = NULL, *previous = NULL;
	for (i = 0; i < MAX_HASH_SIZE; i++) {
		current = symbol_temp[i];
		while (current != NULL) {
			previous = current;
			current = current->nextNode;
			free(previous);
		}
		symbol_temp[i] = NULL;
	}

	// 한줄씩 읽어들임.
	while (fgets(inputLine,MAX,op) != NULL) {

		// 한 줄당 5의 Line Number 증가
		ROWCTR += 5;

		// 토큰을 하나씩 입력 받을 변수 초기화
		label[0] = opcode[0] = operand[0] = exerand[0] = '\0';

		// 오류를 방지하기 위해 읽어들인 줄을 그대로 보존.
		strcpy(preserve,inputLine);

		// operand 중, C'' 안에 들어있는 글자의 변환 작업.
		// C'' 안에 공백(' ')이나 쉼표(','), 탭('\t') 문자를
		// 임의의 값으로 치환함.
		for (i = count = 0; i < strlen(inputLine); i++) {
			// C'' 안에  있는 숫자만 변경함.
			if (inputLine[i] == '\'') {
				count++;
			}
			// 헤더에서 선언 된 상수 sptr_comma, sptr_space, sptr_tab으로 치환
			if (count == 1) {
				switch (inputLine[i]) {
				case  ',': inputLine[i] = sptr_comma;  break;
				case  ' ': inputLine[i] = sptr_space;  break;
				case '\t': inputLine[i] = sptr_tab; break;
				}
			}
		}

		// 읽어들인 한 줄을 토크나이징.
		token_counter = 0;
		token = strtok(inputLine,separator);
		while (token != NULL) {
			if (token[0] == '.') { break; }

			// 첫번째 토큰이 opcode에 존재하거나, assembler directives라면,
			// 라벨이 존재하지 않는 것이므로, 임의로 라벨이 없다는 뜻의
			// '####'을 넣어주고, opcode를 받을 준비를 함.
			if ((strcmp(token,"RESB") == 0 || strcmp(token,"RESW") == 0 ||
			    strcmp(token,"START") == 0 || strcmp(token,"END") == 0 ||
			    strcmp(token,"WORD") == 0 || strcmp(token,"BYTE") == 0 ||
			    hash_find(&opcode_list[hash_function(MneToOp,token,0)],token) >= 0) &&
			    token_counter == 0) {
				strcpy(label,"####");
				token_counter++;
			}

			// 각 token의 갯수마다 어떤 부분을 받고 있는지를 확인하여 저장.
			switch (++token_counter) {
			case 1: strcpy(label,token); break;
			case 2: strcpy(opcode,token); break;
			case 3: strcpy(operand,token); break;
			// Buffer,X처럼 뒤의 'X'는 exerand라는 별도의 변수에 저장.
			case 4: strcpy(exerand,token); break;
			}

			token = strtok(NULL,separator);
		}

		// Opcode가 없는 줄은 올바른 줄(공백줄 혹은 주석줄)이 아니므로 스킵.
		if (opcode[0] == '\0') {
			fprintf(intermediate,"%d\t      \t%s",ROWCTR,preserve);
			continue;
		}

		// C'' 내에 치환된 문자들을 다시 복구.
		for (i = 0; i < strlen(operand); i++) {
			if (operand[i] == sptr_comma) operand[i] = ',';
			if (operand[i] == sptr_space) operand[i] = ' ';
			if (operand[i] == sptr_tab) operand[i] = '\t';
		}

		// Pass1 알고리즘

		// opcode가 'START' 일 때, 'END' 일 때, 두경우 모두 아닐때.
		if (strcmp(opcode,"START") == 0) {
			// 첫 줄이므로 Line 5가 되며, operand에 있는 값이
			// 이 프로그램의 시작 주소값이 된다. 이를 LOCCTR에 저장.
			ROWCTR = 5;
			sscanf(operand,"%x",&start_address);
			LOCCTR = start_address;
			fprintf(intermediate,"%d\t%04X\t%s\t%s\t%s\n",ROWCTR,LOCCTR,label,opcode,operand);
			continue;
		} else if (strcmp(opcode,"END") == 0) {
			// 'START'일 때, 저장한 start_address에서 현재의 LOCCTR을 빼면,
			// 이 프로그램의 전체 길이를 알 수 있다. 이를 program_length에 저장.
			program_length = LOCCTR - start_address;
			fprintf(intermediate,"%d\t%04X\t%s\t%s\t%s\n",ROWCTR,LOCCTR,label,opcode,operand);
			break;
		} else {
			CURCTR = LOCCTR;
			// 라벨이 존재하는 경우, Symbol Table 검사
			if (label[0] != '\0' && strcmp(label,"####") != 0) {
				// symbol table을 검사하여, 만약 존재하면, duplicate 오류.
				if (hash_find(&symbol_temp[hash_function(MneToOp,label,0)],label) >= 0) {
					printf("ERROR: Line %d: Label '%s' was already existed.\n",ROWCTR,label);
					isError = -1;
				} else {
				// 존재하지 않는다면 symbol table에 저장.
					hash_add(&symbol_temp[hash_function(MneToOp,label,0)],label,LOCCTR);
				}
			}
			// opcode가 존재하는지, 아닌지 확인.
			if (hash_find(&opcode_list[hash_function(MneToOp,opcode,0)],opcode) >= 0) {
				// 존재하면 3 Bytes 명령이므로 LOCCTR에 3 가산.
				LOCCTR += 0x3;
			} else if (strcmp(opcode,"WORD") == 0) {
				// WORD 일 경우, 3 Bytes 이므로 LOCCTR에 3 가산.
				LOCCTR += 0x3;
			} else if (strcmp(opcode,"RESW") == 0) {
				// RESW 일 경우, operand의 숫자를 읽고, 그 수에 3을 곱하여 가산.
				sscanf(operand,"%d",&i);
				LOCCTR += (0x3 * i);
			} else if (strcmp(opcode,"RESB") == 0) {
				// RESB 일 경우, operand의 숫자를 읽고, 그 수만큼 가산.
				sscanf(operand,"%d",&i);
				LOCCTR += (0x1 * i);
			} else if (strcmp(opcode,"BYTE") == 0) {
				// BYTE 일 경우, 'X'와 'C'일 때를 나누어 몇개인지를 계산하여 가산.
				if (operand[0] == 'X') {
					for (i = 2, count = 0; i < strlen(operand); i++) {
						if (operand[i] == '\'') break;
						else count++;
					}
					// X'' 안에 16진수가 홀수개이면 에러
					if (count%2 != 0) {
						printf("ERROR: Line %d: The number of Hexadecimal is not even number.\n",ROWCTR);
						isError = -1;
					}
					// Hexadecimal은 2개에 1 Bytes이므로 갯수에 2를 나눔.
					LOCCTR += count/2;
				} else if (operand[0] == 'C') {
					for (i = 2, count = 0; i < strlen(operand); i++) {
						if (operand[i] == '\'') break;
						else count++;
					}
					LOCCTR += count;
				}	
			} else {
				// 위 모든 경우에 해당되지 않으면, Invalid Opcode 에러 출력.
				printf("ERROR: Line %d: Invalid Opcode. '%s'\n",ROWCTR,opcode);
				isError = -1;
			}
			// 결과값을 intermediate 파일에 저장.
			fprintf(intermediate,"%d\t%04X\t%s\t%s\t%s\t%s\n",ROWCTR,CURCTR,label,opcode,operand,exerand);
		}
	}

	// intermediate와 .asm파일 종료.
	fclose(intermediate);
	fclose(op);

	// PASS 2

	// 읽기 모드로 intermediate 를 읽어들임.
	op = fopen("intermediate","r");
	if (op == NULL) {
		printf("ERROR: intermediate file absent.\b");
		return -1;
	}

	// 변수 선언
	char iseparator[] = "\t\n";
	char objectLine[MAX];
	char objectStr[MAX];

	int objectcode_length;
	int objectcode_copy;
	int objectcode_digit;

	// object_program을 만들기 위한, objectLine을 초기화.
	objectLine[0] = '#';
	for (i = 1; i < 71; i++) {
		objectLine[i] = '\0';
	}

	// 한 줄씩 읽어들임.
	while (fgets(inputLine,MAX,op) != NULL) {

		// 오류를 대비해 입력 줄을 그대로 복사.
		strcpy(preserve,inputLine);

		// tokenizing을 위해 변수 초기화.
		token_counter = 0;
		label[0] = opcode[0] = operand[0] = exerand[0] = '\0';

		// C'' 안에 있는 공백, 콤마, 탭을 치환.(Pass 1과 동일)
		for (i = count = 0; i < strlen(inputLine); i++) {
			if (inputLine[i] == '\'') {
				count++;
			}
			if (count == 1) {
				switch (inputLine[i]) {
				case  ' ': inputLine[i] = sptr_space; break;
				case  ',': inputLine[i] = sptr_comma; break;
				case '\t': inputLine[i] = sptr_tab;   break;
				}
			}
		}

		// Tokenizing
		token = strtok(inputLine,iseparator);
		while (token != NULL) {
			if (token[0] == '.') break;

			// Pass 1에서 심볼이 없는 곳을 '####'으로 채워줬으므로,
			// 라벨이 '####'라면 무시하고 OPCODE를 입력받음.
			if (strcmp(token,"####") == 0) {
				token_counter++;	
			}  else {
				// 각각 입력 받은 순서에 따라 입력을 받음.
				switch (++token_counter) {
				case 1: sscanf(token,"%d",&ROWCTR); break;
				case 2: sscanf(token,"%x",&LOCCTR); break;
				case 3: strcpy(label,token);	    break;
				case 4: strcpy(opcode,token);	    break;
				case 5: strcpy(operand,token);	    break;
				case 6: strcpy(exerand,token);	    break;
				}
			}
			token = strtok(NULL,iseparator);
		}

		// 치환 했던 공백, 콤마, 탭을 복구.
		for (i = 0; i < strlen(operand); i++) {
			if (operand[i] == sptr_comma) operand[i] = ',';
			if (operand[i] == sptr_space) operand[i] = ' ';
			if (operand[i] == sptr_tab) operand[i] = '\t';
		}

		// opcode가 없는 줄은 assemble listing과 object program
		// 모두에 필요가 없으므로 무시하고 다음 줄 읽음
		if (opcode[0] == '\0') {
			continue;
		}

		// opcode가 'START', 'END', 두 경우 모두 아닐 때로 구분.
		if (strcmp(opcode,"START") == 0) {
			// object program에서 프로그램 이름을 6자리로 출력하므로,
			// 뒷부분 남는 만큼 공백을 삽입하여 맞춰줌.
			for (i = count = 0; i < 6; i++) {
				if (count == 0 && label[i] == '\0') {
					count = 1;
				}
				if (count == 1) {
					label[i] = ' ';
				}
			}

			// 임시 object 파일과 listing 파일에 출력.
			fprintf(obj,"H%s%06X%06X",label,start_address,program_length);
			fprintf(lst,"%s",preserve);

		} else if (strcmp(opcode,"END") == 0) {
			// 작성 중이던 줄을 끊고 다음 줄로 넘어가서, 'E' Record 출력
			if (objectcode_length == 0) {
				// 작성 중이던 줄이 없다면 바로 출력.
			} else {
				fprintf(obj,"%02X",objectcode_length/2);
				fprintf(obj,"%s\n",objectLine);	
			}
			fprintf(obj,"E%06X\n",initial_address);
			fprintf(lst,"%d\t    \t      \t%s\t%s\n",ROWCTR,opcode,operand);
		} else {
			// 변수 초기화 및 opcode mnemonic의 opcode 값으로 찾아옴.
			objectcode = 0x000000;
			opcode_num = hash_find(&opcode_list[hash_function(MneToOp,opcode,0)],opcode);

			// 존재하는 opcode 일 경우,
			if (opcode_num >= 0) {
				// operand 가 존재하면, symbol table에서 검사.
				if (operand[0] != '\0') {
					operand_num = hash_find(&symbol_temp[hash_function(MneToOp,operand,0)],operand);
					// symbol table에 존재한다면, objectcode 생성.
					if (operand_num >= 0) {
						objectcode += operand_num;
					} else {
					// 없다면 undefined symbol 에러 출력.
						printf("ERROR: Line %d: undefined symbol used\n",ROWCTR);
						isError = -1;
					}
				}
			
				// X Register 연산이 존재할 경우,	
				if (exerand[0] != '\0') {
					if (exerand[0] == 'X' || exerand[0] == 'x') {
						// 9번째 bit에 1을 가산.
						objectcode += 0x008000;
					}
				}

				// opcode가 존재하므로 항상 명령은 3 Bytes, 6 글자.
				count = 6;
				objectcode += (opcode_num * 0x010000);

				// 생성 된 integer의 objectcode를 string으로 변환.
				i = 0; objectStr[count] = '\0';
				objectcode_copy = objectcode;
				while (i != count) {
					objectcode_digit = objectcode_copy % 0x10;
					objectcode_copy /= 0x10;

					objectStr[count-i-1] = objectcode_digit + ((objectcode_digit >= 0xA)?'A' - 0xA:'0');
					i++;
				}
			// opcode에 'BYTE'가 있을 경우,
			} else if (strcmp(opcode,"BYTE") == 0) {
				// 몇 글자인지 알 수 없으므로 변수 초기화
				count = 0;
				// C 일 경우, 홑따옴표(')가 나오기 전까지 갯수를 체크.
				if (operand[0] == 'C') {
					for (i = 2; i < strlen(operand); i++) {
						if (operand[i] == '\'') {
							break;
						} else {
							// 한 글자에 2개의 Hexadecimal이 나오므로 2개씩 저장.
							// 4F -> '4' , 'F' 로 저장.
							objectStr[count] = operand[i] / 0x10;
							objectStr[count+1] = operand[i] % 0x10;

							objectStr[count] += ((objectStr[count] >= 0xA)?'A' - 0xA:'0');
							objectStr[count+1] +=  ((objectStr[count+1] >= 0xA)?'A' - 0xA:'0');

							// 한 캐릭터당 2개씩 증가.
							count+=2;
						}	
					}
				// X 일 경우, 반드시 짝수개의 Hexadecimal이 나와야 하므로 2개씩 저장.
				} else if (operand[0] == 'X') {
					for (i = 2; i < strlen(operand); i += 2) {
						if (operand[i] == '\'') {
							break;
						} else {
							// 옳지 않은 16진수가 들어오면 에러
							int j;
							for (j = i; j < i + 2; j++) {
								if ((operand[j] >= '0' && operand[j] <= '9') ||
								    (operand[j] >= 'A' && operand[j] <= 'F') ||
								    (operand[j] >= 'a' && operand[j] <= 'f')) {
									continue;
								} else if (operand[j] == '\'') {
									continue;
								}  else {
									printf("ERROR: Line %d: Invalid Hexadecimal [%c].\n",ROWCTR,operand[j]);
									isError = -1;
								}
							}

							// 2글자씩 objectStr에 저장함.
							objectStr[count] = operand[i];
							objectStr[count+1] = operand[i+1];

							count+=2;
						}
					}
				}
				// objectStr에 문자열로 저장됨.
				objectStr[count] = '\0';

			// opcode에 'WORD'가 들어있을 경우,
			} else if (strcmp(opcode,"WORD") == 0) {
				// operand 값을 integer로 변환하여 숫자를 셈.
				sscanf(operand,"%d",&operand_num);

				// 만약 0보다 작은 음수라면, 이를 2의 보수로 변환.
				if (operand_num < 0) {
					operand_num *= -1;
					operand_num ^= 0xFFFFFF;
					operand_num += 1;
				}
				// objectcode를 만들고, 6자리 object code.
				objectcode += operand_num;
				count = 6;

				// integer로 만들어진 objectcode를 문자열로 변환.
				i = 0; objectStr[count] = '\0';
				objectcode_copy = objectcode;
				while (i != count) {
					objectcode_digit = objectcode_copy % 0x10;
					objectcode_copy /= 0x10;

					objectStr[count-i-1] = objectcode_digit + ((objectcode_digit >= 0xA)?'A' - 0xA:'0');
					i++;
				}

			} else {
			// 위 경우 모두에 해당되지 않는다면, 그냥 출력.
			// RESW , RESB 와 같은 경우.
				fprintf(lst,"%s",preserve);
			}

			// Object Program을 만드는 ObjectLine이 아래 조건이 만족 되면, 새로운 T 레코드 준비.
			//
			// 1) objectLine[0] = '#' 인 경우, H 레코드를 쓴 직후. 즉, 프로그램의 첫 부분.
			// 2) 앞으로 추가 될 count값을 더하고 나면 60줄이 초과 되는 경우,
			// 3) opcode 자리에 'RESB', 'RESW'가 나오고, 새로운 줄이 아닐 경우.
			if (objectLine[0] == '#' || objectcode_length  + count > 60 || 
			    ((strcmp(opcode,"RESB") == 0 || strcmp(opcode,"RESW") == 0) && objectcode_length > 0)) {
				// 지금까지 입력 된 object code들을 한번에 출력해주고 줄바꿈 시켜줌.
				if (objectLine[0] != '#') {
					fprintf(obj,"%02X",objectcode_length/2);
					fprintf(obj,"%s",objectLine);
				}
				fprintf(obj,"\n");
				// objectLine 변수는 초기화 시켜줌.
				objectcode_length = 0;
				for (i = 0; i < 71; i++) {
					objectLine[i] = '\0';
				}
			}

			// RESB 혹은 RESW는 더 이상 할일이 없으므로 continue.
			if (strcmp(opcode,"RESB") * strcmp(opcode,"RESW") == 0) {
				continue;
			} else if (initial_address < 0) {
				initial_address = LOCCTR;
			}

			// objectLine이 비어져있다면, 새로운 T 레코드 준비.
			if (objectLine[0] == '\0') {
				fprintf(obj,"T%06X",LOCCTR);
			}

			// 윗부분에서 열심히 변경한 objectcode의 문자열을 objectLine에 복사해줌.
			for (i = 0; i < strlen(objectStr); i++) {
				objectLine[objectcode_length++] = objectStr[i];
			}

			// 만약 X Register를 포함하면 쉼표를 붙여줌.
			if (exerand[0] != '\0') {
				strcat(operand,",");
				strcat(operand,exerand);
			}

			fprintf(lst,"%d\t%04X\t%s\t%s\t%s\t%s\n",ROWCTR,LOCCTR,label,opcode,operand,objectStr);
		}	
	}

	// 임시 리스트 파일과 오브젝트 파일 종료.
	fclose(lst);
	fclose(obj);

	// 만약 에러가 없다면, 지금까지 저장해놓은 Symbol Table을
	// 'Symbol' 명령어를 통해 볼 수 있도록 헤더 부분을 변경해줌.
	if (isError == 0) {
		// 그 전까지 가지고 있던 Linked-List를 FREE 해줌.
		for (i = 0; i < MAX_HASH_SIZE; i++) {
			if (symbol_list[i] == NULL) break;
		
			current = symbol_list[i];
			while (current != NULL) {
				previous = current;
				current = current->nextNode;
				free(previous);
			}
		}

		// symbol_temp 의 값을 symbol_list 값으로 변경.
		for (i = 0; i < MAX_HASH_SIZE; i++) {
			symbol_list[i] = symbol_temp[i];
			symbol_temp[i] = NULL;
		}

		// 총 symbol의 갯수가 몇개인지 카운트 하여 symbol_count에 저장.
		for (i = symbol_count = 0; i < MAX_HASH_SIZE; i++) {
			current = symbol_list[i];

			while (current != NULL) {
				symbol_count++;
				current = current->nextNode;
			}
		}
	}

	// 에러 여부를 가지고 리턴.
	return isError;

}

//-----------------------------------------------//
// 함수이름: assemble_run
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: assemble() 함수를 호출하며, 리턴값을
//    	     통해 오류가 있는지 보고, 적절한 후처리 수행.
//-----------------------------------------------//
void assemble_run (void) {

	char filename_list[MAX];
	char filename_obj[MAX];
	int i, assembleReturn;

	// 입력 받은 파일 이름과 같은 .lst와 .obj를 만들기 위한 문자열 작업.
	for (i = 0; i < strlen(parameter[1]); i++) {
		if (parameter[1][i] == '.') break;
		filename_list[i] = filename_obj[i] = parameter[1][i];
	}
	filename_list[i] = filename_obj[i] = '\0';

	// 확장자를 붙여줘서 파일을 만들 준비를 함.
	strcat(filename_list,".lst");
	strcat(filename_obj, ".obj");

	// assemble() 함수의 리턴값이 0 으로, 오류가 없다면.
	if ((assembleReturn = assemble()) == 0) {
		// 어셈블이 완료 되었다는 문구를 출력해줌.
		printf("\toutput files: [%s], [%s]\n",filename_list,filename_obj);

		// .lst 파일과 .obj 파일을 쓰기모드로 만들어냄.
		FILE *lst = fopen(filename_list,"w");
		FILE *obj = fopen(filename_obj,"w");

		// 지금까지 임시로 작성 해 놓은 temp파일을 읽어들임.
		FILE *inter_lst = fopen("temp.lst","r");
		FILE *inter_obj = fopen("temp.obj","r");

		char inputLine[MAX];

		// 각각의 파일들을 그대로 복사시켜 .lst와 .obj를 만들어냄.
		//
		// 동일한 파일 이름을 가지는 .asm파일을 오류가 있을 때와 없을 때,
		// 어느 경우에 assemble을 하더라도, 항상 assemble이 성공한 경우의
		// .lst와 .obj 파일을 가지고 있기 위해 임시 파일을 사용함.
		while (fgets(inputLine,MAX,inter_lst) != NULL) {
			fprintf(lst,"%s",inputLine);
		} fclose(lst); fclose(inter_lst);

		while (fgets(inputLine,MAX,inter_obj) != NULL) {
			fprintf(obj,"%s",inputLine);
		} fclose(obj); fclose(inter_obj);
	// 만약 return값이 -2이면, history에 저장하지 않음. isError = 1
	} else if (assembleReturn == -2) { isError = 1; }

	// 사용 된 intermediate 파일과 임시 파일을 삭제.
	remove("intermediate");
	remove("temp.lst");
	remove("temp.obj");

}

//-----------------------------------------------//
// 함수이름: disassemble
// 리 턴 값: 오류가 잆으면 -1, 없으면 0
// 파라미터: 없음
// 목    표: .obj 파일을 읽어들여 디어셈블을 수행.
//-----------------------------------------------//

int disassemble (void) {

	// 파일이 읽어들여지지 않았다면 오류.
	if (parameter_count != 2) {
		ERROR(ERR_PAR);
		return -2;
	}

	int i, j;

	// 확장자가 .obj가 아니면 에러
	char *extension;
	extension = parameter[1] + strlen(parameter[1]) - 4;
	if (strcmp(extension,".obj") != 0) {
		printf("ERROR: Input File is not [.obj] file.\n");
		return -2;
	}

	// obj 파일을 읽어들임.
	FILE *obj = fopen(parameter[1],"r");
	if (obj == NULL) {
		printf("ERROR: There isn't such a file.\n");
		return -2;
	}

	// 변수 선언 및 초기화
	char inputLine[MAX];
	char inputOp[MAX];

	// 디어셈블을 위한 intermediate파일.
	FILE *dlt = fopen("intermediate_dlt","w");
	char label[MAX];
	char opcode[MAX];
	char operand[MAX];

	char *opcode_finder = NULL;
	char program_name[MAX];

	int opcode_num;
	int operand_num;
	int LOCCTR, initial_address = -1;

	int count;

	// 한 줄씩 읽어들임.
	while (fgets(inputLine,MAX,obj) != NULL) {

		//'H' 레코드일 경우,
		if (inputLine[0] == 'H') {
			// opcode부분은 'START'가 됨.
			strcpy(opcode,"START");

			// 프로그램 이름을 나타내는 label 입력.
			strncpy(label,inputLine+1,6);
			label[6] = '\0';

			// operand 에는 프로그램의 시작 주소값을 입력. 
			strncpy(operand,inputLine+7,6);
			operand[6] = '\0';

			sscanf(operand,"%x",&operand_num);
			sprintf(operand,"%04X",operand_num);

			strcpy(program_name,label);

			// LOCCTR 을 프로그램의 시작 주소값으로 설정
			LOCCTR = operand_num;
			fprintf(dlt,"%04X\t%s\t%s\t%s\n",LOCCTR,label,opcode,operand);
		}else if (inputLine[0] == 'E') {
			// 'E' 레코드일 경우, 프로그램 이름을 출력하며 종료
			fprintf(dlt,"\t\t%s\t%04X\n","END",initial_address);
		}else{

			// 변수 초기화.
			count = 0;
			for (i = 9; i < strlen(inputLine); i++) inputOp[i] = 0;
			for (i = 9; i < strlen(inputLine); ) {
				// 일렬로 나올 된 object program에서 어디 부분이 opcode인지를 확인.
				opcode[0] = inputLine[i];
				opcode[1] = inputLine[i+1];
				opcode[2] = '\0';

				sscanf(opcode,"%x",&opcode_num);
				
				// opcode_finder를 통하여 mnemonic이 있는지를 mnemonic table에서 확인.
				opcode_finder = hash_find_opcode(&mnemonic_list[hash_function(OpToMne,NULL,opcode_num)],opcode_num);
				if (opcode_finder != NULL && strlen(inputLine) - 1 >= i + 6) {
					// opcode가 존재하고, 3 Bytes가 보장 되는 부분일 경우.
					if (inputOp[i-1] == 3) {
						inputOp[i-1] = 4;
					}

					// inputOp 문자열의 부분을 '1'로 변경하여, Opcode라는 것을 표시.
					inputOp[i] = 1;
					inputOp[i+1] = 1;

					// i 값에 6을 증가시켜, 6 글자 넘어감.
					i = i + 6;
					count = 0;
				} else {
					// opcode가 존재하지 않는다면, 이를 Hexadecimal로 인식.
					if (count == 0) {
						// 첫번째로 나온 Hexadecimal이면 '2'를 넣어서, 이 부분부터 Hexadecimal임을 알림.
						inputOp[i] = 2;
						inputOp[i+1] = 3;
					} else {
						// 일반적인 경우, inputOp에 3을 넣어서 이 부분은 Hexadecimal임을 알림.
						inputOp[i] = 3;
						inputOp[i+1] = 3;
					}

					// Hexadecimal이 3 Byte를 넘어갈 경우, 잘라줌.
					if (++count == 3) {
						// inputOp를 4로 설정하여 끊어야 함을 표시.
						inputOp[i+1] = 4;
						count = 0;
					}
					// i 값에 2를 증가 시켜, 2 글자 넘어감.
					i = i + 2;
				}
			}

			// 이번 T 레코드의 시작위치를 label에 저장.
			strncpy(label,inputLine+1,6);
			label[6] = '\0';

			// label에 저장 된 시작위치를 integer로 변경하여 LOCCTR에 저장.
			sscanf(label,"%x",&LOCCTR);
			
			if (initial_address < 0) initial_address = LOCCTR;

			for (i = 9; i < strlen(inputLine) - 1; ) {

				// 위에서 만든 inputOp를 이용하여, 이 부분이 어떤 부분이지를 확인.
				if (inputOp[i] == 1) {
					// 이 부분이 Opcode가 가능한 부분일 경우,
					opcode[0] = inputLine[i];
					opcode[1] = inputLine[i+1];
					opcode[2] = '\0';

					sscanf(opcode,"%x",&opcode_num);

					// opcode mnemonic을 찾아서 opcode_finder에 저장.
					opcode_finder = hash_find_opcode(&mnemonic_list[hash_function(OpToMne,NULL,opcode_num)],opcode_num);

					// 뒷부분 4자리를 입력받아 operand로 변환.
					strncpy(operand,inputLine+i+2,4);
					operand[4] = '\0';

					sscanf(operand,"%x",&operand_num);

					// 만약 9번째 bit가 1일경우, X Register를 사용하므로, 출력.
					if (operand_num >= 0x8000) {
						sprintf(operand,"%04X,X",operand_num - 0x8000);
					} else {
						sprintf(operand,"%04X",operand_num);
					}

					// RSUB 는 operand가 필요 없으므로 생략.
					if (opcode_num == 0x4C) operand[0] = '\0';

					fprintf(dlt,"%04X\t\t%s\t%s\t%02X%04X\n",LOCCTR,opcode_finder,operand,opcode_num,operand_num);

					// i에 6을 증가시켜, 6글자를 넘어감.
					// LOCCTR에는 3을 증가시켜, 주소값 반영.
					i = i + 6;
					LOCCTR += 3;
				} else {
					// Hexadecimal일 경우, inputOp가 4가 나올 때까지 찾음. (4가 Hexadecimal의 끝을 나타냄)
					for (j = i, count = 0; j < strlen(inputLine); j++) {
						count++;
						if (inputOp[j] == 4) break;
					}

					// Mnemonic은 'BYTE'가 됨.
					strcpy(opcode,"BYTE");
					strncpy(operand,inputLine+i,count);
					operand[count] = '\0';

					// operand에 Hexadecimal 값을 저장.
					sscanf(operand,"%x",&operand_num);
					
					sprintf(operand,"X'%02X'",operand_num);
					fprintf(dlt,"%04X\t\t%s\t%s\t%02X\n",LOCCTR,opcode,operand,operand_num);

					// i를 count만큼 증가시켜, 그만큼 건너뜀.
					i = i + count;
					// 두글자당, 1 byte이므로 2로 나누어 LOCCTR에 저장.
					LOCCTR += count/2;
				}				
			}
		}
	}

	// intermediate 파일 종료.
	fclose(dlt);

	return 0;

}

//-----------------------------------------------//
// 함수이름: disassemble_run
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: disaassemble()을 호출하고 오류가 있는지
// 	     없는지에 따라 적절한 후처리 수행.
//-----------------------------------------------//
void disassemble_run (void) {

	char filename_dlt[MAX];
	int i, disassembleReturn;

	// .obj 와 동일한 이름의 .dlt 파일 이름을 만듬.
	for (i = 0; i < strlen(parameter[1]); i++) {
		if (parameter[1][i] == '.') break;
		filename_dlt[i] = parameter[1][i];
	}
	filename_dlt[i] = '\0';
	strcat(filename_dlt, ".dlt");

	// disassemble() 을 호출하고 오류가 없다는 뜻의 0이 리턴되면,
	if ((disassembleReturn = disassemble()) == 0) {
		// 디스어셈블이 완료 되었다는 문구를 출력
		printf("\toutput files: [%s]\n",filename_dlt);

		// .dlt파일을 쓰기 모드로 열고, intermediate_dlt를 읽기모드로 열음.
		FILE *dlt = fopen(filename_dlt,"w");
		FILE *inter_dlt = fopen("intermediate_dlt","r");
		char inputLine[MAX];

		// intermediate_dlt 에 있는 내용을 그대로 .dlt로 복사.
		//
		// 항상 옳바른 디어셈블이 된 .dlt 파일을 보존해야하므로,
		// 오류가 없다는 것이 확인 되었을 때, .dlt 파일을 생성.
		while (fgets(inputLine,MAX,inter_dlt) != NULL) {
			fprintf(dlt,"%s",inputLine);
		} fclose(dlt); fclose(inter_dlt);	
	// 만약 return값이 -2이면, history에 저장하지 않음. isError = 1
	} else if (disassembleReturn == -2) { isError = 1; }

	// intermediate_dlt를 삭제
	remove("intermediate_dlt");

	return;

}

//-----------------------------------------------//
// 함수이름: hash_find
// 리 턴 값: 해당하는 mnemonic의 opcode값, 없으면 -1
// 파라미터: hash table을 나타내는 포인터 head, mnemonic 문자열.
// 목    표: mnemonic 을 읽어들여 해당하는 opcode를 리턴.
//-----------------------------------------------//
int hash_find (hash_node **head, char opcode[]) {

	hash_node *current;

	// linked-list를 따라가며 동일한 mnemonic이 있는지 검사.
	current = *head;
	while (current != NULL) {
		if (strcmp(current->mnemonic,opcode) == 0) {
			return current->opcode;
		}
		current = current->nextNode;
	}

	// 없으면 -1을 리턴
	return -1;
}

//-----------------------------------------------//
// 함수이름: hash_find_opcode
// 리 턴 값: char 포인터
// 파라미터: hash table을 나타내는 포인터 head, opcode 정수.
// 목    표: opcode 값을 읽어들여 해당하는 mnemonic을 리턴.
//-----------------------------------------------//
char* hash_find_opcode (hash_node **head, int opcode) {

	hash_node *current;

	// linked-list를 탐색하며 해당하는 mnemonic을 반환.
	current = *head;
	while (current != NULL) {
		if (current->opcode == opcode) {
			return current->mnemonic;
		}
		current = current->nextNode;
	}

	// 없으면 NULL을 리턴
	return NULL;

}

//-----------------------------------------------//
// 함수이름: symtab_list
// 리 턴 값: 없음
// 파라미터: 없음
// 목    표: 'symbol' 명령어를 통해 symbol table 출력
//-----------------------------------------------//
void symtab_list (void) {

	// symbol_count가 0 이면, symbol이 없는 것이므로 오류 출력
	if (symbol_count == 0) {
		printf("ERROR: There is no Symbol Table.\n");
		isError = 1;
		return;
	}

	if  (parameter_count != 1) {
		ERROR(ERR_PAR);
		isError = 1;
		return;
	}

	// 변수 선언
	int i, count;
	hash_node *current;

	// 내림차순으로 저장한 배열
	symtab table[symbol_count];

	// symbol table을 탐색하며 값들을 배열에 저장함.
	for (i = count = 0; i < MAX_HASH_SIZE; i++) {
		if (symbol_list[i] == NULL) continue;
		else current = symbol_list[i];

		while (current != NULL) {
			strcpy(table[count].label,current->mnemonic);
			table[count].address = current->opcode;
			current = current->nextNode;
			count++;
		}	
	}

	// qsort를 이용하여 배열을 mnemonic에 대해서 내림차순 정렬.
	qsort(table,symbol_count,sizeof(symtab),symtab_comp);

	// 하나씩 출력함.
	for (i = 0; i < symbol_count; i++) {

		printf("\t%s\t%4X\n",table[i].label,table[i].address);

	}	

	return;
}

//-----------------------------------------------//
// 함수이름: symtab_comp
// 리 턴 값: strcmp의 결과값
// 파라미터: 비교할 대상 a와 b
// 목    표: qsort를 사용하기 위한 compare 함수
//-----------------------------------------------//
int symtab_comp (const void *a, const void *b) {

	symtab *sa = (symtab *)a;
	symtab *sb = (symtab *)b;

	// 각각의 symbol 이 가진 mnemonic에 대하여 strcmp 값을 리턴
	return strcmp(sb->label,sa->label);

}
