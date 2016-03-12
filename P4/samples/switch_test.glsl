int foo(int i) {
  switch(i) {
    case 1: break;
    case 2: i += 1;
      break;
    case 3: i = 3;
    break;
    case 4: i = 15;
    default: i = 3;
  }
  return i;
}
