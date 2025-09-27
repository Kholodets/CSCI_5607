square: Square.cpp
	g++ Square.cpp glad/glad.c -lSDL3 -lGL -ldl -I/usr/include/SDL3/ -o square

report: report/report.typ zip
	typst compile --features html --format html report/report.typ report/index.html

zip: square
	tar -czf report/macle119_proj1.tar.gz square Square.cpp geom_lib_2d.h brick.ppm goldy.ppm multivector.h pga.h glad primitives.h Makefile
