#pragma once
#include "CNote.h"
class CWaveNote :
    public CNote
{
public:
    CWaveNote();
    virtual ~CWaveNote(void);

    int Measure() const { return m_measure; }
    double Beat() const { return m_beat; }
    const std::wstring& Instrument() const { return m_instrument; }
    IXMLDOMNode* Node() { return m_node; }
    void XmlLoad(IXMLDOMNode* xml, std::wstring& instrument);
    int WaveIndex() const { return m_waveIndex; }

public:
    bool operator<(const CWaveNote& b);

private:
    std::wstring m_instrument;
    int m_measure;
    double m_beat;
    CComPtr<IXMLDOMNode> m_node;
    int m_waveIndex;
};

