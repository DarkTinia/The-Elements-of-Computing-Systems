#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ERR(...) \
	do \
	{ \
		fprintf(stderr, __VA_ARGS__); \
		exit(0); \
	} while(0)

#define CHARSET 128
#define INVALID -1

typedef struct _trieNode
{
	struct _trieNode *next[CHARSET];
	short value;
} trieNode;

trieNode *symbols;

int getVal(char c)
{
	if(c > 0 && c < 128)
		return (int)c;
	ERR("Invalid character in symbol name\n");
}

trieNode *newNode()
{
	trieNode *ret = NULL;

	ret = (trieNode *)calloc(1, sizeof(trieNode));
	if(!ret)
		ERR("Could not allocate memory for trie node\n");

	for(int i = 0; i < CHARSET; ++i)
		ret->next[i] = NULL;
	ret->value = INVALID;

	return ret;
}

void trieInsert(trieNode *node, char *str, short value)
{
	// printf("%s : %d\n", str, value);

	while(*str)
	{
		int idx = getVal(*str);
		if(!node->next[idx])
			node->next[idx] = newNode();
		node = node->next[idx];
		++str;
	}

	node->value = value;
}

short trieSearch(trieNode *node, char *str)
{
	while(*str)
	{
		int idx = getVal(*str);
		if(!node->next[idx])
			return INVALID;
		node = node->next[idx];
		++str;
	}

	return node->value;
}

FILE *createOutput(char *inputFileName)
{
	FILE *ret = NULL;
	char *outputFileName;
	char *end;

	size_t len = strlen(inputFileName);
	end = inputFileName + len;

	while(end > inputFileName && *end != '.' && *end != '/' && *end != '\\')
		--end;

	if(end > inputFileName && *end == '.' && *(end-1) != '/' && *(end-1) != '\\')
		len = end - inputFileName;

	outputFileName = calloc(sizeof(char), len + 6);
	strncpy(outputFileName, inputFileName, len);
	strcat(outputFileName, ".hack");

	if(!strcmp(inputFileName, outputFileName))
		ERR("Can't assemble .hack file\n");

	ret = fopen(outputFileName, "w");
	free(outputFileName);

	return ret;
}

void initialiseSymbolTable()
{
	symbols = newNode();

	FILE *predefined = fopen("predefined.sym", "r");
	if(!predefined)
	{
		fprintf(stderr, "Missing predefined.sym\nPredefined symbols not loaded\n");
		return;
	}

	char str[100];
	short value;

	while(fscanf(predefined, "%s %hd", str, &value) == 2)
		trieInsert(symbols, str, value);

	fclose(predefined);
}

void loadLabels(FILE *input)
{
	short lineCount = 0;
	char c, label[100], newLineFlag = 0;
	while(!feof(input))
	{
		c = getc(input);
		if(c == '/')
		{
			while(c != '\n')
				c = getc(input);
		}
		else if(c == '(')
		{
			fscanf(input, "%[^)])", label);
			trieInsert(symbols, label, lineCount);
			while(c != '\n')
				c = getc(input);
		}
		else if(!newLineFlag && (c == '0' || c == '1' || c == '-' || c == '@' || c == 'A' || c == 'M' || c == 'D'))
		{
			newLineFlag = 1;
			++lineCount;
		}
		if(c == '\n')
			newLineFlag = 0;
	}
}

