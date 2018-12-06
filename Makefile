
.PHONY: clean doc help

all:
	$(MAKE) -C src/

doc:
	$(MAKE) -C doc/

clean:
	$(MAKE) -C src clean
	$(MAKE) -C doc clean

help:
	@echo "make       - Build matches"
	@echo "make doc   - Build matches documentation"
	@echo "make clean - Remove all generated files"
	@echo "make help  - Show this help"

