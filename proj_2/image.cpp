//CSCI 5607 HW 2 - Image Conversion Instructor: S. J. Guy <sjguy@umn.edu>
//In this assignment you will load and convert between various image formats.
//Additionally, you will manipulate the stored image data by quantizing, cropping, and suppressing channels

#include "image.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include <fstream>
using namespace std;

/**
 * Image
 **/
Image::Image (int width_, int height_){

    assert(width_ > 0);
    assert(height_ > 0);

    width           = width_;
    height          = height_;
    num_pixels      = width * height;
    sampling_method = IMAGE_SAMPLING_POINT;
    
    data.raw = new uint8_t[num_pixels*4];
		int b = 0; //which byte to write to
		for (int j = 0; j < height; j++){
			for (int i = 0; i < width; i++){
				data.raw[b++] = 0;
				data.raw[b++] = 0;
				data.raw[b++] = 0;
				data.raw[b++] = 0;
			}
		}

    assert(data.raw != NULL);
}

Image::Image(const Image& src){
	width           = src.width;
	height          = src.height;
	num_pixels      = width * height;
	sampling_method = IMAGE_SAMPLING_POINT;
	
	data.raw = new uint8_t[num_pixels*sizeof(Pixel)];
	
	memcpy(data.raw, src.data.raw, num_pixels*sizeof(Pixel));
}

Image::Image(char* fname){

	int numComponents; //(e.g., Y, YA, RGB, or RGBA)

	//Load the pixels with STB Image Lib
	uint8_t* loadedPixels = stbi_load(fname, &width, &height, &numComponents, 4);
	if (loadedPixels == NULL){
		printf("Error loading image: %s", fname);
		exit(-1);
	}

	//Set image member variables
	num_pixels = width * height;
	sampling_method = IMAGE_SAMPLING_POINT;

  //Copy the loaded pixels into the image data structure
	data.raw = new uint8_t[num_pixels*sizeof(Pixel)];
	memcpy(data.raw, loadedPixels, num_pixels*sizeof(Pixel));
	free(loadedPixels);
}

Image::~Image(){
    delete[] data.raw;
    data.raw = NULL;
}

void Image::Write(char* fname){
	
	int lastc = strlen(fname);

	switch (fname[lastc-1]){
	   case 'g': //jpeg (or jpg) or png
	     if (fname[lastc-2] == 'p' || fname[lastc-2] == 'e') //jpeg or jpg
	        stbi_write_jpg(fname, width, height, 4, data.raw, 95);  //95% jpeg quality
	     else //png
	        stbi_write_png(fname, width, height, 4, data.raw, width*4);
	     break;
	   case 'a': //tga (targa)
	     stbi_write_tga(fname, width, height, 4, data.raw);
	     break;
	   case 'p': //bmp
	   default:
	     stbi_write_bmp(fname, width, height, 4, data.raw);
	}
}


void Image::Brighten (double factor){
	int x,y;
	for (x = 0 ; x < Width() ; x++){
		for (y = 0 ; y < Height() ; y++){
			Pixel p = GetPixel(x, y);
			Pixel scaled_p = p*factor;
			GetPixel(x,y) = scaled_p;
		}
	}
}

void Image::ExtractChannel(int channel){
	int x,y;
	for (x = 0 ; x < Width() ; x++){
		for (y = 0 ; y < Height() ; y++){
			Pixel p = GetPixel(x, y);
			Pixel ext = Pixel((channel==0)?p.r:0,(channel==1)?p.g:0,(channel==2)?p.b:0,p.a);
			GetPixel(x,y) = ext;
		}
	}
}


void Image::Quantize (int nbits){
	int x,y;
	for (x = 0; x < Width(); ++x) {
		for (y = 0; y < Height(); ++y) {
			Pixel p = GetPixel(x, y);
			Pixel quant_p = PixelQuant(p, nbits);
			GetPixel(x, y) = quant_p;
		}
	}
}

Image* Image::Crop(int x, int y, int w, int h){
	Image *cropped = new Image(w, h);
	for (int i = 0; i < Width(); ++i) {
		for (int j = 0; j < Height(); ++j) {
			Pixel p = GetPixel(x + i, y + j);
			cropped->GetPixel(i,j) = p;
		}
	}
	return cropped;
}


