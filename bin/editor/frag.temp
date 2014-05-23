#version 420

varying vec2 texcoord;
varying vec2 vpos;
uniform sampler2D texture1;
uniform float time;
uniform vec2 resolution;
uniform vec4 color;

void main(void)
{
	vec2 p = vpos;
	float r= length(p);
	float a = atan(p.y, p.x);
	vec3 uv;
	float t = time * 0.2;
	float x = t*t;
	float y = p.y + 0.005*sin(p.x*20);
	uv.x = p.x/abs(p.y) + sin(x);
	uv.y = t*50 + 1 / abs(y);
	uv.z = abs(p.y);
	
	float s = 2*pow(sin(1 + 2*x),3);
	
	vec3 col = texture2D(texture1, uv.yx).rgb * uv.z;
	//col.r *= s;
	//col.b *= (2 - col.r - col.g);
	
	gl_FragColor = vec4(col.rgb + 0.1, 1);
}
