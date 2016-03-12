int foo() {
    int result;
    result = 0;
    int i;
    for (i = 0; i < 3; i++) {
        int j;
        for (j = 0; j < 3; j++) {
            int k;
            for (k = 0; k < 3; k++) {
                result += j + k + i;
            }
        }
    }
    return result;
}
