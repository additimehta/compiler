#!/bin/bash

mkdir -p tests
i=1

make_test() {
  filename="tests/test$i.wlp4"
  echo "$1" > "$filename"
  echo "Generated $filename"
  i=$((i+1))
}

##############################################
#        PROBLEM 4: PROCEDURE TEST CASES     #
##############################################

############ GOOD PROGRAMS ###################

# 1. Single extra procedure, no params
make_test 'int f() {
  return 1;
}
int wain(int a, int b) {
  return f();
}'

# 2. Procedure returning its parameter
make_test 'int id(int x) {
  return x;
}
int wain(int a, int b) {
  return id(a);
}'

# 3. Two procedures calling each other
make_test 'int g() {
  return 7;
}
int f() {
  return g();
}
int wain(int a, int b) {
  return f();
}'

# 4. Procedure with multiple parameters
make_test 'int add(int x, int y) {
  return x;
}
int wain(int a, int b) {
  return add(a, b);
}'

# 5. Procedure calling itself indirectly
make_test 'int h(int x) {
  return x;
}
int g(int x) {
  return h(x);
}
int wain(int a, int b) {
  return g(a);
}'

# 6. Multiple procedures with locals
make_test 'int f(int x) {
  int y = x;
  return y;
}
int wain(int a, int b) {
  return f(a);
}'

# 7. Procedure calling wain parameters
make_test 'int echo(int x) {
  return x;
}
int wain(int a, int b) {
  return echo(b);
}'

# 8. Valid nested calls
make_test 'int f(int x) {
  return x;
}
int g(int x) {
  return f(x);
}
int wain(int a, int b) {
  return g(a);
}'

############ BAD PROGRAMS ####################

# 1. ❌ Duplicate procedure declaration
make_test 'int f() {
  return 1;
}
int f() {        // duplicate
  return 2;
}
int wain(int a, int b) {
  return f();
}'

# 2. ❌ Calling undeclared procedure
make_test 'int wain(int a, int b) {
  return missing();
}'

# 3. ❌ Calling a variable (variable() is invalid)
make_test 'int wain(int a, int b) {
  int x = 1;
  return x();    // error: x is not a procedure
}'

# 4. ❌ Procedure return type mismatch (should return int)
make_test 'int f(int x) {
  return;        // missing expression
}
int wain(int a, int b) {
  return f(a);
}'

# 5. ❌ Wrong number of arguments
make_test 'int f(int x, int y) {
  return x;
}
int wain(int a, int b) {
  return f(a);   // too few args
}'

# 6. ❌ Too many arguments
make_test 'int f() {
  return 0;
}
int wain(int a, int b) {
  return f(a);   // too many args
}'

# 7. ❌ Wrong argument type (passing pointer when int required)
make_test 'int f(int x) {
  return x;
}
int wain(int a, int* b) {
  return f(b);   // wrong type
}'

# 8. ❌ Procedure declared with void-like return (missing return expr)
make_test 'int f(int x) {
  return;  // invalid
}
int wain(int a, int b) {
  return 0;
}'

# 9. ❌ Procedures cannot contain statements (they must contain only dcls)
make_test 'int f(int x) {
  x = 5;     // invalid statement in procedure
  return x;
}
int wain(int a, int b) {
  return f(a);
}'

# 10. ❌ Call before declaration (WLP4 forbids calling procedures before declaration)
make_test 'int wain(int a, int b) {
  return f();   // f not declared yet
}
int f() {
  return 1;
}'
