#include <stdio.h>
#include <stdlib.h>

//	並べ替え基準を示す関数(昇順)
int cmpnum(const void *n1, const void *n2)
{
    if (*(unsigned char *)n1 > *(unsigned char *)n2)
    {
        return 1;
    }
    else if (*(unsigned char *)n1 < *(unsigned char *)n2)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int main(void)
{
    //	並べ替え対象の配列データ
    unsigned char num[] = {'a', 'c', 'b', 'd', 'f', 'e', 'z'};
    int i;

    //	クリックソートによる並べ替え
    qsort(num, sizeof(num) / sizeof(num[0]), sizeof(unsigned char), cmpnum);

    //	並べ替え結果の表示
    for (i = 0; i < sizeof(num) / sizeof(num[0]); i++)
    {
        printf("%d\n", num[i]);
    }

    return 0;
}
