

#version 330 core
#extension GL_ARB_shading_language_420pack: require


#extension GL_ARB_texture_gather: require



layout(std140) uniform;
#define FRAG_COLOR		0

layout(location = FRAG_COLOR, index = 0) out vec4 outColour;



in vec4 gl_FragCoord;




// START UNIFORM DECLARATION

	
		
struct ShadowReceiverData
{
    mat4 texViewProj;
	vec2 shadowDepthRange;
	vec4 invShadowMapSize;
};

struct Light
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;

	vec3 attenuation;
	vec3 spotDirection;
	vec3 spotParams;
};

//Uniforms that change per pass
layout(binding = 0) uniform PassBuffer
{
	//Vertex shader (common to both receiver and casters)
	mat4 viewProj;


	//Vertex shader
	mat4 view;
	ShadowReceiverData shadowRcv[4];
	//-------------------------------------------------------------------------

	//Pixel shader
	mat3 invViewMatCubemap;



	float pssmSplitPoints0;
	float pssmSplitPoints1;
	float pssmSplitPoints2;	Light lights[2];

	//f3dData.x = minDistance;
	//f3dData.y = invMaxDistance;
	//f3dData.z = f3dNumSlicesSub1;
	//f3dData.w = uint cellsPerTableOnGrid0 (floatBitsToUint);
	vec4 f3dData;
	vec4 f3dGridHWW[5];
	
} pass;

	
	
//Uniforms that change per Item/Entity, but change very infrequently
struct Material
{
	/* kD is already divided by PI to make it energy conserving.
	  (formula is finalDiffuse = NdotL * surfaceDiffuse / PI)
	*/
	vec4 bgDiffuse;
	vec4 kD; //kD.w is alpha_test_threshold
	vec4 kS; //kS.w is roughness
	//Fresnel coefficient, may be per colour component (vec3) or scalar (float)
	//F0.w is transparency
	vec4 F0;
	vec4 normalWeights;
	vec4 cDetailWeights;
	vec4 detailOffsetScaleD[4];
	vec4 detailOffsetScaleN[4];

	uvec4 indices0_3;
	//uintBitsToFloat( indices4_7.w ) contains mNormalMapWeight.
	uvec4 indices4_7;
};

layout(binding = 1) uniform MaterialBuf
{
	Material m[256];
} materialArray;

	
//Uniforms that change per Item/Entity
layout(binding = 2) uniform InstanceBuffer
{
    //.x =
	//The lower 9 bits contain the material's start index.
    //The higher 23 bits contain the world matrix start index.
    //
    //.y =
    //shadowConstantBias. Send the bias directly to avoid an
    //unnecessary indirection during the shadow mapping pass.
    //Must be loaded with uintBitsToFloat
    uvec4 worldMaterialIdx[4096];
} instance;



// END UNIFORM DECLARATION
in block
{

    
		
			flat uint drawId;
				
			vec3 pos;
			vec3 normal;
			vec3 tangent;
				flat float biNormalReflection;							
			vec2 uv0;
			vec4 uv1;
		
			vec4 posL0;
			vec4 posL1;
			vec4 posL2;
			vec4 posL3;		float depth;			

} inPs;




/*layout(binding = 1) */uniform usamplerBuffer f3dGrid;
/*layout(binding = 2) */uniform samplerBuffer f3dLightList;

uniform sampler2DArray textureMaps[3];


	uint diffuseIdx;
	uint normalIdx;
	uint specularIdx;
	uint roughnessIdx;


	
	
	
	

	
	
	
	


vec4 diffuseCol;
vec3 specularCol;

float ROUGHNESS;

Material material;
vec3 nNormal;










uniform sampler2DShadow texShadowMap[4];

float getShadow( sampler2DShadow shadowMap, vec4 psPosLN, vec4 invShadowMapSize )
{
	float fDepth = psPosLN.z;
	vec2 uv = psPosLN.xy / psPosLN.w;

	float retVal = 0;



	vec2 fW;
	vec4 c;

	
		

		
			retVal += texture( shadowMap, vec3( uv, fDepth ) ).r;
		
	

	

	

	return retVal;
}




