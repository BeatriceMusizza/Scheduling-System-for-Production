EASYLOCAL = ../easylocal-3
FLAGS = -std=c++17 -Wall -Wfatal-errors -O3 #-Wno-sign-compare 
COMPOPTS = -I$(EASYLOCAL)/include $(FLAGS) -I/usr/local/include
LINKOPTS = -lboost_program_options -pthread -L/usr/local/lib

SOURCE_FILES = Prod_Data.cc Prod_Helpers.cc  Prod_Main.cc
OBJECT_FILES = Prod_Data.o Prod_Helpers.o  Prod_Main.o
HEADER_FILES = Prod_Data.hh Prod_Helpers.hh

Prod.exe: $(OBJECT_FILES)
	g++ $(OBJECT_FILES) $(LINKOPTS) -o Prod.exe

Prod_Data.o: Prod_Data.cc Prod_Data.hh
	g++ -c $(COMPOPTS) Prod_Data.cc

Prod_Helpers.o: Prod_Helpers.cc Prod_Helpers.hh Prod_Data.hh
	g++ -c $(COMPOPTS) Prod_Helpers.cc

Prod_Main.o: Prod_Main.cc Prod_Helpers.hh Prod_Data.hh
	g++ -c $(COMPOPTS) Prod_Main.cc

clean:
	rm -f $(OBJECT_FILES) Prod



