#include "pch.h"
#include "CToneInstrument.h"
#include <Notes.h>

CToneInstrument::CToneInstrument()
{
	m_duration = 0.1;
    m_attack = 0.05;
    m_release = 0.05;
}

void CToneInstrument::Start()
{
    m_sinewave.SetSampleRate(GetSampleRate());
    m_sinewave.Start();
    m_time = 0;
}


bool CToneInstrument::Generate()
{
    // Tell the component to generate an audio sample
    m_sinewave.Generate();

    double volumeMultiplier = 1.;
    if (m_time < m_attack)
    {
        volumeMultiplier = m_time / m_attack;
    }
    else if (m_time > (m_duration - m_release))
    {
        volumeMultiplier = (m_duration - m_time) / m_release;
    }

    // Read the component's sample and make it our resulting frame.
    m_frame[0] = m_sinewave.Frame(0) * volumeMultiplier;
    m_frame[1] = m_sinewave.Frame(1) * volumeMultiplier;

    // Update time
    m_time += GetSamplePeriod();

    // We return true until the time reaches the duration.
    return m_time < m_duration;
}

void CToneInstrument::SetNote(CNote* note)
{
    // Get a list of all attribute nodes and the
    // length of that list
    CComPtr<IXMLDOMNamedNodeMap> attributes;
    note->Node()->get_attributes(&attributes);
    long len;
    attributes->get_length(&len);

    // Loop over the list of attributes
    for (int i = 0; i < len; i++)
    {
        // Get attribute i
        CComPtr<IXMLDOMNode> attrib;
        attributes->get_item(i, &attrib);

        // Get the name of the attribute
        CComBSTR name;
        attrib->get_nodeName(&name);

        // Get the value of the attribute.  A CComVariant is a variable
        // that can have any type. It loads the attribute value as a
        // string (UNICODE), but we can then change it to an integer 
        // (VT_I4) or double (VT_R8) using the ChangeType function 
        // and then read its integer or double value from a member variable.
        CComVariant value;
        attrib->get_nodeValue(&value);

        if (name == "duration")
        {
            value.ChangeType(VT_R8);
            SetDuration(value.dblVal * (60. / 120.));
        }
        else if (name == "note")
        {
            SetFreq(NoteToFrequency(value.bstrVal));
        }
    }
}
