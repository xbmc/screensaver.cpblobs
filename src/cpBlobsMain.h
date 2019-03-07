/*
 *  Copyright (C) 2005-2019 Team Kodi
 *  Copyright (C) Simon Windmill (siw@coolpowers.com)
 *  Ported to Kodi GL4 by Alwin Esch <alwinus@kodi.tv>
 *  This file is part of Kodi - https://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <kodi/addon-instance/Screensaver.h>
#include <kodi/gui/gl/GL.h>
#include <kodi/gui/gl/Shader.h>
#include <glm/gtc/type_ptr.hpp>

struct sLight
{
  glm::vec3 vertex;
  glm::vec3 normal;
  glm::vec4 color;
  glm::vec2 coord;
};

/*
 * stuff for the background plane
 */
struct BG_VERTEX
{
  glm::vec3 position;
  glm::vec4 color;
};

class CBlobby;

class ATTRIBUTE_HIDDEN CScreensaverCpBlobs
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver,
    public kodi::gui::gl::CShaderProgram
{
public:
  CScreensaverCpBlobs();
  virtual ~CScreensaverCpBlobs();

  bool Start() override;
  void Stop() override;
  void Render() override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

private:
  static const float g_fTickSpeed;

  glm::mat4 m_projMat;
  glm::mat4 m_modelMat;
  glm::mat3 m_normalMat;
  glm::mat4 m_textureMat;
  GLint m_texture0Used = 0;
  GLint m_texture1Used = 0;

  GLint m_projMatLoc = -1;
  GLint m_modelViewMatLoc = -1;
  GLint m_transposeAdjointModelViewMatrixLoc = -1;
  GLint m_textureMatLoc = -1;
  GLint m_uTexture0UsedLoc = -1;
  GLint m_uTexture1UsedLoc = -1;
  GLint m_uTexUnit0 = -1;
  GLint m_uTexUnit1 = -1;
  GLint m_hVertex = -1;
  GLint m_hNormal = -1;
  GLint m_hCoord = -1;
  GLint m_hColor = -1;

  GLuint m_vertexVBO = 0;
  GLuint m_indexVBO = 0;

  GLuint m_cubeTexture;
  GLuint m_diffuseTexture;
  GLuint m_specTexture;

  CBlobby *m_pBlobby;
  BG_VERTEX m_BGVertices[4];
  float m_fFOV, m_fAspectRatio;
  float m_fTicks;
  glm::vec3 m_WorldRotSpeeds;
  std::string m_strCubemap;
  std::string m_strDiffuseCubemap;
  std::string m_strSpecularCubemap;
  bool m_bShowCube;
  bool m_bShowBlob;
  bool m_bShowDebug;
  glm::vec4 m_BGTopColor, m_BGBottomColor;

  bool m_startOK = false;

  void SetDefaults();
  void SetupGradientBackground(const glm::vec4& dwTopColor, const glm::vec4& dwBottomColor);
  void RenderGradientBackground();
};
