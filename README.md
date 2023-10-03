## DCC Converter from decimal

![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
Small app, that convert numbers from decimal system
to bin, oct and hex system.

### Compilation

Compiler version:
```bash
gcc (Ubuntu 11.4.0-1ubuntu1~22.04) 11.4.0
Copyright (C) 2021 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

No special options required. Use only:
```bash
gcc -Wall -g dc.c -o dcc
```

I use name `dcc` but you can use any other you like.
Make a symlink:
```bash
sudo ln -s ${pwd}/[app_name] /usr/local/bin/
```

To see short manual use `dcc` command without params:
```bash
[dcc] Converter for decimal numbers to hex, oct, bin systems.
Version: 0.1.0;
Max positive num: 18446744073709551615 (2^64-1);
Max negative num: 9223372036854775808 (2^63);
Supported operations with int and float numbers, separator for
float numbers is '.' .
Commands:

	 -b[B] - convert number to binary system;

	 -o[O] - convert number to octal system;

	 -h[H] - convert number to hex system;

	 -l[L] - convert hex numbers [A-F] to lower. (capitalized by default)

Presets:

	 --prec=[number] - set precision for float numbers representation.
	                   Support number from 0 to 32 (numbers bigger as 32
	                   will be converted to 32).

Signs:

	 (Not implemented this version)
	 [+] for positive nums;
	 [-] for negative nums.
```

For representation hex numbers you can use option `-l[L]` to
convert literal hex numbers to lower case (is upper case by default).
So use command `dcc -hl 10241024`:
```bash
0x9c4400
```
or by default `dcc -h 10241024`:
```bash
0x9C4400
```

For convertation float numbers use `.` as separator. You can use
preset `--prec` to set count of numbers after separator.
(this version round not supported).
Maximal precision is 32, so if you set number bigger, it will
be converted to 32. Sample `dcc -h --prec=12 10241024.10241024`:
```bash
0x9C4400.1A378EB79354
```

## Valgrind

Profiler results. Command `valgrind -s --track-origins=yes --leak-check=full`:
```bash
==22361== Memcheck, a memory error detector
==22361== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==22361== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==22361== Command: ./dcc -b 18441044073709551615
==22361== 
0b1111111111101011101111111110000101010100110001111011111111111111
==22361== 
==22361== HEAP SUMMARY:
==22361==     in use at exit: 0 bytes in 0 blocks
==22361==   total heap usage: 3 allocs, 3 frees, 100 bytes allocated
==22361== 
==22361== All heap blocks were freed -- no leaks are possible
==22361== 
==22361== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

`valgrind -s --track-origins=yes --leak-check=full ./dcc -b --prec=32 18441044073709551615.1844166618816661`
One more check (with big float number):
```bash
==22381== Memcheck, a memory error detector
==22381== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==22381== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==22381== Command: ./dcc -b --prec=32 18441044073709551615.1844166618816661
==22381== 
0b1111111111101011101111111110000101010100110001111011111111111111.00101111001101011110111000101011
==22381== 
==22381== HEAP SUMMARY:
==22381==     in use at exit: 0 bytes in 0 blocks
==22381==   total heap usage: 5 allocs, 5 frees, 236 bytes allocated
==22381== 
==22381== All heap blocks were freed -- no leaks are possible
==22381== 
==22381== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

## About code quality

Work in progress))))))

## Bugs & errors

You can contact me: r5railmodels@gmail.com

