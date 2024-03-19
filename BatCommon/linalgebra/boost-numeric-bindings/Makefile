LIBTOPDIR:=../../INCLUDE/
ifndef CDMS_CAP_MAKE
LIBTOPDIR:=../../../BUILD/
endif
INCFILES := $(addprefix $(LIBTOPDIR)linalgebra/boost-numeric/include/,$(shell find boost -name '*.hpp' -o -name '*.h'))


all: ccode

ccode:

purge:

install:

inc: $(INCFILES)
	@echo Built boost-numeric-bindings ...

$(INCFILES):
	@echo $(INCFILES)
	for header in `find boost -name '*.hpp' -o -name '*.h'`; do \
	  tgt_dir=$(LIBTOPDIR)linalgebra/boost-numeric/include; \
	  mkdir -p `dirname $$tgt_dir/$$header`; \
	  ln -fs $(addprefix $(shell pwd)/,$$header) `dirname $$tgt_dir/$$header` ; \
	  done

