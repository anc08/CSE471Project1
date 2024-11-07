#pragma once
#include <list>
#include <CToneInstrument.h>
#include <CWavetableInstrument.h>
#include "msxml2.h"
#include <string>
#include <CNote.h>
#include <CWaveNote.h>

using namespace std;

class CSynthesizer
{
	public:
	//! Number of audio channels
    int GetNumChannels() {return m_channels;}

	//! Sample rate in samples per second
    double GetSampleRate() {return m_sampleRate;}

	//! Sample period in seconds (1/samplerate)
    double GetSamplePeriod() {return m_samplePeriod;}
    
	//! Set the number of channels
    void SetNumChannels(int n) {m_channels = n;}

	//! Set the sample rate
    void SetSampleRate(double s) {m_sampleRate = s;  m_samplePeriod = 1.0 / s;}

    //! Set the wave table
    void AddWaveToTable(LPCTSTR w) { m_waveTable.push_back(&CWaveIn(w)); }

    //! Clear the wave table
    void ClearWaveTable() {m_waveTable.clear();}

private:
    int		m_channels;
    double	m_sampleRate;
    double	m_samplePeriod;
    double  m_time;
    std::list<CInstrument*>  m_instruments;
    double  m_bpm;                  //!< Beats per minute
    int     m_beatspermeasure;  //!< Beats per measure
    double  m_secperbeat;        //!< Seconds per beat
    std::vector<CNote> m_notes;
    int m_currentNote;          //!< The current note we are playing
    int m_measure;              //!< The current measure
    double m_beat;              //!< The current beat within the measure
    std::vector<CWaveIn*> m_waveTable;
    CString* m_filename_cache;

public:
    CSynthesizer();
    void Start();
    bool Generate(double* frame);
    //! Get the time since we started generating audio
    double GetTime() { return m_time; }
    void Clear(void);
    void OpenScore(CString& filename);
    void XmlLoadScore(IXMLDOMNode* xml);
    void XmlLoadInstrument(IXMLDOMNode* xml);
    void XmlLoadNote(IXMLDOMNode* xml, std::wstring& instrument);
    void XmlLoadWavetable(IXMLDOMNode* xml, std::wstring& instrument);
    void XmlLoadWave(IXMLDOMNode* xml, std::wstring& instrument);
    string CanonicalPath(string path);
};

#pragma comment(lib, "msxml2.lib")