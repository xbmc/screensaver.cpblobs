/*
 *  Copyright (C) 2005-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) Simon Windmill (siw@coolpowers.com)
 *  Ported to Kodi GL4 by Alwin Esch <alwinus@kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <kodi/AddonBase.h>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

class CScreensaverCpBlobs;

class ATTR_DLL_LOCAL CIsoSurface
{
public:
  CIsoSurface(CScreensaverCpBlobs* base);
  virtual ~CIsoSurface() = default;

  virtual float Sample(float mx, float my, float mz) = 0;

  void March();
  void Render();
  void SetDensity(int density);

  float m_TargetValue;
  std::vector<glm::vec3> m_pVxs;
  std::vector<glm::vec3> m_pNorms;
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
