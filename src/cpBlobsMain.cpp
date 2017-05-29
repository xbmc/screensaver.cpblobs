// cpBlobs
// Kodi screensaver displaying metaballs moving around in an environment
// Simon Windmill (siw@coolpowers.com)

#include <kodi/addon-instance/Screensaver.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SOIL.h>

#include "Blobby.h"

/*
 * stuff for the environment cube
 */
struct CubeVertex
{
  CVector position;
  CVector normal;
};

/*
 * man, how many times have you typed (or pasted) this data for a cube's
 * vertices and normals, eh?
 */
CubeVertex g_cubeVertices[] =
{
  {CVector(-1.0f, 1.0f,-1.0f), CVector(0.0f, 0.0f,1.0f), },
  {CVector( 1.0f, 1.0f,-1.0f), CVector(0.0f, 0.0f,1.0f), },
  {CVector(-1.0f,-1.0f,-1.0f), CVector(0.0f, 0.0f,1.0f), },
  {CVector( 1.0f,-1.0f,-1.0f), CVector(0.0f, 0.0f,1.0f), },

  {CVector(-1.0f, 1.0f, 1.0f), CVector(0.0f, 0.0f, -1.0f), },
  {CVector(-1.0f,-1.0f, 1.0f), CVector(0.0f, 0.0f, -1.0f), },
  {CVector( 1.0f, 1.0f, 1.0f), CVector(0.0f, 0.0f, -1.0f), },
  {CVector( 1.0f,-1.0f, 1.0f), CVector(0.0f, 0.0f, -1.0f), },

  {CVector(-1.0f, 1.0f, 1.0f), CVector(0.0f, -1.0f, 0.0f), },
  {CVector( 1.0f, 1.0f, 1.0f), CVector(0.0f, -1.0f, 0.0f), },
  {CVector(-1.0f, 1.0f,-1.0f), CVector(0.0f, -1.0f, 0.0f), },
  {CVector( 1.0f, 1.0f,-1.0f), CVector(0.0f, -1.0f, 0.0f), },

  {CVector(-1.0f,-1.0f, 1.0f), CVector(0.0f,1.0f, 0.0f), },
  {CVector(-1.0f,-1.0f,-1.0f), CVector(0.0f,1.0f, 0.0f), },
  {CVector( 1.0f,-1.0f, 1.0f), CVector(0.0f,1.0f, 0.0f), },
  {CVector( 1.0f,-1.0f,-1.0f), CVector(0.0f,1.0f, 0.0f), },

  {CVector( 1.0f, 1.0f,-1.0f), CVector(-1.0f, 0.0f, 0.0f), },
  {CVector( 1.0f, 1.0f, 1.0f), CVector(-1.0f, 0.0f, 0.0f), },
  {CVector( 1.0f,-1.0f,-1.0f), CVector(-1.0f, 0.0f, 0.0f), },
  {CVector( 1.0f,-1.0f, 1.0f), CVector(-1.0f, 0.0f, 0.0f), },

  {CVector(-1.0f, 1.0f,-1.0f), CVector(1.0f, 0.0f, 0.0f), },
  {CVector(-1.0f,-1.0f,-1.0f), CVector(1.0f, 0.0f, 0.0f), },
  {CVector(-1.0f, 1.0f, 1.0f), CVector(1.0f, 0.0f, 0.0f), },
  {CVector(-1.0f,-1.0f, 1.0f), CVector(1.0f, 0.0f, 0.0f), }
};

/*
 * stuff for the background plane
 */
struct BG_VERTEX
{
  CVector position;
  CRGBA color;
};

class CScreensaverCpBlobs
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceScreensaver
{
public:
  CScreensaverCpBlobs();
  virtual ~CScreensaverCpBlobs();

  virtual bool Start() override;
  virtual void Stop() override;
  virtual void Render() override;

private:
  static const float g_fTickSpeed;
  
  GLuint m_cubeTexture;
  GLuint m_diffuseTexture;
  GLuint m_specTexture;

  Blobby *m_pBlobby;
  BG_VERTEX m_BGVertices[4];
  float m_fFOV, m_fAspectRatio;
  float m_fTicks;
  CVector m_WorldRotSpeeds;
  std::string m_strCubemap;
  std::string m_strDiffuseCubemap;
  std::string m_strSpecularCubemap;
  bool m_bShowCube;
  bool m_bShowBlob;
  bool m_bShowDebug;
  CRGBA m_BGTopColor, m_BGBottomColor;

  void SetDefaults();
  void SetupGradientBackground(const CRGBA& dwTopColor, const CRGBA& dwBottomColor);
  void RenderGradientBackground();
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

  m_pBlobby = new Blobby();
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
  m_cubeTexture = SOIL_load_OGL_single_cubemap(m_strCubemap.c_str(), "UWSNED", 4, 0, 0);
  m_diffuseTexture = SOIL_load_OGL_single_cubemap(m_strDiffuseCubemap.c_str(), "UWSNED", 4, 0, 0);
  m_specTexture = SOIL_load_OGL_single_cubemap(m_strSpecularCubemap.c_str(), "UWSNED", 4, 0, 0);

  SetupGradientBackground(m_BGTopColor, m_BGBottomColor);

  return true;
}

/*
 * Kodi tells us to stop the screensaver
 * we should free any memory and release
 * any resources we have created.
 */
