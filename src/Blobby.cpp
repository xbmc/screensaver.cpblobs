/*
 *  Copyright (C) 2005-2020 Team Kodi
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
