int foo(int i) {
  switch(i) {
    case 1: break;
    case 2: i += 1;
      break;
    case 3: i = 1;
    default: return i;
  }
  return i;
}
