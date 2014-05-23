#version 420
 

attribute vec4 coord;
attribute vec4 atr;
varying vec2 texcoord;
varying vec2 vpos;
varying vec4 vcolor;

void main(void) {
  gl_Position = vec4(coord.xy, 0, 1);
  texcoord = coord.zw;
  vpos = coord.xy;
  vcolor = atr;
}