all: login supernode user1 user2 user3 clean.o

login: login.o
	g++ -o login login.o -lsocket -lnsl -lresolv

login.o: login.cc
	g++ -c login.cc

supernode: supernode.o
	g++ -o supernode supernode.o -lsocket -lnsl -lresolv

supernode.o: supernode.cc
	g++ -c supernode.cc

user1: user1.o
	g++ -o user1 user1.o -lsocket -lnsl -lresolv

user1.o: user1.cc
	g++ -c user1.cc

user2: user2.o
	g++ -o user2 user2.o -lsocket -lnsl -lresolv

user2.o: user2.cc
	g++ -c user2.cc

user3: user3.o
	g++ -o user3 user3.o -lsocket -lnsl -lresolv

user3.o: user3.cc
	g++ -c user3.cc

clean: clean.o
	rm -f login user1 user2 user3 supernode

clean.o:
	rm -f *.o