void assemble(FILE *input, FILE *output)
{
	char c, command[101];
	short value = 16;
	while(!feof(input))
	{
		fscanf(input, " %c", &c);
		if(c != '/' && c != '(' && c != '\n')
		{
			ungetc(c, input);
			fscanf(input, "%s", command);
			// printf("%s\n", command);
			if(command[0] == '@')
			{
				short v;
				if(command[1] >= '0' && command[1] <= '9')
					v = atoi(&command[1]);
				else
					v = trieSearch(symbols, &command[1]);
				if(v == INVALID)
				{
					v = value++;
					trieInsert(symbols, &command[1], v);
				}
				putc('0', output);
				for(int i = 14; i >= 0; --i)
					putc(((v >> i) & 1) + '0', output);
			}
			else
			{
				char *dest = 0, *comp = &command[0], *jump = 0;

				fprintf(output, "111");

				for(int i = 0; command[i]; ++i)
				{
					if(command[i] == '=')
					{
						dest = &command[0];
						command[i] = '\0';
						comp = &command[i+1];
					}
					if (command[i] == ';')
					{
						command[i] = 0;
						jump = &command[i+1];
					}
				}

				if(!strcmp(comp, "0"))
					fprintf(output, "0101010");
				else if(!strcmp(comp, "1"))
					fprintf(output, "0111111");
				else if(!strcmp(comp, "-1"))
					fprintf(output, "0111010");
				else if(!strcmp(comp, "D"))
					fprintf(output, "0001100");
				else if(!strcmp(comp, "A"))
					fprintf(output, "0110000");
				else if(!strcmp(comp, "M"))
					fprintf(output, "1110000");
				else if(!strcmp(comp, "!D"))
					fprintf(output, "0001101");
				else if(!strcmp(comp, "!A"))
					fprintf(output, "0110001");
				else if(!strcmp(comp, "!M"))
					fprintf(output, "1110001");
				else if(!strcmp(comp, "-D"))
					fprintf(output, "0001111");
				else if(!strcmp(comp, "-A"))
					fprintf(output, "0110011");
				else if(!strcmp(comp, "-M"))
					fprintf(output, "1110011");
				else if(!strcmp(comp, "D+1"))
					fprintf(output, "0011111");
				else if(!strcmp(comp, "A+1"))
					fprintf(output, "0110111");
				else if(!strcmp(comp, "M+1"))
					fprintf(output, "1110111");
				else if(!strcmp(comp, "D-1"))
					fprintf(output, "0001110");
				else if(!strcmp(comp, "A-1"))
					fprintf(output, "0110010");
				else if(!strcmp(comp, "M-1"))
					fprintf(output, "1110010");
				else if(!strcmp(comp, "D+A"))
					fprintf(output, "0000010");
				else if(!strcmp(comp, "D+M"))
					fprintf(output, "1000010");
				else if(!strcmp(comp, "D-A"))
					fprintf(output, "0010011");
				else if(!strcmp(comp, "D-M"))
					fprintf(output, "1010011");
				else if(!strcmp(comp, "A-D"))
					fprintf(output, "0000111");
				else if(!strcmp(comp, "M-D"))
					fprintf(output, "1000111");
				else if(!strcmp(comp, "D&A"))
					fprintf(output, "0000000");
				else if(!strcmp(comp, "D&M"))
					fprintf(output, "1000000");
				else if(!strcmp(comp, "D|A"))
					fprintf(output, "0010101");
				else if(!strcmp(comp, "D|M"))
					fprintf(output, "1010101");

				if(dest)
				{
					if(!strcmp(dest, "M"))
						fprintf(output, "001");
					else if(!strcmp(dest, "D"))
						fprintf(output, "010");
					else if(!strcmp(dest, "MD"))
						fprintf(output, "011");
					else if(!strcmp(dest, "A"))
						fprintf(output, "100");
					else if(!strcmp(dest, "AM"))
						fprintf(output, "101");
					else if(!strcmp(dest, "AD"))
						fprintf(output, "110");
					else if(!strcmp(dest, "AMD"))
						fprintf(output, "111");
				}
				else
					fprintf(output, "000");


				if(jump)
				{
					if(!strcmp(jump, "JGT"))
						fprintf(output, "001");
					else if(!strcmp(jump, "JEQ"))
						fprintf(output, "010");
					else if(!strcmp(jump, "JGE"))
						fprintf(output, "011");
					else if(!strcmp(jump, "JLT"))
						fprintf(output, "100");
					else if(!strcmp(jump, "JNE"))
						fprintf(output, "101");
					else if(!strcmp(jump, "JLE"))
						fprintf(output, "110");
					else if(!strcmp(jump, "JMP"))
						fprintf(output, "111");
				}
				else
					fprintf(output, "000");
			}
			putc('\n', output);
		}
		while(c != '\n')
			c = getc(input);
	}
}

int main(int argc, char *argv[])
{
	if(argc < 2)
		ERR("Usage: assembler file.asm\n");

	FILE *input = fopen(argv[1], "r");
	if(!input)
		ERR("Invalid input file %s\n", argv[1]);

	FILE *output = createOutput(argv[1]);
	if(!output)
		ERR("Could not create output file\n");

	initialiseSymbolTable();

	loadLabels(input);

	rewind(input);

	assemble(input, output);

	fclose(input);
	fclose(output);

	return 0;
}
