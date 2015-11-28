/*! \file computePatchOpt.c
 *  \brief     This implements the computePatchOpt program.
 *  \author    Walter Castro
 *  \author    Lucas Bualo
 *  \version   1.0
 *  \date      2015
 *  \warning   Usage: computePatchOpt originalFile resultFile
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define LINE_MAX_SIZE 100

/**
 *Maximum number of unsigned int, wich is going to be taken as infinite
 */
const unsigned int INF = UINT_MAX;

/**
 *Number of lines in the input file
 */
unsigned int n = 0; 

/**
 *Number of lines in the result file
 */
unsigned int m = 0; 

/**
 *array that will get the result file information
 */
char* F2 = NULL; 

/**
 *array that will get the input file information
 */
char* F1 = NULL; 

/**
 *array that will save the last instruction for each patch
 *from Ai to Bj
 */
char* instruction = NULL;

/**
 *array that will save the Patch before saving the file
 */
char* patch = NULL; 

/**
 *array that will safe the optimal costs
 */
unsigned int* optimal = NULL; 

/**
 *Variables for loops
 */
int p = 0;
int k;
int l;
int w;

/*!
 * Function that gets the length of the line we send.
 * 
 * @param line: The string we will calculate the length.
 * \returns The length of the line
 */

unsigned int Length(char* line) {
  unsigned int p = 0;

  while (line[p] != '\n') {
    p++;
  }
  return p;
}


/*!
 * Function that fills the optimal patch cost matrix and
 * the optimal patch last instruction for each case.
 * 
 * @param  i: Unsigned integer to the indexation of the input file.
 * @param  j: Unsigned integer to the indexation of the result file.
 * \returns The length of the line
 */
unsigned int findPatch (unsigned int i, unsigned int j) {
  int p = 0;
  if (optimal[(m+1) * i + j] != INF) {
    //If it is already calculated, we return the value already stocked
    return optimal[(m+1) * i + j];
  
  } else {
    if ((i==0) && (j==0)) {
      //The limit constrained for minimum i and j values
      optimal[(m+1) * i + j] = 0;
    
    } else {
      //If the output has no more lines, then we have to erase all the rest in the input
      if (j == 0) { 
      	if (i == 1) {
      	  optimal[(m+1) * i + j] = 10;
      	  sprintf(&instruction[(m+1) * LINE_MAX_SIZE * i+ LINE_MAX_SIZE * j], "d %u\n", i);
      	
        }	else {
      	  optimal[(m+1) * i + j] = 15;
      	  sprintf(&instruction[(m+1) * LINE_MAX_SIZE * i+ LINE_MAX_SIZE * j], "D 1 %u\n", i);
      	}

      } else if (i == 0) {
        //If the input has no more lines, then we have to add all the rest of the output

      	int costAdd = 0;
      	for (p=0; p<j; p++) {
      	  costAdd +=  10 + Length (&F2[(LINE_MAX_SIZE) * p]);
      	  sprintf(&instruction[(m+1) * LINE_MAX_SIZE * i+ LINE_MAX_SIZE * j], "+ %u\n%s", i, &F2[(LINE_MAX_SIZE) * p]);
      	}
      	optimal[(m+1) * i + j] = costAdd;
      
      } else if (strcmp(&F1[(LINE_MAX_SIZE) * (i-1)], &F2[(LINE_MAX_SIZE) * (j-1)]) == 0) {
	      optimal[(m+1) * i + j] = findPatch (i-1, j-1);
      
      } else {
      	unsigned int Subs = findPatch (i-1, j-1) + 10 + Length(&F2[(LINE_MAX_SIZE) * (j-1)]); //Cost of Substitution
      	unsigned int Add = findPatch (i, j-1) + 10 + Length(&F2[(LINE_MAX_SIZE) * (j-1)]); //Cost of addition
      	unsigned int* DelMul = (unsigned int*) malloc(i*sizeof(unsigned int)); //Cost of Delete multiple lines
      	
        DelMul[0] = findPatch (i-1, j) + 10; //Cost of Delete 1 line
      	for (p = 1; p<i; p++) {
      	  DelMul[p] = findPatch (i-(p+1), j) + 15; //p+1 is the number of lines we are going to delete
      	}
      	
        unsigned int DelMax = 0; //The number of lines to delete from the i-th line
      	unsigned int Del = INF; //Cost of deleting to find the maximum
      	
        for (p = 0; p<i; p++) {
      	  if (Del > DelMul[p]) {
      	    Del = DelMul[p];
      	    DelMax = p+1;
      	  }
      	}

      	if (Subs < Add) {
      	  
          if (Subs < Del) {
      	    optimal[(m+1) * i + j] = Subs;
      	    sprintf(&instruction[(m+1) * LINE_MAX_SIZE * i+ LINE_MAX_SIZE * j], "= %u\n%s", i, &F2[(LINE_MAX_SIZE) * (j-1)]);
      	  
          } else {
      	    optimal[(m+1) * i + j] = Del;
      	  
            if (DelMax == 0){ 
      	      sprintf(&instruction[(m+1) * LINE_MAX_SIZE * i+ LINE_MAX_SIZE * j], "d %u\n", i);
      	    
            } else {
      	      sprintf(&instruction[(m+1) * LINE_MAX_SIZE * i+ LINE_MAX_SIZE * j], "D %u %u\n", i-1, DelMax);
      	    }
      	  }

      	}	else {
          
          //Subs > Add
      	  if (Add < Del) {
      	    optimal[(m+1) * i + j] = Add;
      	    sprintf(&instruction[(m+1) * LINE_MAX_SIZE * i+ LINE_MAX_SIZE * j], "+ %u\n%s", i, &F2[(LINE_MAX_SIZE) * (j-1)]);
      	  
          } else {
      	    optimal[(m+1) * i + j] = Del;
      	  
            if (DelMax == 0){ 
      	      sprintf(&instruction[(m+1) * LINE_MAX_SIZE * i+ LINE_MAX_SIZE * j], "d %u\n", i);
      	  
            } else {
      	      sprintf(&instruction[(m+1) * LINE_MAX_SIZE * i+ LINE_MAX_SIZE * j], "D %u %u\n", i-1, DelMax);
      	    }
      	  }
      	}
        free(DelMul);
      }
    }

    return optimal[(m+1) * i + j];
  }
}

