#include <stdio.h>

int * ptr;

int const * getPTR(){return ptr;}

int main(){
  int i;
  ptr=&i;
  *ptr=1;
  int const * ptr2=getPTR();
  printf("%d\n", *ptr2);
  *ptr2=2;
  printf("%d\n", *ptr2);
  return 0;
}
