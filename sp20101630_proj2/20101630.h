/* 포함되는 파일 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

/* 상수 정의 */
#define MAX 		255
#define MAX_PARAMETER 	5

#define MAX_MEMORY	1048576
#define MAX_OPCODE	10

#define MENU_COUNT	12
#define ERROR_COUNT	10

#define SAVE_HISTORY	1
#define LIST_HISTORY	0

#define MAX_HASH_SIZE	20

#define OpToMne		1
#define MneToOp		0

#define ERR_HEX		0
#define ERR_OUT		1
#define ERR_PAR		2
#define ERR_NOT_IN	3
#define ERR_SO_BIG	4
#define ERR_END_MEMORY	5

/* 전역 변수 */
char command[MAX];
char parameter[MAX_PARAMETER][MAX];
int  parameter_count;
int  isError;

// history()를 만들기 위한 linked-list.
typedef struct H_node {

	char command[MAX];
	struct H_node *nextNode;

}history_node;
history_node *history_head = NULL;

// 가상메모리 memeory 배열
unsigned char memory[MAX_MEMORY];
int dump_head = 0x00000;

// hash_table을 만들기 위한 linked-list.
typedef struct hs_node {

	int opcode;
	char mnemonic[MAX_OPCODE];
	struct hs_node *nextNode;

}hash_node;
hash_node *opcode_list[20];
hash_node *mnemonic_list[20];

char commandMenu[MENU_COUNT][2][MAX] = {{"h"	,"help"	},
					{"d"	,"dir"	},
					{"q"	,"quit"	},
					{"hi", "history"},
					{"du"	,"dump" },
					{"e"	,"edit" },
					{"f"	,"fill" },
					{"reset","reset"},
					{"opcode","opcode"},
					{"mnemonic","mnemonic"},
					{"opcodelist","opcodelist"},
					{"mnemoniclist","mnemoniclist"}};


char errorMenu[ERROR_COUNT][MAX] = {"ERROR: Invalid Hex input.",
				    "ERROR: Outbound Indexing.",
				    "ERROR: Parameter error.",
				    "ERROR: There isn't such component.",
				    "ERROR: Input Value is so large.",
				    "ERROR: Out of Memory Range. Memory Index will be assigned as 0x00."};

/* 함수 원형 */
void printSIC(void);
void command_tokenizing(void);
void initialize_parameter(void);
void initialize_command(void);
void command_analyser(void);
int parameter_analyser(void);
void history(int);
void help(void);
void dir(void);
void dump(void);
void reset(void);
void edit(void);
void fill(void);
void OpcodeMnemonic(int);

int isHex(char*,int*);
void ERROR(int);

void hash_make(void);
void hash_list(int);
void hash_add(hash_node **,char*,int);
int hash_function(int,char*,int);
