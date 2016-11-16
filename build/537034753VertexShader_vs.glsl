

#version 330 core
#extension GL_ARB_shading_language_420pack: require


out gl_PerVertex
{
	vec4 gl_Position;
};

layout(std140) uniform;


mat4 UNPACK_MAT4( samplerBuffer matrixBuf, uint pixelIdx )
{
        vec4 row0 = texelFetch( matrixBuf, int((pixelIdx) << 2u) );
        vec4 row1 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 1u) );
        vec4 row2 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 2u) );
        vec4 row3 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 3u) );
    return mat4( row0.x, row1.x, row2.x, row3.x,
                 row0.y, row1.y, row2.y, row3.y,
                 row0.z, row1.z, row2.z, row3.z,
                 row0.w, row1.w, row2.w, row3.w );
}


mat4x3 UNPACK_MAT4x3( samplerBuffer matrixBuf, uint pixelIdx )
{
        vec4 row0 = texelFetch( matrixBuf, int((pixelIdx) << 2u) );
        vec4 row1 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 1u) );
        vec4 row2 = texelFetch( matrixBuf, int(((pixelIdx) << 2u) + 2u) );
        return mat4x3( row0.x, row1.x, row2.x,
                       row0.y, row1.y, row2.y,
                       row0.z, row1.z, row2.z,
                       row0.w, row1.w, row2.w );
}


in vec4 vertex;


in vec4 qtangent;






in vec2 uv0;
in vec4 uv1;

in uint drawId;




out block
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

} outVs;


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


layout(binding = 0) uniform samplerBuffer worldMatBuf;

// END UNIFORM DECLARATION



vec3 xAxis( vec4 qQuat )
{
	float fTy  = 2.0 * qQuat.y;
	float fTz  = 2.0 * qQuat.z;
	float fTwy = fTy * qQuat.w;
	float fTwz = fTz * qQuat.w;
	float fTxy = fTy * qQuat.x;
	float fTxz = fTz * qQuat.x;
	float fTyy = fTy * qQuat.y;
	float fTzz = fTz * qQuat.z;

	return vec3( 1.0-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy );
}



vec3 yAxis( vec4 qQuat )
{
	float fTx  = 2.0 * qQuat.x;
	float fTy  = 2.0 * qQuat.y;
	float fTz  = 2.0 * qQuat.z;
	float fTwx = fTx * qQuat.w;
	float fTwz = fTz * qQuat.w;
	float fTxx = fTx * qQuat.x;
	float fTxy = fTy * qQuat.x;
	float fTyz = fTz * qQuat.y;
	float fTzz = fTz * qQuat.z;

	return vec3( fTxy-fTwz, 1.0-(fTxx+fTzz), fTyz+fTwx );
}










//SkeletonTransform // !hlms_skeleton


    







void main()
{
    

    mat4x3 worldMat = UNPACK_MAT4x3( worldMatBuf, drawId << 1u);
	
    mat4 worldView = UNPACK_MAT4( worldMatBuf, (drawId << 1u) + 1u );
	

    vec4 worldPos = vec4( (worldMat * vertex).xyz, 1.0f );



	//Decode qTangent to TBN with reflection
	vec3 normal		= xAxis( normalize( qtangent ) );
	
	vec3 tangent	= yAxis( qtangent );
	outVs.biNormalReflection = sign( qtangent.w ); //We ensure in C++ qtangent.w is never 0
	


	
	
	//Lighting is in view space
	outVs.pos		= (worldView * vertex).xyz;
    outVs.normal	= mat3(worldView) * normal;
    outVs.tangent	= mat3(worldView) * tangent;

    gl_Position = pass.viewProj * worldPos;




	

    outVs.posL0 = pass.shadowRcv[0].texViewProj * vec4(worldPos.xyz, 1.0f);
    outVs.posL1 = pass.shadowRcv[1].texViewProj * vec4(worldPos.xyz, 1.0f);
    outVs.posL2 = pass.shadowRcv[2].texViewProj * vec4(worldPos.xyz, 1.0f);
    outVs.posL3 = pass.shadowRcv[3].texViewProj * vec4(worldPos.xyz, 1.0f);


	outVs.posL0.z = outVs.posL0.z * pass.shadowRcv[0].shadowDepthRange.y;
	outVs.posL0.z = (outVs.posL0.z * 0.5) + 0.5;
	outVs.posL1.z = outVs.posL1.z * pass.shadowRcv[1].shadowDepthRange.y;
	outVs.posL1.z = (outVs.posL1.z * 0.5) + 0.5;
	outVs.posL2.z = outVs.posL2.z * pass.shadowRcv[2].shadowDepthRange.y;
	outVs.posL2.z = (outVs.posL2.z * 0.5) + 0.5;
	outVs.posL3.z = outVs.posL3.z * pass.shadowRcv[3].shadowDepthRange.y;
	outVs.posL3.z = (outVs.posL3.z * 0.5) + 0.5;

	outVs.depth = gl_Position.z;



	/// hlms_uv_count will be 0 on shadow caster passes w/out alpha test

	outVs.uv0 = uv0;
	outVs.uv1 = uv1;


	outVs.drawId = drawId;

	
}
