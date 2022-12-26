#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

int checkRows(FILE* f1, FILE* f2); // function to check if rows are equal
int checkColumns(FILE* f1, FILE* f2); // function to check if columns are equal
int getMatrixSize(int m_x, int m_y); // function to get the size needed to create mapped memory
int *readMatrices(FILE* f1, int m_x, int m_y); // function that creates the shared memory and reads from input file to said memory
void calculateRow(int *marray1, int *marray2, int i, int rows); // function that calculates row of result matrix
void writeMatrix(int *marray, int m_x, int m_y); // function that writes result matrix to output

int main(int argc, char *argv[]) {
  FILE *f1;
  FILE *f2;

  // check if exactly two files are present
  if (argc != 3) {
    fprintf(stderr, "usage: addmx file1 file2\n");
    exit(EXIT_FAILURE);
  }

  // check if first file can be opened and is readable
  f1 = fopen(argv[1], "r");
  if (f1 == NULL) {
    fprintf(stderr, "error: cannot open %s\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  // check if second file can be opened and is readable
  f2 = fopen(argv[2], "r");
  if (f2 == NULL) {
    fprintf(stderr, "error: cannot open %s\n", argv[2]);
    exit(EXIT_FAILURE);
  }

  int m_x, m_y; // number of rows and columns of the matrices
  m_x = checkRows(f1, f2);
  m_y = checkColumns(f1, f2);
  int *marray1 = readMatrices(f1, m_x, m_y);
  int *marray2 = readMatrices(f2, m_x, m_y);

  pid_t pid; // process id's
  int i;
  for(i=0; i<m_y ;i++){ // making column number of forks
    if ((pid = fork()) == -1) {
      perror("fork");
      return EXIT_FAILURE;
    }
    if(pid == 0) break;
  }

  if(pid == 0){ // child process
    calculateRow(marray1, marray2, i, m_x);
  }
  if(pid != 0){ // parent process
    if (waitpid(pid, NULL, 0) == -1) {  // waiting for childrens to finish its process
      perror("wait");
      return EXIT_FAILURE;
    }
    writeMatrix(marray1, m_x, m_y);
  }

  // closing files and releasing mapped memory
  fclose(f1);
  fclose(f2);
  munmap(marray1,getMatrixSize(m_x,m_y));
  munmap(marray2,getMatrixSize(m_x,m_y));

  return EXIT_SUCCESS;
}

int checkRows(FILE* f1, FILE* f2){
  int m1_x, m2_x;
  char buffer1, buffer2;

  fscanf(f1, "%d %c ", &m1_x, &buffer1);
  fscanf(f2, "%d %c ", &m2_x, &buffer2);

  if (m1_x != m2_x) {
    fprintf(stderr, "error: matrices have different row sizes\n");
    exit(EXIT_FAILURE);
  }

  return m1_x;
}

int checkColumns(FILE* f1, FILE* f2){
   int m1_y, m2_y;

  fscanf(f1, "%d", &m1_y);
  fscanf(f2, "%d", &m2_y);

  if (m1_y != m2_y) {
    fprintf(stderr, "error: matrices have different sizes\n");
    exit(EXIT_FAILURE);
  }

  return m1_y;
}

int getMatrixSize(int m_x, int m_y){
  int arr[m_x * m_y];
  size_t size_array = sizeof(arr);
  return size_array;
}

int *readMatrices(FILE* f1, int m_x, int m_y){
  int size_array = getMatrixSize(m_x,m_y);
  int *marray = mmap(NULL, size_array , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);

  for (int i = 0; i < m_x * m_y; i++) {
    fscanf(f1, "%d", &marray[i]);
  }

  return marray;
}

void calculateRow(int *marray1, int *marray2, int i, int rows){
  int help = i * rows - i;
    for(int j=0+help; j<rows+help ;j++){
      marray1[i + j] += marray2[i + j];
    }
}

void writeMatrix(int *marray, int m_x, int m_y){
  int n = 0;
    fprintf(stdout, "%dx%d\n", m_x, m_y);
    for (int i = 0; i < m_x * m_y; i++) {
      fprintf(stdout, "%d\t", marray[i]);
      if(n==(m_x-1)) {
        fprintf(stdout, "\n");
        n = 0;
      }
      else n++;
    }
}