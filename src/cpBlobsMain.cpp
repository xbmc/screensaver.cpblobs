/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) Simon Windmill (siw@coolpowers.com)
 *  Ported to Kodi GL4 by Alwin Esch <alwinus@kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

// Kodi screensaver displaying metaballs moving around in an environment

#include "cpBlobsMain.h"
#include "SOIL2/SOIL2.h"
#include "Blobby.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define BUFFER_OFFSET(i) ((char *)nullptr + (i))

/*
 * stuff for the environment cube
 */
struct CubeVertex
{
  glm::vec3 position;
  glm::vec3 normal;
};

/*
 * man, how many times have you typed (or pasted) this data for a cube's
 * vertices and normals, eh?
 */
CubeVertex g_cubeVertices[] =
{
  {glm::vec3(-1.0f, 1.0f,-1.0f), glm::vec3(0.0f, 0.0f,1.0f), },
  {glm::vec3( 1.0f, 1.0f,-1.0f), glm::vec3(0.0f, 0.0f,1.0f), },
  {glm::vec3(-1.0f,-1.0f,-1.0f), glm::vec3(0.0f, 0.0f,1.0f), },
  {glm::vec3( 1.0f,-1.0f,-1.0f), glm::vec3(0.0f, 0.0f,1.0f), },

  {glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), },
  {glm::vec3(-1.0f,-1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), },
  {glm::vec3( 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), },
  {glm::vec3( 1.0f,-1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f), },

  {glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), },
  {glm::vec3( 1.0f, 1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f), },
  {glm::vec3(-1.0f, 1.0f,-1.0f), glm::vec3(0.0f, -1.0f, 0.0f), },
  {glm::vec3( 1.0f, 1.0f,-1.0f), glm::vec3(0.0f, -1.0f, 0.0f), },

  {glm::vec3(-1.0f,-1.0f, 1.0f), glm::vec3(0.0f,1.0f, 0.0f), },
  {glm::vec3(-1.0f,-1.0f,-1.0f), glm::vec3(0.0f,1.0f, 0.0f), },
  {glm::vec3( 1.0f,-1.0f, 1.0f), glm::vec3(0.0f,1.0f, 0.0f), },
  {glm::vec3( 1.0f,-1.0f,-1.0f), glm::vec3(0.0f,1.0f, 0.0f), },

  {glm::vec3( 1.0f, 1.0f,-1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), },
  {glm::vec3( 1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), },
  {glm::vec3( 1.0f,-1.0f,-1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), },
  {glm::vec3( 1.0f,-1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f), },

  {glm::vec3(-1.0f, 1.0f,-1.0f), glm::vec3(1.0f, 0.0f, 0.0f), },
  {glm::vec3(-1.0f,-1.0f,-1.0f), glm::vec3(1.0f, 0.0f, 0.0f), },
  {glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), },
  {glm::vec3(-1.0f,-1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), }
};


/*
 * these global parameters can all be user-controlled via the XML file
 */
const float CScreensaverCpBlobs::g_fTickSpeed = 0.01f;

/*
 * Kodi has loaded us into memory, we should set our core values
 * here and load any settings we may have from our config file
 */
CScreensaverCpBlobs::CScreensaverCpBlobs()
  : m_pBlobby(nullptr),
    m_fTicks(0.0f),
    m_bShowCube(false),
    m_bShowBlob(false),
    m_bShowDebug(true)
{
  m_cubeTexture = 0;
  m_diffuseTexture = 0;
  m_specTexture = 0;

  m_pBlobby = new CBlobby(this);
  m_pBlobby->m_iNumPoints = 5;

  SetDefaults();
  m_fAspectRatio = (float)Width()/(float)Height();
}

CScreensaverCpBlobs::~CScreensaverCpBlobs()
{
  delete m_pBlobby;
}

/*
 * Kodi tells us we should get ready
 * to start rendering. This function
 * is called once when the screensaver
 * is activated by Kodi.
 */
