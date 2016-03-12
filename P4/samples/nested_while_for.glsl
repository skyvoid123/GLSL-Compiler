int foo() {
    int i;
    i = 0;
    int j;
    j = 0;
    int k;
    k = 0;
    int result;
    result = 0;
    for (i = 0; i < 3; i++) {
        while(j < 3) {
            while(k < 3) {
                result += i + j + k;
                k++;
            }
            j++;
        }
    }
    return result;
}
