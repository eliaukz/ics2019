#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define NR_TOKENS 32
// this should be enough
static char buf[65536];
int pos = 0;
int nr_tokens = 0;

uint32_t choose(uint32_t n) { return rand() % n; }

void gen_num() {
  uint32_t num = rand() % 1000;
  char buffer[40];
  snprintf(buffer, sizeof(buffer), "%d", num);
  int len = strlen(buffer);
  strcpy(&buf[pos], buffer);
  pos += len;
  nr_tokens++;
}


void gen(char c){
  if (c == '(') buf[pos] = '(';
  if (c == ')') buf[pos] = ')';
  pos++;
  nr_tokens++;
}


 void gen_rand_op(){
  switch (choose(4)){
    case 0 : buf[pos]='+';break;
    case 1 : buf[pos]='-';break;
    case 2 : buf[pos]='*';break;
    case 3 : buf[pos]='/';break;
  }
  pos++;
  nr_tokens++;
}



 void gen_rand_expr() {
  // if (nr_tokens + 3 >= NR_TOKENS) return;
  switch (choose(3)){
    case 0:  if (nr_tokens + 1 >= NR_TOKENS) return;
             gen_num();  break;
    case 1:  if (nr_tokens + 3 >= NR_TOKENS) return;
             gen('(');  gen_rand_expr();  gen(')');  break;
    case 2:  if (nr_tokens + 3 >= NR_TOKENS) return;
             gen_rand_expr();  gen_rand_op();  gen_rand_expr();  break;
  }
}

static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    pos = 0;
    nr_tokens = 0;
    memset(buf, 0, sizeof(buf));
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
