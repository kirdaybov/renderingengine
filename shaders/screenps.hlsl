struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

struct Ray
{
	float3 origin;
	float3 dir;
	float3 color;
};

struct Material
{
	float3 diffuse;
};

#define LARGE_DISTANCE 999999.9f

float checkSphere(Ray ray, float4 sphere, Material mat, out Ray outRay)
{
	float3 L = sphere.xyz - ray.origin;
	float tCA = dot(ray.dir, L);
	outRay = ray;
	if (tCA < 0.0f)
		return LARGE_DISTANCE;
		
	float d = sqrt(dot(L, L) - tCA*tCA);
	if (d > sphere.w)
		return LARGE_DISTANCE;
	
	float tHC = sqrt(sphere.w*sphere.w - d*d);
	float traveled = tCA - tHC;
	float3 hit = ray.origin + traveled*ray.dir;
	outRay.origin = hit;
	float3 hitNormal = normalize(hit - sphere.xyz);
	outRay.dir = ray.dir + 2 * dot(-ray.dir, hitNormal) * hitNormal;
	outRay.color = mat.diffuse;
	return traveled;
}

float3 traceRay(Ray ray, out Ray outRay)
{
	Ray closestReflectedRay = ray;
	closestReflectedRay.color = 0.0f;
	float closestHit = LARGE_DISTANCE;
	
	Material red;
	red.diffuse = float3(1.0f, 0.0f, 0.0f);
	float dist = checkSphere(ray, float4(0.0f, 0.0f, 2.0f, 0.5f), red, outRay);
	if (dist < closestHit)
	{
		closestReflectedRay = outRay;
		closestHit = dist;
	}
	
	Material green;
	green.diffuse = float3(0.0f, 1.0f, 0.0f);
	dist = checkSphere(ray, float4(1.0f, 0.0f, 1.5f, 0.5f), green, outRay);
	if (dist < closestHit)
	{
		closestReflectedRay = outRay;
		closestHit = dist;
	}
	
	Material blue;
	blue.diffuse = float3(0.0f, 0.0f, 1.0f);
	dist = checkSphere(ray, float4(0.5f, 1.0f, 1.7f, 0.5f), blue, outRay);
	if (dist < closestHit)
	{
		closestReflectedRay = outRay;
		closestHit = dist;
	}
	
	outRay = closestReflectedRay;
	return outRay.color;
}

float3 main(VSOutput input) : SV_Target
{
	float2 screen = float2(1920.f, 1080.f);
	float aspectRatio = screen.x/screen.y;
	
	// z+ towards screen
	// x+ right
	// y+ up
	
	Ray r;
	r.origin = float3(0.0f, 0.0f, 0.0f);
	r.dir = normalize(float3(aspectRatio*(input.pos.x/screen.x*2.0f - 1.0f), -1*(input.pos.y/screen.y*2.0f - 1.0f), 1.0f) - r.origin);
	r.color = float3(0.0f, 0.0f, 0.0f);

	Ray nextRay = r;
	float3 color = 0.f;
	const int BOUNCES = 8;
	float k = 0.5f;
	for (int i = 0; i < BOUNCES; i++)
	{
		color = color + k*traceRay(r, nextRay);
		k = k/2;
		r = nextRay;
	}
	return color;
}