void Image::AddNoise (double factor){
	for (int i = 0; i < Width(); ++i) {
		for (int j = 0; j < Height(); ++j) {
			Pixel p = GetPixel(i, j);
			p.SetClamp(p.r + ((rand() % (int) (factor*255.0)) - (factor*127)),
					p.g + ((rand() % (int) (factor*255.0)) - (factor*127)),
					p.b + ((rand() % (int) (factor*255.0)) - (factor*127))
					);
			GetPixel(i,j) = p;
		}
	}
}

void Image::ChangeContrast (double factor){
	//Component avg_l = Pixel(127,127,127).Luminance();
	Pixel grey = Pixel(127, 127, 127);
	for (int i = 0; i < Width(); ++i) {
		for (int j = 0; j < Height(); ++j) {
			Pixel p = GetPixel(i, j);
			Pixel lerpd = PixelLerp(grey, p, factor);
			GetPixel(i,j) = lerpd;
		}
	}
}


void Image::ChangeSaturation(double factor){
	for (int i = 0; i < Width(); ++i) {
		for (int j = 0; j < Height(); ++j) {
			Pixel p = GetPixel(i, j);
			int lum = (p.r*76 + p.g*150 + p.b*29) >> 8;
			Pixel grey = Pixel(lum, lum, lum);
			Pixel lerpd = PixelLerp(grey, p, factor);
			GetPixel(i,j) = lerpd;
		}
	}
}


//For full credit, check that your dithers aren't making the pictures systematically brighter or darker
void Image::RandomDither (int nbits){
	AddNoise(0.25);
	Quantize(nbits);
}

//This bayer method gives the quantization thresholds for an ordered dither.
//This is a 4x4 dither pattern, assumes the values are quantized to 16 levels.
//You can either expand this to a larger bayer pattern. Or (more likely), scale
//the threshold based on the target quantization levels.
static int Bayer4[4][4] ={
    {15,  7, 13,  5},
    { 3, 11,  1,  9},
    {12,  4, 14,  6},
    { 0,  8,  2, 10}
};


void Image::OrderedDither(int nbits){
	/* WORK HERE  (Extra Credit) */
}

/* Error-diffusion parameters */
const double
    ALPHA = 7.0 / 16.0,
    BETA  = 3.0 / 16.0,
    GAMMA = 5.0 / 16.0,
    DELTA = 1.0 / 16.0;

void Image::FloydSteinbergDither(int nbits){
	for (int i = 0; i < Width(); ++i) {
		for (int j = 0; j < Height(); ++j) {
			Pixel p = GetPixel(i,j);
			Pixel q = PixelQuant(p, nbits);
			Pixel err = Pixel(p.r - q.r, p.g - q.g, p.b - q.b);
			int re = p.r - q.r;
			int ge = p.g - q.g;
			int be = p.b - q.b;
			
			int rn, gn, bn;
			if (i < Width() - 1) {
				Pixel a = GetPixel(i+1, j);
				rn = ComponentClamp(a.r + (ALPHA * re));
				gn = ComponentClamp(a.g + (ALPHA * ge));
				bn = ComponentClamp(a.b + (ALPHA * be));
				GetPixel(i+1, j) = Pixel(rn, gn, bn);
			}
			if (j < Height() - 1 && i > 0) {
				Pixel b = GetPixel(i-1, j+1);
				rn = ComponentClamp(b.r + (BETA * re));
				gn = ComponentClamp(b.g + (BETA * ge));
				bn = ComponentClamp(b.b + (BETA * be));
				GetPixel(i-1, j+1) = Pixel(rn, gn, bn);
			}
			if (j < Height() - 1) {
				Pixel c = GetPixel(i, j+1);
				rn = ComponentClamp(c.r + (GAMMA * re));
				gn = ComponentClamp(c.g + (GAMMA * ge));
				bn = ComponentClamp(c.b + (GAMMA * be));
				GetPixel(i, j+1) = Pixel(rn, gn, bn);
			}
			if (i < Width() - 1 && j < Height() - 1) {
				Pixel d = GetPixel(i+1, j+1);
				rn = ComponentClamp(d.r + (DELTA * re));
				gn = ComponentClamp(d.g + (DELTA * ge));
				bn = ComponentClamp(d.b + (DELTA * be));
				GetPixel(i+1, j+1) = Pixel(rn, gn, bn);
			}

			GetPixel(i,j) = q;
		}
	}
}

