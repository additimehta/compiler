#!/bin/bash

mkdir -p tests

write() {
cat << 'EOF' > "tests/$1"
$2
EOF
}

# -----------------------
# 1. no-param function
# -----------------------
write test1.wlp4 "
int wain(int a, int b) {
  println(foo());
  return 0;
}

int foo() {
  return 7;
}
"

# -----------------------
# 2. one parameter
# -----------------------
write test2.wlp4 "
int wain(int a, int b) {
  println(double(a));
  return 0;
}

int double(int x) {
  return x + x;
}
"

# -----------------------
# 3. three parameters
# -----------------------
write test3.wlp4 "
int wain(int a, int b) {
  println(add3(a, b, 10));
  return 0;
}

int add3(int x, int y, int z) {
  return x + y + z;
}
"

# -----------------------
# 4. local shadows param
# -----------------------
write test4.wlp4 "
int wain(int a, int b) {
  println(foo(a));
  return 0;
}

int foo(int x) {
  int x = 10;
  return x;
}
"

# -----------------------
# 5. multiple locals
# -----------------------
write test5.wlp4 "
int wain(int a, int b) {
  println(test());
  return 0;
}

int test() {
  int a = 3;
  int b = 4;
  int c = 5;
  return a + b + c;
}
"

# -----------------------
# 6. recursion factorial
# -----------------------
write test6.wlp4 "
int wain(int a, int b) {
  println(fact(4));
  return 0;
}

int fact(int n) {
  if (n == 0) {
    return 1;
  }
  return n * fact(n - 1);
}
"

# -----------------------
# 7. while loop
# -----------------------
write test7.wlp4 "
int wain(int a, int b) {
  println(countdown(5));
  return 0;
}

int countdown(int n) {
  int c = 0;
  while (n > 0) {
    c = c + 1;
    n = n - 1;
  }
  return c;
}
"

# -----------------------
# 8. I/O mixed
# -----------------------
write test8.wlp4 "
int wain(int a, int b) {
  putchar(65);
  println(foo());
  return 0;
}

int foo() {
  return 99;
}
"

# -----------------------
# 9. nested procedures
# -----------------------
write test9.wlp4 "
int wain(int a, int b) {
  println(A(3));
  return 0;
}

int A(int x) {
  return B(x + 1);
}

int B(int y) {
  return C(y + 1);
}

int C(int z) {
  return z * 2;
}
"

# -----------------------
# 10. argument evaluation order
# -----------------------
write test10.wlp4 "
int wain(int a, int b) {
  println(sum(a*2, b*3, 7));
  return 0;
}

int sum(int x, int y, int z) {
  return x + y + z;
}
"

echo "All tests generated in ./tests/"

