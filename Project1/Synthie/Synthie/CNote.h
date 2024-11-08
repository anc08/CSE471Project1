#pragma once
#include <vector>
#include <string>
class CNote
{
public:
	CNote();
    virtual ~CNote(void);

    int Measure() const { return m_measure; }
    double Beat() const { return m_beat; }
    const std::wstring& Instrument() const { return m_instrument; }
    IXMLDOMNode* Node() { return m_node; }
    void XmlLoad(IXMLDOMNode* xml, std::wstring& instrument);

public:
    bool operator<(const CNote& b);

private:
    std::wstring m_instrument;
    int m_measure;
    double m_beat;
    CComPtr<IXMLDOMNode> m_node;
};

