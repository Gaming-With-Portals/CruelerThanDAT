#pragma once
const char* solidColorVTX = R"(
    float4x4 WorldViewProj;

    struct VS_INPUT {
        float3 position : POSITION;
    };
    
    struct VS_OUTPUT {
        float4 position : POSITION;
        float4 color : COLOR0;
    };
    
    VS_OUTPUT main(VS_INPUT input) {
        VS_OUTPUT output;
        output.position = float4(input.position, 1.0);
        output.color = float4(abs(input.position.x), abs(input.position.z), abs(input.position.y), 1.0f);
        return output;
    }
)";


const char* solidColorPX = R"(
    struct VS_OUTPUT {
        float4 position : POSITION;
        float4 color : COLOR0;
    };


    float4 main(VS_OUTPUT input) : COLOR {
        return input.color; 
    }
)";