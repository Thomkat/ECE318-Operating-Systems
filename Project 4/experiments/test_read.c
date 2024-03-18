#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    FILE *file1, *file2;
    char buffer1[4096];
    char buffer2[12288];

	file1 = fopen("../filesystem/mountdir/testfile1.txt", "r");
	file2 = fopen("../filesystem/mountdir/testfile2.txt", "r");

    if(!file1 || !file2){
		printf("Error opening files\n");
		return 1;
	}

    /*File 1 is supposed to be 4096 bytes and file 2 is supposed to be 12288 bytes*/
    fread(buffer1, 1, 4096, file1);
    printf("\n\n***********FILE 1***********\n\n%s\n", buffer1);
    fread(buffer2, 1, 12288, file2);
    printf("\n\n***********FILE 2***********\n\n%s\n", buffer2);

    fclose(file1);
    fclose(file2);

    return 0;
}