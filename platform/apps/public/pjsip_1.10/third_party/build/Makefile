#DIRS = resample milenage srtp
DIRS = milenage

# SRTP support.
ifeq ($(strip $(INCLUDE_SRTP)),y)
	DIRS += srtp
endif # $(INCLUDE_SRTP)

include ../../build.mak
include $(PJDIR)/build/common.mak

all clean dep depend distclean realclean:
	for dir in $(DIRS); do \
		if $(MAKE) $(MAKE_FLAGS) -C $$dir $@; then \
		    true; \
		else \
		    exit 1; \
		fi; \
	done

doc:
	