/*!
 * Function that generates the optimal patch.
 * It creates a new file in the main folder called Patch,
 * that has the instructions for creating the result
 * file when it is applied to the original file.
 * 
 * @param originalFile: File pointer to the originalFile.
 * @param resultFile: File pointer to the targetFile.
 * \returns the optimal cost of the Patch saved.
 */
int Patch_solver(FILE *originalFile, FILE *resultFile) {
  char buffer[LINE_MAX_SIZE]; // should suffice for (instruction, 32-bit int)

  //We get the value of n (max number of lines in the original file)
  while (fgets(buffer,LINE_MAX_SIZE, originalFile)!=NULL) {
    n +=1;
  }

  //We find the value of m (max number of lines in the result file)
  while (fgets(buffer,LINE_MAX_SIZE, resultFile)!=NULL) {
    m +=1;
  }

  rewind(originalFile);
  rewind(resultFile);

  //We start the allocation of F1 and F2
  F2 = (char*) malloc(m*LINE_MAX_SIZE*sizeof(char));
  F1 = (char*) malloc(n*LINE_MAX_SIZE*sizeof(char));

  //We create the matrix for the optimal cost of the patch
  optimal = (unsigned int*) malloc((n+1)*(m+1)*sizeof(unsigned int));  

  //As we are finding a minimum value, we initialize the matrix value in the highest possible value
  for (k=0; k < (n+1); k++) {
    for (l=0; l < (m+1); l++) {
     optimal[(m+1) * k + l] = INF;
    }
  }

  //We create the matrix for the last instruction found for the patch
  instruction = (char*) malloc((n+1)*(m+1)*LINE_MAX_SIZE*sizeof(char));

  //We copy the original file into a matrix
  for (p=0; p<n; p++) {
    fgets(&F1[(LINE_MAX_SIZE) * p],LINE_MAX_SIZE,originalFile);
  }

  //We copy the result file into a matrix
  for (p=0; p<m; p++) {
    fgets(&F2[(LINE_MAX_SIZE) * p],LINE_MAX_SIZE,resultFile);
  }

  //We call the function to find the optimal cost of the patch
  k = findPatch (n, m);

  patch = (char*) malloc((n+m)*LINE_MAX_SIZE*sizeof(char));

  p = 0;
  k = n;
  l = m;

  //We determine the instructions of the optimal patch and save them
  while ((k>0) || (l>0)) { 
    sprintf(&patch[(LINE_MAX_SIZE) * p],"%s",&instruction[(m+1) * LINE_MAX_SIZE * k+ LINE_MAX_SIZE * l]);
    p=p+1;
    char inst = instruction[(m+1) * LINE_MAX_SIZE * k+ LINE_MAX_SIZE * l];

    switch (inst) {
      case 'd': 
        k = k-1;
        break;

      case 'D': 
        w = 9; //this would be enought for the digits of the D intsruction
        unsigned int pos =  Length(&instruction[(m+1) * LINE_MAX_SIZE * k+ LINE_MAX_SIZE * l]); //We get the position of the \n character
        char Deletes [10] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}; //We start an array with spaces
       
        //We read the number of lines Deleted in the D instruction
        while (instruction[(m+1) * LINE_MAX_SIZE * k+ LINE_MAX_SIZE * l + pos] != ' ') { 
        	Deletes [w] = instruction[(m+1) * LINE_MAX_SIZE * k+ LINE_MAX_SIZE * l + pos];
        	w--;
        	pos--;
        }

        w = atoi(Deletes);
        k = k - w;
        break;

      case '=':
        k = k-1;
        l = l-1;
        break;

      case '+':
        l = l-1;
        break;

      default:
        k = k-1;
        l = l-1;
        break;
    }
  }

  //This is fori writing the file
  FILE *patchFile;
  patchFile = fopen("./Patch","w");

  while (p >= 0) {
    fprintf(patchFile,"%s",&patch[(LINE_MAX_SIZE) * p]);
    p--;
  }

  fclose(patchFile);
  printf("The optimal patch cost is: %u",optimal[(m+1) * n + m]);
  return optimal[(m+1) * n + m];
}


/**
 * Main function
 * \brief Main function
 * \param argc  A count of the number of command-line arguments
 * \param argv  An argument vector of the command-line arguments.
 * \warning Must be called with two filenames patchFile, originalFile as commandline parameters and in the given order.
 * \returns { 0 if succeeds, exit code otherwise}
 */
int main (int argc, char *argv[]) {
  FILE *originalFile; 
  FILE *resultFile;
  
  if(argc<3){
    fprintf(stderr, "!!!!! Usage: ./PatchSolver originalFile resultFile !!!!!\n");
    exit(EXIT_FAILURE); /* indicate failure.*/
  }
  
  originalFile = fopen(argv[1] , "r" );
  resultFile = fopen(argv[2] , "r" );
  
  Patch_solver(originalFile, resultFile);

  free(patch);
  free(optimal);
  free(instruction);
  free(F2);
  free(F1);

  fclose(originalFile);
  fclose(resultFile);
  return 0;
}
