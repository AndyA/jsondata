# JSON and JSONPath in C

libjsondata provides data structures with JSON semantics (dictionaries and arrays), JSON 
serialisation and efficient support for JSONPath:

http://goessner.net/articles/JsonPath/

## Build

```shell
$ ./setup.sh && ./configure && make test
```

## Install

```shell
$ sudo make install
$ sudo ldconfig
```

## Getting started

```c
/* hello.c */

#include <jd_pretty.h>

int main(void) {
  scope {
    jd_var *hello_world = jd_njv("{\"hello\":\"world\"}");
    jd_printf("The message is %lJ\n", hello_world);
  }
  return 0;
}
```

```shell
$ gcc -std=c99 -o hello -ljsondata hello.c 
$ ./hello 
The message is {
  "hello": "world"
}
```

Andy Armstrong, andy@hexten.net
