/* this program takes two files as arguments and compares them
 * returns 0 or 1 based on if there is too much difference between the colors
 * 0 -> pretty similar
 * 1 -> too much difference
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {

    if(argc != 3) {
        fprintf(stderr, "ERROR: Usage: %s <file1> <file2>\n", argv[0]);
        return 1;
    }
    FILE *f1 = fopen(argv[1], "rb");
    FILE *f2 = fopen(argv[2], "rb");
    if(f1 == NULL) {
        fprintf(stderr, "ERROR: could not open file: %s\n", argv[1]);
        return 1;
    }
    if(f2 == NULL) {
        fprintf(stderr, "ERROR: could not open file: %s\n", argv[2]);
        return 1;
    }

    int b1, b2;

    b1 = fgetc(f1);b2 = fgetc(f2);  // P
    if(b1 != b2)    {return 1;}

    b1 = fgetc(f1);b2 = fgetc(f2);  // 6
    if(b1 != b2)    {return 1;}

    fgetc(f1);fgetc(f2);    // \n
    fscanf(f1, "%d", &b1);fscanf(f2, "%d", &b2);    // width
    if(b1 != b2)    {return 1;}
    fgetc(f1);fgetc(f2);    // ' '
    fscanf(f1, "%d", &b1);fscanf(f2, "%d", &b2);    // height
    if(b1 != b2)    {return 1;}
    fgetc(f1);fgetc(f2);    // ' '
    fscanf(f1, "%d", &b1);fscanf(f2, "%d", &b2);    // 255
    if(b1 != b2)    {return 1;}
    fgetc(f1);fgetc(f2);    // \n

    int lineIndex = 0;
    int byteIndexAtLine = 0;

    int difference_50 = 0;
    int difference_25 = 0;
    int difference_10 = 0;
    int difference_0 = 0;
    while(b1 != EOF && b2 != EOF) {
        b1 = fgetc(f1);
        b2 = fgetc(f2);
        if(abs(b1 - b2) > 50) {
            difference_50++;
        } else if(abs(b1 - b2) > 25) {
            difference_25++;
        } else if(abs(b1 - b2) > 10) {
            difference_10++;
        } else if(abs(b1 - b2) < 10) {
            difference_0++;
        }
        byteIndexAtLine++;
        if(byteIndexAtLine % 16 == 0) {
            byteIndexAtLine = 0;
            lineIndex++;
        }
    }
    printf("differences:\n");
    printf("    < 10: %d\n", difference_0);
    printf("    > 10: %d\n", difference_10);
    printf("    > 25: %d\n", difference_25);
    printf("    > 50: %d\n", difference_50);
    if(difference_50 > 0) {
        return 1;
    }
    if(difference_0 < difference_10 || difference_10 < difference_25) {
        return 1;
    }
    return 0;
}