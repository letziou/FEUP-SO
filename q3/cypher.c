#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1

struct wordsCypher {//struct, holds 2 words and each of their sizes
	char* wordA;
	int Asize;
	char* wordB;
	int Bsize;
};

int str_compare(char str1[], char str2[], int size) {
    for (int i = 0; i < size; i++) {
        if (str1[i] == '\0' || str2[i] == '\0')
            return -1;
        if (str1[i] != str2[i]) //one of these conditions means words are not the same
            return -1;
    }

    return 0;
}

struct wordsCypher* read_into_struct(FILE* fp){//TODO: add error verification to malloc/realloc?
        int i = 0, //i used for counting chars,
        j = 0, //j used for counting positions in struct,
        flag = 0;  //flag used for storing choosing between storing on wordA or wordB

        size_t size = 8;
        char* buf = malloc( sizeof(char) * size);
        char c;

        struct wordsCypher* words = malloc( sizeof(struct wordsCypher) * 1) ;//array of structs

        while ( ( c = fgetc(fp)) != EOF){
            if ( i == size){//if currently read word is at the max size, realloc
                buf = realloc(buf, (size = size*2) );
            }

            if(c!=' '){
                buf[i] = c;
                i++;
            }else if(c == '\n'){
                continue;
            }else if(flag == 0){// guardar palavra em wordA
                buf[i] = '\0';
                words[j].wordA = malloc(i * sizeof(char) );
                words[j].Asize = i;

                strncpy(words[j].wordA, buf, i);

                free(buf); // erase and reallocate buffer
                size = 8;
                buf = malloc( sizeof(char) * size);

                i = 0;
                flag = 1;
            }else if(flag == 1){
                buf[i] = '\0';
                words[j].wordB = malloc(i * sizeof(char) );
                words[j].Bsize = i;

                strncpy(words[j].wordB, buf, i);

                free(buf);
                size = 8;
                buf = malloc( sizeof(char) * size);

                i = 0;
                j++;
                flag = 0;

                words =  realloc(words, sizeof(struct wordsCypher) * (j + 1)  );
            }
        }

        words[j].wordB = malloc(i * sizeof(char) ); //TO-DO:last word isnt being stored in the cycle, probably should fix that
        words[j].Bsize = i;
        strncpy(words[j].wordB, buf, i);

        free(buf);

        return words;
};

int parent(int to_child_fd[2], int to_parent_fd[2]){//TODO: é necessario error prevention para stdin/stdout?
    close(to_child_fd[READ_END]);
    close(to_parent_fd[WRITE_END]);

	char buffer[10];
	char space[1];
	space[0] = ' ';//i want to write a space after writing a word AND it has to pointer soooo...

	while(fscanf(stdin,"%s",buffer) != EOF){
		int len = strlen(buffer);
		if (write(to_child_fd[1], buffer, len) == -1 || write(to_child_fd[1], space, 1) == -1 ) {//write to pipe word by word, add space after word
			fprintf(stderr, "Failed to write to pipe. Cause: %s\n",
					strerror(errno));
		}
	}
\
    close(to_child_fd[WRITE_END]);

	return EXIT_SUCCESS;
}

int child(int to_child_fd[2], int to_parent_fd[2]){
	FILE* fp = fopen("cypher.txt","r");

	if (fp == NULL) {
		close(to_parent_fd[WRITE_END]);
		return EXIT_FAILURE;
	}else{
        struct wordsCypher* words = read_into_struct(fp);

		close(to_child_fd[WRITE_END]);
        close(to_parent_fd[READ_END]);

		char read_word[16];
        int words_quant = sizeof(struct wordsCypher) / sizeof (words);
        char space[1];
        space[0] = ' ';//read above


        FILE *pipe_read_stream = fdopen(to_child_fd[READ_END],"r");

		while(fscanf(pipe_read_stream,"%s",read_word) != EOF){
			int read_len = strlen(read_word);
            int i;//"i" is needed outside the for loop

            for(i = 0; i < words_quant ; i++ ){//TODO:missing implementation to preserve special characters
                if(str_compare(words[i].wordA,read_word,words[i].Asize) == 0){//word read from pipe is found, send back the other one
                    write(to_parent_fd[WRITE_END], words[i].wordB, words[i].Bsize);
                    break;
                }else if(str_compare(words[i].wordB,read_word,words[i].Bsize) == 0){
                    write(to_parent_fd[WRITE_END], words[i].wordA, words[i].Bsize);
                    break;
                }
            }
            if (i == words_quant){
                write(to_parent_fd[WRITE_END], read_word, read_len);
            }
            write(to_parent_fd[WRITE_END], space, 1);//add a space character between words
		}

        for(int i = 0; i < words_quant;i++){//clear all words from struct
            free(words[i].wordA);
            free(words[i].wordB);
        }

        free(words);
        fclose(fp);
        fclose(pipe_read_stream);
        close(to_child_fd[READ_END]);
        close(to_parent_fd[WRITE_END]);

		return EXIT_SUCCESS;

	}
}

int main(int argc, char *argv[]){
	pid_t pid;
	int to_child_fd[2];
    int to_parent_fd[2];

	//Create and open pipe
	if(pipe(to_child_fd) < 0 || pipe(to_parent_fd) < 0 ){
		perror("pipe error");
		exit(EXIT_FAILURE);
	}

	if((pid=fork())<0){
		perror("fork error");
		exit(EXIT_FAILURE);
	}else if(pid>0){
		int r = parent(to_child_fd,to_parent_fd);

		// wait for child and exit
		if (waitpid(pid, NULL, 0) < 0) {
			fprintf(stderr, "Cannot wait for child: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}

        char buf[8];
        int bytes;

        while( (bytes=read(to_parent_fd[READ_END], buf, 8)) > 0 ) {
            write(STDOUT_FILENO,buf,bytes);
        }

        close(to_parent_fd[READ_END]);

		return r;
	}else{
		return child(to_child_fd,to_parent_fd);
	}
}
