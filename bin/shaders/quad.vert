#version 420
 
attribute vec2 position;
varying vec2 texcoord;
varying vec2 vpos;

void main(void) {
  gl_Position = vec4(position.xy, 0, 1);
  texcoord = vec2((position.x + 1)/2, 1 - (position.y + 1)/2);
  vpos = position.xy;
}