void CScreensaverCpBlobs::Stop()
{
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
  // I know I'm not scaling by time here to get a constant framerate,
  // but I believe this to be acceptable for this application
  m_pBlobby->AnimatePoints(m_fTicks);
  m_pBlobby->March();
  glClear(GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(m_fFOV, m_fAspectRatio, 0.05, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_CULL_FACE);

  if (m_bShowDebug )
  {
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_CUBE_MAP);
    CVector vertices[] = {
      CVector(0.25, 0.25, -3), CVector(1, 0.25, -3), CVector(0.25, 1, -3),
      CVector(0, 0, -2), CVector(0.75, 0, -2), CVector(0, 0.75, -2),
      CVector(0.25, 0.25, -4), CVector(1, 0.25, -4), CVector(0.25, 1, -4),
      CVector(0, 0, -5), CVector(0.75, 0, -5), CVector(0, 0.75, -5),
    };
    CVector tricolors[] = {
      CVector(0.88, 0.1, 0.00), CVector(0, 0.5, 1.0),
      CVector(0.88, 0.1, 0.00), CVector(0, 0.5, 1.0)
    };

    glBegin(GL_TRIANGLES);
    for (size_t i=0;i<12;++i)
    {
      glNormal3f(g_cubeVertices[i].normal.x, g_cubeVertices[i].normal.y,
                 g_cubeVertices[i].normal.z);
      glColor4f(tricolors[i / 3].x, tricolors[i / 3].y,
                tricolors[i / 3].z, 1.0);

      glVertex3f(vertices[i].x, vertices[i].y,
                 vertices[i].z);
    }
    glEnd();
    glColor4f(1,1,1,1);
  }

  // setup cubemap
  glEnable(GL_TEXTURE_2D);
  glFrontFace(GL_CW);
  glEnable(GL_TEXTURE_CUBE_MAP);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubeTexture);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_R);
  glEnable(GL_TEXTURE_GEN_T);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
  glTexGenf(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);

  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glRotatef(m_fTicks * 20.0 , 0.0, 1.0, 0.0);

  // draw the box (inside-out)
  if (m_bShowCube )
  {
    glBegin(GL_TRIANGLE_STRIP);
    for (size_t i=0;i<24;++i)
    {
      glNormal3f(g_cubeVertices[i].normal.x, g_cubeVertices[i].normal.y,
                 g_cubeVertices[i].normal.z);
      glVertex3f(g_cubeVertices[i].position.x, g_cubeVertices[i].position.y,
                 g_cubeVertices[i].position.z);
    }
    glEnd();
  }

  if (m_bShowBlob)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_diffuseTexture);
    glDisable(GL_TEXTURE_CUBE_MAP);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_T);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_specTexture);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_T);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glRotatef(m_fTicks * 20.0 , 0.0, 1.0, 0.0);

    glFrontFace(GL_CCW);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -0.8);

    m_pBlobby->Render();
  }

  glActiveTexture(GL_TEXTURE1);
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_CUBE_MAP);
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_TEXTURE_GEN_R);
  glBindTexture(GL_TEXTURE_2D, 0);

  glActiveTexture(GL_TEXTURE0);
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_CUBE_MAP);
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);
  glDisable(GL_TEXTURE_GEN_R);
  glBindTexture(GL_TEXTURE_2D, 0);

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  // increase tick count
  m_fTicks += g_fTickSpeed;
}

void CScreensaverCpBlobs::SetDefaults()
{
  // set any default values for your screensaver's parameters
  m_fFOV = 45.0f;
  m_fAspectRatio = 1.33f;

  m_WorldRotSpeeds.x = 1.0f;
  m_WorldRotSpeeds.y = 0.5f;
  m_WorldRotSpeeds.z = 0.25f;

  m_strCubemap = kodi::GetAddonPath() + "/resources/cube.dds";
  m_strDiffuseCubemap = kodi::GetAddonPath() + "/resources/cube_diffuse.dds";
  m_strSpecularCubemap = kodi::GetAddonPath() + "/resources/cube_specular.dds";

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
  m_BGTopColor = CRGBA(0, 0, 0, 255);
  m_BGBottomColor = CRGBA(0, 0, 100, 255);
}

/*
 * fill in background vertex array with values that will
 * completely cover screen
 */
void CScreensaverCpBlobs::SetupGradientBackground(const CRGBA& dwTopColor, const CRGBA& dwBottomColor)
{
  float x1 = -0.5f;
  float y1 = -0.5f;
  float x2 = (float)Width() - 0.5f;
  float y2 = (float)Height() - 0.5f;

  m_BGVertices[0].position = CVector( x2, y1, 0.0f);
  m_BGVertices[0].color = dwTopColor;

  m_BGVertices[1].position = CVector( x2, y2, 0.0f);
  m_BGVertices[1].color = dwBottomColor;

  m_BGVertices[2].position = CVector( x1, y1, 0.0f);
  m_BGVertices[2].color = dwTopColor;

  m_BGVertices[3].position = CVector( x1, y2, 0.0f);
  m_BGVertices[3].color = dwBottomColor;
}

void CScreensaverCpBlobs::RenderGradientBackground()
{
  glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_CUBE_MAP);
  glBegin(GL_TRIANGLE_STRIP);
  for (size_t i=0;i<4;++i)
  {
    glColor3f(m_BGVertices[i].color.r/255.0, m_BGVertices[i].color.g/255.0,
              m_BGVertices[i].color.b/255.0);
    glVertex3f(m_BGVertices[i].position.x, m_BGVertices[i].position.y,
               m_BGVertices[i].position.z);
  }
  glEnd();
}

ADDONCREATOR(CScreensaverCpBlobs);
