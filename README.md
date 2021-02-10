scexec
======
Arbitrary data executor, mostly useful for shellcode testing.

Usage
-----
```
scexec32 [options] [file]?
	-h        help
	-v        show data information
	-q        show data information and quit
	-f        fork prior to data execution
	-u [id]   set the effective user id
	-g [id]   set the effective group id
```

Compilation
-----------
The `make` variable `ELFBIT` defines the program's bit size (passed on to `gcc -m`). The default value is set to 32.

* Compile both 32 and 64-bit versions
```
user@host:~/scexec$ make
[cc] -Ofast obj/shannon32.o
[cc] -O2 obj/main32.o
[cc] -Wall -fno-stack-protector -Isrc/include -m32 -z execstack scexec32
user@host:~/scexec$ make ELFBIT=64
[cc] -Ofast obj/shannon64.o
[cc] -O2 obj/main64.o
[cc] -Wall -fno-stack-protector -Isrc/include -m64 -z execstack scexec64
user@host:~/scexec$ ls scexec*
scexec32 scexec64
```

* Optionally make `scexec` setuid (in order to use options `-u` and `-g` as any user). Note that this is very dangerous, as this program arbitrarily executes any input data. This should only to be used in controled environments to facilitate the workflow of shellcode testing.
```
user@host:~/scexec$ sudo make setuid
[sudo] password for user:
[chown] root:root scexec32
[chmod] u+s scexec32
```
