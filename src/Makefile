


.c.o:
	$(CC) -c  $< -o $*.o $(CFLAGS)

all: $(EXE)

$(EXE): $(OBJS)
	./buildCounter
	$(CC) -o $(EXE) $^ $(LIBDIR) $(LIBS)
	

clean:
	for i in $(OBJS) ; do \
		rm -rf $$i;\
	done
	rm -rf $(EXE) core *~
	
reall: clean all

install:
	/usr/bin/install -c -o 0 -g 0 -s -m 755 $(EXE) $(INSTALLDIR)/$(EXE)
	chmod +s $(INSTALLDIR)/$(EXE)
	
uninstall:
	rm -f $(INSTALLDIR)/$(EXE)