/*
struct Pair
{
	double x, y;
	Pair (double u, double v): x(u), y(v) {}
};

typedef Pair (*Map)(Pair);
*/

Pixel Image::DTSample (Pair uv, float *filter, int n)
{
	int u = (int) uv.x;
	int v = (int) uv.y;

	if (n % 2 != 1) {
		fprintf(stderr, "incorrect usage of DTSample\n");
		exit(1);
	}

	int s = n/2;

	float r = 0, g = 0, b = 0;
	for (int i = max(-s, u-s) ; i <= min(Width()-1+s, u+s); ++i) {
		for (int j = max(-s, v-s); j <= min(Height()-1+s, v+s); ++j) {
			int x = s + u - i;
			int y = s + v - j;
			Pixel f;
			float h = filter[y*n + x];
			

			int ii = i, jj = j;
			if (ii < 0) {
				ii = -ii % Width();
			}

			if (ii >= Width()) {
				ii = 2 * (Width()-1) - ii;
			}
			
			if (jj < 0) {
				jj = -jj % Height();
			}

			if (jj >= Height()) {
				jj = 2 * (Height()-1) - jj;
			}

			f = GetPixel(ii,jj);
			
			float fr, fg, fb;
			fr = f.r;
			fg = f.g;
			fb = f.b;
			r += fr * h;
			g += fg * h;
			b += fb * h;
		}
	}

	return Pixel(abs(r), abs(g), abs(b));
}

void Image::DTConvolve (Image *source, float *filter, int n)
{
	for (int i = 0; i < Width(); ++i) {
		for (int j = 0; j < Height(); ++j) {
			Pair xy = Pair(i,j);
			GetPixel(i, j) = (*source).DTSample(xy, filter, n);
		}
	}
}



double Gauss2(double x, double y, double a, double xsd2, double ysd2)
{
	return a * exp(-(((x * x)/(2*xsd2)) + (y * y)/(2*ysd2)));
}

// Gaussian blur with size nxn filter
void Image::Blur(int n){

	int d = n*2 + 1;
	float *filter = new float[d*d];
	Image* img_copy = new Image(*this); //This is will copying the image, so you can read the original values for filtering
	
	double sd = (double) d * (double) d;

	float sum = 0;
	for (int i = 0; i < d; ++i) {
		for (int j = 0; j < d; ++j) {
			float g = Gauss2(i-n, j-n, 1, sd, sd);
			filter[j*d + i] = g;
			sum += g;
		}
	}

	for (int i = 0; i < d * d; ++i) filter[i] /= sum;


	DTConvolve(img_copy, filter, d);

	//CTConvolve(img_copy, &GSample, &straight);

	delete img_copy;
	delete[] filter;
}

void Image::Sharpen(int n){
	Image *icopy = new Image(*this);

	icopy->Blur(n);
	for (int i = 0; i < Width(); ++i) {
		for (int j = 0; j < Height(); ++j) {
			Pixel a = icopy->GetPixel(i, j);
			Pixel b = GetPixel(i,j);

			Pixel c = PixelLerp(a, b, 2);
			GetPixel(i,j) = c;
		}
	}

	delete icopy;
}

int between(int x, int y)
{
	/*
	float a = (float) x / 255.0;
	float b = (float) y / 255.0;

	float comb = sqrt(a*a + b*b);

	return (int) (comb * 255);
	*/
	return sqrt(x*x + y*y);
}

void Image::EdgeDetect(){
	Image *udi = new Image(*this);
	float udf[9] = {-1.0, -2.0, -1.0,
		0,0,0,
		1.0, 2.0, 1.0
	};

	udi->DTConvolve(this, udf, 3);
	
	Image *lri = new Image(*this);
	float lrf[9] = {-1, 0, 1.0,
		-2.0, 0, 2.0,
		-1.0, 0, 1.0
	};

	lri->DTConvolve(this, lrf, 3);

	for (int i = 0; i < Width(); ++i) {
		for (int j = 0; j < Height(); ++j) {
			Pixel v = udi->GetPixel(i,j);
			Pixel h = lri->GetPixel(i,j);
			int r = between(v.r, h.r);
			int g = between(v.g, h.g);
			int b = between(v.b, h.b);
			GetPixel(i,j) = Pixel(r,g,b);
		}
	}
	delete udi;
	delete lri;
			
}

