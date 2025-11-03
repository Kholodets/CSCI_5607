ray: rayTrace_pga.cpp hit.h parse_pga.h
	g++ -fsanitize=address -std=c++14 -fopenmp -O3 rayTrace_pga.cpp -o ray

spheres1: ray
	./ray SphereExamples/spheres1.txt

spheres2: ray
	./ray SphereExamples/spheres2.txt

zip: ray
	tar -czf report/macle119_proj3.tar.gz ray rayTrace_pga.cpp hit.h PGA_3D.h image_lib.h stb_image.h stb_image_write.h multivector.h

report: report/report.typ zip
	typst compile --features html --format html report/report.typ report/index.html

clean:
	rm ray
