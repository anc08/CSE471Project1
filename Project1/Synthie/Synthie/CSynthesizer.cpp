#include "pch.h"
#include "CSynthesizer.h"
#include "xmlhelp.h"
#include <algorithm>
#include "audio/wave.h"

CSynthesizer::CSynthesizer()
{
    CoInitialize(NULL);

	m_channels = 2;
	m_sampleRate = 44100.;
	m_samplePeriod = 1 / m_sampleRate;
	m_time = 0;
    m_bpm = 120;
    m_secperbeat = 60 / m_bpm;
    m_beatspermeasure = 4;
    m_filename_cache = nullptr;
}

void CSynthesizer::Start(void)
{
    m_instruments.clear();
    m_currentNote = 0;
    m_measure = 0;
    m_beat = 0;
    m_time = 0;
}


//! Generate one audio frame
bool CSynthesizer::Generate(double* frame)
{
    //
    // Phase 1: Determine if any notes need to be played.
    //

    while (m_currentNote < (int)m_notes.size())
    {
        // Get a pointer to the current note
        CNote* note = &m_notes[m_currentNote];

        // If the measure is in the future we can't play
        // this note just yet.
        if (note->Measure() > m_measure)
            break;

        // If this is the current measure, but the
        // beat has not been reached, we can't play
        // this note.
        if (note->Measure() == m_measure && note->Beat() > m_beat)
            break;

        //
        // Play the note!
        //

        // Create the instrument object
        CInstrument* instrument = NULL;
        if (note->Instrument() == L"ToneInstrument")
        {
            instrument = new CToneInstrument();
        }
        else if (note->Instrument() == L"WavetableInstrument")
        {
            // Tell instrument which wave to play
            instrument = new CWavetableInstrument();
            int waveIndex = ((CWaveNote*)note)->WaveIndex();
            if (waveIndex >= m_waveTable.size()) waveIndex = 0;
            ((CWavetableInstrument*)instrument)->SetWave(m_waveTable[waveIndex]);
        }

        // Configure the instrument object
        if (instrument != NULL)
        {
            instrument->SetSampleRate(GetSampleRate());
            instrument->SetNote(note);
            instrument->Start();

            m_instruments.push_back(instrument);
        }

        m_currentNote++;
    }

    //
    // Phase 2: Clear all channels to silence 
    //

    for (int c = 0; c < GetNumChannels(); c++)
    {
        frame[c] = 0;
    }

    //
    // Phase 3: Play an active instruments
    //

    //
    // We have a list of active (playing) instruments.  We iterate over 
    // that list.  For each instrument we call generate, then add the
    // output to our output frame.  If an instrument is done (Generate()
    // returns false), we remove it from the list.
    //

    for (list<CInstrument*>::iterator node = m_instruments.begin(); node != m_instruments.end(); )
    {
        // Since we may be removing an item from the list, we need to know in 
        // advance, what is after it in the list.  We keep that node as "next"
        list<CInstrument*>::iterator next = node;
        next++;

        // Get a pointer to the allocated instrument
        CInstrument* instrument = *node;

        // Call the generate function
        if (instrument->Generate())
        {
            // If we returned true, we have a valid sample.  Add it 
            // to the frame.
            for (int c = 0; c < GetNumChannels(); c++)
            {
                frame[c] += instrument->Frame(c);
            }
        }
        else
        {
            // If we returned false, the instrument is done.  Remove it
            // from the list and delete it from memory.
            m_instruments.erase(node);
            delete instrument;
        }

        // Move to the next instrument in the list
        node = next;
    }

    //
    // Phase 4: Advance the time and beats
    //

    // Time advances by the sample period
    m_time += GetSamplePeriod();

    // Beat advances by the sample period divided by the 
    // number of seconds per beat.  The inverse of seconds
    // per beat is beats per second.
    m_beat += GetSamplePeriod() / m_secperbeat;

    // When the measure is complete, we move to
    // a new measure.  We might be a fraction into
    // the new measure, so we subtract out rather 
    // than just setting to zero.
    if (m_beat > m_beatspermeasure)
    {
        m_beat -= m_beatspermeasure;
        m_measure++;
    }

    //
    // Phase 5: Determine when we are done
    //

    // We are done when there is nothing to play.  We'll put something more 
    // complex here later.
    return !m_instruments.empty() || m_currentNote < (int)m_notes.size();
}

void CSynthesizer::Clear(void)
{
    m_instruments.clear();
    m_notes.clear();
}

void CSynthesizer::OpenScore(CString& filename)
{
    Clear();

    // cache filename
    m_filename_cache = &filename;

    //
    // Create an XML document
    //

    CComPtr<IXMLDOMDocument>  pXMLDoc;
    bool succeeded = SUCCEEDED(CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,
        IID_IXMLDOMDocument, (void**)&pXMLDoc));
    if (!succeeded)
    {
        AfxMessageBox(L"Failed to create an XML document to use");
        return;
    }

    // Open the XML document
    VARIANT_BOOL ok;
    succeeded = SUCCEEDED(pXMLDoc->load(CComVariant(filename), &ok));
    if (!succeeded || ok == VARIANT_FALSE)
    {
        AfxMessageBox(L"Failed to open XML score file");
        return;
    }

    //
    // Traverse the XML document in memory!!!!
    // Top level tag is <score>
    //

    CComPtr<IXMLDOMNode> node;
    pXMLDoc->get_firstChild(&node);
    for (; node != NULL; NextNode(node))
    {
        // Get the name of the node
        CComBSTR nodeName;
        node->get_nodeName(&nodeName);

        if (nodeName == "score")
        {
            XmlLoadScore(node);
        }
    }

    sort(m_notes.begin(), m_notes.end());

    m_filename_cache = nullptr;
}

