// cpBlobs
// XBMC screensaver displaying metaballs moving around in an environment
// Simon Windmill (siw@coolpowers.com)

#include <xbmc_scr_dll.h>
#include <libXBMC_addon.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <SOIL.h>
#include "cpBlobsMain.h"

#include "Blobby.h"

Blobby *m_pBlobby;

////////////////////////////////////////////////////////////////////////////////

static float g_fTicks = 0.0f;

////////////////////////////////////////////////////////////////////////////////

// these global parameters can all be user-controlled via the XML file

float g_fTickSpeed = 0.01f;

CVector g_WorldRotSpeeds;
char g_strCubemap[1024];
char g_strDiffuseCubemap[1024];
char g_strSpecularCubemap[1024];

bool g_bShowCube = false;
bool g_bShowBlob = false;
bool g_bShowDebug = true;

int g_BlendStyle;

CRGBA g_BGTopColor, g_BGBottomColor;

float g_fFOV, g_fAspectRatio;

////////////////////////////////////////////////////////////////////////////////

// stuff for the environment cube
struct CubeVertex
{
  CVector position;
  CVector normal;
};

// man, how many times have you typed (or pasted) this data for a cube's
// vertices and normals, eh?
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

////////////////////////////////////////////////////////////////////////////////

// stuff for the background plane

struct BG_VERTEX
{
  CVector position;
  CRGBA color;
};

BG_VERTEX g_BGVertices[4];

GLuint cubeTexture=0;
GLuint diffuseTexture=0;
GLuint specTexture=0;
ADDON::CHelper_libXBMC_addon *XBMC           = NULL;

////////////////////////////////////////////////////////////////////////////////

// fill in background vertex array with values that will
// completely cover screen
void SetupGradientBackground(const CRGBA& dwTopColor, const CRGBA& dwBottomColor)
{
  float x1 = -0.5f;
  float y1 = -0.5f;
  float x2 = (float)m_iWidth - 0.5f;
  float y2 = (float)m_iHeight - 0.5f;

  g_BGVertices[0].position = CVector( x2, y1, 0.0f);
  g_BGVertices[0].color = dwTopColor;

  g_BGVertices[1].position = CVector( x2, y2, 0.0f);
  g_BGVertices[1].color = dwBottomColor;

  g_BGVertices[2].position = CVector( x1, y1, 0.0f);
  g_BGVertices[2].color = dwTopColor;

  g_BGVertices[3].position = CVector( x1, y2, 0.0f);
  g_BGVertices[3].color = dwBottomColor;
}

///////////////////////////////////////////////////////////////////////////////


void RenderGradientBackground()
{
  glDisable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_CUBE_MAP);
  glBegin(GL_TRIANGLE_STRIP);
  for (size_t i=0;i<4;++i)
  {
    glColor3f(g_BGVertices[i].color.r/255.0, g_BGVertices[i].color.g/255.0,
              g_BGVertices[i].color.b/255.0);
    glVertex3f(g_BGVertices[i].position.x, g_BGVertices[i].position.y,
               g_BGVertices[i].position.z);
  }
  glEnd();
}

void SetDefaults()
{
  // set any default values for your screensaver's parameters
  g_fFOV = 45.0f;
  g_fAspectRatio = 1.33f;

  g_WorldRotSpeeds.x = 1.0f;
  g_WorldRotSpeeds.y = 0.5f;
  g_WorldRotSpeeds.z = 0.25f;

  XBMC->GetSetting("__addonpath__", g_strCubemap);
  strcat(g_strCubemap, "/resources/cube.dds");
  XBMC->GetSetting("__addonpath__", g_strDiffuseCubemap);
  strcat(g_strDiffuseCubemap, "/resources/cube_diffuse.dds");
  XBMC->GetSetting("__addonpath__", g_strSpecularCubemap);
  strcat(g_strSpecularCubemap, "/resources/cube_specular.dds");

  m_pBlobby->m_fMoveScale = 0.3f;


  g_bShowCube = true;
  g_bShowBlob = true;
  g_bShowDebug = false;

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
  g_BGTopColor = CRGBA(0, 0, 0, 255);
  g_BGBottomColor = CRGBA(0, 0, 100, 255);
}

////////////////////////////////////////////////////////////////////////////
// XBMC has loaded us into memory, we should set our core values
// here and load any settings we may have from our config file
//
ADDON_STATUS ADDON_Create(void* hdl, void* props)
{
  if (!props)
    return ADDON_STATUS_UNKNOWN;

  if (!XBMC)
    XBMC = new ADDON::CHelper_libXBMC_addon;

  if (!XBMC->RegisterMe(hdl))
  {
    delete XBMC, XBMC=NULL;
    return ADDON_STATUS_PERMANENT_FAILURE;
  }

  SCR_PROPS* scrprops = (SCR_PROPS*)props;

  m_iWidth = scrprops->width;
  m_iHeight  = scrprops->height;

  m_pBlobby = new Blobby();
  m_pBlobby->m_iNumPoints = 5;

  SetDefaults();
  g_fAspectRatio = (float)m_iWidth/(float)m_iHeight;

  return ADDON_STATUS_OK;
}

// XBMC tells us we should get ready
// to start rendering. This function
// is called once when the screensaver
// is activated by XBMC.
extern "C" void Start()
{
  cubeTexture = SOIL_load_OGL_single_cubemap(g_strCubemap, "UWSNED", 4, 0, 0);
  diffuseTexture = SOIL_load_OGL_single_cubemap(g_strDiffuseCubemap, "UWSNED", 4, 0, 0);
  specTexture = SOIL_load_OGL_single_cubemap(g_strSpecularCubemap, "UWSNED", 4, 0, 0);

  SetupGradientBackground(g_BGTopColor, g_BGBottomColor);

  return;
}

// XBMC tells us to render a frame of
// our screensaver. This is called on
// each frame render in XBMC, you should
// render a single frame only - the DX
// device will already have been cleared.
extern "C" void Render()
{
  // I know I'm not scaling by time here to get a constant framerate,
  // but I believe this to be acceptable for this application
  m_pBlobby->AnimatePoints(g_fTicks);
  m_pBlobby->March();
  glClear(GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(g_fFOV, g_fAspectRatio, 0.05, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_CULL_FACE);

  if (g_bShowDebug )
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
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_R);
  glEnable(GL_TEXTURE_GEN_T);
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
  glTexGenf(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);

  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glRotatef(g_fTicks * 20.0 , 0.0, 1.0, 0.0);

  // draw the box (inside-out)
  if (g_bShowCube )
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


  if (g_bShowBlob)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, diffuseTexture);
    glDisable(GL_TEXTURE_CUBE_MAP);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_T);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, specTexture);
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
    glRotatef(g_fTicks * 20.0 , 0.0, 1.0, 0.0);

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
  g_fTicks += g_fTickSpeed;
}

// XBMC tells us to stop the screensaver
// we should free any memory and release
// any resources we have created.
extern "C" void ADDON_Stop()
{
  if (cubeTexture)
    glDeleteTextures(1, &cubeTexture);

  if (diffuseTexture)
    glDeleteTextures(1, &diffuseTexture);

  if (specTexture)
    glDeleteTextures(1, &specTexture);

  delete m_pBlobby;
}

extern "C" void ADDON_Destroy()
{
}

extern "C" ADDON_STATUS ADDON_GetStatus()
{
  return ADDON_STATUS_OK;
}

extern "C" ADDON_STATUS ADDON_SetSetting(const char *strSetting, const void *value)
{
  return ADDON_STATUS_OK;
}
