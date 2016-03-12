int a;
int b;

int foo() {
    int result;
    switch(a) {
        case 1:
            switch(b) {
                case 1: 
                    result = 1;
                    break;
                case 2:
                    result = 2;
                    break;
                default:
                    result = 3;
                    break;
            }
            break;
        case 2:
            switch(b) {
                case 1:
                    result = 3;
                    break;
                case 2:
                    result = 6;
                    break;
                default:
                    result = 9;
                    break;
            }
            break;
        default:
            result = 10;
            break;
    }
    return result;
}
