int foo() {
    int i;
    i = 0;
    int result;
    result = 0;
    while (true) {
        int j;
        j = 0;
        while (true) {
            int k;
            k = 0;
            while(true) {
                result += i + j + k;
                if (k == 3) {
                    break;
                }
                k++;
            }
            if ( j == 3) {
                break;
            }
            j++;
        }
        if (i == 3) {
            break;
        }
        i++;
    }
    return result;
}
        
