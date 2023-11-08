ifndef ROOT
	ROOT = ../../..
endif

include         $(ROOT)/config.mak

INCLDIR	= $(ROOT)/INCLUDE
LIBDIR		= $(ROOT)/LIB
BINS		= sima blockchain  spacer
HEADERS	= utils.h user_event_handlers.h msg_definition.h entity_definition.h lunes.h lunes_constants.h
#------------------------------------------------------------------------------

CFLAGS		+= $(OPTFLAGS) -I. -I$(INCLDIR) `pkg-config --cflags glib-2.0`
LDFLAGS		= -L$(LIBDIR)
LIBS		= -lartis_static -lpthread -lm `pkg-config --libs glib-2.0`
LDFLAGS		+= $(LIBS)
#------------------------------------------------------------------------------

all:	$(BINS) 

blockchain:	blockchain.o utils.o user_event_handlers.o lunes.o $(HEADERS)
	$(CC) -o $@ $(CFLAGS) blockchain.o utils.o user_event_handlers.o lunes.o $(LDFLAGS)

.c:
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS) 

#------------------------------------------------------------------------------

clean :
	rm -f  $(BINS) *.o *~ 
	rm -f  *.out *.err
	rm -f *.finished	

cleanall : clean 
	rm -f  *.dat *.log *.dot
	rm -f evaluation/*.ps
#------------------------------------------------------------------------------
