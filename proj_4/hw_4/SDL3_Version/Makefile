coltri: TriangleColored.cpp
	g++ TriangleColored.cpp glad/glad.c -lSDL3 -lGL -ldl -I/usr/include/SDL3 -o tricol

cube3: Cube3D.cpp
	g++ Cube3D.cpp glad/glad.c -lSDL3 -lGL -ldl -I/usr/include/SDL3 -o cube3

cubelit: CubeLit.cpp
	g++ CubeLit.cpp glad/glad.c -lSDL3 -lGL -ldl -I/usr/include/SDL3 -o cubelit

model: ModelLoad.cpp
	g++ ModelLoad.cpp glad/glad.c -lSDL3 -lGL -ldl -I/usr/include/SDL3 -o model

report: report/report.typ
	typst compile --features html --format html report/report.typ report/index.html
