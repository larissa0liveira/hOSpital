# Grupo 013
# 60522 - João Rebelo
# 60807 - Juliana Duarte
# 59830 - Larissa Oliveira

CC = gcc
OBJ_dir = obj
OBJECTOS = main.o memory.o process.o patient.o receptionist.o doctor.o synchronization.o configuration.o log.o hosptime.o hospsignal.o stats.o 

main.o = main.h memory.h process.h synchronization.h log.h hospsignal.h configuration.h
memory.o = memory.h
process.o = main.h memory.h process.h hospsignal.h patient.h receptionist.h doctor.h
patient.o = main.h memory.h patient.h hosptime.h
receptionist.o = main.h memory.h receptionist.h hosptime.h
doctor.o = main.h memory.h doctor.h hosptime.h

synchronization.o = synchronization.h
configuration.o = configuration.h main.h
log.o = log.h
hosptime.o = memory.h hosptime.h
hospsignal.o = main.h memory.h hospsignal.h
stats.o = stats.h main.h

vpath %.o $(OBJ_dir)
hospital: $(OBJECTOS)
	$(CC) $(addprefix $(OBJ_dir)/,$(OBJECTOS)) -o bin/hOSpital -g
%.o: src/%.c $($@)
	$(CC) -Wall -I include -o $(OBJ_dir)/$@ -c $<
clean:
	rm -f obj/*.o
	rm -f bin/hOSpital