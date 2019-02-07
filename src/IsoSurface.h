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

#include <kodi/AddonBase.h>
#include <glm/gtc/type_ptr.hpp>

class CScreensaverCpBlobs;

class ATTRIBUTE_HIDDEN CIsoSurface
{
public:
  CIsoSurface(CScreensaverCpBlobs* base);
  virtual ~CIsoSurface();

  virtual float Sample(float mx, float my, float mz) = 0;

  void March();
  void Render();
  void SetDensity(int density);

  float m_TargetValue;
  glm::vec3 *m_pVxs;
  glm::vec3 *m_pNorms;
  int m_iVxCount;
  int m_iFaceCount;

private:
  void GetNormal(glm::vec3& normal, glm::vec3& position);
  void MarchCube(float mx, float my, float mz, float scale);
  float GetOffset(float val1, float val2, float wanted);

  int m_DatasetSize;
  float m_StepSize;

  CScreensaverCpBlobs* m_base;
};
