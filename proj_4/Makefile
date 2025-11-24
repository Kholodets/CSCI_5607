report: report/report.typ
	typst compile --features html --format html report/report.typ report/index.html

game: game.cpp
	g++ game.cpp glad/glad.c -lSDL3 -lGL -ldl -I/usr/include/SDL3 -o game

zip: game.cpp
	tar -czf macle119_proj4.tar.gz game.cpp glad glm Makefile model test_keys.txt 