bool CScreensaverCpBlobs::Start()
{
  std::string fraqShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
  {
    kodi::Log(ADDON_LOG_ERROR, "Failed to create and compile shader");
    return false;
  }

  m_cubeTexture = SOIL_load_OGL_single_cubemap(m_strCubemap.c_str(), "UWSNED", 4, 0, 0);
  if (m_cubeTexture == 0)
  {
    kodi::Log(ADDON_LOG_ERROR, "failed to create SOIL texture '%s', SOIL error '%s'", m_strCubemap.c_str(), SOIL_last_result());
    return false;
  }
  m_diffuseTexture = SOIL_load_OGL_single_cubemap(m_strDiffuseCubemap.c_str(), "UWSNED", 4, 0, 0);
  if (m_diffuseTexture == 0)
  {
    kodi::Log(ADDON_LOG_ERROR, "failed to create SOIL texture '%s', SOIL error '%s'", m_strDiffuseCubemap.c_str(), SOIL_last_result());
    return false;
  }
  m_specTexture = SOIL_load_OGL_single_cubemap(m_strSpecularCubemap.c_str(), "UWSNED", 4, 0, 0);
  if (m_specTexture == 0)
  {
    kodi::Log(ADDON_LOG_ERROR, "failed to create SOIL texture '%s', SOIL error '%s'", m_strSpecularCubemap.c_str(), SOIL_last_result());
    return false;
  }

  SetupGradientBackground(m_BGTopColor, m_BGBottomColor);

  glGenBuffers(1, &m_vertexVBO);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  m_startOK = true;
  return true;
}

/*
 * Kodi tells us to stop the screensaver
 * we should free any memory and release
 * any resources we have created.
 */
void CScreensaverCpBlobs::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;

  if (m_cubeTexture)
    glDeleteTextures(1, &m_cubeTexture);

  if (m_diffuseTexture)
    glDeleteTextures(1, &m_diffuseTexture);

  if (m_specTexture)
    glDeleteTextures(1, &m_specTexture);
}

/*
 * Kodi tells us to render a frame of our screensaver. This is called on each
 * frame render in Kodi, you should render a single frame only - the DX device
 * will already have been cleared.
 */
void CScreensaverCpBlobs::Render()
{
  if (!m_startOK)
    return;

  /*
   * Following Extra work done here in render to prevent problems with controls
   * from Kodi and during window moving.
   * TODO: Maybe add a separate interface call to inform about?
   */
  //@{
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

  glVertexAttribPointer(m_hVertex, 3, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, vertex)));
  glEnableVertexAttribArray(m_hVertex);

  glVertexAttribPointer(m_hNormal, 3, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, normal)));
  glEnableVertexAttribArray(m_hNormal);

  glVertexAttribPointer(m_hColor, 4, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, color)));
  glEnableVertexAttribArray(m_hColor);

  glVertexAttribPointer(m_hCoord, 2, GL_FLOAT, GL_TRUE, sizeof(sLight), BUFFER_OFFSET(offsetof(sLight, coord)));
  glEnableVertexAttribArray(m_hCoord);
  //@}

  // I know I'm not scaling by time here to get a constant framerate,
  // but I believe this to be acceptable for this application
  m_pBlobby->AnimatePoints(m_fTicks);
  m_pBlobby->March();
  glClear(GL_DEPTH_BUFFER_BIT);

  m_modelMat = glm::mat4(1.0f);

  if (!m_bShowCube)
  {
    m_projMat = glm::mat4(1.0f);
    RenderGradientBackground();
  }

  m_projMat = glm::perspective(glm::radians(m_fFOV), m_fAspectRatio, 0.05f, 100.0f);
  m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));
  m_textureMat = glm::rotate(glm::mat4(1.0f), glm::radians(m_fTicks * 20.0f), glm::vec3(0.0f, 1.0f, 0.0f));

  glEnable(GL_CULL_FACE);

  if (m_bShowDebug)
  {
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    static glm::vec3 vertices[] = {
      glm::vec3(0.25, 0.25, -3), glm::vec3(1, 0.25, -3), glm::vec3(0.25, 1, -3),
      glm::vec3(0, 0, -2), glm::vec3(0.75, 0, -2), glm::vec3(0, 0.75, -2),
      glm::vec3(0.25, 0.25, -4), glm::vec3(1, 0.25, -4), glm::vec3(0.25, 1, -4),
      glm::vec3(0, 0, -5), glm::vec3(0.75, 0, -5), glm::vec3(0, 0.75, -5),
    };
    static glm::vec4 tricolors[] = {
      glm::vec4(0.88f, 0.1f, 0.0f, 1.0f), glm::vec4(0.0f, 0.5f, 1.0f, 1.0f),
      glm::vec4(0.88f, 0.1f, 0.0f, 1.0f), glm::vec4(0.0f, 0.5f, 1.0f, 1.0f)
    };

    static sLight light[12];
    for (size_t i = 0; i < 12; ++i)
    {
      light[i].normal = g_cubeVertices[i].normal;
      light[i].color = tricolors[i / 3];
      light[i].vertex = vertices[i];
    }
    EnableShader();
    glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*12, light, GL_STATIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    DisableShader();
  }

  // setup cubemap
  m_texture0Used = 1;
  glFrontFace(GL_CW);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeTexture);

  // draw the box (inside-out)
  if (m_bShowCube)
  {
    sLight light[24];
    for (size_t i = 0; i < 24; ++i)
    {
      light[i].normal = g_cubeVertices[i].normal;
      light[i].color = glm::vec4(1.0f);
      light[i].vertex = g_cubeVertices[i].position;
    }

    EnableShader();
    glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*24, light, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 24);
    DisableShader();
  }

  if (m_bShowBlob)
  {
    // On original was with "glDisable(GL_TEXTURE_CUBE_MAP);" this unused.
    // For this the teture is still bind but with m_texture0Used = 0 to shader
    // prevented to use
    m_texture0Used = 0;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_diffuseTexture);

    m_texture1Used = 1;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_specTexture);

    glFrontFace(GL_CCW);
    m_modelMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.8f));
    m_normalMat = glm::transpose(glm::inverse(glm::mat3(m_modelMat)));
    m_pBlobby->Render();
  }

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);
  m_texture1Used = 0;

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  m_texture0Used = 0;

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);

  // increase tick count
  m_fTicks += g_fTickSpeed;

  glDisableVertexAttribArray(m_hVertex);
  glDisableVertexAttribArray(m_hNormal);
  glDisableVertexAttribArray(m_hColor);
  glDisableVertexAttribArray(m_hCoord);
}

