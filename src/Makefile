LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11 
OBJS   = main.o init.o event.o draw.o battery.o cpu.o autoscript.o pixmap.o
EXE    = wmlaptop


.c.o:
	$(CC) $(CFLAGS) -c $< -o $*.o

all: $(EXE)

$(EXE): $(OBJS)
	./buildCounter
	$(CC) $(LDFLAGS) -o $(EXE) $^ $(LIBDIR) $(LIBS)
	

clean:
	for i in $(OBJS) ; do \
		rm -rf $$i;\
	done
	rm -rf $(EXE) core *~
	
reall: clean all

install:
	/usr/bin/install -c -o 0 -g 0 -m 755 $(EXE) $(INSTALLDIR)/$(EXE)
	chmod +s $(INSTALLDIR)/$(EXE)
	
uninstall:
	rm -f $(INSTALLDIR)/$(EXE)
