#include <stdio.h>

void stuff(int thing[10][100]) {
}

void stuff2(int thing[10][100], int other_thing) {
}

int main() {
  int tab[10][100];
  int b=sizeof(int);
  int c=3;
#pragma smecy stream_loop
  while (1) {
#pragma smecy stage arg(1,out) map(PE,1)
      stuff(tab);
#pragma smecy stage arg(1,inout) map(OpenCL)
      stuff2(tab,c);
#pragma smecy stage arg(1,in) map(EdkDSP)
      stuff(tab);
  }

  return 0;
}
