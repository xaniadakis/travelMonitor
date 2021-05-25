# OBJS 		= $(filter-out Monitor.o, $(wildcard *.o))
# SOURCE		= $(wildcard *.c)
PARENTOBJS 	= travelMonitor.o travelMonitorUtil.o myLib.o hashTable.o bloomFilter.o skipList.o hashFunctions.o 
CHILDOBJS	= Monitor.o monitorUtil.o myLib.o hashTable.o bloomFilter.o skipList.o hashFunctions.o 
SOURCE		= travelMonitor.c Monitor.c travelMonitorUtil.c myLib.c hashTable.c bloomFilter.c skipList.c hashFunctions.c monitorSignals.c parentSignals.c
HEADER		= $(wildcard ../lib/*.h)
OUT  		= travelMonitor
CC			= gcc
FLAGS  		= -c 
DEBUGFLAGS	= -g -c -Wall -Wextra # -pedantic -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror #-Wsign-conversion

debug 				?= 0
virusesfile 		?= smallvirusesFile
countriesfile	 	?= smallcountriesFile
numlines 			?= 200
duplicatesallowed 	?= 1
nummonitors			?= 3
bloomsize 			?= 10000
buffersize 			?= 1000
input_dir			?= ../script/input_dir
inputfile 			?= inputFile
numFilesPerDirectory?= 3
scr_input_dir 		?= input_dir

all:	script	\
	Monitor	\
	$(OUT)	
	
run:
#	./travelMonitor -m 3 -b 100 -s 1000 -i ../script/input_dir
	./$(OUT) -m $(nummonitors) -b $(buffersize) -s $(bloomsize) -i $(input_dir)

valgrind:	
#	valgrind --trace-children=yes --track-fds=yes valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file="valgrind-out.txt" ./$(OUT) -m $(nummonitors) -b $(buffersize) -s $(bloomsize) -i $(input_dir)
#	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file="valgrind-out.txt"  ./travelMonitor -m 3 -b 1000 -s 1000 -i ../script/input_dir
#	--trace-children=yes
#	valgrind --show-leak-kinds=all --trace-children=yes --track-origins=yes --leak-check=full --log-file="valgrind-out1.txt" -v ./$(OUT) -m 1 -b $(buffersize) -s $(bloomsize) -i $(input_dir)
	valgrind --leak-check=full --show-leak-kinds=all ./$(OUT) -m 1 -b $(buffersize) -s $(bloomsize) -i $(input_dir)

$(OUT): $(PARENTOBJS) 
#	$(CC) -g -D_XOPEN_SOURCE=700 $(PARENTOBJS) -o $@ -lm 
	$(CC) -g -D_XOPEN_SOURCE=700 -o $@ parentSignals.c $(PARENTOBJS) -lm


Monitor: $(CHILDOBJS) 
#	$(CC) -g -D_XOPEN_SOURCE=700 $(CHILDOBJS) signals.c -o $@ -lm
	$(CC) -g -D_XOPEN_SOURCE=700 -o $@ monitorSignals.c $(CHILDOBJS) -lm

travelMonitor.o: travelMonitor.c
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) travelMonitor.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) travelMonitor.c 
else
	$(CC) $(FLAGS) travelMonitor.c  
endif
endif

Monitor.o: Monitor.c
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) Monitor.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) Monitor.c 
else
	$(CC) $(FLAGS) Monitor.c  
endif
endif

# parentSignals.o: parentSignals.c
# ifeq	($(debug),2)
# 	$(CC) $(DEBUGFLAGS) parentSignals.c 2>/dev/null
# else
# ifeq	($(debug),1)
# 	$(CC) $(DEBUGFLAGS) parentSignals.c 
# else
# 	$(CC) $(FLAGS) parentSignals.c  
# endif
# endif

travelMonitorUtil.o: travelMonitorUtil.c 
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) travelMonitorUtil.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) travelMonitorUtil.c 
else
	$(CC) $(FLAGS) travelMonitorUtil.c  
endif
endif

monitorUtil.o: monitorUtil.c 
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) monitorUtil.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) monitorUtil.c 
else
	$(CC) $(FLAGS) monitorUtil.c  
endif
endif

myLib.o: myLib.c
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) myLib.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) myLib.c
else
	$(CC) $(FLAGS) myLib.c
endif
endif

hashTable.o: hashTable.c 
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) hashTable.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) hashTable.c
else
	$(CC) $(FLAGS) hashTable.c
endif
endif

bloomFilter.o: bloomFilter.c 
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) bloomFilter.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) bloomFilter.c
else
	$(CC) $(FLAGS) bloomFilter.c
endif
endif

skipList.o: skipList.c
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) skipList.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) skipList.c
else
	$(CC) $(FLAGS) skipList.c
endif
endif

hashFunctions.o: hashFunctions.c 
ifeq	($(debug),2)
	$(CC) $(DEBUGFLAGS) hashFunctions.c 2>/dev/null
else
ifeq	($(debug),1)
	$(CC) $(DEBUGFLAGS) hashFunctions.c
else
	$(CC) $(FLAGS) hashFunctions.c
endif
endif

.PHONY:	script
script:	../script/create_infiles.sh
	cd ../script; ./testFile.sh $(virusesfile) $(countriesfile) $(numlines) $(duplicatesallowed);
	cd ../script; rm -rf input_dir; ./create_infiles.sh $(inputfile) $(scr_input_dir) $(numFilesPerDirectory); 

.PHONY:	clean
clean:
	rm -f $(CHILDOBJS) $(PARENTOBJS) $(OUT) Monitor

.PHONY:	cleanall
cleanall:
	rm -f $(CHILDOBJS) $(PARENTOBJS) $(OUT) Monitor
	rm -f ../script/inputFile
	rm -rf ../script/input_dir
	rm -f valgrind-out.txt
	rm -f ../tmp/namedPipes/*
	rm -rf ../log_folder