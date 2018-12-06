SRC=matches.doxyfile
OUTDIR=outputdoc
HTML=$(OUTDIR)/html/index.html

.PHONY: clean

all: $(HTML)
	@echo HTML: $(HTML)

$(HTML):
	@echo HTML
	doxygen $(SRC)

clean:
	@rm -r $(OUTDIR) 2> /dev/null || true

