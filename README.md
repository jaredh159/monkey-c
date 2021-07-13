# monkey-c

my bumbling attempt to write an intepreter and compiler for thorsten ball's monkey
language in C

[https://interpreterbook.com/](https://interpreterbook.com/)

## usage

```bash
# build, and put the executable in ./.bin/monkey
$ make monkey

# optional, move into $PATH, examples below assume this
$ sudo cp ./.bin/monkey /usr/local/bin

# start an interactive REPL session
$ monkey

# execute a monkey file (file must end in .mky)
$ monkey run fib.mky

# execute a monkey file with the INTERPRETER
$ monkey run -i fib.mky

# execute a monkey file with the COMPILER (this is the default)
$ monkey run -c fib.mky

# measure the time taken during program execution with the `-m` flag:
# if you're interested in the performance, build with optimizations
# by running `OPTIMIZE=true make monkey`
$ monkey run -m fib.mky

# execute an arbitratry snippet of monkey code passed as cli arg:
$ monkey run -e "let x = 1; let y = 2; x + y;"

# run all the tests
$ make test_all
```
