#pragma once
#include "CInstrument.h"
#include "audio/Wave.h"
#include <CSineWave.h>
#include <CWaveNote.h>
#include <list>
#include <algorithm>
class CWavetableInstrument :
    public CInstrument
{
public:
    virtual void Start();
    virtual bool Generate();

    void SetFreq(double f) { m_sinewave.SetFreq(f); }
    void SetAmplitude(double a) { m_sinewave.SetAmplitude(a); }
    void SetDuration(double d) { m_duration = d; }
    void SetNote(CNote* note);
    void SetWave(CWaveIn* w) { m_wavein = w; }
    void SetLoopStart(double s) { m_loopStart = s; }
    void SetLoopEnd(double e) { m_loopEnd = e; }

    //! Access a generated audio frame
    virtual const double* Frame() { return m_frame; }

    //! Access one channel of a generated audio frame
    virtual double Frame(int c) { return (double)m_shortFrame[c]; }

private:
    LPCTSTR m_wave;
    CWaveIn* m_wavein;
    double m_loopStart;
    double m_loopEnd;
    CSineWave   m_sinewave;
    double m_duration;
    double m_time;
    double m_attack;
    double m_release;
    double m_pitch;

    short m_shortFrame[];
public:

    CWavetableInstrument();
};

