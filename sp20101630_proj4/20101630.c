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
	char separator[] = " \t\n/";
	char *token;

	char mnemonic[MAX_OPCODE];
	int  opcode;
	int format;

	// 파일로부터 한줄씩 입력 받음.
	while (fgets(inputline,MAX,fp) != NULL) {
		count = 0;

		// separator을 통해 tokenizing을 한다.
		token = strtok(inputline,separator);
		while (token != NULL) {
			// 나머지 Token은 필요 없으므로 배제.
			if (count == 3) {
				count = 0;
				break;
			}
			if (count == 2) {
				count++;
				sscanf(token,"%d",&format);
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
		hash_add(&opcode_list[count],mnemonic,opcode,format);	

		// hash_function을 통해 index를 구함
		// OpToMne를 파라미터로 넘겨주어 Mnemonic Table의 hash_function 계산.
		count = hash_function(OpToMne,NULL,opcode);

		// 해당 Index에 Linked-List 에 저장.
		hash_add(&mnemonic_list[count],mnemonic,opcode,format);
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
			printf("[%s,%02X,%d]",current->mnemonic,current->opcode,current->format);
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
// 리 턴 값: 추가 된 node의 포인터
// 파라미터: hash_function에서 받은 인덱스 head.
//	     입력 될 mnemonic 과 opcode.
// 목    표: Linked-List를 만들고 추가한다.
//-----------------------------------------------//
hash_node* hash_add (hash_node **head, char *mnemonic, int opcode, int format) {

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
	current->format = format;
	current->nextNode = NULL;

	if (*head == NULL) *head = current;
	else previous->nextNode = current;

	return current;
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
// 함수이름: assemble_subrun
// 리 턴 값: isError. 에러가 있으면 음수.
// 파라미터: assemble_run에서 받아온 asm소스.
//	     **source와 전체 길이 sourceLine, 그리고
//	     줄값 ROWCTR을 Call by reference로 받음.
// 목    표: assemble을 할 소스를 가지고 실제로
//	     2 Pass 알고리즘을 통해 assemble을 수행함.
//	     Assemble Listing과 Object File을 생성.
//-----------------------------------------------//
int assemble_subrun (char **source, int sourceLine, int *ROWCTR) {

	// 필요한 변수 설정
	int i, j, count, check, line, MODE, isError;
	int start_address, origin_address, initial_address, program_length;

	int *LOCCTR, LOCCUR;
	char **LOCMNE;
	int LOCIDX, LOCMNECNT;

	char *token;
	int token_counter;
	char separator[] = " \t\n\r";

	char preserve[MAX];
	char label[MAX];
	char opcode[MAX];
	char operand[MAX];
	char token_operand[MAX];

	// Intermediate 파일 생성 및 Table Pointer 선언
	hash_node *hash_cursor;
	FILE *intermediate = fopen("intermediate","a");
	hash_node *current = NULL, *previous = NULL;

	// Assemble을 위해 필요한 테이블인
	// LITTAB, SYMTAB, EXTREF, EXTDEF, MODIFICATION 초기화
	isError = 0;
	literal_count = 0;
	for (i = 0; i < MAX_HASH_SIZE; i++) {
		current = symbol_temp[i];
		while (current != NULL) {
			previous = current;
			current = current->nextNode;
			free(previous);
		}
		symbol_temp[i] = NULL;
	}
	
	// LITTA 초기화
	current = littab_list;
	while (current != NULL) {
		previous = current;
		current = current->nextNode;
		free(previous);
	}
	littab_list = NULL;

	// EXTREF TABLE 초기화
	current = extref_list;
	while (current != NULL) {
		previous = current;
		current = current->nextNode;
		free(previous);
	}
	extref_list = NULL;

	// EXTDEF TABLE 초기화
	current = extdef_list;
	while (current != NULL) {
		previous = current;
		current = current->nextNode;
		free(previous);
	}
	extdef_list = NULL;

	// MODIFICATION TABLE 초기화
	current = modification_list;
	while (current != NULL) {
		previous = current;
		current = current->nextNode;
		free(previous);
	}
	modification_list = NULL;

	// BLOCK을 대비하여, LOCCTR을 배열로 구성.
	// INDEX는 BLOCK의 번호로 지정.
	LOCCTR = (int *)malloc(sizeof(int));

	LOCMNE = (char **)malloc(sizeof(char *));
	LOCMNE[0] = (char *)malloc(sizeof(char)*MAX);
	LOCMNE[0][0] = '\0';

	// 반복문에 사용 될 변수들 초기값 설정
	program_name[0] = '\0';
	LOCIDX = 0; LOCMNECNT = 1;
	LOCCTR[LOCIDX] = 0; MODE = 0;

	// for문을 통해 첫번째 줄부터 처리
	for (line = 0; line <= sourceLine; line++) {
		*ROWCTR += 5;

		// tokenizing 전에 한 줄을 보존
		strcpy(preserve,source[line]);
		label[0] = opcode[0] = operand[0] = '\0';

		// operand 에서 C'  '안에 존재하는 문구 중, separator에 있는 것을 임시로 치환.
		for (i = j = 0; i < strlen(source[line]); i++) {
			if (source[line][i] == '\'') j++;
			else if (j == 1) {
				switch (source[line][i]) {
					case  ',': source[line][i] = sptr_comma;	break;
					case  ' ': source[line][i] = sptr_space;	break;
					case '\t': source[line][i] = sptr_tab;	break;
				}
			}
		}

		// tokenizing 시작
		token_counter = check = 0;
		token = strtok(source[line],separator);
		while (token != NULL) {
			// check를 통해 Token이 Opcode인지, Label인지 확인.
			// token_counter를 통해 몇번째 Token인지를 확인.
			check = 0; token_counter++;

			// Token의 첫번째 값이 '.'이라면 주석이므로 끝.
			if (token[0] == '.') break;

			// Opcode Tab을 통해 Opcode인지 확인하여 check를 1로 변경.
			if (((token[0] == '+' && hash_find(&opcode_list[hash_function(MneToOp,token+1,0)],token+1) >= 0) ||
			     (hash_find(&opcode_list[hash_function(MneToOp,token,0)],token) >= 0)) && token_counter == 1) {
				check  = 1;
			// Opcode가 아니면, Assemble Directives인지를 확인.
			} else if (token_counter == 1) {
				for (i = 0; i < ASM_INS; i++) {
					if (strcmp(token,assemble_instruction[i]) == 0) {
						check = 1;
					}
				}
			}

			// 첫번째로 나온 Token이 Opcode와 Assemble Directives라면 Label은 없음.
			if (check == 1) {
				strcpy(label,"####");
				token_counter++;
			}

			// token_counter에 따라, label이나 opcode로 저장.
			switch (token_counter) {
				case 1: strcpy(label,token);	break;
				case 2: strcpy(opcode,token);	break;
			}

			// OPERAND를 입력 받을 차례라면, 맨 뒷줄까지 모두 토크나이징.
			if (token_counter < 2) {
				token = strtok(NULL,separator);
			} else {
				token = strtok(NULL,"\n\r");
				break;
			}
		} 	

		// operand를 입력 받음. 공백(' ')과 탭('\t')을 제거함. (TRIM 작업)
		check = 0;
		if (token_counter == 2 && token != NULL) {
			for (i = j = 0; *(token+i) != '\0'; i++) {
				if (*(token+i) == '\'') check++;
				// 주석문이 나온다면 BREAK
				if (*(token+i) == '.' && check != 1) break;
				if (*(token+i) != ' ' && *(token+i) != '\t') {
					operand[j++] = *(token+i);
				}
			}
			operand[j] = '\0';
		}

		// 치환되었던 부분을 다시 복원.
		for (i = j = 0; i < strlen(operand);  i++) {
			if (operand[i] == '\'') j++;
			else if (j == 1) {
				switch (operand[i]) {
				case sptr_comma: operand[i] = ','; break;
				case sptr_space: operand[i] = ' '; break;
				case sptr_tab:   operand[i] = '\t'; break;
				}
			}
		}

		// 만약 해당 줄에 opcode가 없다면, 주석이거나 빈 줄이므로 continue
		if (opcode[0] == '\0' && line != sourceLine) {
			continue;
		}

		// 토크나이징 끝

		// 현재의 주소값을 나타내는 LOCCUR에 현재 BLOCK의 주소값 저장
		LOCCUR = LOCCTR[LOCIDX];

		// OPCODE가 START거나 CSECT일 때.
		if (strcmp(opcode,"START") == 0 || strcmp(opcode,"CSECT") == 0) {
			// MODE 변수에 현재가 어떤 소스인지를 저장함.
			if (strcmp(opcode,"START") == 0) {
				MODE = START_MODE;	
				sscanf(operand,"%d",&start_address);
			} else {
			  	MODE = CSECT_MODE;
				start_address = 0;
			}

			// 프로그램의 이름이 정해지고, 시작주소가 정해진다.
			strcpy(program_name,label);
			LOCCTR[LOCIDX] = start_address;

			fprintf(intermediate,"%4d\t%04X\t%d\t%s\t%s\t%s\n",*ROWCTR,LOCCUR,LOCIDX,label,opcode,operand);
			continue;
		// OPCODE가 END거나, LTORG거나, 마지막줄을 스캔했을 경우.
		} else if (strcmp(opcode,"END") == 0 || strcmp(opcode,"LTORG") == 0 || line == sourceLine) {
			if (strcmp(opcode,"END") == 0 || strcmp(opcode,"LTORG") == 0)
				fprintf(intermediate,"%4d\t-1\t-1\t####\t%s\t%s\n",*ROWCTR,opcode,operand);
					
			// LITERAL TABLE을 탐색하며 LITERAL POOL을 생성
			current = littab_list;
			while (current != NULL) {
				// LITERAL POOL에 저장되지 않은 경우
				if (current->opcode < 0) {
					// INTERMEDIATE 파일에 저장
					fprintf(intermediate,"-1\t%04X\t%d\t####\t%s\t####\n",LOCCUR,LOCIDX,current->mnemonic);
					current->opcode = LOCCUR;
					current->format = LOCIDX;
					
					// LITERAL POOL이 생성되면서, 주소값을 계산해줌.
					// C 일 경우,
					if (current->mnemonic[1] == 'C') {
						for (i = 3, count = 0; i < strlen(current->mnemonic); i++) {
							if (current->mnemonic[i] == '\'') break;
							else count++;
						}
						LOCCTR[LOCIDX] += count;
					// X 일 경우,
					} else if (current->mnemonic[1] == 'X') {
						for (i = 3, count = 0; i < strlen(current->mnemonic); i++) {
							if (current->mnemonic[i] == '\'') break;
							else count++;
						}
						LOCCTR[LOCIDX] += count/2;
					} else {
						LOCCTR[LOCIDX]  += 3;
					}
					LOCCUR = LOCCTR[LOCIDX];
				}
				current = current->nextNode;
			}
			continue;
		// 일반적인 OPCODE가 나왔을 경우.
		} else {
			// LABEL이 존재할 경우, (즉, LABEL이 '####'이 아닐경우)
			if (strcmp(label,"####") != 0) {
				hash_cursor = hash_pointer(&symbol_temp[hash_function(MneToOp,label,0)],label,0);
				// 이미 존재하는 심볼일 경우, Duplicate 에러 출력
				if (hash_cursor != NULL) {
					printf("ERROR: Line %d: Label '%s' was already existed.\n",*ROWCTR,label);
					isError = -1;
				} else {
					// 만약 EQU라면 EQU_TABLE과 SYMBOL_TABLE에 모두 저장.
					if (strcmp(opcode,"EQU") == 0) {
						// EXPRESSION의 계산을 위해 EXPRESSION_CALCULATOR 함수 호출
						LOCCUR = expression_calculator(label,opcode,operand,ROWCTR,LOCCTR[LOCIDX],NULL,&isError);
					}
					
					// 현재 주소를 나타내는 LOCCUR값으로 SYMBOL을 저장. BLOCK NUMBER도 같이 저장.
					current = hash_pointer(&equ_list,label,0);
					if (current != NULL && current->format == 0) {
						hash_add(&symbol_temp[hash_function(MneToOp,label,0)],label,LOCCUR,0);
					} else {
						hash_add(&symbol_temp[hash_function(MneToOp,label,0)],label,LOCCUR,LOCIDX);
					}

					// EXTDEF에 존재하는 SYMBOL이라면 이 값을 저장해줌.
					current = extdef_list;
					while (current != NULL) {
						if (strcmp(current->mnemonic,label) == 0) {
							current->opcode = LOCCTR[LOCIDX];
							current->format = LOCIDX;
							break;
						}
						current = current->nextNode;
					}
				}
			}

			// LITERAL이 존재한다면 LITERAL TABLE에 저장.
			if (operand[0] == '=') {
				// '=*' 이라면 주소값을 붙혀서 LITTAB에 저장. (나올때마다 다른 값이어야 하므로)
				if (operand[1] == '*') {
					sprintf(operand,"=*%04X",LOCCUR);
				}
				// 중복 되는 LITERAL이 없다면, LITTAB에 저장
				if ((hash_cursor = hash_pointer(&littab_list,operand,0)) == NULL) {
					hash_add(&littab_list,operand,-1,-1);
					literal_count++;
				}
			}

			// OPCODE 부분에 들어있는 MNEMONIC의 정보를 가진 OPCODE TABLE의 포인터를 구함.
			if (opcode[0] == '+') {
				hash_cursor = hash_pointer(&opcode_list[hash_function(MneToOp,opcode+1,0)],opcode+1,0);
			} else {
				hash_cursor = hash_pointer(&opcode_list[hash_function(MneToOp,opcode,0)],opcode,0);
			}

			// 포인터가 존재하지 않는다면, check = 1로 assign.
			if (hash_cursor == NULL) check = 1; else check = 0;

			// check가 1이라면, Assemble Directives인지 검사. 
			if (check == 1) {
				// BYTE라면 'C'일때와 'X'일때로 나누어 주소값 계산
				if (strcmp(opcode,"BYTE") == 0) {
					if (operand[0] == 'C') {
						for (i = 2, count = 0; i < strlen(operand); i++) {
							if (operand[i] == '\'') break;
							count++;
						}
					} else if (operand[0] == 'X') {
						for (i = 2, count = 0; i < strlen(operand); i++) {
							if (operand[i] == '\'') break;
							if (operand[i] >= 'a' && operand[i] <= 'z') {
								operand[i] -= 32;
							}
							count++;
						}
						count /= 2;
					}
					LOCCTR[LOCIDX] += count;
				// RESB, RESW, WORD는 주어진 만큼 주소값 계산
				} else if (strcmp(opcode,"WORD") == 0) {	
					LOCCTR[LOCIDX] += 3;
				} else if (strcmp(opcode,"RESB") == 0) {
					sscanf(operand,"%d",&count);
					LOCCTR[LOCIDX] += count;
				} else if (strcmp(opcode,"RESW") == 0) {
					sscanf(operand,"%d",&count);
					LOCCTR[LOCIDX] += count * 3;
				// ORG가 나왔을 경우는 두가지로 나뉜다.
				} else if (strcmp(opcode,"ORG") == 0) {
					// OPERAND가 없다면, 기존의 ADDRESS로 복원
					if (operand[0] == '\0') {
						LOCCTR[LOCIDX] = origin_address;
					} else {
					// OPERAND에 EXPRESSION이 있다면,
					// EXPRESSION_CALCULATOR을 통해 그 값을 계산.
					// 그리고 기존의 ADDRESS를 ORIGIN_ADDRESS 에 저장.
						origin_address = LOCCTR[LOCIDX];
						LOCCTR[LOCIDX] = expression_calculator(label,opcode,operand,ROWCTR,LOCCTR[LOCIDX],NULL,&isError);
					}
				// EQU는 아무 행동도 취하지 않음. 라벨 처리때 함께 처리됨.
				} else if (strcmp(opcode,"EQU") == 0) {

				// USE를 사용한다면, 해당 BLOCK의 이름과 함께 INDEX, LOCATION을 저장
				} else if (strcmp(opcode,"USE") == 0) {
					// OPERAND가 없다면, 0번 INDEX로 변경
					if (operand[0] == '\0') LOCIDX = 0;
					else {
						// OPERAND가 있다면, 그 이름을 사용하는 BLOCK으로 INDEX 변경
						for (i = count = 0; i < LOCMNECNT; i++) {
							if (strcmp(LOCMNE[i],operand) == 0) {
								LOCIDX = i;
								count = 1;	
							}
						}

						// 만약 새로 나온 BLOCK이라면, 새로 추가하여 LOCATION을 0 으로 둠.
						if (count == 0) {
							LOCMNECNT++;
							LOCMNE = (char **)realloc(LOCMNE,sizeof(char *)*(LOCMNECNT));
							LOCMNE[LOCMNECNT - 1] = (char *)malloc(sizeof(char)*MAX);
							strcpy(LOCMNE[LOCMNECNT - 1],operand);

							LOCCTR = (int *)realloc(LOCCTR,sizeof(int)*(LOCMNECNT));
							LOCCTR[LOCMNECNT - 1] = 0;

							// 새로 추가된 INDEX로 변경
							LOCIDX = LOCMNECNT - 1;
						}
					}
					LOCCUR = LOCCTR[LOCIDX];
				// EXTDEF라면, OPERAND에 주어진 라벨들을 EXTDEF_LIST에 저장
				} else if (strcmp(opcode,"EXTDEF") == 0) {
					LOCCUR = -1;
					
					strcpy(token_operand,operand);
					token = strtok(token_operand," ,");
					while (token != NULL) {
						hash_add(&extdef_list,token,0,0);
						token = strtok(NULL," ,");
					}
				// EXTREF라면, OPERAND에 주어진 라벨들을 LOCATION '0'으로 저장
				} else if (strcmp(opcode,"EXTREF") == 0) {
					LOCCUR = -1;
					
					strcpy(token_operand,operand);
					token = strtok(token_operand," ,");
					while (token != NULL) {
						// SYMTAB 과 EXTREF_LIST에 저장.
						hash_add(&symbol_temp[hash_function(MneToOp,token,0)],token,0,0);
						hash_add(&extref_list,token,0,0);
						token = strtok(NULL," ,");
					}
				// BASE, NOBASE는 패스1 에서 아무것도 하지 않음.
				} else if (strcmp(opcode,"BASE") == 0) {
				} else if (strcmp(opcode,"NOBASE") == 0) {
					LOCCUR = -1;
				} else {
				// 위 모든 경우에 해당되지 않는다면 INVALID OPCODE.
					printf("ERROR: Invalid Opcode [%s].\n",opcode);
					isError = -1;
					continue;
				}
			} else {
				// 일반적인 OPCODE라면, 각 OPCODE에 해당하는 포맷만큼 주소값 증가.
				if (hash_cursor->format == 3) {
					if (opcode[0] == '+') {
						LOCCTR[LOCIDX] += 0x4;
					} else {
						LOCCTR[LOCIDX] += 0x3;
					}
				} else if (hash_cursor->format == 2) {
					LOCCTR[LOCIDX] += 0x2;
				} else if (hash_cursor->format == 1) {
					LOCCTR[LOCIDX] += 0x1;
				}
			}
		}
		// ROW와 LOC를 출력. LOCCUR이 -1이라면 LOCATION이 없는 줄
		fprintf(intermediate,"%4d\t",*ROWCTR);
		if (LOCCUR >= 0)
			fprintf(intermediate,"%04X\t",LOCCUR);
		else
			fprintf(intermediate,"-1\t");
		
		// EQU의 값이 RELATIVE는 BLOCK에 영향을 받고,
		// EQU의 값이 ABSOLUTE는 BLOCK에 영향을 받지 않음.
		// 또한, 이를 통해 BLOCK의 영향을 안 받으면 -1.
		if (strcmp("EQU",opcode) == 0) {
			hash_cursor = hash_pointer(&equ_list,label,0);
			if (hash_cursor != NULL && hash_cursor->format != 0) {
				fprintf(intermediate,"%d\t",LOCIDX);
			} else {
				fprintf(intermediate,"-1\t");
			}
		} else {
			if (LOCCUR >= 0) 
				fprintf(intermediate,"%d\t",LOCIDX);
			else
				fprintf(intermediate,"-1\t");
		}
		// LABEL이 없다면, '####'를 출력
		if (strcmp(label,"####") != 0)
			fprintf(intermediate,"%s\t",label);
		else
			fprintf(intermediate,"####\t");
		// OPCODE와 OPERAND를 출력
		fprintf(intermediate,"%s\t",opcode);
		fprintf(intermediate,"%s\n",operand);
	}
	fclose(intermediate);

	// BLOCK의 시작주소와 그 길이를 통해 주소값 계산
	program_length = LOCCTR[LOCMNECNT - 1];
	for (i = LOCMNECNT - 1; i >= 0; i--) {
		free(LOCMNE[i]);
		
		LOCCTR[i] = 0;
		if (i != 0) {
			// 앞서 있는 BLOCK들의 길이를 모두 더함.
			for (j = 0; j < i; j++) {
				LOCCTR[i] += LOCCTR[j];
			}
		}
	}
	// PROGRAM_LENGTH는 마지막 BLOCK의 시작주소 + 마지막 BLOCK의 길이
	program_length += LOCCTR[LOCMNECNT - 1];
	free(LOCMNE);

	// PASS 2 알고리즘
	FILE *lst = fopen("temp.lst","a");
	FILE *obj = fopen("temp.obj","a");

	intermediate = fopen("intermediate","r");

	// 변수 설정
	char inputLine[MAX], objectcode_str[MAX], objectLine[MAX];
	int ROW, LOC, BLK, Base;
	unsigned int objectcode;

	// 변수 초기화
	Base = initial_address = -1; objectLine[0] = '\0';
	while (1) {
		// 매번 초기화 되어야 할 변수값 설정
		ROW = LOC = BLK = 0;
		opcode[0] = label[0] = operand[0] = objectcode_str[0] = '\0';

		// INTERMEDIATE 파일을 한 줄씩 읽어들여 INPUTLINE에 저장
		if (fgets(inputLine,MAX,intermediate) != NULL) {
			strcpy(preserve,inputLine);
	
			// operand 에서 C'  '안에 존재하는 문구 중, separator에 있는 것을 임시로 치환.
			for (i = j = 0; i < strlen(inputLine); i++) {
				if (inputLine[i] == '\'') j++;
				else if (j == 1) {
					switch (inputLine[i]) {
						case  ',': inputLine[i] = sptr_comma;	break;
						case  ' ': inputLine[i] = sptr_space;	break;
						case '\t': inputLine[i] = sptr_tab;	break;
					}
				}
			}

			// TOKENIZING을 통해 ROW, LOC, BLK, LABEL, OPCODE, OPERAND 입력.
			token_counter = 0;
			token = strtok(inputLine,"\t\n");
			while (token != NULL) {
				// TOKEN_COUNTER를 통해 어떤 것을 입력 받아야 하는지를 확인
				switch (token_counter++) {
				case 0: sscanf(token,"%d",&ROW); break;
				case 1: sscanf(token,"%x",&LOC); break;
				case 2: sscanf(token,"%d",&BLK); break;
				case 3: strcpy(label,token);	 break;
				case 4: strcpy(opcode,token);	 break;
				case 5: strcpy(operand,token);	 break;
				}
				token = strtok(NULL,"\t\n");
			}
	
			// 치환되었던 부분을 다시 복원.
			for (i = j = 0; i < strlen(operand);  i++) {
				if (operand[i] == '\'') j++;
				else if (j == 1) {
					switch (operand[i]) {
					case sptr_comma: operand[i] = ','; break;
					case sptr_space: operand[i] = ' '; break;
					case sptr_tab:   operand[i] = '\t'; break;
					}
				}
			}
 
		// 더 이상 읽을 줄이 없다면, OPCODE를 END_로 변경.
		} else {
			strcpy(opcode,"END_");
		}

		// OPCODE가 'START'거나 'CSECT'일 때.
		if (strcmp(opcode,"START") == 0 || strcmp(opcode,"CSECT") == 0) {
			for (i = strlen(label); i < 6; i++) {
				label[i] = ' ';
			} label[i] = '\0';
			// ASSEMBLE LISTING과 OBJECT FILE의 H레코드를 작성.
			fprintf(lst,"%4d\t%04X\t",ROW,LOC);
			// BLOCK이 사용 되었다면, BLOCK NUMBER를 출력
			if (LOCMNECNT > 1)
				fprintf(lst,"%d\t",BLK);
			fprintf(lst,"%s\t%s\t%s\t\t%s\n",label,opcode,operand,objectcode_str);
			fprintf(obj,"H%6s%06X%06X\n",label,start_address,program_length);

			// EXTDEF과 EXTREF가 있다면, D레코드와 R레코드를 작성.
			if (extdef_list != NULL) {
				fprintf(obj,"D");

				// EXTDEF TABLE을 탐색하며 주소와 블락 주소를 더하여 출력
				current = extdef_list;
				while (current != NULL) {
					fprintf(obj,"%s",current->mnemonic);
					for (i = strlen(current->mnemonic); i < 6; i++)
						fprintf(obj," ");
					fprintf(obj,"%06X",current->opcode+LOCCTR[current->format]);
					current = current->nextNode;
				}
				fprintf(obj,"\n");
			}

			// EXTREF TABLE을 탐색하며 바로 바로 출력
			if (extref_list != NULL) {
				fprintf(obj,"R");

				current = extref_list;
				while (current != NULL) {
					fprintf(obj,"%s",current->mnemonic);
					for (i = strlen(current->mnemonic); i < 6; i++)
						fprintf(obj," ");
					current = current->nextNode;
				}
				fprintf(obj,"\n");
			}

			// 다시 한 줄을 읽어들임.
			continue;
		// OPCODE가 'END_'일 때. 즉, 더 이상 읽을 줄이 없을 경우.
		} else if (strcmp(opcode,"END_") == 0) {
			// T 레코드를 작성 중이었다면, 길이를 저장하고 끝맺음.
			if (objectLine[0] != '\0') {
				check = (strlen(objectLine) - 9) / 2;
				objectLine[7] = (check / 0x10);
				objectLine[8] = (check % 0x10);
			
				objectLine[7] += (objectLine[7] >= 0xA)?'A'-0xA:'0';
				objectLine[8] += (objectLine[8] >= 0xA)?'A'-0xA:'0';

				fprintf(obj,"%s\n",objectLine);
			}

			// MODIFICATION 레코드를 처리하기 위
			current = modification_list;
			while (current != NULL) {
				// MODIFICATION의 주소와 수정해야할 갯수, 가산(+)하거나 감산(-)해야할 라벨을 출력
				fprintf(obj,"M%06X%02X%s\n",current->opcode,current->format,current->mnemonic);
				current = current->nextNode;
			}

			// END 레코드를 출력.
			fprintf(obj,"E");
			if (MODE == START_MODE) {
				fprintf(obj,"%06X",initial_address);
			}
			fprintf(obj,"\n");

			// 반복문을 빠져나옴.
			break;
		} else if (((opcode[0] == '+' && hash_find(&opcode_list[hash_function(MneToOp,opcode+1,0)],opcode+1) >= 0) ||
		     (hash_find(&opcode_list[hash_function(MneToOp,opcode,0)],opcode) >= 0))) {
			
			// 첫 INSTRUCTION의 주소를 INITIAL_ADDRESS에 저장
			if (initial_address < 0)
				initial_address = LOC;
			// OPCODE가 존재하는지 확인
			if (opcode[0] == '+')
				hash_cursor = hash_pointer(&opcode_list[hash_function(MneToOp,opcode+1,0)],opcode+1,0);
			else
				hash_cursor = hash_pointer(&opcode_list[hash_function(MneToOp,opcode,0)],opcode,0);

			// OPCODE가 없다면..!
			if (hash_cursor == NULL) {
				// 그냥 00으로 처리
				sprintf(objectcode_str,"00");
			// RSUB는 특수한 OPCODE로써, 3형식이면서 OPERAND를 가지지 않으므로, 따로 처리
			} else if (strcmp(opcode,"RSUB") == 0) {
				sprintf(objectcode_str,"4F0000");
			// 3형식의 OPCODE 일 때,
			} else if (hash_cursor->format == 3) {

				// 필요한 변수 설정
				check = 0;
				int disp, ta;

				// OBJECTCODE에 OPCODE 번호를 저장.
				objectcode = hash_cursor->opcode * 0x10000;
				strcpy(token_operand,operand);

				// ',X'가 붙은 경우에는 그 부분을 떼어냄.
				// 라벨을 SEARCH 할 수 없기에 떼어냄.
				for (i = 0; i < strlen(operand); i++) {
					if (operand[i] == ',') {
						operand[i] = '\0';
						break;
					}
				}

				// Target Address를 구함. SYMBOL TABLE에서 찾을 수 없다면, TA 는 -1
				if (operand[0] == '#' || operand[0] == '@') {
					hash_cursor = hash_pointer(&symbol_temp[hash_function(MneToOp,operand+1,0)],operand+1,0);
					if (hash_cursor == NULL)
						ta = -1;
					else
						ta = hash_cursor->opcode;
				} else if (operand[0] == '=') {
					// OPERAND가 LITERAL이라면, LITTAB에서 찾음.
					if (strcmp(operand,"=*") == 0) {
						sprintf(token_operand,"=*%04X",LOC);
					} else {
						strcpy(token_operand,operand);
					}
					// LITERAL TABLE에 있다면, TARGET ADDRESS가 설정되고 없다면 -1
					hash_cursor = hash_pointer(&littab_list,token_operand,0);
					if (hash_cursor != NULL) {
						ta = hash_cursor->opcode;
					} else {
						ta = -1;
					}
				} else {
					// 위 경우 모두에 해당되지 않는다면 심플 어드레싱으로 SYMBOL TABLE 탐색
					hash_cursor = hash_pointer(&symbol_temp[hash_function(MneToOp,operand,0)],operand,0);
					if (hash_cursor == NULL)
						ta = -1;
					else
						ta = hash_cursor->opcode;
				}
				strcpy(operand,token_operand);

				// TARGET ADDRESS가 정해지지 않았다면, '#정수'일 경우이므로,
				// 정수를 계산하여 TARGET ADDRESS에 저장.
				if (ta < 0) {
					if (operand[0] != '#') {
						// TARGET ADDRESS가 설정되지 않았는데, CONSTANT도 아니라면, UNDEFINED ERROR
						printf("ERROR: Line %d: Undefined Symbol [%s] is used.\n",ROW,operand);
						isError = -1;
					} else {
						// 만약 OPERAND 부분이 숫자가 아니라면 UNDEFINED SYMBOL 에러
						for (i = 1; i < strlen(operand); i++) {
							if (i == 1 && operand[i] == '-') continue;
							if (operand[i] < '0' || operand[i] > '9') {
								// 첫 글자가 '-'인 경우를 제외하고 숫자가 아니라면 UNDEFINED ERROR
								printf("ERROR: Line %d: Undefined Symbol [%s] is used.\n",ROW,operand);
								isError = -1;
								break;
							}
						}
						// 숫자를 INTEGER로 SCAN
						if (i == strlen(operand)) {
							sscanf(operand+1,"%d",&disp);
							check = 4;

							// 그 값이 너무 크다면 에러
							if ((opcode[0] == '+' && (disp > 0x7FFFF || disp < -1 * 0x80000)) ||
							    (opcode[0] != '+' && (disp > 0x7FF || disp < -1 * 0x800))) {
								printf("ERROR: Line %d: Constant [%s] is too big.\n",*ROWCTR,operand);
								isError = -1;
							}

							// 그 값이 음수라면 2'S COMPLEMENT 를 통해 변환
							if (disp < 0) {
								if (opcode[0] == '+') {
									disp *= -1;
									disp ^= 0xFFFFF;
									disp += 1;
								} else {
									disp *= -1;
									disp ^= 0xFFF;
									disp += 1;
								}
							}
						}
					}
				} else {
				// EQU 값이 Absolute면 BLOCK처리 하지 않음, relative라면 BLOCK 처리.
				// BLOCK을 처리 한다는 뜻은 해당 BLOCK의 시작주소를 TARGET ADDRESS에 더해줌.
					current = hash_pointer(&equ_list,hash_cursor->mnemonic,0);
					if (current != NULL) {
						if (current->format != 0)
							ta += LOCCTR[hash_cursor->format];	
					} else {
						ta += LOCCTR[hash_cursor->format];
					}
				}

				// '+'를 가지는 4형식이라면 TARGET ADDRESS가 곧 DISP가 된다.
				if (check == 0 && opcode[0] == '+') {
					disp = ta;
					check = 1;
				}

				// 일반적인 형태일 경우.
				if (check == 0) {
					// TARGET ADDRESS에서 PROGRAM COUNTER를 뺀 값이 DISP.
					disp = ta - (LOC + 3 + LOCCTR[BLK]);

					// DISP가 PC RELATIVE로 도달하지 못할 경우,
					if (disp < -2048 || disp > 2047) {

						// BASE가 존재 한다면,
						if (Base >= 0) {
							// TARGET ADDRESS에서 BASE를 뺀 값이 DISP.
							disp = ta - Base;

							// BASE RELATIVE로 도달하지 못할 경우,
							if (disp < 0 || disp > 4095) {
								check = 0;
							} else {
								check = 3;
							}
						}
						// PC , BASE RELATIVE가 모두 안되면 CAN NOT REACH 에러.
						if (check == 0) {
							printf("ERROR: Line %d: Can not reach to Target Address.\n",ROW);
							isError = -1;
						}
					} else {
						check = 2;
					}
				}

				// n 비트와 i 비트를 계산
				if (operand[0] == '#') {
					objectcode += 0x010000;
				} else if (operand[0] == '@') {
					objectcode += 0x020000;
				} else {
					objectcode += 0x030000;
				}

				// ',X' 를 가진다면 x비트를 1로 계산.
				if (operand[strlen(operand)-2] == ',' &&
				    operand[strlen(operand)-1] == 'X') {
					objectcode += 0x008000;
				}

				// 4형식 명령어일 경우,
				if (opcode[0] == '+') {
					objectcode += 0x001000;
					// 4바이트로 늘리고 DISP를 저장.
					objectcode *= 0x100;
					objectcode += disp;

					// LABEL이 존재한다면,
					if (check == 1) {
						if (operand[0] == '@' || operand[0] == '#') {
							strcpy(token_operand,operand+1);
						} else {
							strcpy(token_operand,operand);
						}
						// 맨 뒷 부분이 ',X'이라면 지워줘야 정상적인 LABEL 탐색
						if (operand[strlen(operand)-2] == ',' &&
						    operand[strlen(operand)-1] == 'X') {
							token_operand[strlen(operand)-2] = '\0';
						}

						// EXTREF에 존재하는지 확인
						hash_cursor = hash_pointer(&extref_list,token_operand,0);
						
						// EXTREF에 존재한다면, MODIFICATION 레코드 작성 준비
						count = 0;
						if (hash_cursor != NULL) {
							sprintf(token_operand,"+%s",hash_cursor->mnemonic);
							count = 1;
						} else {
							// EXTREF에 없는데, EQU에 있다면 RELATIVE일 때만
							// MODIFICATION 레코드 작성 준비
							hash_cursor = hash_pointer(&equ_list,token_operand,0);
							if (hash_cursor != NULL && hash_cursor->format == 0) {
								count = 0;
							} else {
								count = 1;
								sprintf(token_operand,"+%s",program_name);
							}
						}
						
						// MODIFICATION 레코드 준비가 된 상태라면 MODIFICATION TABLE에 저장
						if (count == 1)
							hash_add(&modification_list,token_operand,LOC+1+LOCCTR[BLK],5);
					}
				} else {
					// PC RELATVIE 혹은 BASE RELATIVE에 따라
					// OBJECTCODE에 값을 저장.
					if (check == 2) {
						objectcode += 0x002000;

						// 음수일 경우, 2'S COMPLEMENT.
						if (disp < 0) {
							disp *= -1;
							disp ^= 0xFFF;
							disp += 1;
						}
					} else if (check == 3) {
						objectcode += 0x004000;
					}
					// DISP 를 OJBECTCODE에 저장함으로써, OBJECTCODE를 완성
					objectcode += disp;
				}

				// 만들어진 OBJECT CODE를 문자열로 변경.
				if (opcode[0] != '+')
					sprintf(objectcode_str,"%06X",objectcode);
				else
					sprintf(objectcode_str,"%08X",objectcode);
			} else if (hash_cursor->format == 2) {

				// 2바이트로 OBJECT CODE 생성
				objectcode = hash_cursor->opcode * 0x100;

				// OPERAND를 ','로 tokenizing.
				token_counter = 0;
				strcpy(token_operand,operand);
				token = strtok(token_operand,",");
				while (token != NULL) {
					check = 0;
					// 레지스터 별로 번호를 지정해줌
					     if (strcmp(token,"A") == 0) check = 0;
					else if (strcmp(token,"X") == 0) check = 1;
					else if (strcmp(token,"L") == 0) check = 2;
					else if (strcmp(token,"B") == 0) check = 3;
					else if (strcmp(token,"S") == 0) check = 4;
					else if (strcmp(token,"T") == 0) check = 5;
					else if (strcmp(token,"F") == 0) check = 6;
					else if (strcmp(token,"PC") == 0) check = 8;
					else if (strcmp(token,"SW") == 0) check = 9;

					// Register에 대해 값을 계산하여 ObjectCode에 저장
					if (token_counter++ == 0) {
						objectcode += check * 0x10;
					}else{
						objectcode += check;
					}
					token = strtok(NULL,",");
				}

				// 저장 된 object code를 문자열로 저장
				sprintf(objectcode_str,"%04X",objectcode);
			} else if (hash_cursor->format == 1) {
				// 포맷 1이라면, 바로 objectcode를 저장
				objectcode = hash_cursor->opcode;
				sprintf(objectcode_str,"%02X",objectcode);
			}
		} else if (strcmp(opcode,"WORD") == 0) {
			
			// WORD의 값이 EXPRESSION일 경우, 그 값을 계산
			count = expression_calculator(label,opcode,operand,&ROW,LOC+LOCCTR[BLK],LOCCTR,&isError);

			// WORD 범위를 벗어날 경우, 에러
			if (count > 0xFFFFFF) {
				printf("ERROR: LINE %d: Value [%s] is too big to store at WORD.\n",*ROWCTR,operand);
				isError = -1;
				count = 0;
			}

			// 음수일 경우, 2'S COMPLEMENT로 변경
			if (count < 0) {
				count *= -1;
				count ^= 0xFFFFFF;
				count += 1;
			} 

			objectcode = count;

			// OBJECTCODE를 문자열로 저장.
			sprintf(objectcode_str,"%06X",objectcode);

		} else if (strcmp(opcode,"BYTE") == 0 || opcode[0] == '=') {
			// LITERAL일 경우, 맨 앞의 '='를 없앰.
			if (opcode[0] == '=') { 
				strcpy(operand,opcode+1);
			}

			// 그 값이 '*' 일 경우, LITERAL에 저장 할때 같이 붙힌 주소값을 읽어들임.
			if (operand[0] == '*') {
				sscanf(operand+1,"%X",&objectcode);
				sprintf(objectcode_str,"%06X",objectcode);

				// '=*'은 RELATIVE TERM이므로, MODIFICATION RECORD 생성
				strcpy(token_operand,"+");
				strcat(token_operand,program_name);
				hash_add(&modification_list,token_operand,LOC+LOCCTR[BLK],6);
			// C 혹은 X일 경우,
			} else if (operand[0] == 'C') {
				for (i = 2, j = 0; i < strlen(operand); i++) {
					if (operand[i] == '\'') break;
					else {
						// 해당 문자의 ASCII 값을 16진수로 저장.
						objectcode_str[j++] = operand[i] / 0x10;
						objectcode_str[j++] = operand[i] % 0x10;

						objectcode_str[j-2] += (objectcode_str[j-2] >= 0xA)?'A'-0xA:'0';
						objectcode_str[j-1] += (objectcode_str[j-1] >= 0xA)?'A'-0xA:'0';
					}
				}
				objectcode_str[j] = '\0';
			} else {
				// X 일 경우, 그대로 저장.
				for (i = 2, j = 0; i < strlen(operand); i++) {
					if (operand[i] == '\'') break;
					else {
						objectcode_str[j++] = operand[i];
					}
				}
				objectcode_str[j] = '\0';
			}

			if (opcode[0] == '=')
				operand[0] = '\0';
		} else if (strcmp(opcode,"BASE") == 0) {
		// BASE의 값을 Expression으로 계산.
			Base = expression_calculator(label,opcode,operand,&ROW,LOC+LOCCTR[BLK],LOCCTR,&isError);
			LOC = -1;
		// NOBASE라면 BASE 값을 음수로 두어 BASE RELATIVE가 안됨을 명시.
		} else if (strcmp(opcode,"NOBASE") == 0) {
			Base = -1;
		} else {

		}

		// ROW, LOC, BLK에 대해 존재할 경우만 출력.
		// -1 일 경우는 출력하는 의미가 없다는 뜻.
		if (ROW < 0)
			fprintf(lst,"\t");
		else
			fprintf(lst,"%4d\t",ROW);
		if (LOC < 0)
			fprintf(lst,"\t");
		else
			fprintf(lst,"%04X\t",LOC);
		if (LOCMNECNT > 1) {
			if (BLK >= 0)
				fprintf(lst,"%d",BLK);
			fprintf(lst,"\t");
		}

		// LABEL, OPCODE, OPERAND, OBJECT CODE에 대해 존재할 경우만 출력.
		if (strcmp(label,"####") == 0)
			fprintf(lst,"\t");
		else
			fprintf(lst,"%s\t",label);
		if (opcode[0] == '\0')
			fprintf(lst,"\t");
		else
			fprintf(lst,"%s\t",opcode);
		if (operand[0] == '\0')
			fprintf(lst,"\t");
		else
			fprintf(lst,"%s\t",operand);
		if (objectcode_str[0] == '\0')
			fprintf(lst,"\t\n");
		else
			fprintf(lst,"\t%s\n",objectcode_str);

		// OBJECT FILE을 만들 때, T레코드를 작성.
		LOC += LOCCTR[BLK];
		if (strlen(objectLine) - 9 + strlen(objectcode_str) > 60 ||
		    strcmp(opcode,"RESB") == 0 || strcmp(opcode,"RESW") == 0 || strcmp(opcode,"USE") == 0) {
			// RESB, RESW, USE 를 사용 하거나, 70개를 넘어갔을 경우, 기존에 쓰던 줄을 끝맺음.
			if (objectLine[0] != '\0') {
				check = (strlen(objectLine) - 9) / 2;
				objectLine[7] = (check / 0x10);
				objectLine[8] = (check % 0x10);
				
				// T 레코드의 길이를 저장.
				objectLine[7] += (objectLine[7] >= 0xA)?'A'-0xA:'0';
				objectLine[8] += (objectLine[8] >= 0xA)?'A'-0xA:'0';

				fprintf(obj,"%s\n",objectLine);
				objectLine[0] = '\0';
			}
		}
		if (objectcode_str[0] == '\0') continue;
		// 새로운 T 레코드를 작성
		if (objectLine[0] == '\0') {
			sprintf(objectLine,"T%06X  ",LOC);
		}
		// 새로 추가 되는 OBJECTCODE_STR을 뒤에다가 덧붙임
		sprintf(objectLine,"%s%s",objectLine,objectcode_str);
		objectcode_str[0] = '\0';
	}

	// 파일 종료
	fclose(lst);
	fclose(obj);

	// 에러가 없다면, SYMBOL_SET에 SYMBOL에 저장.
	if (isError == 0) {
		for (i = 0; i < MAX_HASH_SIZE; i++) {
			current = symbol_temp[i];
			while (current != NULL) {
				// SYMBOL_SET에 SYMBOL을 저장하면서,
				// 지금의 프로그램 이름(CSECT 혹은 MAIN PROGRAM)을 같이 저장
				previous = hash_add(&symbol_set[i], current->mnemonic, current->opcode + LOCCTR[current->format], current->format);
				strcpy(previous->value,program_name);

				// 옮긴 후, SYMBOL_TEMP는 FREE함
				previous = current;
				current = current->nextNode;
				free(previous);
			}
			symbol_temp[i] = NULL;
		}
	}

	// INTERMEDIATE 파일을 삭제.
	free(LOCCTR);
	remove("intermediate");

	return isError;
}

//-----------------------------------------------//
// 함수이름: assemble
// 리 턴 값: 오류가 있으면 음수, 없으면 0
// 파라미터: 없음
// 목    표: 소스를 읽어들여, CSECT를 구분하여
//	     ASSEMBLE을 따로 진행.
//-----------------------------------------------//
int assemble (void) {

	// PARAMETER에 파일이 없으면 에러.
	if (parameter_count != 2) {
		ERROR(ERR_PAR);
		return -2;
	}

	// asm 파일이 아니라면 오류
	if (strcmp(".asm",(parameter[1] + strlen(parameter[1]) - 4)) != 0) {
		printf("ERROR: Target File must be [.asm] file.\n");
		return -2;
	}

	// 파일을 열어, 없다면 오류
	FILE *input = fopen(parameter[1],"r");
	if (input == NULL) {
		printf("ERROR: There isn't such file\n");
		return -2;
	}

	// 변수 설정
	char **source;
	char preserve[MAX];

	source = (char **)malloc(sizeof(char *));
	source[0] = (char *)malloc(sizeof(char)*MAX);
	int sourceLine = 0, check, run_result, i, ROWCTR = 0, Error = 0;
	char *token;
	char separator[] = " \t\n\r";

	// SYMBOL_SET 을 초기화 시켜줌.
	hash_node *current, *previous;
	for (i = 0; i < MAX_HASH_SIZE; i++) {
		current = symbol_set[i];
		while (current != NULL) {
			previous = current;
			current = current->nextNode;
			free(previous);
		}
		symbol_set[i] = NULL;
	}

	// source에 한 줄씩 읽어들임.
	while (fgets(source[sourceLine],MAX,input) != NULL) {
		strcpy(preserve,source[sourceLine]);

		// tokenizing을 통해 CSECT 혹은 END가 있는지 확인
		check = i = 0;
		token = strtok(preserve,separator);
		while (token != NULL) {
			// TOKEN 줄이 주석이라면 BREAK.
			if (token[0] == '.') break;
			i++;

			// 토큰의 값이 CSECT거나, END이면 각각의 MODE를 지정해주고 BREAK.
			if (strcmp(token,"CSECT") == 0) { check = CSECT_MODE; break; }
			if (strcmp(token,"END") == 0)   { check = START_MODE; break; }

			// LABEL이나 OPCODE에 없다면 끝냄.
			if (i == 2) break;
			token = strtok(NULL,separator);
		}
		strcpy(preserve,source[sourceLine]);

		if (check == 0) {
			// 소스가 끝나지 않았다면, 계속해서 읽어들임.
			sourceLine++;
			source = (char **)realloc(source,sizeof(char *)*(sourceLine + 1));
			source[sourceLine] = (char *)malloc(sizeof(char)*MAX);
		} else {
			// CSECT_MODE 라면, 마지막 줄이 END가 아니므로 sourceLine - 1 만큼만 parameter로 넘김.
			if (check == CSECT_MODE)
				run_result = assemble_subrun(source,sourceLine - 1,&ROWCTR);
			// START_MODE 라면, 마지막 줄이 END가 되므로, sourceLine 만큼만 parameter로 넘김.
			else if (check == START_MODE)
				run_result = assemble_subrun(source,sourceLine,&ROWCTR);

			if (run_result < 0) Error = -1;

			// source를 메모리에서 해제시킨다.
			for (i = 0; i <= sourceLine; i++) {
				free(source[i]);
			} free(source);

			source = (char **)malloc(sizeof(char*) * 1);
			source[0] = (char *)malloc(sizeof(char)*MAX);
			sourceLine = 0;

			// CSECT_MODE 일 경우, 읽어들인 값을 저장하고 다음 줄을 읽음.
			if (check == CSECT_MODE) {
				source = (char **)realloc(source,sizeof(char *)*2);
				source[1] = (char *)malloc(sizeof(char)*MAX);
				sourceLine = 1;
				strcpy(source[0],preserve);
			}
		}
	}

	// 파일 읽기가 끝나면 메모리 해제.
	for (i = 0; i <= sourceLine; i++) {
		free(source[i]);
	}free(source);
	
	// 모든 프로그램의 구동 결과, ERROR가 없다면, SYMBOL_LIST에 SYMBOL_SET을 저장함.
	if (Error == 0) {
		for (i = symbol_count = 0; i < MAX_HASH_SIZE; i++) {
			// 기존의 SYMBOL_LIST를 초기화
			current = symbol_list[i];
			while (current != NULL) {
				previous = current;
				current = current->nextNode;
				free(previous);
			}
			// 헤더를 변경해줌.
			symbol_list[i] = symbol_set[i];
			symbol_set[i] = NULL;
			
			// SYMBOL의 갯수를 COUNT함.
			current = symbol_list[i];
			while (current != NULL) {
				symbol_count++;
				current = current->nextNode;
			}
		}
	}

	// 리턴
	return Error;
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
		filename_list[i] = parameter[1][i];
		filename_obj[i] = parameter[1][i];
	}
	filename_obj[i] = '\0';
	filename_list[i] = '\0';

	// 확장자를 붙여줘서 파일을 만들 준비를 함.
	strcat(filename_list,".lst");
	strcat(filename_obj,".obj");

	// assemble() 함수의 리턴값이 0 으로, 오류가 없다면.
	if ((assembleReturn = assemble()) == 0) {
		// 어셈블이 완료 되었다는 문구를 출력해줌.
		printf("\toutput files: [%s]",filename_list);

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
			if (inputLine[0] == 'H') {
				// CSECT를 가지고 있다면, CSECT의 이름으로 파일 이름 설정.
				if (obj == NULL) {
					for (i = 0; i < 6; i++) {
						if (inputLine[i+1] == ' ') break;
						filename_obj[i] = inputLine[i+1];
					}
					// OBJ 파일은 CSECT의 이름.OBJ가 됨.
					filename_obj[i] = '\0';
					strcat(filename_obj,".obj");

					// CSECT의 이름으로 obj파일을 생성.
					obj = fopen(filename_obj,"w");
				}
			}
			// 줄은 읽었는데 저장할 파일이 없다면 CONTINUE.
			if (obj == NULL) continue;
			fprintf(obj,"%s",inputLine);
			if (inputLine[0] == 'E') {
				// END 레코드가 나올 경우
				fclose(obj);
				obj = NULL;
				printf(", [%s]",filename_obj);
			}
		} fclose(inter_obj);

		printf("\n");
	// 만약 return값이 -2이면, history에 저장하지 않음. isError = 1
	} else if (assembleReturn == -2) { isError = 1; }

	// 사용 된 intermediate 파일과 임시 파일을 삭제.
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
					fprintf(dlt,"%04X\t%04X\t%s\t%s\t%02X\n",LOCCTR,LOCCTR,opcode,operand,operand_num);

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
// 함수이름: expression_calculator
// 리 턴 값: expression을 계산한 값.
// 파라미터: label, opcode, operand, ROWCTR, LOCCUR, isError
// 목    표: expression을 사용한 경우, 식을 계산하여
//	     그 값을 리턴.
//-----------------------------------------------//
int expression_calculator (char label[], char opcode[], char operand[], int *ROWCTR, int LOCCUR, int *LOCCTR, int *isError) {

	// 변수 설정
	int result, i, j, expr_cnt, expr_term, total_term;
	char temp[MAX], expr_optr;

	// 식을 저장하기 위한 struct.
	typedef struct _node {
		char word[MAX];
		int value;
	}expr;

	// 변수 초기화
	expr **expr_list = NULL;
	expr *righthand;

	hash_node *hash_cursor, *hash_picker;

	expr_cnt = expr_term = total_term = 0;
	// 문자열로 된 식을 LABEL과 연산자로 구분하여 저장.
	for (i = j = 0; i <= strlen(operand); i++) {
		// 값이 연산자일 때.
		if (operand[i] == '+' || operand[i] == '-' || operand[i] == '\0') {
			temp[j] = '\0';
			// 라벨의 저장을 끝내고 그 값을 expr_list에 저장.
			if (j == 0) {
				strcpy(temp,"0");
			}
			// EXPR_LIST 가 비어있다면, MALLOC하고 아니라면 REALLOC을 통해 재할당
			if (expr_list != NULL) {
				expr_list = (expr **)realloc(expr_list,sizeof(expr *) * (expr_cnt + 1));
			} else {
				expr_list = (expr **)malloc(sizeof(expr *));
			}
			expr_list[expr_cnt] = (expr *)malloc(sizeof(expr));
			strcpy(expr_list[expr_cnt]->word,temp);		
			if (j != 0) expr_cnt++;
			j = 0;

			// 연산자를 expr_list에 저장.
			if (operand[i] != '\0') {
				expr_list = (expr **)realloc(expr_list,sizeof(expr *) * (expr_cnt + 1));
				expr_list[expr_cnt] = (expr *)malloc(sizeof(expr));

				expr_list[expr_cnt]->word[0] = operand[i];
				expr_list[expr_cnt]->word[1] = '\0';
				j = 0; expr_cnt++;
			}
		} else {
			// 연산자가 아니라면 label의 한글자씩 temp에 저장
			temp[j++] = operand[i];
		}
	}

	// expr_optr은 unary operator을 나타냄.
	expr_optr = '+';
	for (i = 0; i < expr_cnt; i++) {
		expr_list[i]->value = 0;
		// 숫자라면 그 값을 저장하여 value에 저장
		if (expr_list[i]->word[0] >= '0' && expr_list[i]->word[0] <= '9') {
			sscanf(expr_list[i]->word,"%d",&expr_list[i]->value);
		// 연산자일 경우, value 값은 -1.
		} else if (expr_list[i]->word[0] == '+' || expr_list[i]->word[0] == '-') {
			expr_list[i]->value = -1;
			// expr_optr 의 연산자를 해당 연산자로 바꿈.
			expr_optr = expr_list[i]->word[0];
		// 문자열 값이 '*'인 경우, value에 현재 주소인 LOCCUR로 저장.
		} else if (strcmp(expr_list[i]->word,"*") == 0) {
			expr_list[i]->value = LOCCUR;
			// relative term이므로 expr_term을 +1
			expr_term++;
			total_term++;
		} else {
			// j에 symbol_temp의 LOCATION을 assign.
			hash_picker = hash_pointer(&symbol_temp[hash_function(MneToOp,expr_list[i]->word,0)],expr_list[i]->word,0);
			if (hash_picker == NULL) {
				printf("ERROR: Line %d: Symbol [%s] doesn't exists.\n",*ROWCTR,expr_list[i]->word);
				*isError = -1;

				expr_list[i]->value = 0;
				if (expr_optr == '+') total_term++;
				else if (expr_optr == '-') total_term--;
			} else {
				// External Reference에 존재하는 SYMBOL이라면, modification 레코드 생성.
				hash_cursor = hash_pointer(&extref_list,expr_list[i]->word,0);
				if (hash_cursor != NULL) {
					// External Symbol를 WORD에서 Expression으로 사용할 때.
					if (strcmp(opcode,"WORD") == 0) {
						temp[0] = expr_optr; temp[1] = '\0';
						strcat(temp,expr_list[i]->word);
						hash_add(&modification_list,temp,LOCCUR,6);
					}
				} else {
					// PASS2 일경우, BLOCK 처리
					if (LOCCTR != NULL && hash_picker->format >= 0) {
						expr_list[i]->value += LOCCTR[hash_picker->format];
					}
					// EQU_LIST에서 탐색을 하여, 없거나 RELATIVE TERM 이라면 EXPR_TERM 가감산
					hash_cursor = hash_pointer(&equ_list,expr_list[i]->word,0);
					if (hash_cursor == NULL || (hash_cursor != NULL && hash_cursor->format != 0)) {
						// expr_optr에 대하여 expr_term의 값을 변경
						if (expr_optr == '+') expr_term++;
						else if (expr_optr == '-') expr_term--;
					}
				}
				// EQU_LIST를 탐색 하여, 없거나 RELATIVE TERM이라면 TOTAL_TERM 가감산
				hash_cursor = hash_pointer(&equ_list,expr_list[i]->word,0);
				if (hash_cursor == NULL || (hash_cursor != NULL && hash_cursor->format != 0)) {
					if (expr_optr == '+') total_term++;
					else if (expr_optr == '-') total_term--;
				}
				// 값이 음수일 경우,
				if (hash_picker->opcode / 0x800000 > 0) {
					// 2'S COMPLEMENT로 변환
					expr_list[i]->value += ((hash_picker->opcode - 1)^(0xFFFFFF))*-1;
				} else {
					expr_list[i]->value += (hash_picker->opcode);
				}
			}
		}
	}

	// opcode가 BASE, EQU와 ORG 가 아니라면,
	if (strcmp(opcode,"BASE") != 0 && strcmp(opcode,"EQU") != 0 && strcmp(opcode,"ORG") != 0) {
		// expr_term이 1 혹은 -1 일 경우, Modification 레코드가 필요
		if (abs(expr_term) == 1) {
			strcpy(temp,"+");
			strcat(temp,program_name);
			hash_add(&modification_list,temp,LOCCUR,6);
		}
	}

	// expr_term이 -1, 0, 1 이 아니라면 잘못 된 Expression.
	if (total_term != 0 && total_term != 1) {
		printf("ERROR: Line %d: Expression is not relative term or absolute term.\n",*ROWCTR);
		*isError = -1;

		result = 0;
	} else {
		// expr_list에 따라 값을 계산.
		righthand = NULL; result = 0;
		for (i = 0; i < expr_cnt; i++) {
			// 연산자가 나왔을 경우,
			if (expr_list[i]->value == -1) {
				// Expression의 마지막이 연산자일 경우, 에러
				if (i + 1 >= expr_cnt) {
					printf("ERROR: Line %d: Expression is something wrong.\n",*ROWCTR);
					*isError = -1;
					
					result = 0;
					break;
				// 아닐 경우, righthand에 expr_list[i+1]으로 포인팅.
				} else {
					righthand = expr_list[i+1];
				}

				// 연산자에 따라 result값에 가감산.
				switch (expr_list[i]->word[0]) {
				case '+': result = result +  (1)*righthand->value; break;
				case '-': result = result + (-1)*righthand->value; break;
				}
			} else {
				// expr_list의 value값을 result에 저장.
				if (i == 0 && result == 0) {
					result = expr_list[0]->value;
				} else {
					continue;
				}
			}
		}

		// 결과가 음수 일경우, 2'S COMPLEMENT로 변환
		if (result < 0) {
			result *= -1;
			result ^= 0xFFFFFF;
			result += 1;
		}
	}

	// opcode가 EQU일 경우.
	if (strcmp(opcode,"EQU") == 0) {
		// 계산 된 result값을 equ_list에 저장.
		hash_add(&equ_list,label,result,total_term);
	}

	// 메모리 해제.
	for (i = 0; i < expr_cnt; i++) {
		free(expr_list[i]);
	}
	free(expr_list);

	return result;
}

//-----------------------------------------------//
// 함수이름: hash_pointer
// 리 턴 값: LABEL, ADDRESS에 일치하는 값의 주소값
// 파라미터: hash_node**, LABEL의 char*, 주소값의 int
// 목    표: 파라미터로 받은 정보와 일치하는 값을
//   	     찾아서 주소값을 리턴.
//-----------------------------------------------//
hash_node* hash_pointer (hash_node **head, char mnemonic[], int opcode) {

	// 변수 설정
	hash_node *current;

	current = *head;
	while (current != NULL) {
		// LABEL의 문자열이 NULL이 아니면
		if (mnemonic != NULL) {
			// 해당 문자열과 동일한 값을 가지는 노드를 리턴.
			if (strcmp(current->mnemonic,mnemonic) == 0) {
				return current;
			}
		} else {
			// 해당 주소값을 가지는 노드를 리턴.
			if (current->opcode == opcode) {
				return current;
			}
		}
		current = current->nextNode;
	}

	return NULL;
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
			strcpy(table[count].value,current->value);

			current = current->nextNode;
			count++;
		}	
	}

	// qsort를 이용하여 배열을 mnemonic에 대해서 내림차순 정렬.
	qsort(table,count,sizeof(symtab),symtab_comp);
	qsort(table,count,sizeof(symtab),symtab_sect);

	printf("The number of Symbols: %d\n",symbol_count);

	// 하나씩 출력함.
	for (i = 0; i < count; i++) {
		if (i > 0 && strcmp(table[i].value,table[i-1].value) != 0) printf("\n");
		printf("\t[%6s]\t%s\t%04X\n",table[i].value,table[i].label,table[i].address);
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

	// strcmp를 통해 우선순위를 정해주고 그 값을 리턴
	return strcmp(sb->label,sa->label);
}

//-----------------------------------------------//
// 함수이름: symtab_sect
// 리 턴 값: strcmp의 결과값
// 파라미터: 비교할 대상 a와 b
// 목    표: qsort를 사용하기 위한 compare 함수
//-----------------------------------------------//
int symtab_sect (const void *a, const void *b) {

	symtab *sa = (symtab *)a;
	symtab *sb = (symtab *)b;

	// strcmp를 통해 우선순위를 정해주고 그 값을 리턴
	return strcmp(sa->value,sb->value);
}


