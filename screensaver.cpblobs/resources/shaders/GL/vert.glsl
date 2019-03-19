#version 150

// Attributes
in vec4 a_vertex;
in vec4 a_normal;
in vec4 a_color;
in vec2 a_coord;

// Uniforms
uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_transposeAdjointModelViewMatrix;
uniform mat4 u_textureMatrix;
uniform bool u_texture0Enabled;
uniform bool u_texture1Enabled;

// Varyings
out vec4 v_frontColor;
out vec3 v_texCoord0;
out vec3 v_texCoord1;

vec3 ReflectionMap(const vec3 U, const vec3 N)
{
  vec3 R;
  R = reflect(U, N);
  R = normalize(R);
  return R;
}

void main()
{
  gl_Position = u_projectionMatrix * u_modelViewMatrix * a_vertex;

  if (u_texture0Enabled || u_texture1Enabled)
  {
    vec4 position = u_textureMatrix * a_vertex;
    vec3 p = (vec3(position))/position.w;

    // transformed normal
    vec3 n = u_transposeAdjointModelViewMatrix * a_normal.rgb;

    // gen texture coordinates
    vec3 sMap = ReflectionMap(normalize(p), normalize(n));
    if (!u_texture1Enabled)
      v_texCoord0 = sMap;
    else
      v_texCoord0 = vec3(a_coord, 1.0);

    v_texCoord1 = sMap;
  }

  v_frontColor = a_color;
}
