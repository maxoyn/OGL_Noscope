
#version 330 core
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
void main()
{
    FragColor = vec4(1.0); // white color to color the cube representing the lightsource

}


