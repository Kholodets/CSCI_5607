= CSCI 5607 Homework 4
== Lexi MacLean

This assignment just goes over some of the basics of using OpenGL with SDL.
We can see some basic shaders, models, and window features of the framwork in the provided code.
For the homework, I compiled the provided examples and experimented with some small changes to observe their effects:

== A.

  #html.elem("img", attrs: (src: "images/tricol.png", width: "600"))[]
  #html.elem("img", attrs: (src: "images/cube3.png", width: "600"))[]
  #html.elem("img", attrs: (src: "images/cubelit.png", width: "600"))[]
  #html.elem("img", attrs: (src: "images/model.png", width: "600"))[]
  #html.elem("img", attrs: (src: "images/teapot.png", width: "600"))[]

== B.

=== Q1.

By removing this call, we disable the test that prevents the drawing of triangles which are obscured by another.
This means that 

  #html.elem("img", attrs: (src: "images/cube_funny.png", width: "600"))[]

=== Q2.

I just modified the line that checks the keyup event to exit on `SDLK_ESCAPE` to also check `SDLK_Q`

```if (windowEvent.type == SDL_EVENT_KEY_UP && (windowEvent.key.key == SDLK_ESCAPE || windowEvent.key.key == SDLK_Q) )```

=== Q3. 
I changed the line in the fragment shader from `"const vec3 lightDir = vec3(0,1,1);"` to `"const vec3 lightDir = vec3(0,0,-1);"`


  #html.elem("img", attrs: (src: "images/light-below.png", width: "600"))[]

=== Q4.

Here's the render of the triangle I made, it rotates still :p but it's there.

  #html.elem("img", attrs: (src: "images/bigtri.png", width: "600"))[]




