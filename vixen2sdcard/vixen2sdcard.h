#ifndef __vixen2sdcard_H__
#define __vixen2sdcard_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "vixen2sdcard.h"
#endif

#include <wx/wx.h>
#include <wx/tglbtn.h>
#include <wx/filename.h>
#include <wx/config.h>
#include <wx/xml/xml.h>
#include <stdint.h>


#define LOG_MSG_TO_STDOUT
//#define LOG_MSG_TO_WINDOW

#if defined(LOG_MSG_TO_WINDOW)
#define printf(...) (wxLogMessage(__VA_ARGS__))
#elif defined(LOG_MSG_TO_STDOUT)
#else
#define printf(...)
#endif


class MyFrame: public wxFrame
{
public:
    MyFrame( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE );
private:
	wxFlexGridSizer *grid;
	wxScrolledWindow *scroll;
	int parse_count;
	int parse_command_len;
	uint8_t parse_buf[4096];
private:
	void OnQuit(wxCommandEvent &event);
	void OnBrowse(wxCommandEvent &event);
	void OnChoose(wxCommandEvent &event);
	void OnConvert(wxCommandEvent &event);
	void OnIdle(wxIdleEvent &event);
	void err(const wxString& message);
	DECLARE_EVENT_TABLE()
};


class MyApp: public wxApp
{
public:
	MyApp();
	virtual bool OnInit();
	virtual int OnExit();
};

#endif
