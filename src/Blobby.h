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

#pragma once

#include "IsoSurface.h"

const int MAXBLOBPOINTS = 5;

struct BlobPoint
{
  glm::vec3 m_Position;
  float m_fInfluence;
  glm::vec3 m_Speeds;
};

class CScreensaverCpBlobs;

class ATTRIBUTE_HIDDEN CBlobby : public CIsoSurface
{

public:
  CBlobby(CScreensaverCpBlobs* base) : CIsoSurface(base), m_base(base) { }

  float Sample(float mx, float my, float mz) override;
  void AnimatePoints(float ticks);

  BlobPoint m_BlobPoints[MAXBLOBPOINTS];
  int m_iNumPoints;
  float m_fMoveScale; // we scale the movements so we don't see the ugly boundary cases

private:
  CScreensaverCpBlobs* m_base;
};
