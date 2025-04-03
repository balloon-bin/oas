.PHONY: all clean distclean release debug afl asan msan validate analyze fuzz

debug: 
	make -rRf make/debug.mk all

all: debug release afl asan msan
	

release: 
	make -rRf make/release.mk all

afl:
	make -rRf make/afl.mk all

fuzz:
	make -rRf make/afl.mk fuzz

asan:
	make -rRf make/asan.mk all

msan:
	make -rRf make/msan.mk all

validate: asan msan debug
	./validate.sh

analyze:
	make -rRf make/analyze.mk clean all

clean:
	make -rRf make/release.mk clean
	make -rRf make/debug.mk clean
	make -rRf make/afl.mk clean
	make -rRf make/msan.mk clean
	make -rRf make/asan.mk clean
	make -rRf make/analyze.mk clean
	rm -rf build/

distclean: clean
	make -rRf make/afl.mk distclean
	make -rRf make/analyze.mk distclean
	rm -rf reports/
