#version 150

// Uniforms
uniform samplerCube u_texUnit0;
uniform samplerCube u_texUnit1;
uniform bool u_texture0Enabled;
uniform bool u_texture1Enabled;

// Varyings
in vec4 v_frontColor;
in vec3 v_texCoord0;
in vec3 v_texCoord1;

out vec4 fragColor;

void main()
{
  vec4 color = v_frontColor;

  if (u_texture1Enabled)
  {
    if (u_texture0Enabled)
    {
      vec4 texture0Color = texture(u_texUnit0, v_texCoord0);
      color.rgb = mix(color.rgb, texture0Color.rgb, texture0Color.a);
    }
    color *= texture(u_texUnit1, v_texCoord1);
  }
  else if (u_texture0Enabled)
    color *= texture(u_texUnit0, v_texCoord0) * v_frontColor;

  fragColor = color;
}
