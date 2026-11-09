#include <stdio.h>
#include <string.h>
int main() {
  //defne a big buffer
  char bigbuffer[1024];
  
  char smallstring1[8]="write1\n"; // \0 behind
  char smallstring2[8]="write2\n";
  printf("smallstring1=%s\n", smallstring1);//length 7
  printf("len(smallstring1)=%d\n", strlen(smallstring1));//length 7
  printf("smallstring2=%s\n", smallstring2);
  printf("len(smallstring2)=%d\n", strlen(smallstring2));//length 7
  
  int sizeofstring = 0;
  sizeofstring += strlen(smallstring1);
  strcpy(bigbuffer,smallstring1);
  //bigbuffer[sizeofstring]='\n';
  //sizeofstring++;
  strcpy(bigbuffer+sizeofstring,smallstring2);
  sizeofstring += strlen(smallstring2);
  //bigbuffer[sizeofstring]='\n';
  //sizeofstring++;
  bigbuffer[sizeofstring]='\0';
  sizeofstring++;
  
  //printf("sizeofstring=%d\n", sizeofstring);
  printf("%s",bigbuffer);
  
  return 0;
}
