.PHONY: greet build rebuild run clean

greet:
	@echo "Terminating make - please specify target explicitly"
	@echo " build : fast rebuild / build"
	@echo " rebuild : full rebuild"
	@echo " run : run after fast rebuild / build"
	@echo " clean : perform full clean"

# Швидка компіляція
build: program

# Повна перекомпіляція
rebuild: clean program

# Запуск після компіляції
run: build
	./program

# Видалення всіх об'єктних файлів і виконуваного файлу
clean:
	rm -rvf *.o program

# Компіляція виконуваного файлу
program:
	gcc -o program LB2.c -lm -pthread

