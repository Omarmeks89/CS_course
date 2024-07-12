C=gcc
WFLAGS=-g -Wall -Wextra -Werror -Wformat
STD=c89

ESTRM?=2
MSTRM?=1

SOURCE=
OUT_STREAMS=-DESTRM=$(ESTRM) -DMSTRM=$(MSTRM)

bsa_ptrs_test: $(SOURCE)
	$(C) $(WFLAGS) $(OUT_STREAMS) -std=$(STD) -lm test/unit/test_bsa_handle_invalid_pointers.c -o bsa_ptrs_test

bsa_hierarhies_test: $(SOURCE)
	$(C) $(WFLAGS) $(OUT_STREAMS) -std=$(STD) -lm test/unit/test_hierarhies_creation.c -o bsa_hierarhies_test

clear:
	rm -rf *_test *.txt