vec3 getTSNormal( vec3 uv )
{
	vec3 tsNormal;

	//Normal texture must be in U8V8 or BC5 format!
	tsNormal.xy = texture( textureMaps[1], uv ).xy;

	tsNormal.z	= sqrt( max( 0, 1.0 - tsNormal.x * tsNormal.x - tsNormal.y * tsNormal.y ) );

	return tsNormal;
}






//Default BRDF
vec3 BRDF( vec3 lightDir, vec3 viewDir, float NdotV, vec3 lightDiffuse, vec3 lightSpecular )
{
	vec3 halfWay= normalize( lightDir + viewDir );
	float NdotL = clamp( dot( nNormal, lightDir ), 0.0, 1.0 );
	float NdotH = clamp( dot( nNormal, halfWay ), 0.0, 1.0 );
	float VdotH = clamp( dot( viewDir, halfWay ), 0.0, 1.0 );

	float sqR = ROUGHNESS * ROUGHNESS;

	//Roughness/Distribution/NDF term (GGX)
	//Formula:
	//	Where alpha = roughness
	//	R = alpha^2 / [ PI * [ ( NdotH^2 * (alpha^2 - 1) ) + 1 ]^2 ]
	float f = ( NdotH * sqR - NdotH ) * NdotH + 1.0;
	float R = sqR / (f * f + 1e-6f);

	//Geometric/Visibility term (Smith GGX Height-Correlated)

	float Lambda_GGXV = NdotL * sqrt( (-NdotV * sqR + NdotV) * NdotV + sqR );
	float Lambda_GGXL = NdotV * sqrt( (-NdotL * sqR + NdotL) * NdotL + sqR );

	float G = 0.5 / (( Lambda_GGXV + Lambda_GGXL + 1e-6f ) * 3.141592654);


	//Formula:
	//	fresnelS = lerp( (1 - V*H)^5, 1, F0 )
	float fresnelS = material.F0.x + pow( 1.0 - VdotH, 5.0 ) * (1.0 - material.F0.x);

	//We should divide Rs by PI, but it was done inside G for performance
	vec3 Rs = ( fresnelS * (R * G) ) * specularCol.xyz * lightSpecular;

	//Diffuse BRDF (*Normalized* Disney, see course_notes_moving_frostbite_to_pbr.pdf
	//"Moving Frostbite to Physically Based Rendering" Sebastien Lagarde & Charles de Rousiers)
	float energyBias	= ROUGHNESS * 0.5;
	float energyFactor	= mix( 1.0, 1.0 / 1.51, ROUGHNESS );
	float fd90			= energyBias + 2.0 * VdotH * VdotH * ROUGHNESS;
	float lightScatter	= 1.0 + (fd90 - 1.0) * pow( 1.0 - NdotL, 5.0 );
	float viewScatter	= 1.0 + (fd90 - 1.0) * pow( 1.0 - NdotV, 5.0 );


	float fresnelD = 1.0f - fresnelS;

	//We should divide Rd by PI, but it is already included in kD
	vec3 Rd = (lightScatter * viewScatter * energyFactor * fresnelD) * diffuseCol.xyz * lightDiffuse;

	return NdotL * (Rs + Rd);
}






