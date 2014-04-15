MODULES := sample submodules-sample

.PHONY: all clean

all:
	@for i in $(MODULES); do \
		$(MAKE) -C $$i; \
	done

clean:
	@for i in $(MODULES); do \
		$(MAKE) -C $$i clean; \
	done