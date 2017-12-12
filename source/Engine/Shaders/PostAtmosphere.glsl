<VERTEX>
	#version 330 core
	
	in vec3 position;
	
	out vec4 Texcoord;
	
	uniform mat4 model;
	uniform mat4 worldViewProj;
	
	void main()
	{
		vec4 pos = model*vec4(position, 1.0);
		pos = worldViewProj*pos;
		gl_Position = pos;
		Texcoord = pos;
	}
</VERTEX>
<FRAGMENT>
	#version 330 core
	
	#include "Common.glsl"
	#include "CommonDeferred.glsl"
	#include "CommonPBR.glsl"
	#include "CommonAtmosphere.glsl"
	
	in vec4 Texcoord;
	
	//out
	layout (location = 0) out vec4 outColor;
	
	GBUFFER_SAMPLER
	uniform mat4 viewProjInv;
	
	uniform vec3 Position;
	
	uniform vec3 SunDir;
	uniform float SunIntensity = 1;
	
	uniform float EPSILON = 0.0001f;

	//precomputed textures
	uniform sampler3D uTexInscatter;
	uniform sampler2D uTexIrridiance;
	uniform sampler2D uTexTransmittance;

	vec3 GetInscatteredLight(in vec3 pos, in vec3 camPos, in vec3 viewDir, inout vec3 attenuation, inout float irradianceFactor)
	{
		vec3 inscatteredLight = vec3(0);
		//Calculate atmoSPHERE intersections
		float ffDist = 0;
		float bfDist = 0;
		float pathLength = 0;
		if(intersectSphere(viewDir, Position, camPos, Radius, ffDist, bfDist))
		{
			pathLength = min(length(pos-camPos), bfDist);
			if(pathLength >= ffDist)
			{
				//intersection is behind front edge of atmosphere
				vec3 startPos = camPos + viewDir * ffDist;
				float startPosHeight = length(startPos - Position);
				
				// starting position of path is now ensured to be inside atmosphere
				// was either originally there or has been moved to top boundary
				float muStartPos = dot(startPos, viewDir) / startPosHeight;
				float nuStartPos = dot(viewDir, SunDir);
				float musStartPos = dot(startPos, SunDir) / startPosHeight;

				// in-scattering for infinite ray (light in-scattered when
				// no surface hit or object behind atmosphere)
				vec4 inscatter = max(texture4D(uTexInscatter, startPosHeight, muStartPos, musStartPos, nuStartPos), 0.0f);
				
				vec3 endPos = camPos + viewDir * pathLength;
				float endPosHeight = length(endPos - Position);
				float musEndPos = dot(pos, SunDir) / endPosHeight;
				
				pathLength -= ffDist;
				
				attenuation = analyticTransmittance(startPosHeight, muStartPos, pathLength);
				if(pathLength < bfDist-ffDist)
				{
					float muEndPos = dot(pos, viewDir) / endPosHeight;
					vec4 inscatterSurface = texture4D(uTexInscatter, endPosHeight, muEndPos, musEndPos, nuStartPos);
					inscatter = max(inscatter-attenuation.rgbr*inscatterSurface, 0.0f);
					irradianceFactor = 1.0f;
				}

				float muHorizon = -sqrt(1.0 - (SurfaceRadius / startPosHeight) * (SurfaceRadius / startPosHeight));
				// avoids imprecision problems near horizon by interpolating between
				// two points above and below horizon
				// fíx described in chapter 5.1.2
				if (abs(muStartPos - muHorizon) < EPSILON_INSCATTER)
				{
					float mu = muHorizon - EPSILON_INSCATTER;
					float samplePosHeight = sqrt(startPosHeight*startPosHeight +pathLength*pathLength+2.0f*startPosHeight* pathLength*mu);
					float muSamplePos = (startPosHeight * mu + pathLength)/ samplePosHeight;
					vec4 inScatter0 = texture4D(uTexInscatter, startPosHeight, mu, musStartPos, nuStartPos);
					vec4 inScatter1 = texture4D(uTexInscatter, samplePosHeight, muSamplePos, musEndPos, nuStartPos);
					vec4 inScatterA = max(inScatter0-attenuation.rgbr*inScatter1,0.0);

					mu = muHorizon + EPSILON_INSCATTER;
					samplePosHeight = sqrt(startPosHeight*startPosHeight +pathLength*pathLength+2.0f* startPosHeight*pathLength*mu);
					muSamplePos = (startPosHeight * mu + pathLength) / samplePosHeight;
					inScatter0 = texture4D(uTexInscatter, startPosHeight, mu, musStartPos, nuStartPos);
					inScatter1 = texture4D(uTexInscatter, samplePosHeight, muSamplePos, musEndPos, nuStartPos);

					vec4 inScatterB = max(inScatter0 - attenuation.rgbr * inScatter1, 0.0f);
					float t = ((muStartPos - muHorizon) + EPSILON_INSCATTER) / (2.0 * EPSILON_INSCATTER);

					inscatter = mix(inScatterA, inScatterB, t);
				}

				// avoids imprecision problems in Mie scattering when sun is below
				//horizon
				// fíx described in chapter 5.1.3
				inscatter.w *= smoothstep(0.00, 0.02, musStartPos);
				float phaseR = phaseFunctionR(nuStartPos);
				float phaseM = phaseFunctionM(nuStartPos);
				inscatteredLight = max(inscatter.rgb * phaseR + getMie(inscatter)* phaseM, 0.0f) * SunIntensity;
			}
		}
		////Debugging
		//attenuation *= pathLength/(Radius)*10;

		return inscatteredLight;
	}

	vec3 GetReflectedLight(in vec3 norm, in float reflectance, in vec3 col, in vec3 surfacePos, in vec3 attenuation, in float irradianceFactor)
	{
		float lightIntensity = SunIntensity * reflectance;
		float lightScale = max(dot(norm, SunDir), 0.0f);
		// irradiance at surface position due to sky light
		float surfacePosHeight = length(surfacePos);
		float musSurfacePos = dot(surfacePos, SunDir) / surfacePosHeight;
		vec3 irradianceSurface = irradiance(uTexIrridiance, surfacePosHeight, musSurfacePos) * irradianceFactor;
		// attenuate direct sun light on its path from top of atmosphere to
		// surface position
		vec3 attenuationSunLight = transmittance(uTexTransmittance, surfacePosHeight,musSurfacePos);
		vec3 reflectedLight = col * (lightScale * attenuationSunLight + irradianceSurface) * lightIntensity;
		// attenuate again on path from surface position to camera
		reflectedLight *= attenuation;

		return reflectedLight;
	}
	
	void main()
	{
		vec2 tc = (Texcoord.xyz/Texcoord.w).xy;//+vec2(1))*0.5f;
		vec3 viewRay = (viewProjInv * vec4(tc, 1, 1)).xyz;
		tc += vec2(1);
		tc *= 0.5f;
		UNPACK_GBUFFER(tc, viewRay) //maybe use ao??
		
		//View dir and reflection
		vec3 viewDir = normalize(viewRay);
		
		vec3 attenuation = vec3(1);
		float irradianceFactor = 0;	

		float reflectance = 1 - rough;

		vec3 inscatteredLight = GetInscatteredLight(pos, camPos, viewDir, attenuation, irradianceFactor);
		vec3 reflectedLight = GetReflectedLight(norm, reflectance, baseCol, pos, attenuation, irradianceFactor);
		
		//output
		outColor = vec4(reflectedLight+inscatteredLight, 1);
	}
</FRAGMENT>