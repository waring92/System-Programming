/* 포함되는 파일 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>

/* 상수 정의 */
#define MAX 		255
#define MAX_PARAMETER 	50

#define MAX_MEMORY	1048576
#define MAX_OPCODE	10

#define MENU_COUNT	20
#define ERROR_COUNT	10

#define SAVE_HISTORY	1
#define LIST_HISTORY	0

#define DIR_DIRECTORY	0
#define DIR_TYPE	1

#define MAX_HASH_SIZE	20

#define OpToMne		1
#define MneToOp		0

#define ERR_HEX		0
#define ERR_OUT		1
#define ERR_PAR		2
#define ERR_NOT_IN	3
#define ERR_SO_BIG	4
#define ERR_END_MEMORY	5

#define sptr_comma	-1
#define sptr_space	-2
#define sptr_tab	-3

#define ASM_INS		16

#define START_MODE	2
#define CSECT_MODE	1

#define EXT_DEF		1
#define EXT_REF		2

#define reg_A		0
#define reg_X		1
#define reg_L		2
#define reg_B		3
#define reg_S		4
#define reg_T		5
#define reg_F		6
#define reg_PC		8
#define reg_SW		9
/* 전역 변수 */
char command[MAX];
char parameter[MAX_PARAMETER][MAX];
int  parameter_count;
int  isError;

char program_name[MAX];

// history()를 만들기 위한 linked-list.
typedef struct H_node {

	char command[MAX];
	struct H_node *nextNode;

}history_node;
history_node *history_head = NULL;

// 가상메모리 memeory 배열
unsigned char memory[MAX_MEMORY];
unsigned char breakp[MAX_MEMORY];
int dump_head = 0x00000;
int progaddr = 0x00000;
int execaddr = -1;
int runaddr = 0x00000;

// 가상레지스터
int register_array[10] = {0,};
int register_L[10] = {0,};
int Lndex = 0;

// hash_table을 만들기 위한 linked-list.
typedef struct hs_node {

	int opcode;
	char mnemonic[MAX_OPCODE];
	int format;
	char value[MAX];
	struct hs_node *nextNode;

}hash_node;
hash_node *opcode_list[MAX_HASH_SIZE];
hash_node *mnemonic_list[MAX_HASH_SIZE];

typedef struct {

	int address;
	char label[MAX_OPCODE];
	char value[MAX];

}symtab;

int	  symbol_count;
hash_node *symbol_list[MAX_HASH_SIZE];
hash_node *symbol_temp[MAX_HASH_SIZE];
hash_node *symbol_set[MAX_HASH_SIZE];

int literal_count;
hash_node *littab_list;
hash_node *equ_list;
hash_node *extref_list;
hash_node *extdef_list;
hash_node *modification_list;

hash_node *estab;
hash_node *bptable;

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
					{"mnemoniclist","mnemoniclist"},
					{"assemble","assemble"},
					{"type","type"},
					{"symbol","symbol"},
					{"disassemble","disassemble"},
					{"progaddr","progaddr"},
					{"loader","loader"},
					{"run","run"},
					{"bp","bp"}};


char errorMenu[ERROR_COUNT][MAX] = {"ERROR: Invalid Hex input.",
				    "ERROR: Outbound Indexing.",
				    "ERROR: Parameter error.",
				    "ERROR: There isn't such component.",
				    "ERROR: Input Value is so large.",
				    "ERROR: Out of Memory Range. Memory Index will be assigned as 0x00."};

char assemble_instruction[ASM_INS][MAX] = {"RESB","RESW","START","END","BYTE","WORD","BASE","NOBASE",
			         	   "EQU","LTORG","ORG","USE","CSECT","EXTDEF","EXTREF"};

/* 함수 원형 */
void printSIC(void);
void command_tokenizing(void);
void initialize_parameter(void);
void initialize_command(void);
void command_analyser(void);
int parameter_analyser(void);
void history(int);
void help(void);
void dir(int);
void dump(void);
void reset(void);
void edit(void);
void fill(void);
void OpcodeMnemonic(int);

void progaddress(void);
void linking_loader(void);
void run(void);
int  cmp(int, int);
void bp(void);
void print_register(void);

unsigned int getaddress (unsigned int);
int assign (int,int,int);
int load (int,int,unsigned int);

int isHex(char*,int*);
void ERROR(int);
void hash_make(void);
void hash_list(int);
hash_node* hash_add(hash_node **,char*,int,int);
int hash_function(int,char*,int);
int assemble_subrun(char **,int,int*);
int assemble(void);
void assemble_run(void);
int disassemble(void);
void disassemble_run(void);
int expression_calculator(char *,char *,char *, int *, int, int *, int *);
hash_node* hash_pointer (hash_node **,char*,int);
int hash_find(hash_node **,char*);
char* hash_find_opcode(hash_node **, int);
void symtab_list(void);
int symtab_comp(const void *, const void *);
int symtab_sect(const void *, const void *);
