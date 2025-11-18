#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MEM_CELLS 30000

#define BRACKET_STACK_SIZE 256


void strip_ext(char *fname)
{
    char *end = fname + strlen(fname);

    while (end > fname && *end != '.') {
        --end;
    }

    if (end > fname) {
        *end = '\0';
    }
}


int assemble(char *cur_folder, char *filename, char *asm_name, char* exe_name){

    strcpy(exe_name,filename);
    strcat(exe_name,".exe");

    char cmd[1024];
    //sprintf(cmd,"fasmw17332\\FASM.EXE %s %s",asm_name,exe_name); //dont call directly 

    sprintf(cmd,"assemble.bat %s %s",asm_name,exe_name);

    printf("Executing: %s\n",cmd);

    system(cmd);

    return 0;
}

int compile(char* filename, char* asm_name, int len, char *input){

    
    strcpy(asm_name,filename);
    strcat(asm_name,".asm");


    // Create a file
    FILE *fptr = fopen(asm_name, "w");

    int bracket_count = 0;
    int brackets[BRACKET_STACK_SIZE];
    int bracket_ptr = 0;

    int label = 0;




    fprintf(fptr, "format pe console\n\ninclude 'win32ax.inc'\n\nentry start\n\n"); //init slug

    fprintf(fptr, "section '.data' data readable writable\n\n");



    fprintf(fptr,"strPause db 'pause',0\n\n");

    //MAKE SURE THIS IS DECLARED LAST OR IT WILL TAKE UP SPACE IN THE EXE
    fprintf(fptr,"Memory rb %d\n\n",MEM_CELLS); //define array length

    fprintf(fptr, "section '.text' code readable executable\n\n");


    fprintf(fptr,"start:\n");
    //code here

    fprintf(fptr,"\txor ebx, ebx\n"); //ensure ebx is set to 0
    int index = 0;

    char strbuf[156];

    while (index < len)
    {
        switch (input[index])
        {
        case '+': 

            int inc = 1;
            while (input[++index] == '+')
            {
                inc++;
            }
            index--;

            if(inc == 1){
                fprintf(fptr,"\tinc byte [Memory + ebx]\n");
            }
            else
            {
                fprintf(fptr,"\tadd byte [Memory + ebx], %d\n",inc);
            }
            
            break;
        case '-': 
        
            int dec = 1;
            while (input[++index] == '-')
            {
                dec++;
            }
            index--;

            if(dec == 1){
                fprintf(fptr,"\tdec byte [Memory + ebx]\n");
            }
            else
            {
                fprintf(fptr,"\tsub byte [Memory + ebx], %d\n",dec);
            }

            
            
            break;


        case '.': 

            fprintf(fptr,"\tcinvoke putchar, dword [Memory + ebx]\n");

            break; // print
        case ',': 
            fprintf(fptr,"\tcinvoke getchar\n");
            fprintf(fptr,"\tmov [Memory + ebx], al\n");
            break; //read in

        case '>': //move mem ptr right

            int ir = 1;
            while (input[++index] == '>')
            {
                ir++;
            }
            index--;

            if(ir == 1){
                fprintf(fptr,"\tinc ebx\n");
            }
            else
            {
                fprintf(fptr,"\tadd ebx, %d\n",ir);
            }

            break;
        case '<': //move mem ptr left

            int il = 1;
            while (input[++index] == '<')
            {
                il++;
            }
            index--;
            if(il == 1){
                fprintf(fptr,"\tdec ebx\n");
            }
            else
            {
                fprintf(fptr,"\tsub ebx, %d\n",il);
            }
            
            break;
        case '[': //jump past matching ] if cell is zero

            int fb = 1;
            while (input[++index] == '[')
            {
                fb++;
            }
            index--;

            fprintf(fptr,"\tmov al, byte [Memory + ebx]\n");
            fprintf(fptr,"\ttest al, al\n");
            fprintf(fptr,"\tjz LB%d\n",bracket_count);

            fprintf(fptr,"LF%d:\n",bracket_count);

            
            for (size_t i = 0; i < fb; i++)
            {
                brackets[bracket_ptr++] = bracket_count; //inc bracket counter and add to stack
            }
            bracket_count++;
            
            break;
        case ']': //jump back to matching [ if cell is not zero

            fprintf(fptr,"\tmov al, byte [Memory + ebx]\n");
            fprintf(fptr,"\ttest al, al\n");

            fprintf(fptr,"\tjnz LF%d\n",brackets[--bracket_ptr]);
            if(bracket_ptr == 0 || brackets[bracket_ptr] != brackets[bracket_ptr - 1]){
                fprintf(fptr,"LB%d:\n",brackets[bracket_ptr]);
            }
            

            break;
        }

        index++; //increment index to next cell
    }
    fprintf(fptr,"\n\tcinvoke putchar,0Ah\n\tcinvoke system,strPause\n");


    fprintf(fptr, "\tpush 0\n\tcall [ExitProcess]\n\n"); //return at end of program

    char *exitproc = 
"section '.idata' import data readable \n\
\n\
library kernel32,'kernel32.dll',\\ \n\
    msvcrt,'msvcrt.dll' \n\
\n\
import kernel32,\\ \n\
          ExitProcess,'ExitProcess' \n\
import msvcrt,\\ \n\
          system,'system',\\ \n\
          putchar,'putchar',\\ \n\
          getchar,'getchar',\\ \n\
          printf,'printf' ";

    fprintf(fptr,exitproc);

    // Close the file
    fclose(fptr); 
    return 0;
}



int main(int argc, char **argv){
    if(argc == 1){
        printf("please input a file\n");
        return -1;
    }

    char name[256];
    strcpy(name,argv[1]);
    strip_ext(name);

    FILE *f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;

    printf("read:\n%s\n\n",string);

    char asm_name[256];
    compile(name,asm_name,fsize,string);

    printf("Compiled\n");


    char exe_name[256];
    assemble(argv[0], name,asm_name, exe_name);

    printf("Assembled into %s\n",exe_name);


    free(string);
    return 0;
}