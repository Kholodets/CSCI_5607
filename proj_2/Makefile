image: image.cpp
	g++ -fsanitize=address -std=c++14 main.cpp pixel.cpp image.cpp -o image
zip: image
	tar -czf report/macle119_proj2.tar.gz image image.cpp image.h pixel.cpp pixel.h main.cpp stb_image.h stb_image_write.h
report: report/report.typ zip
	typst compile --features html --format html report/report.typ report/index.html
clean:
	rm image
