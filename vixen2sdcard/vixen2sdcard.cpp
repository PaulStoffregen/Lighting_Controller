/*  Vixen2sdcard lighting data converter
 *  Copyright 2014, Paul Stoffregen (paul@pjrc.com)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "vixen2sdcard.h"
#endif

#include "wx/wxprec.h"
#include "vixen2sdcard.h"


//------------------------------------------------------------------------------
// MyFrame
//------------------------------------------------------------------------------

wxFileName *input_file;
wxFileName *output_file;
wxTextCtrl *input_name;
wxTextCtrl *output_name;
wxConfig *config;


#define ID_BROWSE	10000
#define ID_CHOOSE	10001
#define ID_CONVERT	10002


BEGIN_EVENT_TABLE(MyFrame,wxFrame)
	EVT_MENU(wxID_EXIT, MyFrame::OnQuit)
	EVT_BUTTON(ID_BROWSE, MyFrame::OnBrowse)
	EVT_BUTTON(ID_CHOOSE, MyFrame::OnChoose)
	EVT_BUTTON(ID_CONVERT, MyFrame::OnConvert)
	//EVT_IDLE(MyFrame::OnIdle)
END_EVENT_TABLE()

MyFrame::MyFrame( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxFrame( parent, id, title, position, size, style )
{
	wxStaticText *txt;
	wxBoxSizer *hbox, *main_vbox;
	wxButton *binput, *boutput, *conv;

	#ifdef LOG_MSG_TO_WINDOW
	wxLog::SetActiveTarget(new wxLogWindow(this, "Debug Messages"));
	#endif

	input_file = new wxFileName();
	output_file = new wxFileName();

	wxMenuBar *menubar = new wxMenuBar;
	wxMenu *menu = new wxMenu;
	menu->Append(wxID_EXIT, "Quit", "");
	menubar->Append(menu, "File");
	SetMenuBar(menubar);
	CreateStatusBar(1);

	main_vbox = new wxBoxSizer(wxVERTICAL);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	txt = new wxStaticText(this, -1, "Vixen 2 to SD Card File Converter");
	hbox->Add(txt);
	main_vbox->Add(hbox, wxSizerFlags().Center().Border(wxALL, 10));

	hbox = new wxBoxSizer(wxHORIZONTAL);
	txt = new wxStaticText(this, -1, "Input File:");
	input_name = new wxTextCtrl(this, -1, input_file->GetFullPath());
	binput = new wxButton(this, ID_BROWSE, "Browse...");
	hbox->Add(txt, wxSizerFlags().Center());
	hbox->Add(input_name, wxSizerFlags().Proportion(1).Expand().Border(wxLEFT, 12));
	hbox->Add(binput, wxSizerFlags().Border(wxLEFT, 4));
	main_vbox->Add(hbox, wxSizerFlags().Border(wxALL, 6).Expand());

	hbox = new wxBoxSizer(wxHORIZONTAL);
	txt = new wxStaticText(this, -1, "Output File:");
	output_name = new wxTextCtrl(this, -1, output_file->GetFullPath());
	boutput = new wxButton(this, ID_CHOOSE, "Choose...");
	hbox->Add(txt, wxSizerFlags().Center());
	hbox->Add(output_name, wxSizerFlags().Proportion(1).Expand().Border(wxLEFT, 4));
	hbox->Add(boutput, wxSizerFlags().Border(wxLEFT, 4));
	main_vbox->Add(hbox, wxSizerFlags().Border(wxALL, 6).Expand());

	hbox = new wxBoxSizer(wxHORIZONTAL);
	conv = new wxButton(this, ID_CONVERT, "Convert");
	hbox->Add(conv);
	main_vbox->Add(hbox, wxSizerFlags().Center().Border(wxALL, 10));

	config = new wxConfig("vixen2sdcard", "PaulStoffregen");
	wxString str;
	str = config->Read("input");
	input_name->SetValue(str);
	str = config->Read("output");
	output_name->SetValue(str);

	SetSizer(main_vbox);
}

void MyFrame::OnBrowse(wxCommandEvent &event)
{
	//printf("OnBrowse: ");
	*input_file = input_name->GetValue();
	//printf("file: %s, ", input_file->GetFullName().c_str());
	//printf("path: %s\n", input_file->GetPath().c_str());
	wxFileDialog d(this, "Choose Input File", input_file->GetPath(),
		input_file->GetFullName(), "Vixen Sequence (*.vix)|*.vix",
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (d.ShowModal() == wxID_OK) {
		wxString selected = d.GetPath();
		*input_file = selected;
		input_name->SetValue(selected);
		output_file->SetName(input_file->GetName());
		output_file->SetExt("txt");
		output_name->SetValue(output_file->GetFullPath());
		config->Write("input", input_name->GetValue());
		config->Write("output", output_name->GetValue());
		config->Flush();
	}
}

void MyFrame::OnChoose(wxCommandEvent &event)
{
	//printf("OnChoose: ");
	*output_file = output_name->GetValue();
	//printf("file: %s, ", output_file->GetFullName().c_str());
	//printf("path: %s\n", output_file->GetPath().c_str());
	wxFileDialog d(this, "Save File", output_file->GetPath(),
		output_file->GetFullName(), "Text File (*.txt)|*.txt", wxFD_SAVE);
	if (d.ShowModal() == wxID_OK) {
		wxString selected = d.GetPath();
		//printf("selected = %s\n", selected.c_str());
		*output_file = selected;
		output_name->SetValue(output_file->GetFullPath());
		config->Write("output", output_name->GetValue());
		config->Flush();
	}
}


wxXmlNode *find(wxXmlNode *node, const wxString& name)
{
	if (!node) return NULL;
	//printf("find: this=%s\n", node->GetName().c_str());
	if (node->GetName() == name) return node;
	for (wxXmlNode *ch = node->GetChildren(); ch; ch = ch->GetNext()) {
		wxXmlNode *t = find(ch, name);
		if (t) return t;
	}

	return NULL;
}

wxString text(wxXmlNode *node)
{
	if (!node) return "";
	wxXmlNode *ch = node->GetChildren();
	if (!ch) return "";
	if (ch->GetName() != "text") return "";
	return ch->GetContent();
}

int base64decode (const char *in, size_t inLen, unsigned char *out, size_t *outLen);


void MyFrame::err(const wxString& message)
{
	SetStatusText("Opps, could not convert Vixen data");
	wxMessageDialog dlg(this, message, "Error", wxCANCEL);
	dlg.ShowModal();
}


void MyFrame::OnConvert(wxCommandEvent &event)
{
	//printf("OnConvert\n");
	config->Write("input", input_name->GetValue());
	config->Write("output", output_name->GetValue());
	config->Flush();

	*input_file = input_name->GetValue();
	if (!input_file->FileExists()) {
		err("The input file does not seem to exist.  Please check the name and location.");
		return;
	}
	if (!input_file->IsFileReadable()) {
		err("Unable to get permission to read the input file.  Please check security settings.  Perhaps the file is owned by a different user?");
		return;
	}

	wxXmlDocument doc;
	wxXmlNode *root, *channeltop, *channel;

	if (!doc.Load(input_name->GetValue())) {
		err("An error occurred while reading the input file.  Maybe it's missing or corrupted?");
		return;
	}
	root = doc.GetRoot();

	wxString period = text(find(root, "EventPeriodInMilliseconds"));
	unsigned long period_millis;
	if (!period.ToULong(&period_millis)) {
		err("Unable to find EventPeriodInMilliseconds, maybe the input file is corrupted?\n");
		return;
	}
	if (period_millis < 10) {
		err("The update period is too short.  10 milliseconds is the minimum\n");
		return;
	}
	if (period_millis > 2000) {
		err("The update period is too long.  2 seconds is the minimum\n");
		return;
	}
	//printf("period = %lu\n", period_millis);

	channeltop = find(root, "Channels");
	size_t count = 0;
	channel = find(channeltop, "Channel");
	while (channel && channel->GetName() == "Channel") {
		count++;
		channel = channel->GetNext();
	}
	//printf("channels = %lu\n", count);

	wxString data = text(find(root, "EventValues"));
	size_t inlen = data.Len();
	//printf("base64 length = %lu\n", inlen);
	size_t outlen = data.Len() * 3 / 4 + 1000;
	unsigned char *out = (unsigned char *)malloc(outlen);
	base64decode(data.c_str(), inlen, out, &outlen);

	//printf("decoded length = %lu\n", outlen);
	size_t samples = outlen / count;
	//printf("samples = %lu\n", samples);

	FILE *fp = fopen(output_name->GetValue().c_str(), "w");
	if (!fp) {
		free(out);
		err("Unable to write output file.  Perhaps the SD card is not inserted, or the output location does not exist?");
		return;
	}

	fprintf(fp, "%lu\n", period_millis);
	for (unsigned int n = 0; n < samples; n++) {
		for (unsigned int c = 0; c < count; c++) {
			fprintf(fp, "%02X", out[c * samples + n]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
	free(out);

	wxString mesg;
	mesg.Printf("%lu channels, %lu seconds", (unsigned long)count,
		(unsigned long)(samples * period_millis / 1000));
	SetStatusText("Success: " + mesg);
	wxMessageDialog dlg(this, "Successfully converted data:\n" + mesg, "Completed", wxOK);
	dlg.ShowModal();

}




void MyFrame::OnIdle(wxIdleEvent &event)
{
	//uint8_t buf[1024];
	//int r;
	//printf("Idle event\n");
	//event.RequestMore(true);
}


void MyFrame::OnQuit( wxCommandEvent &event )
{
     Close( true );
}





//------------------------------------------------------------------------------
// MyApp
//------------------------------------------------------------------------------

IMPLEMENT_APP(MyApp)

MyApp::MyApp()
{
}

bool MyApp::OnInit()
{

	MyFrame *frame = new MyFrame(NULL, -1, "Vixen 2 SD Card",
		wxPoint(180,120), wxSize(520,300) );
	frame->Show( true );
    
	return true;
}

int MyApp::OnExit()
{
	delete config;
	return 0;
}





// from
// http://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64#C_2

#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66
 
static const unsigned char d[] = {
    66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,
    54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,
    29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66
};
 
int base64decode (const char *in, size_t inLen, unsigned char *out, size_t *outLen) { 
    const char *end = in + inLen;
    size_t buf = 1, len = 0;
 
    while (in < end) {
        unsigned char c = d[(int)(*in++)];
 
        switch (c) {
        case WHITESPACE: continue;   /* skip whitespace */
        case INVALID:    return 1;   /* invalid input, return error */
        case EQUALS:                 /* pad character, end of data */
            in = end;
            continue;
        default:
            buf = buf << 6 | c;
 
            /* If the buffer is full, split it into bytes */
            if (buf & 0x1000000) {
                if ((len += 3) > *outLen) return 1; /* buffer overflow */
                *out++ = buf >> 16;
                *out++ = buf >> 8;
                *out++ = buf;
                buf = 1;
            }   
        }
    }
 
    if (buf & 0x40000) {
        if ((len += 2) > *outLen) return 1; /* buffer overflow */
        *out++ = buf >> 10;
        *out++ = buf >> 2;
    }
    else if (buf & 0x1000) {
        if (++len > *outLen) return 1; /* buffer overflow */
        *out++ = buf >> 4;
    }
 
    *outLen = len; /* modify to reflect the actual output size */
    return 0;
}




