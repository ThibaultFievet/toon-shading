#version 330

const float PI = 3.14159265359;
const float TWOPI = 6.28318530718;
const float PI_2 = 1.57079632679;
const float DEG2RAD = TWOPI / 360.0;

precision highp int;

uniform sampler2D uTexture;
uniform int uEdgeDetectionEnabled = 0;
uniform float uEdgeDetectionFactor = 0.3;
uniform vec3 uEdgeColor = vec3(0.0);
uniform int uLevels = 4;
float scaleFactor = 1.0 / uLevels;
uniform float uIntensity = 1.0;

in vec3 vPosition_vs;
in vec3 vNormal_vs;
in vec2 vTexCoords;
in mat4 world;

uniform vec4 uDiffuse;
uniform vec4 uAmbient;
uniform vec4 uSpecular;
uniform vec4 uEmissive;
uniform float uShininess;
uniform int uTexCount;

struct PointLight
{
	vec3 Position;
	vec3 Color;
	float Intensity;
};

struct DirectionalLight
{
	vec3 Direction;
	vec3 Color;
	float Intensity;
};

struct SpotLight
{
	vec3 Position;
	float Angle;
	vec3 Direction;
	float PenumbraAngle;
	vec3 Color;
	float Intensity;
};

out vec4 fFragColor;

vec3 CelShading(in vec3 diffuseColor, in vec3 specularColor, in float ndotl, in float ndoth)
{
    float E = fwidth(ndoth);
    //if (ndoth > 0.5 - E && ndoth < 0.5 + E) ndoth = smoothstep(0.5 - E, 0.5 + E, ndoth);
    ndoth = step(0.5, ndoth);
    
    return (diffuseColor * floor(ndotl * uLevels) * scaleFactor + specularColor * ndoth);
}

int EdgeDetection(in vec3 v)
{
    int edgeColor = 0;
    if(uEdgeDetectionEnabled == 1 && uEdgeDetectionFactor > 0.005) edgeColor = (dot(v, vNormal_vs) >  uEdgeDetectionFactor) ? 0 : 1;
    
    return edgeColor;
}

vec3 pointLights( in PointLight p, in vec3 n, in vec3 v, in vec3 diffuseColor, in vec3 specularColor, in float specularPower)
{
	vec3 l = normalize(p.Position  - vPosition_vs);
    float ndotl = max(dot(n, l), 0.0);
    
    vec3 h = normalize(l+v);
    float ndoth = max(dot(n, h), 0.0);
    ndoth = pow(ndoth, specularPower);
    float d = distance(p.Position, vPosition_vs);
    float att = 1.f / (d*d);
    
    if(EdgeDetection(v) == 1) return uEdgeColor;
	return att * p.Color * p.Intensity * CelShading(diffuseColor, specularColor, ndotl, ndoth);
}

vec3 directionalLights( in DirectionalLight d, in vec3 n, in vec3 v, in vec3 diffuseColor, in vec3 specularColor, in float specularPower)
{
	vec3 l = normalize(-d.Direction);
	float ndotl = max(dot(n, l), 0.0);
	
    vec3 h = normalize(l+v);
    float ndoth = max(dot(n, h), 0.0);
    ndoth = pow(ndoth, specularPower);
    
    if(EdgeDetection(v) == 1) return uEdgeColor;
    return d.Color * d.Intensity * CelShading(diffuseColor, specularColor, ndotl, ndoth);
}

vec3 spotLights( in SpotLight s, in vec3 n, in vec3 v, in vec3 diffuseColor, in vec3 specularColor, in float specularPower)
{
	vec3 l = normalize(s.Position - vPosition_vs);
	float a = cos(s.Angle * DEG2RAD);
	float pa = cos(s.PenumbraAngle * DEG2RAD);
	float ndotl =  max(dot(n, l), 0.0);
	
    vec3 h = normalize(l+v);
    float ndoth = max(dot(n, h), 0.0);
    ndoth = pow(ndoth, specularPower);
    
	float ldotd = dot(-l, normalize(s.Direction));
	float fallof = clamp(pow( (ldotd  - a) /  (a-pa), 4), 0.0, 1.0);
	float d = distance(s.Position, vPosition_vs);
	float att = 1.f / (d*d);
	
	if(EdgeDetection(v) == 1) return uEdgeColor;
	return att * fallof * s.Color * s.Intensity * CelShading(diffuseColor, specularColor, ndotl, ndoth);
}

void main(void){

    vec3 diffuseColor = texture(uTexture, vTexCoords).rgb;
    vec3 specularColor = texture(uTexture, vTexCoords).rgb;
    float SpecularPower = 0.0;
    
    vec3 n = normalize(vNormal_vs);
	vec3 v = normalize(-vPosition_vs);
    
    PointLight p1;
    p1.Position = vec3(world * vec4(0.0, 3.0, 0.0, 1.0));
    p1.Color = vec3(1.0, 1.0, 1.0);
    p1.Intensity = uIntensity;
    
    DirectionalLight d1;
    d1.Direction = vec3(0.0, -1.0, -1.0);
    d1.Color = vec3(1.0, 1.0, 1.0);
    d1.Intensity = 0.5;
    
    SpotLight s1;
    s1.Position = vec3(world * vec4(0.0, 6.0, 0.0, 1.0));
    s1.Color = vec3(1.0, 1.0, 1.0);
    s1.Intensity = 0.0;
	s1.Angle = 30.0;
	s1.Direction = vec3(world * vec4(0.0, 0.0, 0.0, 1.0));
	s1.PenumbraAngle = 40.0;
    
    fFragColor = vec4(pointLights(p1, n, v, diffuseColor, specularColor, SpecularPower) + 
		directionalLights(d1, n, v, diffuseColor, specularColor, SpecularPower) +
		spotLights(s1, n, v, diffuseColor, specularColor, SpecularPower), 1.0);
}