void CSynthesizer::XmlLoadScore(IXMLDOMNode* xml)
{
    // Get a list of all attribute nodes and the
    // length of that list
    CComPtr<IXMLDOMNamedNodeMap> attributes;
    xml->get_attributes(&attributes);
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

        if (name == L"bpm")
        {
            value.ChangeType(VT_R8);
            m_bpm = value.dblVal;
            m_secperbeat = 1 / (m_bpm / 60);
        }
        else if (name == L"beatspermeasure")
        {
            value.ChangeType(VT_I4);
            m_beatspermeasure = value.intVal;
        }

    }


    CComPtr<IXMLDOMNode> node;
    xml->get_firstChild(&node);
    for (; node != NULL; NextNode(node))
    {
        // Get the name of the node
        CComBSTR name;
        node->get_nodeName(&name);
        if (name == L"instrument")
        {
            XmlLoadInstrument(node);
        }
    }
}

void CSynthesizer::XmlLoadInstrument(IXMLDOMNode* xml)
{
    wstring instrument = L"";

    // Get a list of all attribute nodes and the
    // length of that list
    CComPtr<IXMLDOMNamedNodeMap> attributes;
    xml->get_attributes(&attributes);
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

        // Get the value of the attribute.  
        CComVariant value;
        attrib->get_nodeValue(&value);

        if (name == "instrument")
        {
            instrument = value.bstrVal;
        }
    }


    CComPtr<IXMLDOMNode> node;
    xml->get_firstChild(&node);
    for (; node != NULL; NextNode(node))
    {
        // Get the name of the node
        CComBSTR name;
        node->get_nodeName(&name);

        if (name == L"note")
        {
            XmlLoadNote(node, instrument);
        }
        if (name == L"wavetable")
        {
            XmlLoadWavetable(node, instrument);
        }
    }
}

void CSynthesizer::XmlLoadNote(IXMLDOMNode* xml, std::wstring& instrument)
{
    if (instrument == L"WavetableInstrument") m_notes.push_back(CWaveNote());
    else m_notes.push_back(CNote());
    m_notes.back().XmlLoad(xml, instrument);
}

void CSynthesizer::XmlLoadWavetable(IXMLDOMNode* xml, std::wstring& instrument)
{
    CComPtr<IXMLDOMNode> node;
    xml->get_firstChild(&node);
    for (; node != NULL; NextNode(node))
    {
        // Get the name of the node
        CComBSTR name;
        node->get_nodeName(&name);
        if (name == L"wav")
        {
            XmlLoadWave(node, instrument);
        }
    }
}

void CSynthesizer::XmlLoadWave(IXMLDOMNode* xml, std::wstring& instrument)
{
    // Get a list of all attribute nodes and the
    // length of that list
    CComPtr<IXMLDOMNamedNodeMap> attributes;
    xml->get_attributes(&attributes);
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

        // Get the value of the attribute.  
        CComVariant value;
        attrib->get_nodeValue(&value);

        if (name == "path")
        {
            // LOAD FROM FILEPATH
            string filepath = CanonicalPath(string(*m_filename_cache + "\\" + value.bstrVal));
            char* patharr = new char[filepath.length() + 1];
            strcpy(patharr, filepath.c_str());
            CStringW cstrpath = CStringW(patharr);
            //ALL IS GOOD UP UNTIL HERE BUT I CAN'T FIGURE OUT HOW TO PROPERLY CONVERT TO LPCTSTR. also even doing it a different way it doesn't work for some reason
            AddWaveToTable((LPCTSTR)patharr);
        }
    }
}

string CSynthesizer::CanonicalPath(string path)
{
    list<string> canonical_path = list<string>();
    bool loop = true;
    size_t previous = 0;
    const string delim[] = { "/", "\\", "\\\\" };
    constexpr int delimcount = sizeof(delim) / sizeof(delim[0]);
    size_t next[delimcount];
    int closest = -1;
    
    for (int i = 0; i < delimcount; i++)
    {
        next[i] = path.find(delim[i], previous);
        if (next[i] != string::npos && (closest < 0 || next[i] < next[closest])) closest = i;
    }
    if (closest < 0) loop = false;

    std::string token;
    while (loop)
    {
        token = path.substr(previous, next[closest] - previous);
        if (token == "..")
        {
            canonical_path.pop_back();
        }
        else if (token == ".")
        {
            // nothing
        }
        else if (token.find(".score") != string::npos)
        {
            // nothing
        }
        else
        {
            canonical_path.push_back(token);
        }

        previous = next[closest] + (delim[closest]).length();
        closest = -1;
        for (int i = 0; i < delimcount; i++)
        {
            next[i] = path.find(delim[i], previous);
            if (next[i] != string::npos && (closest < 0 || next[i] < next[closest])) closest = i;
        }
        if (closest < 0) loop = false;
    }

    

    string final = "";
    for (string part : canonical_path) final = final + part + "\\";
    token = path.substr(previous);
    // final remaining token
    if (token == "..")
    {
        canonical_path.pop_back();
    }
    else if (token == ".")
    {
        // nothing
    }
    else if (token.find(".score") != string::npos)
    {
        // nothing
    }
    else
    {
        canonical_path.push_back(token);
    }
    final = final + token;

    return final;
}