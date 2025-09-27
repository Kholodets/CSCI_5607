= CSCI 5607 Project 1
== Lexi MacLean

#figure(
html.elem("video", attrs: (controls: "", width: "400"))[
  #html.elem("source", attrs: (src: "required_features.webm", type: "video/webm"))[]
],
caption: [Video Demonstrating the Required Features.]
)

=== Click Detection

Click detection was straightfoward, I resued some code from the homework but extended it to work with squares instead of triangles.
It sees whether the mouse is closer to the edge or a vertex, or absolutely close enough to either to do rotation or scaling.
Then, if neither, it checks if the mouse is in the square, allowing for translation.

=== Transformation

Most of my implementation went pretty smoothly.
My intuitive guess for smooth controls worked well once bugs were worked out.
A couple times I mixed up some of the global variable keywords like `clicked_pos` and `clicked_mouse` which caused some strange issues including `NaN` propogation, but these were resolved simply.

The scaling works by projecting the dragged-to point onto the line made by joining the clicked point and the center of the square, and then comparing that projection's distance from the center of the square with the clicked point to obtain the scale.

The rotation works a similar way, getting the angle from the line made from the clicked point to the center and the line from the dragged point to the center.
It took a bit of fiddling with signs and reversals to make sure it didn't rotate in reverse or flip around at 180 degrees.

Resetting the squares position on a keypress was as simple as duplicating the procedure from the `mouseDragged` function, but setting all of `rect_pos`, `rect_scale`, and `rect_angle` to initial values (`(0,0)`, `1`, and `0`), then updating the verticies and lines accordingly.

=== Reading in PPM

Reading in a PPM is straightforward thanks to its being a simple ASCII Format.

=== Gamma Correction

The `brick.ppm` image is stored with gamma correction applied.
Gamma correction is a technique for slicing up the available colorspace in a way that appears more even to the human eye, improving banding effects etc. without needing to increase the bit depth.
To correct for this, I wrote a function to undo gamma correction as the image is read in from ppm:

#figure(
```c
//...
#define GAMMA 2.2

int igam(int x)
{
	float v = x/255.0f;
	float p = pow(v, 1/GAMMA);
	return (int) (p*255);
}
//...

unsigned char* loadImage(int& img_w, int& img_h)
{
  //...
        for (int i = 0; i < img_h; i++) {
                for (int j = 0; j < img_w; j++) {
                        int red, green, blue;
                        ppmFile >> red >> green >> blue;
                        img_data[i*img_w*4 + j*4] = igam(red);
                        img_data[i*img_w*4 + j*4 + 1] = igam(green);i
                        img_data[i*img_w*4 + j*4 + 2] = igam(blue);
                        img_data[i*img_w*4 + j*4 + 3] = 255; // Alpha
                }
        }
  //...
}
```,
caption: [Code snippets demonstrating PPM image loading and inverse gamma correction.]
)

This approach solves the oddly dark appearance of `brick.ppm`:

#figure(
  html.elem("div")[
    #html.elem("img", attrs: (src: "brick_no_gamma.png", width: "250"))[]
    #html.elem("img", attrs: (src: "brick_gamma.png", width: "250"))[]
  ],
  caption: [Images showing the `brick.ppm` file displayed in my program without and with gamma correction applied (`GAMMA = 1.0` and `GAMMA = 2.2`).]
)

*HOWEVER*, the main advantage of gamma correction is to account for limitations of the encoded colorspace.
Here, we're just taking it from the encoded int, reversing the correction, and then putting it back into an 8-bit integer.
This negates most of the advantages of this encoding.
I could supply the texture as floats which would be an improvement, however I didn't want to mess with any of the other functions when I implemented this.
