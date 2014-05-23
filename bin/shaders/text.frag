#version 420
 
varying vec2 texcoord;
varying vec2 vpos;
varying vec4 vcolor;
uniform sampler2D texture1;
uniform vec4 color = vec4(1,1,1,1);
 
void main(void)
{
	float a = texture2D(texture1, texcoord).r;
	float fade = (1 - length(vpos.y)/sqrt(2));
	fade = pow(fade, 2);
	vec3 col = vcolor.rgb;
	gl_FragColor = vec4(col, vcolor.a * a);
}