/*

void Image::CTConvolve (Image *source, Filter f, Map xyuv)
{
	for (int i = 0; i < Width(); ++i) {
		for (int j = 0; j < Height(); ++j) {
			Pair xy = Pair(i,j);
			Pair uv = (*xyuv)(xy);
			GetPixel(i, j) = (*source).CTSample(uv, f);
		}
	}
}
*/

Pair scaleMap(Pair xy, double sx, double sy)
{
	return Pair(xy.x / sx, xy.y / sy);
}

Image* Image::Scale(double sx, double sy){
	int w = sx * Width();
	int h = sy * Height();

	double ascale = (abs(sx) + abs(sy)) / 2;

	Image *scaled = new Image(abs(w),abs(h));

	for (int i = min(0,w); i < max(0,w); ++i) {
		for (int j = min(0,h); j < max(0,h); ++j) {
			Pair uv = scaleMap(Pair(i,j), sx, sy);			
			scaled->GetPixel(i-min(0,w), j-min(0,h)) = Sample(uv.x,uv.y, 1/ascale);
		}
	}
	return scaled;
}

Pair rotMap(Pair xy, double t)
{
	return Pair(xy.x * cos(-t) - xy.y * sin(-t), xy.x * sin(-t) + xy.y * cos(-t));
}


Image* Image::Rotate(double angle)
{
	Pair bl = Pair(0,0);
	Pair tr = rotMap(Pair(Width(), Height()), -angle);
	Pair tl = rotMap(Pair(0,Height()), -angle);
	Pair br = rotMap(Pair(Width(), 0), -angle);


	int max_x = max(max(bl.x, tr.x), max(tl.x, br.x));
	int min_x = min(min(bl.x, tr.x), min(tl.x, br.x));
	int max_y = max(max(bl.y, tr.y), max(tl.y, br.y));
	int min_y = min(min(bl.y, tr.y), min(tl.y, br.y));
	
	int w = max_x - min_x;
	int h = max_y - min_y;

	Image *rotated = new Image(w,h);

	for (int i = min_x; i < max_x; ++i) {
		for (int j = min_y; j < max_y; ++j) {
			Pair uv = rotMap(Pair(i,j), angle);			
			rotated->GetPixel(i-min_x, j-min_y) = Sample(uv.x,uv.y, 1);
		}
	}
	return rotated;
}

void Image::Fun(){
	/* WORK HERE */
}

/**
 * Image Sample
 **/
void Image::SetSamplingMethod(int method){
   assert((method >= 0) && (method < IMAGE_N_SAMPLING_METHODS));
   sampling_method = method;
}


float gsamp(Pair uv, double r)
{
	double a, sd2;

	sd2 = r*r;
	a = 0.159 / (r*r);
	double f = Gauss2(uv.x, uv.y, a, sd2, sd2);
	return f;
}

float lsamp(Pair uv, double r)
{
	if (abs(uv.x) > r || abs(uv.y) > r) return 0;
	double v = (r-abs(uv.x)) * (r-abs(uv.y)) / ((r * r)) ;
	return v/(r*r);
}



Pixel Image::Sample (double u, double v, double rd){
	Filter filter;
	switch(sampling_method)
	{
		case 0:
			filter = &gsamp;
			break;
		case 1:
			filter = &lsamp;
			break;
		case 2:
			return GetPixel(max(0,min((int)round(u),Width()-1)), max(0,min((int)round(v),Height()-1)));
	}
	float r = 0, g = 0, b = 0;
	if (rd < 1) rd = 1;
	for (int i = max((int)u-(int)(4*rd), 0); i <= min((int)u+(int)(4*rd), Width()-1); ++i) {
		for (int j = max((int) v-(int)(4*rd), 0); j <= min((int) v+(int)(4*rd),Height()-1); ++j) {
			double x = u - (double) i;
			double y = v - (double) j;
			Pixel f;

			f = GetPixel(i,j);
			float h = (*filter)(Pair(x,y), rd);

			r += (double) f.r * h;
			g += (double) f.g * h;
			b += (double) f.b * h;
		}
	}

	return Pixel(r, g, b);
}
