#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[]){

	FILE *  fp;
	int writeText;
	int lines = 1;
	char currChar;

	if( argc == 2){
		fp= fopen(argv[1], "r");
		writeText = -1;
	}else if( argc == 3 && strcmp(argv[1], "-l") == 0){
		fp = fopen(argv[2], "r");
		writeText = 0;
	}else{
		printf("usage: phrases [-l] file\n");
		return 1;
	}

	if (!fp){
		printf("couldn read file\n");
		return 1;
	}
	

	if(writeText == 0){
		printf("[%d] ",lines);

		while( (currChar = fgetc(fp)) != EOF){
			if(currChar!='\n') printf("%c",currChar);
			
			if(currChar == '.' || currChar == '!' || currChar == '?'){
				lines++;
				printf("\n[%d]",lines);
			}
		}
	}else{
		while( (currChar = fgetc(fp)) != EOF){
			if(currChar == '.' || currChar == '!' || currChar == '?'){
				lines++;
			}
		}
		printf("%d",lines);
	}

	
	fclose(fp);
	printf("\n");
	return 0;
}
