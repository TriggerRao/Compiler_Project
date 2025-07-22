#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lexer.h"
#include "parser.h"

int main()
{   
   // char* fileName = "test_cases_for_lexer/t2.txt";
    char* fileName = "parser_test_cases/t5.txt";
    int keepRunning = 1;
    populateKeywords();
    initialiseParser();
    while(keepRunning){
        printf("Enter your choice: ");
        int choice;
        scanf("%d", &choice);
        switch (choice)
        {
            case 0:
                printf("Exiting the code....\n");
                keepRunning = 0;
                break;
            case 1:
                removeComments(fileName);
                break;
            case 2:
                printLexemes(fileName);
                break;
            case 3:
                printLexemesinFile(fileName);
                parse_input_get_tree();
                printTree(root, "", 1);
                printTreeInOrder(root);
                break;
            case 4:
                //timeTaken(fileName);
                break;
            default:
                keepRunning=0;
                break;
        }
        if(keepRunning == 0){
            break;
        }
    }
}