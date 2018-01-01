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
	uniform vec2 uSunSize;
	
	uniform float EPSILON = 0.0001f;

	//precomputed textures
	uniform sampler3D uTexInscatter;
	uniform sampler3D uTexSingleMie;//unused
	uniform sampler2D uTexIrridiance;
	uniform sampler2D uTexTransmittance;

	uniform AtmosphereParameters uAtmosphere;
	uniform vec3 uSkySpectralRadToLum;
	uniform vec3 uSunSpectralRadToLum;

	const vec3 kSphereCenter = vec3(0.0, 0.0, 1.0);
	const float kSphereRadius = 1.0;
	const vec3 kSphereAlbedo = vec3(0.8);
	const vec3 kGroundAlbedo = vec3(0.0, 0.0, 0.04);

	//vec3 GetInscatteredLight(in vec3 pos, in vec3 camPos, in vec3 viewDir, inout vec3 attenuation, inout float irradianceFactor)
	//{
	//	vec3 inscatteredLight = vec3(0);
	//	//Calculate atmoSPHERE intersections
	//	float ffDist = 0;
	//	float bfDist = 0;
	//	float pathLength = 0;
	//	if(intersectSphere(viewDir, Position, camPos, Radius, ffDist, bfDist))
	//	{
	//		pathLength = min(length(pos-camPos), bfDist);
	//		if(pathLength >= ffDist)
	//		{
	//			//intersection is behind front edge of atmosphere
	//			vec3 startPos = camPos + viewDir * ffDist;
	//			float startPosHeight = length(startPos - Position);
				
	//			// starting position of path is now ensured to be inside atmosphere
	//			// was either originally there or has been moved to top boundary
	//			float muStartPos = dot(startPos, viewDir) / startPosHeight;
	//			float nuStartPos = dot(viewDir, SunDir);
	//			float musStartPos = dot(startPos, SunDir) / startPosHeight;

	//			// in-scattering for infinite ray (light in-scattered when
	//			// no surface hit or object behind atmosphere)
	//			vec4 inscatter = max(texture4D(uTexInscatter, startPosHeight, muStartPos, musStartPos, nuStartPos), 0.0f);
				
	//			vec3 endPos = camPos + viewDir * pathLength;
	//			float endPosHeight = length(endPos - Position);
	//			float musEndPos = dot(pos, SunDir) / endPosHeight;
				
	//			pathLength -= ffDist;
				
	//			attenuation = analyticTransmittance(startPosHeight, muStartPos, pathLength);
	//			if(pathLength < bfDist-ffDist)
	//			{
	//				float muEndPos = dot(pos, viewDir) / endPosHeight;
	//				vec4 inscatterSurface = texture4D(uTexInscatter, endPosHeight, muEndPos, musEndPos, nuStartPos);
	//				inscatter = max(inscatter-attenuation.rgbr*inscatterSurface, 0.0f);
	//				irradianceFactor = 1.0f;
	//			}

	//			float muHorizon = -sqrt(1.0 - (SurfaceRadius / startPosHeight) * (SurfaceRadius / startPosHeight));
	//			// avoids imprecision problems near horizon by interpolating between
	//			// two points above and below horizon
	//			// fíx described in chapter 5.1.2
	//			if (abs(muStartPos - muHorizon) < EPSILON_INSCATTER)
	//			{
	//				float mu = muHorizon - EPSILON_INSCATTER;
	//				float samplePosHeight = sqrt(startPosHeight*startPosHeight +pathLength*pathLength+2.0f*startPosHeight* pathLength*mu);
	//				float muSamplePos = (startPosHeight * mu + pathLength)/ samplePosHeight;
	//				vec4 inScatter0 = texture4D(uTexInscatter, startPosHeight, mu, musStartPos, nuStartPos);
	//				vec4 inScatter1 = texture4D(uTexInscatter, samplePosHeight, muSamplePos, musEndPos, nuStartPos);
	//				vec4 inScatterA = max(inScatter0-attenuation.rgbr*inScatter1,0.0);

	//				mu = muHorizon + EPSILON_INSCATTER;
	//				samplePosHeight = sqrt(startPosHeight*startPosHeight +pathLength*pathLength+2.0f* startPosHeight*pathLength*mu);
	//				muSamplePos = (startPosHeight * mu + pathLength) / samplePosHeight;
	//				inScatter0 = texture4D(uTexInscatter, startPosHeight, mu, musStartPos, nuStartPos);
	//				inScatter1 = texture4D(uTexInscatter, samplePosHeight, muSamplePos, musEndPos, nuStartPos);

	//				vec4 inScatterB = max(inScatter0 - attenuation.rgbr * inScatter1, 0.0f);
	//				float t = ((muStartPos - muHorizon) + EPSILON_INSCATTER) / (2.0 * EPSILON_INSCATTER);

	//				inscatter = mix(inScatterA, inScatterB, t);
	//			}

	//			// avoids imprecision problems in Mie scattering when sun is below
	//			//horizon
	//			inscatter.w *= smoothstep(0.00, 0.02, musStartPos);
	//			float phaseR = phaseFunctionR(nuStartPos);
	//			float phaseM = phaseFunctionM(nuStartPos);
	//			inscatteredLight = max(inscatter.rgb * phaseR + getMie(inscatter)* phaseM, 0.0f) * SunIntensity;
	//		}
	//	}
	//	////Debugging
	//	//attenuation *= pathLength/(Radius)*10;

	//	return inscatteredLight;
	//}

	//vec3 GetReflectedLight(in vec3 norm, in float reflectance, in vec3 col, in vec3 surfacePos, in vec3 attenuation, in float irradianceFactor)
	//{
	//	float lightIntensity = SunIntensity * reflectance;
	//	float lightScale = max(dot(norm, SunDir), 0.0f);
	//	// irradiance at surface position due to sky light
	//	float surfacePosHeight = length(surfacePos);
	//	float musSurfacePos = dot(surfacePos, SunDir) / surfacePosHeight;
	//	vec3 irradianceSurface = irradiance(uTexIrridiance, surfacePosHeight, musSurfacePos) * irradianceFactor;
	//	// attenuate direct sun light on its path from top of atmosphere to
	//	// surface position
	//	vec3 attenuationSunLight = transmittance(uTexTransmittance, surfacePosHeight,musSurfacePos);
	//	vec3 reflectedLight = col * (lightScale * attenuationSunLight + irradianceSurface) * lightIntensity;
	//	// attenuate again on path from surface position to camera
	//	reflectedLight *= attenuation;

	//	return reflectedLight;
	//}

    vec3 GetSolarRadiance() 
	{
		return uAtmosphere.solar_irradiance / (PI * uAtmosphere.sun_angular_radius * uAtmosphere.sun_angular_radius);
    }
    vec3 GetSkyRadiance( vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance) 
	{
		return GetSkyRadiance(uAtmosphere, uTexTransmittance, uTexInscatter, uTexSingleMie,
			camera, view_ray, shadow_length, sun_direction, transmittance);
    }
    vec3 GetSkyRadianceToPoint( vec3 camera, vec3 point, float shadow_length, vec3 sun_direction, out vec3 transmittance) 
	{
		return GetSkyRadianceToPoint(uAtmosphere, uTexTransmittance, uTexInscatter, uTexSingleMie,
			camera, point, shadow_length, sun_direction, transmittance);
    }
    vec3 GetSunAndSkyIrradiance( vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance) 
	{
		return GetSunAndSkyIrradiance(uAtmosphere, uTexTransmittance, uTexIrridiance, p, normal, sun_direction, sky_irradiance);
    }
    vec3 GetSolarLuminance() 
	{
		return uAtmosphere.solar_irradiance / (PI * uAtmosphere.sun_angular_radius * uAtmosphere.sun_angular_radius) * uSunSpectralRadToLum;
    }
    vec3 GetSkyLuminance( vec3 camera, vec3 view_ray, float shadow_length, vec3 sun_direction, out vec3 transmittance) 
	{
		return GetSkyRadiance(uAtmosphere, uTexTransmittance, uTexInscatter, uTexSingleMie,
			camera, view_ray, shadow_length, sun_direction, transmittance) * uSkySpectralRadToLum;
    }
    vec3 GetSkyLuminanceToPoint( vec3 camera, vec3 point, float shadow_length, vec3 sun_direction, out vec3 transmittance) 
	{
		return GetSkyRadianceToPoint(uAtmosphere, uTexTransmittance, uTexInscatter, uTexSingleMie,
			camera, point, shadow_length, sun_direction, transmittance) * uSkySpectralRadToLum;
    }
    vec3 GetSunAndSkyIlluminance( vec3 p, vec3 normal, vec3 sun_direction, out vec3 sky_irradiance) 
	{
		vec3 sun_irradiance = GetSunAndSkyIrradiance( uAtmosphere, uTexTransmittance, uTexIrridiance, 
			p, normal, sun_direction, sky_irradiance);
		sky_irradiance *= uSkySpectralRadToLum;
		return sun_irradiance * uSunSpectralRadToLum;
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

		//vec3 inscatteredLight = GetInscatteredLight(pos, camPos, viewDir, attenuation, irradianceFactor);
		//vec3 reflectedLight = GetReflectedLight(norm, reflectance, baseCol, pos, attenuation, irradianceFactor);
		
		//output
		//outColor = vec4(reflectedLight+inscatteredLight, 1);

		// Tangent of the angle subtended by this fragment.
		//float fragment_angular_size = length(dFdx(view_ray) + dFdy(view_ray)) / length(view_ray);
		float fragment_angular_size = length(dFdx(viewRay) + dFdy(viewRay)) / length(viewRay);

		//float shadow_in;
		//float shadow_out;

		// Hack to fade out light shafts when the Sun is very close to the horizon.
		float lightshaft_fadein_hack = smoothstep( 0.02, 0.04, dot(normalize(camPos - Position), SunDir));

		// Compute the distance between the view ray line and the Earth center,
		// and the distance between the camera and the intersection of the view
		// ray with the ground (or NaN if there is no intersection).
		// Compute the radiance reflected by the ground, if the ray intersects it.
		vec3 finCol = vec3(0);

		float bfDist; //back face
		float ffDist; //front face
		if (intersectSphere(viewDir, Position, camPos, Radius, ffDist, bfDist)) 
		{
			float ground_alpha = 0.0;
			vec3 ground_radiance = vec3(0.0);

			float fragDist = length(pos-camPos);
			float pathLength = min(fragDist, bfDist);
			if(pathLength >= ffDist)
			{
				vec3 point = camPos + viewDir * pathLength;
				vec3 localPoint = point - Position;
				vec3 normal = normalize(localPoint);

				// Compute the radiance reflected by the ground.
				vec3 sky_irradiance;
				vec3 sun_irradiance = GetSunAndSkyIrradiance( localPoint, normal, SunDir, sky_irradiance);
				//ground_radiance = kGroundAlbedo * (1.0 / PI) * ( sun_irradiance /* * GetSunVisibility(point, SunDir) */ + sky_irradiance /** GetSkyVisibility(point)*/);
				ground_radiance = kGroundAlbedo * (1.0 / PI) * ( sun_irradiance + sky_irradiance );

				float shadow_length = 0;//max(0.0, min(shadow_out, ffDist) - shadow_in) * lightshaft_fadein_hack;
				vec3 transmittance;
				vec3 in_scatter = GetSkyRadianceToPoint(camPos - Position, localPoint, shadow_length, SunDir, transmittance);
				ground_radiance = ground_radiance * transmittance + in_scatter;
				ground_alpha = fragDist < bfDist ? 1.0 : 0.0;
			}
			// Compute the radiance of the sky.
			float shadow_length = 0;//max(0.0, shadow_out - shadow_in) * lightshaft_fadein_hack;
			vec3 transmittance;
			vec3 radiance = GetSkyRadiance( camPos - Position, viewDir, shadow_length, SunDir, transmittance);

			// If the view ray intersects the Sun, add the Sun radiance.
			if (dot(viewDir, SunDir) > uSunSize.y) 
			{
				radiance = radiance + transmittance * GetSolarRadiance();
			}
			radiance = mix(radiance, ground_radiance, ground_alpha);

			finCol = vec3(radiance);
		}
		outColor = vec4(finCol, 1);
	}
</FRAGMENT>