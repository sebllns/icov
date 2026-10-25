Shader "Icov/Equirect2"
{
    Properties
    {
        // we have removed support for texture tiling/offset,
        // so make them not be displayed in material inspector
        [NoScaleOffset] _MainTex ("Texture", 2D) = "white" {}

        _CameraHFov("Camera horizontal field of view in degrees", Range (0, 360.0)) = 100.0
        _CameraVFov("Camera vertical field of view in degrees", Range (0, 360.0)) = 100.0
        _PovAngleY("Horizontal Point Of View angle in degrees", Float) = 0.0
        _PovAngleX("Vertical Point Of View angle in degrees", Float) = 0.0


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


            float clampAngle(float angle)
            {
                if(angle < 0.0f)
                {
                    angle += 360.0f;
                }

                if(angle > 360.0f)
                {
                    angle -= 360.0f;
                }

                if(angle < 0.0f || angle > 360.0f) return clampAngle(angle);

                return angle;
            }
            
            // texture we will sample
            sampler2D _MainTex;

            float _CameraHFov, _CameraVFov, _PovAngleY, _PovAngleX;

            #define PI 3.141592653589793
            #define DEG2RAD 0.01745329251994329576923690768489


            //tools
            fixed3 rotateXY(fixed3 p, fixed2 angle) {
                fixed2 c = cos(angle), s = sin(angle);
                p = fixed3(p.x, c.x*p.y + s.x*p.z, -s.x*p.y + c.x*p.z);
                return fixed3(c.y*p.x + s.y*p.z, p.y, -s.y*p.x + c.y*p.z);
            }

            // pixel shader; returns low precision ("fixed4" type)
            // color ("SV_Target" semantic)
            fixed4 frag (v2f i) : SV_Target
            {    

 
                _PovAngleX = clamp(_PovAngleX, -90, 90);
        
                //place 0,0 in center from -1 to 1 ndc
                fixed2 uv = 2.0 * i.uv - 1;

                 //to spherical
                fixed3 camDir = normalize(fixed3(uv * fixed2(tan(0.5 * _CameraHFov * DEG2RAD), tan(0.5 * _CameraVFov * DEG2RAD)), 1.0));

                float theta = _PovAngleY * DEG2RAD;
                float phi = _PovAngleX * DEG2RAD;

                // if(abs(i.uv.x - 0.5) < 0.2 && i.uv.y < (tan(0.7 * PI / 2 - PI / 4) * 0.5 + 0.5))
                // {
                //     return fixed4(1,1,0,1);
                // } 

                //rotate
                fixed3 rd = normalize(rotateXY(camDir, fixed2(phi, theta)));

                //radial azmuth polar
                fixed2 texCoord = fixed2(atan2(rd.x, rd.z) + PI, acos(-rd.y)) / fixed2(2.0 * PI, PI);

                fixed4 col = 0.5 * tex2D(_MainTex, i.uv);

                // x = (atan2(rd.x, rd.z) + PI) / 2.0 * PI
                // x1 = tan(x * 2.O * PI - PI)

                // y = acos(-rd.y) / PI
                // y1 = -cos(y * PI)

                // float x = texCoord.x;
                // float y = texCoord.y;

                float x = texCoord.x;
                float y = texCoord.y;

                float x1 = tan(x * 2.0 * PI - PI);
                float y1 = -cos(y * PI);

                if(abs(x1) < 0.8 && abs(y1) < 0.4) 
                {
                    col *= 2;
                }


                return col * i.color;
            }
            ENDCG
        }
    }
}