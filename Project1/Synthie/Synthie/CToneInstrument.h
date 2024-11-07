#pragma once
#include "CInstrument.h"
#include <CSineWave.h>
class CToneInstrument :
    public CInstrument
{
public:
    virtual void Start();
    virtual bool Generate();

    void SetFreq(double f) { m_sinewave.SetFreq(f); }
    void SetAmplitude(double a) { m_sinewave.SetAmplitude(a); }
    void SetDuration(double d) { m_duration = d; }
    void SetNote(CNote* note);

private:
    CSineWave   m_sinewave;
    double m_duration;
    double m_time;
    double m_attack;
    double m_release;
public:

    CToneInstrument();
};

