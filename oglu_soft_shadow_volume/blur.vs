varying vec2 vTexCoord;

void main()
{
	//Tranform to clip space.
	gl_Position = ftransform();
	
	vTexCoord = gl_MultiTexCoord0.xy;
}