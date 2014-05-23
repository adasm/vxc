#version 420

varying vec2 texcoord;
varying vec2 vpos;
uniform sampler2D texture1;
uniform float time;
uniform vec2 resolution;
uniform vec4 color;

void main(void)
{
	float fade = (1 - length(vpos.y)/sqrt(2));
	fade = pow(fade, 0.2);
	gl_FragColor = vec4(color.rgb, color.a * (fade));
}
