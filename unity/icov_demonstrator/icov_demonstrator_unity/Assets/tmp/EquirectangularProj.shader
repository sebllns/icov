Shader "tmp/EquirectProj"
{
    Properties
    {
        // we have removed support for texture tiling/offset,
        // so make them not be displayed in material inspector
        [NoScaleOffset] _MainTex ("Texture", 2D) = "white" {}

    }
    SubShader
    {
        Pass
        {

             Tags
             {
                 "Queue" = "Transparent"
                 "RenderType" = "Transparent"
             }
             Blend SrcAlpha OneMinusSrcAlpha

            CGPROGRAM
            // use "vert" function as the vertex shader
            #pragma vertex vert
            // use "frag" function as the pixel (fragment) shader
            #pragma fragment frag

            // vertex shader inputs
            struct appdata
            {
                float4 vertex : POSITION; // vertex position
                float2 uv : TEXCOORD0; // texture coordinate
                float4 color    : COLOR;     //    The color value of this vertex specifically. //  
            };

            // vertex shader outputs ("vertex to fragment")
            struct v2f
            {
                float2 uv : TEXCOORD0; // texture coordinate
                float4 vertex : SV_POSITION; // clip space position
                float4  color : COLOR;
            };

            // vertex shader
            v2f vert (appdata v)
            {
                v2f o;
                // transform position to clip space
                // (multiply with model*view*projection matrix)
                o.vertex = UnityObjectToClipPos(v.vertex);
                // just pass the texture coordinate
                o.uv = v.uv;
                o.color = v.color;
                return o;
            }


            // texture we will sample
            sampler2D _MainTex;

            #define PI 3.141592653589793
            #define DEG2RAD 0.01745329251994329576923690768489

            // pixel shader; returns low precision ("fixed4" type)
            // color ("SV_Target" semantic)
            fixed4 frag (v2f i) : SV_Target
            {    
                int count = 10;
                float strength = 0.05f;

                float theta = 90*DEG2RAD + (i.uv.x*2 + 1) * 3.141592654 / 2;
                float phi = 0*DEG2RAD + (-(i.uv.y * 2 - 1) * 3.141592654) / 2;

                float x = cos(phi) * sin(theta);
                float y = sin(phi);
                float z = cos(phi) * cos(theta);

                float scale = 1.0/x;
                float2 px;
                float2 offset;

                 px.x = (-z * scale + 1.0) / 2.0f;
                 px.y = (y * scale + 1.0) / 2.0f;
                
                if(abs(px.x-0.5) < 1.0 && abs(px.y-0.5) < 1.0)
                {  
                return tex2D(_MainTex, fixed2(px.x, px.y)) * i.color;
                }

                return fixed4(0,0,0,1);
            }
            ENDCG
        }
    }
}