void CScreensaverCpBlobs::SetDefaults()
{
  // set any default values for your screensaver's parameters
  m_fFOV = 45.0f;
  m_fAspectRatio = 1.33f;

  m_WorldRotSpeeds.x = 1.0f;
  m_WorldRotSpeeds.y = 0.5f;
  m_WorldRotSpeeds.z = 0.25f;

  m_strCubemap = kodi::GetAddonPath("resources/cube.dds");
  m_strDiffuseCubemap = kodi::GetAddonPath("resources/cube_diffuse.dds");
  m_strSpecularCubemap = kodi::GetAddonPath("resources/cube_specular.dds");

  m_pBlobby->m_fMoveScale = 0.3f;

  m_bShowCube = true;
  m_bShowBlob = true;
  m_bShowDebug = false;

  m_pBlobby->m_BlobPoints[0].m_Position.x = 0.5f;
  m_pBlobby->m_BlobPoints[0].m_Position.y = 0.5f;
  m_pBlobby->m_BlobPoints[0].m_Position.z = 0.5f;
  m_pBlobby->m_BlobPoints[0].m_fInfluence = 0.25f;
  m_pBlobby->m_BlobPoints[0].m_Speeds.x = 2.0f;
  m_pBlobby->m_BlobPoints[0].m_Speeds.y = 4.0f;
  m_pBlobby->m_BlobPoints[0].m_Speeds.z = 0.0f;
  m_pBlobby->m_BlobPoints[1].m_Position.x = 0.6f;
  m_pBlobby->m_BlobPoints[1].m_Position.y = 0.5f;
  m_pBlobby->m_BlobPoints[1].m_Position.z = 0.5f;
  m_pBlobby->m_BlobPoints[1].m_fInfluence = 0.51f;
  m_pBlobby->m_BlobPoints[1].m_Speeds.x = -4.0f;
  m_pBlobby->m_BlobPoints[1].m_Speeds.y = 2.0f;
  m_pBlobby->m_BlobPoints[1].m_Speeds.z = 0.0f;
  m_pBlobby->m_BlobPoints[2].m_Position.x = 0.3f;
  m_pBlobby->m_BlobPoints[2].m_Position.y = 0.5f;
  m_pBlobby->m_BlobPoints[2].m_Position.z = 0.3f;
  m_pBlobby->m_BlobPoints[2].m_fInfluence = 0.1f;
  m_pBlobby->m_BlobPoints[2].m_Speeds.x = -2.0f;
  m_pBlobby->m_BlobPoints[2].m_Speeds.y = 0.0f;
  m_pBlobby->m_BlobPoints[2].m_Speeds.z = 3.0f;
  m_pBlobby->m_BlobPoints[3].m_Position.x = 0.5f;
  m_pBlobby->m_BlobPoints[3].m_Position.y = 0.5f;
  m_pBlobby->m_BlobPoints[3].m_Position.z = 0.5f;
  m_pBlobby->m_BlobPoints[3].m_fInfluence = 0.25f;
  m_pBlobby->m_BlobPoints[3].m_Speeds.x = 0.0f;
  m_pBlobby->m_BlobPoints[3].m_Speeds.y = 2.0f;
  m_pBlobby->m_BlobPoints[3].m_Speeds.z = 1.0f;
  m_pBlobby->m_BlobPoints[4].m_Position.x = 0.5f;
  m_pBlobby->m_BlobPoints[4].m_Position.y = 0.5f;
  m_pBlobby->m_BlobPoints[4].m_Position.z = 0.5f;
  m_pBlobby->m_BlobPoints[4].m_fInfluence = 0.15f;
  m_pBlobby->m_BlobPoints[4].m_Speeds.x = 0.5f;
  m_pBlobby->m_BlobPoints[4].m_Speeds.y = 0.0f;
  m_pBlobby->m_BlobPoints[4].m_Speeds.z = 1.0f;

  m_pBlobby->SetDensity( 32 );
  m_pBlobby->m_TargetValue = 24.0f;
  m_BGTopColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
  m_BGBottomColor = glm::vec4(0.0f, 0.0f, 0.4f, 1.0f);
}

