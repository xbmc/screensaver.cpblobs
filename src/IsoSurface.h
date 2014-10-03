#ifndef ISOSURFACE_H
#define ISOSURFACE_H

#include "types.h"

class IsoSurface
{
public:
    IsoSurface();
    ~IsoSurface();

    virtual float Sample( float mx, float my, float mz );
    void March();
    void Render();
    void SetDensity( int density );

    float m_TargetValue;
    CVector *m_pVxs;
    CVector *m_pNorms;
    int m_iVxCount;
    int m_iFaceCount;

private:
    void GetNormal( CVector&normal, CVector&position );
    void MarchCube( float mx, float my, float mz, float scale );
    float GetOffset( float val1, float val2, float wanted );

    int m_DatasetSize;
    float m_StepSize;
};

#endif
