

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










in uint drawId;





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
};

//Uniforms that change per pass
layout(binding = 0) uniform PassBuffer
{
	//Vertex shader (common to both receiver and casters)
	mat4 viewProj;


	//Vertex shader
	vec2 depthRange;

	
} pass;


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

layout(binding = 0) uniform samplerBuffer worldMatBuf;

// END UNIFORM DECLARATION










//SkeletonTransform // !hlms_skeleton


    







void main()
{
    

    mat4x3 worldMat = UNPACK_MAT4x3( worldMatBuf, drawId );
	

    vec4 worldPos = vec4( (worldMat * vertex).xyz, 1.0f );




	
	
	//Lighting is in view space
	
    
    

    gl_Position = pass.viewProj * worldPos;




    float shadowConstantBias = uintBitsToFloat( instance.worldMaterialIdx[drawId].y );

	

	//We can't make the depth buffer linear without Z out in the fragment shader;
	//however we can use a cheap approximation ("pseudo linear depth")
	//see http://www.yosoygames.com.ar/wp/2014/01/linear-depth-buffer-my-ass/
	gl_Position.z = (gl_Position.z + shadowConstantBias * pass.depthRange.y) * pass.depthRange.y * gl_Position.w;


	/// hlms_uv_count will be 0 on shadow caster passes w/out alpha test




	
}
