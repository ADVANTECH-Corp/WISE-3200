# Makefile for Btdiag

CFLAGS += -g -DDEBUG -DLINUX_X86
LDLIBS = -levent

.PHONY: all clean

Btdiag: Btdiag.o connection.o hciattach_rome.o privateMembers.o hci_api.o

Btdiag.o: Btdiag.c
	${CC} -c $(CFLAGS) Btdiag.c
connection.o: connection.c connection.h
	${CC} -c $(CFLAGS) connection.c
hciattach_rome.o: hciattach_rome.c hciattach_rome.h
	${CC} -c $(CFLAGS) hciattach_rome.c
privateMembers.o : privateMembers.c privateMembers.h
	${CC} -c $(CFLAGS) privateMembers.c
hci_api.o : hci_api.c hci_api.h
	${CC} -c $(CFLAGS) hci_api.c

clean:
	-$(RM) *.o 
	-$(RM)  Btdiag
