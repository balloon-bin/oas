BUILD_DIR=build/analyze/
-include make/base.mk

analyze:
	mkdir z-p reports/static-analysis
	scan-build -o reports/static-analysis/ -plist-html --status-bugs make -rRf make/analyze.mk all

distclean: clean
	rm -rf reports/static-analysis
