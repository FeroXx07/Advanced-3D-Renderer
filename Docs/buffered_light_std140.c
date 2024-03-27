
struct Light
{
	unsigned int type;
	vec3 color;
	vec3 direction;
	vec3 position;
}

layout (binding = 0, std140) uniform GlobalParams
{
									// base alignment  // base offset	// aligned offset	// bytes used
	vec3 uCameraPosition;     		// 16              // 0	 			// 0 				// 0..15
	unsigned int uLightCount; 		// 4               // 16			// 16               // 16..20
	struct Light					// 16              // 20			// 32               // 20..31 (align begin)
	{
		unsigned int type;			// 4			   // 32			// 32				// 32..35 uLight[0]
		vec3 color;					// 16			   // 36			// 48				// 48..63 uLight[0]
		vec3 direction;				// 16			   // 64			// 64				// 64..79 uLight[0]
		vec3 position;				// 16			   // 80			// 80 				// 80..95 uLight[0]
									// 16			   // 96			// 96				// padding end of uLight[0] 
									
									// 4			   // 96			// 96				// 96..99 uLight[1]
									// 16			   // 100			// 112				// 112..127 uLight[1]
									// 16			   // 128			// 128				// 128..143 uLight[1]
									// 16			   // 144			// 144 				// 144..159 uLight[1]
									// 16			   // 160			// 160				// 160..176 padding end of uLight[1] 
	} uLight[16];     		   		
}; 