void main()
{
    

	
		uint materialId	= instance.worldMaterialIdx[inPs.drawId].x & 0x1FFu;
		material = materialArray.m[materialId];
	
	diffuseIdx			= material.indices0_3.x & 0x0000FFFFu;
	normalIdx			= material.indices0_3.x >> 16u;
	specularIdx			= material.indices0_3.y & 0x0000FFFFu;
	roughnessIdx		= material.indices0_3.y >> 16u;











	



	/// Sample detail maps and weight them against the weight map in the next foreach loop.


	diffuseCol = texture( textureMaps[0], vec3( inPs.uv0.xy, diffuseIdx ) );


	/// 'insertpiece( SampleDiffuseMap )' must've written to diffuseCol. However if there are no
	/// diffuse maps, we must initialize it to some value. If there are no diffuse or detail maps,
	/// we must not access diffuseCol at all, but rather use material.kD directly (see piece( kD ) ).
	

	/// Blend the detail diffuse maps with the main diffuse.


	/// Apply the material's diffuse over the textures
	
		diffuseCol.xyz *= material.kD.xyz;
	




	//Normal mapping.
	vec3 geomNormal = normalize( inPs.normal ) ;
	vec3 vTangent = normalize( inPs.tangent );

	//Get the TBN matrix
	vec3 vBinormal	= normalize( cross( geomNormal, vTangent ) * inPs.biNormalReflection );
	mat3 TBN		= mat3( vTangent, vBinormal, geomNormal );

	nNormal = getTSNormal( vec3( inPs.uv0.xy, normalIdx ) );
	



    float fShadow = 1.0;
    if( inPs.depth <= pass.pssmSplitPoints0 )
        fShadow = getShadow( texShadowMap[0], inPs.posL0, pass.shadowRcv[0].invShadowMapSize );
	else if( inPs.depth <= pass.pssmSplitPoints1 )
        fShadow = getShadow( texShadowMap[1], inPs.posL1, pass.shadowRcv[1].invShadowMapSize );
	else if( inPs.depth <= pass.pssmSplitPoints2 )
        fShadow = getShadow( texShadowMap[2], inPs.posL2, pass.shadowRcv[2].invShadowMapSize );


	specularCol = texture( textureMaps[0], vec3(inPs.uv0.xy, specularIdx) ).xyz * material.kS.xyz;
ROUGHNESS = material.kS.w * texture( textureMaps[2], vec3(inPs.uv0.xy, roughnessIdx) ).x;
ROUGHNESS = max( ROUGHNESS, 0.001f );

	/// If there is no normal map, the first iteration must
	/// initialize nNormal instead of try to merge with it.

	
	


	/// Blend the detail normal maps with the main normal.




	nNormal = normalize( TBN * nNormal );


	//Everything's in Camera space

	vec3 viewDir	= normalize( -inPs.pos );
	float NdotV		= clamp( dot( nNormal, viewDir ), 0.0, 1.0 );


	vec3 finalColour = vec3(0);


	



	finalColour += BRDF( pass.lights[0].position, viewDir, NdotV, pass.lights[0].diffuse, pass.lights[0].specular ) * fShadow;





	vec3 lightDir;
	float fDistance;
	vec3 tmpColour;
	float spotCosAngle;

	//Point lights


	//Spot lights
	//spotParams[0].x = 1.0 / cos( InnerAngle ) - cos( OuterAngle )
	//spotParams[0].y = cos( OuterAngle / 2 )
	//spotParams[0].z = falloff

	lightDir = pass.lights[1].position - inPs.pos;
	fDistance= length( lightDir );
	spotCosAngle = dot( normalize( inPs.pos - pass.lights[1].position ), pass.lights[1].spotDirection );

	if( fDistance <= pass.lights[1].attenuation.x && spotCosAngle >= pass.lights[1].spotParams.y )
	{
		lightDir *= 1.0 / fDistance;
	
	
		float spotAtten = clamp( (spotCosAngle - pass.lights[1].spotParams.y) * pass.lights[1].spotParams.x, 0.0, 1.0 );
		spotAtten = pow( spotAtten, pass.lights[1].spotParams.z );
	
		tmpColour = BRDF( lightDir, viewDir, NdotV, pass.lights[1].diffuse, pass.lights[1].specular ) * getShadow( texShadowMap[3], inPs.posL3, pass.shadowRcv[3].invShadowMapSize );
		float atten = 1.0 / (1.0 + (pass.lights[1].attenuation.y + pass.lights[1].attenuation.z * fDistance) * fDistance );
		finalColour += tmpColour * (atten * spotAtten);
	}


	float f3dMinDistance	= pass.f3dData.x;
	float f3dInvMaxDistance	= pass.f3dData.y;
	float f3dNumSlicesSub1	= pass.f3dData.z;
	uint cellsPerTableOnGrid0= floatBitsToUint( pass.f3dData.w );

	// See C++'s Forward3D::getSliceAtDepth
	/*float fSlice = 1.0 - clamp( (-inPs.pos.z + f3dMinDistance) * f3dInvMaxDistance, 0.0, 1.0 );
	fSlice = (fSlice * fSlice) * (fSlice * fSlice);
	fSlice = (fSlice * fSlice);
	fSlice = floor( (1.0 - fSlice) * f3dNumSlicesSub1 );*/
	float fSlice = clamp( (-inPs.pos.z + f3dMinDistance) * f3dInvMaxDistance, 0.0, 1.0 );
	fSlice = floor( fSlice * f3dNumSlicesSub1 );
	uint slice = uint( fSlice );

	//TODO: Profile performance: derive this mathematically or use a lookup table?
	uint offset = cellsPerTableOnGrid0 * (((1u << (slice << 1u)) - 1u) / 3u);

	float lightsPerCell = pass.f3dGridHWW[0].w;

	//pass.f3dGridHWW[slice].x = grid_width / renderTarget->width;
	//pass.f3dGridHWW[slice].y = grid_height / renderTarget->height;
	//pass.f3dGridHWW[slice].z = grid_width * lightsPerCell;
	//uint sampleOffset = 0;

	uint sampleOffset = offset +
						uint(floor( gl_FragCoord.y * pass.f3dGridHWW[slice].y ) * pass.f3dGridHWW[slice].z) +
						uint(floor( gl_FragCoord.x * pass.f3dGridHWW[slice].x ) * lightsPerCell);


	uint numLightsInGrid = texelFetch( f3dGrid, int(sampleOffset) ).x;

	for( uint i=0u; i<numLightsInGrid; ++i )
	{
		//Get the light index
		uint idx = texelFetch( f3dGrid, int(sampleOffset + i + 1u) ).x;

		//Get the light
		vec4 posAndType = texelFetch( f3dLightList, int(idx) );

		vec3 lightDiffuse	= texelFetch( f3dLightList, int(idx + 1u) ).xyz;
		vec3 lightSpecular	= texelFetch( f3dLightList, int(idx + 2u) ).xyz;
		vec4 attenuation	= texelFetch( f3dLightList, int(idx + 3u) ).xyzw;

		vec3 lightDir	= posAndType.xyz - inPs.pos;
		float fDistance	= length( lightDir );

		if( fDistance <= attenuation.x )
		{
			lightDir *= 1.0 / fDistance;
			float atten = 1.0 / (1.0 + (attenuation.y + attenuation.z * fDistance) * fDistance );
			
				atten *= max( (attenuation.x - fDistance) * attenuation.w, 0.0f );
			

			if( posAndType.w == 1.0 )
			{
				//Point light
				vec3 tmpColour = BRDF( lightDir, viewDir, NdotV, lightDiffuse, lightSpecular );
				finalColour += tmpColour * atten;
			}
			else
			{
				//spotParams.x = 1.0 / cos( InnerAngle ) - cos( OuterAngle )
				//spotParams.y = cos( OuterAngle / 2 )
				//spotParams.z = falloff

				//Spot light
				vec3 spotDirection	= texelFetch( f3dLightList, int(idx + 4u) ).xyz;
				vec3 spotParams		= texelFetch( f3dLightList, int(idx + 5u) ).xyz;

				float spotCosAngle = dot( normalize( inPs.pos - posAndType.xyz ), spotDirection.xyz );

				float spotAtten = clamp( (spotCosAngle - spotParams.y) * spotParams.x, 0.0, 1.0 );
				spotAtten = pow( spotAtten, spotParams.z );
				atten *= spotAtten;

				if( spotCosAngle >= spotParams.y )
				{
					vec3 tmpColour = BRDF( lightDir, viewDir, NdotV, lightDiffuse, lightSpecular );
					finalColour += tmpColour * atten;
				}
			}
		}
	}

	




	//Linear to Gamma space
	outColour.xyz	= sqrt( finalColour );



	outColour.w		= 1.0;



	
}