/*
 * fill in background vertex array with values that will
 * completely cover screen
 */
void CScreensaverCpBlobs::SetupGradientBackground(const glm::vec4& dwTopColor, const glm::vec4& dwBottomColor)
{
  float x1 = -1.0f;
  float y1 = -1.0f;
  float x2 = 1.0f;
  float y2 = 1.0f;

  m_BGVertices[0].position = glm::vec3(x2, y1, 0.0f);
  m_BGVertices[0].color = dwTopColor;

  m_BGVertices[1].position = glm::vec3(x2, y2, 0.0f);
  m_BGVertices[1].color = dwBottomColor;

  m_BGVertices[2].position = glm::vec3(x1, y1, 0.0f);
  m_BGVertices[2].color = dwTopColor;

  m_BGVertices[3].position = glm::vec3(x1, y2, 0.0f);
  m_BGVertices[3].color = dwBottomColor;
}

void CScreensaverCpBlobs::RenderGradientBackground()
{
  sLight light[4];
  for (size_t i = 0; i < 4; ++i)
  {
    light[i].color = m_BGVertices[i].color;
    light[i].vertex = m_BGVertices[i].position;
  }
  EnableShader();
  glBufferData(GL_ARRAY_BUFFER, sizeof(sLight)*4, light, GL_STATIC_DRAW);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  DisableShader();
}

void CScreensaverCpBlobs::OnCompiledAndLinked()
{
  // Variables passed directly to the Vertex shader
  m_projMatLoc = glGetUniformLocation(ProgramHandle(), "u_projectionMatrix");
  m_modelViewMatLoc = glGetUniformLocation(ProgramHandle(), "u_modelViewMatrix");
  m_transposeAdjointModelViewMatrixLoc = glGetUniformLocation(ProgramHandle(), "u_transposeAdjointModelViewMatrix");
  m_textureMatLoc = glGetUniformLocation(ProgramHandle(), "u_textureMatrix");
  m_uTexture0UsedLoc = glGetUniformLocation(ProgramHandle(), "u_texture0Enabled");
  m_uTexture1UsedLoc = glGetUniformLocation(ProgramHandle(), "u_texture1Enabled");
  m_uTexUnit0 = glGetUniformLocation(ProgramHandle(), "u_texUnit0");
  m_uTexUnit1 = glGetUniformLocation(ProgramHandle(), "u_texUnit1");

  m_hVertex = glGetAttribLocation(ProgramHandle(), "a_vertex");
  m_hNormal = glGetAttribLocation(ProgramHandle(), "a_normal");
  m_hColor = glGetAttribLocation(ProgramHandle(), "a_color");
  m_hCoord = glGetAttribLocation(ProgramHandle(), "a_coord");
}

bool CScreensaverCpBlobs::OnEnabled()
{
  // This is called after glUseProgram()
  glUniformMatrix4fv(m_projMatLoc, 1, GL_FALSE, glm::value_ptr(m_projMat));
  glUniformMatrix4fv(m_modelViewMatLoc, 1, GL_FALSE, glm::value_ptr(m_modelMat));
  glUniformMatrix3fv(m_transposeAdjointModelViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(m_normalMat));
  glUniformMatrix4fv(m_textureMatLoc, 1, GL_FALSE, glm::value_ptr(m_textureMat));
  glUniform1i(m_uTexture0UsedLoc, m_texture0Used);
  glUniform1i(m_uTexture1UsedLoc, m_texture1Used);
  glUniform1i(m_uTexUnit0, 0);
  glUniform1i(m_uTexUnit1, 1);
  return true;
}

ADDONCREATOR(CScreensaverCpBlobs);
