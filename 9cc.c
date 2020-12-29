#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

//　トークンの種類
typedef enum{
  TK_RESERVED,
  TK_NUM,
  TK_E0F,
} TokenKind;

typedef struct Token Token;

//　トークン型
struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
};

//　現在着目しているトークン
Token *token;

//　エラーを報告するための関数
//　printfと同じ引数をとる
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 入力プログラム
char *user_input;

// エラー箇所を報告する
void error_at (char *loc, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号の時には
// トークンを１つ読み進めて真を返す
// それ以外の時には偽を返す
bool consume(char op){
  if (token->kind != TK_RESERVED || token->str[0] != op)
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待している記号の時にはトークンを１つ読み進める
// それ以外の場合にはエラーを報告する
void expect(char op){
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error("'%c'ではありません", op);
  token = token->next;
}

// 次のトークンが数値の場合,　トークンを１つ読み進めてその数値を返す
// それ以外の場合にはエラーを報告する
int expect_number(){
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof(){
  return token->kind == TK_E0F;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}
// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p){
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p){
    // 空白文字をスキップ
    if (isspace(*p)){
      p++;
      continue;
    }

    if (*p == '+' || *p == '-'){
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)){
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(cur->str, "トークナイズできません");
    //error("トークナイズできません");
  }

  new_token(TK_E0F, cur, p);
  return head.next;
}


int main(int argc, char **argv){
  if (argc != 2){
    fprintf (stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  // トークナイズする
  token = tokenize(argv[1]);

  // アセンブリの前半部分を出力
  printf (".globl main\n");
  printf("main:\n");

  // 式の最初は数でなければならないのでそれをチェック
  // 最初のmov命令を出力
  printf("  mov x0, %d\n", expect_number());
  
  // `+＜数＞`あるいは`-＜数＞`というトークンの並びを消費しつつ
  // アセンブリを出力
  while (!at_eof()) {
    if (consume('+')){
      printf("  add x0, x0, %d\n", expect_number());
      continue;
    }

    expect('-');
    printf("  sub x0, x0, %d\n", expect_number());
    }


  printf("  ret\n");
  return 0;
}


