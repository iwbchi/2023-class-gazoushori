#include <stdio.h>
#include <string.h>
 
int main(void) {
    char str1[16] = "Hello World!";
    char str2[16];
    
    // 文字列のコピー
    strcpy_s(str2, str1);
    printf("str2の文字列は: %s\n", str2);
    
    return 0;
}
