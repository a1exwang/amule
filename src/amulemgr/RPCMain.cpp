#include "Looper.h"
#include "Server.h"

#include <amule.h>
#include <amuleDlg.h>
#include <SearchDlg.h>
#include <MuleNotebook.h>
#include <SearchListCtrl.h>
#include <SearchFile.h>
#include <DownloadQueue.h>
#include <GuiEvents.h>
#include <TransferWnd.h>
#include <DownloadListCtrl.h>
#include <PartFile.h>
#include <muuli_wdr.h>

#include <wx/app.h>
#include <wx/choice.h>
#include <wx/window.h>

#include <thread>
#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include <boost/asio.hpp>
#include <mutex>
#include <condition_variable>
#include <iostream>

using namespace std;
using namespace AW;
using boost::asio::ip::tcp;

AW::string WxStringToAwString(const wxString& str) {
	//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	//return convert.to_bytes(wstring(str.GetData()));
	return AW::string(str.ToUTF8().data());
}

wxString AwStringToWxString(const AW::string& str) {
	//std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
	//return wxString(convert.from_bytes(str));
	return wxString::FromUTF8(str.c_str());
}

mutex mutexPre;
mutex mutexAfter;

mutex muEvent;
condition_variable conEvent;

void wait() {
	unique_lock<mutex> lock1(muEvent);
	conEvent.wait(lock1);
}

extern "C" void serverStart(void* searchDialogPtr) {
	thread([&, searchDialogPtr]() -> void {
		AwRpc* awrpc;
		auto rpcTable = std::vector<std::shared_ptr<AbstractServerBase>>({
			
			std::shared_ptr<AbstractServerBase>(new Server<AW::string, AW::string>([](AW::string v) -> AW::string { return v; }, t("echo"))),
			std::shared_ptr<AbstractServerBase>(new Server<std::vector<AW::string>, AW::string, AW::uint32>([&, searchDialogPtr](AW::string keyWord, AW::uint32 waitSeconds) -> std::vector<AW::string> {

				//mutexPre.lock();
				//auto searchDlg = theApp->amuledlg->m_searchwnd;
				auto searchDlg = (CSearchDlg*)searchDialogPtr;
				// set search parameters

				static_cast<wxChoice*>(searchDlg->FindWindow(ID_SEARCHTYPE))->SetSelection(2);
				dynamic_cast<wxTextCtrl*>(searchDlg->FindWindow(IDC_SEARCHNAME))->SetValue(AwStringToWxString(keyWord));
				//dynamic_cast<wxTextCtrl*>(searchDlg->FindWindow(IDC_SEARCHNAME))->SetValue(L"1234");

				int index = searchDlg->m_notebook->GetPageCount();
				cout << keyWord << endl;
				// notify the UI thread to start search
				wxCommandEvent ev(wxEVT_COMMAND_BUTTON_CLICKED, IDC_STARTS);
				searchDlg->AddPendingEvent(ev);
				// *********************
				// here we need to wait for the button clicked event to finish
				//wait(); 
				wxMilliSleep(1000);
				//mutexPre.unlock();

				// wait for search results
				// change this number to change result count

				vector<AW::string> ret;
				wxSleep(waitSeconds);
				
				//mutexAfter.lock();
				// get search results
				CSearchListCtrl* page = dynamic_cast<CSearchListCtrl*>(searchDlg->m_notebook->GetPage(index));
				
				for (int i = 0; i < page->GetItemCount(); ++i) {
					CSearchFile* cfile = reinterpret_cast<CSearchFile*>(page->GetItemData(i));
					wxString ed2k = theApp->CreateED2kLink(cfile) + wxString(_("\n"));
					ret.push_back(WxStringToAwString(ed2k));
					//ret.push_back("ok\n");
				}
				//mutexAfter.unlock();

				return ret;
			}, t("searchByKeyword")))
		});
		awrpc = new AwRpc(rpcTable);
		awrpc->startService();
	}).detach();
}
