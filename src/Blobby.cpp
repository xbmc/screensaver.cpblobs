/*
 *  Copyright (C) 2005-2020 Team Kodi (https://kodi.tv)
 *  Copyright (C) Simon Windmill (siw@coolpowers.com)
 *  Ported to Kodi GL4 by Alwin Esch <alwinus@kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

// metaball classed derived from IsoSurface

#include "Blobby.h"
#include <math.h>

// the core of what metaballs are - a series of points and influences,
// so to sample them we average all the points at a particular location
float CBlobby::Sample( float mx, float my, float mz )
{
  float result = 0.0f;

  for (int n = 0; n < m_iNumPoints; n++)
  {
    float fDx, fDy, fDz;
    fDx = mx - m_BlobPoints[n].m_Position.x;
    fDy = my - m_BlobPoints[n].m_Position.y;
    fDz = mz - m_BlobPoints[n].m_Position.z;
    result += m_BlobPoints[n].m_fInfluence/(fDx*fDx + fDy*fDy + fDz*fDz);
  }

  return result;
}

///////////////////////////////////////////////////////////////////////////////

// called to update positions of all points
void CBlobby::AnimatePoints(float ticks)
{
  for (int n = 0; n < m_iNumPoints; n++)
  {
    if (m_BlobPoints[n].m_Speeds.x != 0.0f)
      m_BlobPoints[n].m_Position.x = (sinf(ticks*m_BlobPoints[n].m_Speeds.x) * m_fMoveScale) + 0.5f;
    if (m_BlobPoints[n].m_Speeds.y != 0.0f)
      m_BlobPoints[n].m_Position.y = (sinf(ticks*m_BlobPoints[n].m_Speeds.y) * m_fMoveScale) + 0.5f;
    if (m_BlobPoints[n].m_Speeds.z != 0.0f)
      m_BlobPoints[n].m_Position.z = (sinf(ticks*m_BlobPoints[n].m_Speeds.z) * m_fMoveScale) + 0.5f;
  }

  return;
}
