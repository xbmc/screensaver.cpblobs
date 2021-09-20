/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) Simon Windmill (siw@coolpowers.com)
 *  Ported to Kodi GL4 by Alwin Esch <alwinus@kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
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
