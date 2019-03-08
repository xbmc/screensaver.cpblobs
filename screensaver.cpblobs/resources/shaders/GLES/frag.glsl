#version 100

precision mediump float;

// Uniforms
uniform samplerCube u_texUnit0;
uniform samplerCube u_texUnit1;
uniform bool u_texture0Enabled;
uniform bool u_texture1Enabled;

// Varyings
varying vec4 v_frontColor;
varying vec3 v_texCoord0;
varying vec3 v_texCoord1;

void main()
{
  vec4 color = v_frontColor;

  if (u_texture1Enabled)
  {
    if (u_texture0Enabled)
    {
      vec4 texture0Color = textureCube(u_texUnit0, v_texCoord0);
      color.rgb = mix(color.rgb, texture0Color.rgb, texture0Color.a);
    }
    color *= textureCube(u_texUnit1, v_texCoord1);
  }
  else if (u_texture0Enabled)
    color *= textureCube(u_texUnit0, v_texCoord0) * v_frontColor;

  gl_FragColor = color;
}
