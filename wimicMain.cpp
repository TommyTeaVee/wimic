/*
   WiMic remote wireless microphone server/client.
   Copyright (c) 2020 Hiroshi Takey F. <htakey@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "wimicMain.h"
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <ifaddrs.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/mount.h>
#ifdef __MSYS__
#include <sys/dirent.h>
#include <sys/cygwin.h>
#endif // __MSYS__
#include <libgen.h>

#define ARGVCNT 4
#define MAX_CHAR_PATH 512

#ifdef __WXGTK__
#include "resources/logo_wimic.xpm"
#endif

#define DEF_CONF_STRING \
            "channels = ( {\n" \
            "   name = \"WiMic\";\n" \
            "   parent = \"\";\n" \
            "   description = \"WiMic channel.\";\n" \
            "   }\n" \
            ");\n"

bool wimicDialog::_started = false;
bool wimicDialog::_server_started = false;

struct _argmain {
    char *argv[ARGVCNT];
};

/*
struct _status_system {
    bool conected;
    bool close_app;
    bool server_running;
    bool server_shuttingdown;
};

_status_system status_system;
*/

int main_start(int argc, char *argv[]);

namespace WIMIC {
    bool get_connected();
    void stop_threading();
    void detect_devices();
}

namespace UMSERVER {
extern "C" {
    int server_main_start(int argc, char **argv);
    void Server_shutdown();
    bool server_is_running();
    char *get_default_certkey_path();
}
}

//(*InternalHeaders(wimicDialog)
#include <wx/font.h>
#include <wx/intl.h>
#include <wx/settings.h>
#include <wx/string.h>
//*)


//(*IdInit(wimicDialog)
const long wimicDialog::ID_BUTTON1 = wxNewId();
const long wimicDialog::ID_BUTTON2 = wxNewId();
const long wimicDialog::ID_BUTTON3 = wxNewId();
const long wimicDialog::ID_LISTBOX1 = wxNewId();
const long wimicDialog::ID_BUTTON4 = wxNewId();
const long wimicDialog::ID_STATICTEXT2 = wxNewId();
const long wimicDialog::ID_STATICTEXT3 = wxNewId();
const long wimicDialog::ID_STATICTEXT1 = wxNewId();
const long wimicDialog::ID_LED1 = wxNewId();
const long wimicDialog::ID_STATICTEXT4 = wxNewId();
const long wimicDialog::ID_STATICTEXT5 = wxNewId();
const long wimicDialog::ID_PANEL1 = wxNewId();
const long wimicDialog::ID_TEXTCTRL1 = wxNewId();
const long wimicDialog::ID_HYPERLINKCTRL1 = wxNewId();
const long wimicDialog::ID_PANEL2 = wxNewId();
const long wimicDialog::ID_NOTEBOOK1 = wxNewId();
const long wimicDialog::ID_TIMER1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(wimicDialog,wxDialog)
    //(*EventTable(wimicDialog)
    //*)
    EVT_CLOSE(wimicDialog::OnCloseWindow)
END_EVENT_TABLE()

wimicDialog::wimicDialog(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(wimicDialog)
    Create(parent, wxID_ANY, _("WiMic Server/Client"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxCLOSE_BOX|wxMINIMIZE_BOX, _T("wxID_ANY"));
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
    Panel1 = new wxPanel(Notebook1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_PANEL1"));
    GridBagSizer1 = new wxGridBagSizer(0, 0);
    GridBagSizer1->AddGrowableCol(2);
    GridBagSizer1->AddGrowableRow(2);
    BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    close = new wxButton(Panel1, ID_BUTTON1, _("close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    BoxSizer2->Add(close, 1, wxALL|wxALIGN_TOP, 5);
    GridBagSizer1->Add(BoxSizer2, wxGBPosition(2, 1), wxDefaultSpan, wxBOTTOM|wxALIGN_RIGHT|wxALIGN_BOTTOM, 5);
    BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    start_client = new wxButton(Panel1, ID_BUTTON2, _("start"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    BoxSizer4->Add(start_client, 1, wxALL|wxALIGN_TOP, 5);
    stop_server = new wxButton(Panel1, ID_BUTTON3, _("stop"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    BoxSizer4->Add(stop_server, 1, wxALL|wxALIGN_TOP, 5);
    GridBagSizer1->Add(BoxSizer4, wxGBPosition(2, 0), wxDefaultSpan, wxBOTTOM, 5);
    BoxSizer5 = new wxBoxSizer(wxVERTICAL);
    ListBox1 = new wxListBox(Panel1, ID_LISTBOX1, wxDefaultPosition, wxSize(300,100), 0, 0, wxVSCROLL|wxHSCROLL, wxDefaultValidator, _T("ID_LISTBOX1"));
    BoxSizer5->Add(ListBox1, 1, wxALL|wxALIGN_LEFT, 5);
    BoxSizer6 = new wxBoxSizer(wxHORIZONTAL);
    select_dev = new wxButton(Panel1, ID_BUTTON4, _("select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    BoxSizer6->Add(select_dev, 0, wxALL|wxALIGN_TOP, 5);
    BoxSizer7 = new wxBoxSizer(wxVERTICAL);
    StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Device selected:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    wxFont StaticText1Font(10,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL,false,_T("Arial"),wxFONTENCODING_DEFAULT);
    StaticText1->SetFont(StaticText1Font);
    BoxSizer7->Add(StaticText1, 0, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT, 5);
    dev_label_sel = new wxStaticText(Panel1, ID_STATICTEXT3, _("dev_label_sel"), wxDefaultPosition, wxSize(200,15), 0, _T("ID_STATICTEXT3"));
    dev_label_sel->SetForegroundColour(wxColour(0,0,0));
    wxFont dev_label_selFont(10,wxFONTFAMILY_SWISS,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD,false,wxEmptyString,wxFONTENCODING_DEFAULT);
    dev_label_sel->SetFont(dev_label_selFont);
    BoxSizer7->Add(dev_label_sel, 1, wxALL|wxALIGN_LEFT, 5);
    BoxSizer6->Add(BoxSizer7, 1, wxALIGN_TOP, 5);
    BoxSizer5->Add(BoxSizer6, 0, wxALIGN_LEFT, 5);
    GridBagSizer1->Add(BoxSizer5, wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_LEFT|wxALIGN_TOP, 5);
    FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
    BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    lblstatconnection = new wxStaticText(Panel1, ID_STATICTEXT1, _("Server:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    BoxSizer3->Add(lblstatconnection, 1, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_TOP, 5);
    Led1 = new wxLed(Panel1,ID_LED1,wxColour(255,0,0),wxColour(0,255,0),wxColour(255,0,0),wxDefaultPosition,wxDefaultSize);
    Led1->SwitchOff();
    BoxSizer3->Add(Led1, 1, wxALL|wxSHAPED, 5);
    FlexGridSizer1->Add(BoxSizer3, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
    BoxSizer8 = new wxBoxSizer(wxHORIZONTAL);
    StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT4, _("IP:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    BoxSizer8->Add(StaticText2, 0, wxTOP|wxBOTTOM|wxLEFT|wxALIGN_TOP, 5);
    local_ip_label = new wxStaticText(Panel1, ID_STATICTEXT5, _("local_ip_label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    local_ip_label->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));
    BoxSizer8->Add(local_ip_label, 1, wxALL|wxALIGN_TOP, 5);
    FlexGridSizer1->Add(BoxSizer8, 1, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
    GridBagSizer1->Add(FlexGridSizer1, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_LEFT|wxALIGN_TOP, 5);
    Panel1->SetSizer(GridBagSizer1);
    GridBagSizer1->Fit(Panel1);
    GridBagSizer1->SetSizeHints(Panel1);
    about_panel = new wxPanel(Notebook1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, 0, _T("ID_PANEL2"));
    text_about = new wxTextCtrl(about_panel, ID_TEXTCTRL1, _("text_about"), wxPoint(128,80), wxSize(112,32), wxTE_NO_VSCROLL|wxTE_MULTILINE|wxTE_READONLY|wxTE_RICH|wxTE_AUTO_URL|wxTE_CENTRE|wxNO_BORDER, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    text_about->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    hyper_link_license = new wxHyperlinkCtrl(about_panel, ID_HYPERLINKCTRL1, _("hyper_link_license"), wxEmptyString, wxPoint(136,136), wxDefaultSize, wxHL_CONTEXTMENU|wxHL_ALIGN_CENTRE|wxNO_BORDER|wxTRANSPARENT_WINDOW, _T("ID_HYPERLINKCTRL1"));
    Notebook1->AddPage(Panel1, _("Main"), true);
    Notebook1->AddPage(about_panel, _("About"), false);
    BoxSizer1->Add(Notebook1, 1, wxALL|wxALIGN_TOP, 5);
    SetSizer(BoxSizer1);
    timer_connect_status.SetOwner(this, ID_TIMER1);
    BoxSizer1->Fit(this);
    BoxSizer1->SetSizeHints(this);
    Center();

    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wimicDialog::OnQuit);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wimicDialog::Onstart_clientClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wimicDialog::Onstop_clientClick);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&wimicDialog::OnButton1Click);
    Panel1->Connect(wxEVT_KILL_FOCUS,(wxObjectEventFunction)&wimicDialog::OnPanel1KillFocus,0,this);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&wimicDialog::Ontimer_connect_statusTrigger);
    Connect(wxEVT_PAINT,(wxObjectEventFunction)&wimicDialog::OnPaint);
    Connect(wxEVT_KILL_FOCUS,(wxObjectEventFunction)&wimicDialog::OnKillFocus);
    //*)

    _wm_utils = new CLWimicUtils;

    stop_server->Enable(false);
    wmsystem_status.close_app = false;
    timer_connect_status.Stop();
    Led1->SetOnOrOff(false);
    _detect_devices();
    _make_about();

    if (wmsystem_status.autostart_mode) {
        wxCommandEvent event;
        Onstart_clientClick(event);
    }
}

wimicDialog::~wimicDialog()
{
    exit(1);
    //(*Destroy(wimicDialog)
    //*)
}

void wimicDialog::OnCloseWindow(wxCloseEvent& event)
{
    Destroy();
}

void wimicDialog::OnQuit(wxCommandEvent& event)
{
    wmsystem_status.conected = WIMIC::get_connected();
    WIMIC::stop_threading();

    wmsystem_status.server_shuttingdown = true;
    wmsystem_status.close_app = true;

    if (!wmsystem_status.conected) {
        Close(true);
    }
}

void *wimicDialog::_main_start(void *arg)
{
    if (_started) {
        return NULL;
    }

    _started = true;
    _argmain *argtmp = (_argmain*)arg;

    main_start(ARGVCNT, argtmp->argv);

    _started = false;
    printf("Exit client\n");
    pthread_exit(NULL);
}

void *wimicDialog::_server_main_start(void *arg)
{
    if (_server_started) {
        return NULL;
    }

    _server_started = true;
    _argmain *argtmp = (_argmain*)arg;

    UMSERVER::server_main_start(ARGVCNT, argtmp->argv);

    printf("Exit server\n");

    _server_started = false;
    pthread_exit(NULL);
}

void wimicDialog::_start_client()
{
    if (_started) {
        wxMessageBox(_("Already running!"),_("Alert"));
    }

    _argmain *argmain = new _argmain;

    for (uint8_t i = 0; i < ARGVCNT; i++) {
        argmain->argv[i] = new char[20];
    }

    sprintf(argmain->argv[0], "%s", "wimic");
    sprintf(argmain->argv[1], "%s", "-s127.0.0.1");
    sprintf(argmain->argv[2], "%s", "-uwimic_server");
    sprintf(argmain->argv[3], "%s", "-padmin");

    pthread_t main_start_thread;
    pthread_attr_t thread_attr_main_start;

    pthread_attr_init(&thread_attr_main_start);
    pthread_attr_setstacksize(&thread_attr_main_start, 2048);

    pthread_attr_setschedpolicy(&thread_attr_main_start, SCHED_FIFO);

    pthread_create(&main_start_thread, &thread_attr_main_start, _main_start, argmain);
}

void wimicDialog::Onstart_clientClick(wxCommandEvent& event)
{
    char *drssl = UMSERVER::get_default_certkey_path();

    _wm_utils->init();
    _make_dir(drssl, 0775);

    _start_server();

    Led1->SwitchOff();

    if (!timer_connect_status.IsRunning()) {
        timer_connect_status.Start(1000);
    }
}

void wimicDialog::Onstop_clientClick(wxCommandEvent& event)
{
    WIMIC::stop_threading();
    wmsystem_status.server_shuttingdown = true;
}

void wimicDialog::_start_server()
{
    if (_server_started) {
        wxMessageBox(_("Server already running!"),_("Alert"));
    }

    _argmain *argmain = new _argmain;

    for (uint8_t i = 0; i < ARGVCNT; i++) {
        argmain->argv[i] = new char[20];
    }

    sprintf(argmain->argv[0], "%s", "wimic");
    sprintf(argmain->argv[1], "%s", "-d");
    sprintf(argmain->argv[2], "%s", "-r");
    sprintf(argmain->argv[3], "%s", "-b1234");

    pthread_t server_main_start_thread;
    pthread_attr_t thread_attr_server_main_start;

    pthread_attr_init(&thread_attr_server_main_start);
    pthread_attr_setstacksize(&thread_attr_server_main_start, 2048);

    pthread_attr_setschedpolicy(&thread_attr_server_main_start, SCHED_FIFO);

    pthread_create(&server_main_start_thread, &thread_attr_server_main_start, _server_main_start, argmain);
}

void wimicDialog::Ontimer_connect_statusTrigger(wxTimerEvent& event)
{
    if (!wmsystem_status.server_running) {
        if (UMSERVER::server_is_running()) {
            wmsystem_status.server_running = true;
            _start_client();
        }
    }

    if (!wmsystem_status.conected) {
        if (wmsystem_status.server_shuttingdown) {
            wmsystem_status.server_shuttingdown = false;
            UMSERVER::Server_shutdown();
            wmsystem_status.server_running = false;
            _detect_devices();
            if (wmsystem_status.close_app) {
                Close(true);
            }
        }
    }

    wmsystem_status.conected = WIMIC::get_connected();

    Led1->SetOnOrOff(wmsystem_status.conected);
    stop_server->Enable(wmsystem_status.conected);
    start_client->Enable(!wmsystem_status.conected);
    select_dev->Enable(!wmsystem_status.conected);
}

void wimicDialog::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
}

void wimicDialog::_make_dir(const char *dir, int perm)
{
    DIR *dr = opendir(dir);
    if (dr == NULL) {
        mkdir(dir, perm);
    }
}

void wimicDialog::OnKillFocus(wxFocusEvent& event)
{
    //event.Skip();
}

void wimicDialog::OnPanel1KillFocus(wxFocusEvent& event)
{
    //event.Skip();
}

void wimicDialog::OnButton1Click(wxCommandEvent& event)
{
    uint8_t sel = ListBox1->GetSelection();
    wmdev.default_dev = wmdev.index_tmp_out[sel];
    dev_label_sel->SetLabel(wxString::FromAscii(wmdev.name[wmdev.default_dev]));
    printf("Device Nr:%d Name: %s\n", wmdev.default_dev, wmdev.name[wmdev.default_dev]);
}

void wimicDialog::_detect_devices()
{
    wxArrayString s1;

    WIMIC::detect_devices();

    uint8_t indextmp = 0;
    for (uint8_t i = 0; i < wmdev.dev_count; i++) {
        if (wmdev.inout_dev[i] == INOUT_DEV::OUTPUT_DEV) {
            s1.Add(wxString::FromAscii(wmdev.name[i]));
            wmdev.index_tmp_out[indextmp] = i;
            indextmp++;
        }
    }

    if (wmsystem_status.default_dev > 0) {
        wmdev.default_dev = wmsystem_status.default_dev;
    }

    ListBox1->Clear();
    ListBox1->InsertItems(s1, 0);
    ListBox1->SetSelection(indextmp - 1);
    dev_label_sel->SetLabel(wxString::FromAscii(wmdev.name[wmdev.default_dev]));

    const char *localip = _get_local_ip();
    local_ip_label->SetLabel(wxString::FromAscii(localip));
}

const char *wimicDialog::_get_local_ip()
{
    struct ifaddrs *ifaddr, *ifa;
    int s;
    static char hostip[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), hostip, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

        switch(ifa->ifa_addr->sa_family) {
            case AF_INET:
                printf("Device: %s Family: IPV4 %s\n", ifa->ifa_name, hostip);
                break;

            case AF_INET6:
                printf("Device: %s Family: IPV6 %s\n", ifa->ifa_name, hostip);
                break;

            default:
                printf("Unknown AF\n");
        }

        if (!(strstr(hostip, "127.")) && (strstr(hostip, "192.") || strstr(hostip, "10.")) && (ifa->ifa_addr->sa_family == AF_INET)) {
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                exit(EXIT_FAILURE);
            }

            printf("\n\tInterface : <%s>\n", ifa->ifa_name);
            printf("\tAddress   : <%s>\n\n", hostip);
            return hostip;
        }
    }

    freeifaddrs(ifaddr);
    return hostip;
}

void wimicDialog::_make_about()
{
    SetIcon(wxICON(logo_wimic));

    text_about->SetValue(_("\n\nWiMic, remote wireless microphone server & client.\n"
                           "Copyright (c) 2020 Hiroshi Takey F. <htakey@gmail.com>\n"
                           "Licensed under GPLv3 License."));

    text_about->SetPosition(wxPoint(0,0));
    text_about->SetSize(Panel1->GetSize().GetWidth(), 120);
    text_about->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

    hyper_link_license->SetLabel(_("https://github.com/hiro2233/wimic/"));
    hyper_link_license->SetPosition(wxPoint(0, text_about->GetSize().GetHeight()));
    hyper_link_license->SetSize(Panel1->GetSize().GetWidth(), 20);
}

int wimicDialog::_lookup_host(const char *host)
{
    struct addrinfo hints, *res;
    int errcode;
    char addrstr[100];
    void *ptr;

    memset(&hints, 0, sizeof (hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    errcode = getaddrinfo(host, NULL, &hints, &res);
    if (errcode != 0) {
        perror ("getaddrinfo");
        return -1;
    }

    while(res) {
        //inet_ntop (res->ai_family, res->ai_addr->sa_data, addrstr, 100);
        switch (res->ai_family) {
        case AF_INET:
            ptr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
            break;
        case AF_INET6:
            ptr = &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
            break;
        }

        inet_ntop(res->ai_family, ptr, addrstr, 100);
        printf("IPv%d address: %s (%s)\n", res->ai_family == PF_INET6 ? 6 : 4, addrstr, res->ai_canonname);
        res = res->ai_next;
    }

    return 0;
}

/************************
 *     Wimic Utils
 ************************/
CLWimicUtils::CLWimicUtils()
{
}

void CLWimicUtils::init()
{
    make_userdatadir();
}

void CLWimicUtils::write_buff_to_file(uint8_t **cbuff, const char file_conf[])
{
    FILE *fdest_wimic_conf = fopen(file_conf, "w");
    fprintf(fdest_wimic_conf, "%s", cbuff[0]);
    fclose(fdest_wimic_conf);
}

uint32_t CLWimicUtils::read_file_to_buff(const char file_conf[], uint8_t **cbuff)
{
    uint32_t sizewimic_conf;

    FILE *fwimicconf= fopen(file_conf, "a+");
    fseek(fwimicconf, 0, SEEK_END);
    sizewimic_conf = ftell(fwimicconf);

    if (sizewimic_conf == 0) {
        fprintf(fwimicconf, "%s", DEF_CONF_STRING);
        sizewimic_conf = ftell(fwimicconf);
    }

    fseek(fwimicconf, 0, SEEK_SET);

    cbuff[0] = new uint8_t[sizewimic_conf + 1];
    memset(cbuff[0], 0, sizewimic_conf + 1);
    fread(cbuff[0], sizeof(uint8_t), sizewimic_conf, fwimicconf);

    fclose(fwimicconf);

    return sizewimic_conf;
}

void CLWimicUtils::conv_to_unix_mix_path(char* name)
{
  while ((name = strchr (name, '\\')) != NULL)
    {
      if (*name == '\\')
	*name = '/';
       name++;
   }
}

void CLWimicUtils::get_cw_dir(char *cwdir)
{
	char current_dir[MAX_CHAR_PATH] = {0};
	getcwd(current_dir, MAX_CHAR_PATH);

    char respath[MAX_CHAR_PATH] = {0};
    uint32_t cntpath = readlink("/proc/self/exe", respath, MAX_CHAR_PATH);
    const char *path = nullptr;

    if (cntpath != -1) {
        path = dirname(respath);
    } else {
        path = current_dir;
    }

#ifdef __MSYS__
    cygwin_conv_path(CCP_POSIX_TO_WIN_A | CCP_ABSOLUTE, current_dir, cwdir, MAX_CHAR_PATH);
    conv_to_unix_mix_path(cwdir);
#elif defined(__UNIX__)
    memcpy(cwdir, path, MAX_CHAR_PATH);
#endif
}

void CLWimicUtils::init_fs_urus_cygmsys()
{
    char wimic_mnt_data_path[MAX_CHAR_PATH] = {0};
    char current_dir[MAX_CHAR_PATH] = {0};
#ifdef __MSYS__
    mount("none", "/", MOUNT_CYGDRIVE | MOUNT_BINARY | MOUNT_NOPOSIX | MOUNT_NOACL | MOUNT_USER_TEMP);

    get_cw_dir(current_dir);

    /* If wimic doesn't have write permission on relative path,
     * then, we mount on wimic data path with write path permission,
     * on windows we mount to executable wimic root drive, this assume
     * that wimic have write permissions.
    */
    sprintf(wimic_mnt_data_path, "%c%c/%s", current_dir[0], current_dir[1], "wimic");
    mkdir(wimic_mnt_data_path, 0775);

    mount(wimic_mnt_data_path, "/system/urus", MOUNT_BIND | MOUNT_BINARY | MOUNT_NOPOSIX | MOUNT_NOACL | MOUNT_USER_TEMP);
    chdir("/system/urus");
    mkdir("slotdata", 0775);
    chdir(current_dir);
#elif defined(__UNIX__)
    get_cw_dir(current_dir);
    chdir("/system/urus");
    mkdir("slotdata", 0775);
    chdir(current_dir);
#endif
}

void CLWimicUtils::log_wimic(char current_dir[], uint8_t **conf_buff, uint32_t sizewimic_conf)
{
    char urus_path[] = "/system/urus/slotdata";
    char buf[MAX_CHAR_PATH] = {0};
    char hdir[50] = {0};
    struct passwd *pw = getpwuid(getuid());
    const char *username = pw->pw_name;

    printf("USER: %s\n", username);
    sprintf(hdir, "%s\n", username);

    FILE *flogwimic= fopen("logfile.txt", "a+");

    fprintf(flogwimic, "\n-----------------\n");
    fprintf(flogwimic, "WiMic log start!\n");
	fprintf(flogwimic, "current working directory BEFORE: %s\n", current_dir);

    fprintf(flogwimic, "USER: %s\n", hdir);
    if (sizewimic_conf > 0) {
        fprintf(flogwimic, "CYGCONVERT: %s \nSize buf conf: %d\ndata:\n\n%s\n", current_dir, sizewimic_conf, conf_buff[0]);
    } else {
        fprintf(flogwimic, "CYGCONVERT: %s \nSize buf conf: %d\ndata:\n%s\n\n", current_dir, sizewimic_conf, "buf conf empty");
    }

    fprintf(flogwimic, "Listing file dirs on '%s':\n\n", urus_path);
    DIR *dirunix;
    struct dirent *ent;
    if ((dirunix = opendir(urus_path)) != NULL) {
      while ((ent = readdir(dirunix)) != NULL) {
        if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }

        char fdir[FILENAME_MAX] = {0};
        char fdirtmp[FILENAME_MAX] = {0};
        struct stat path_stat;

        strcat(fdirtmp, urus_path);
        strcat(fdirtmp, "/");
        strcat(fdirtmp, ent->d_name);
        stat(fdirtmp, &path_stat);
        if (S_ISDIR(path_stat.st_mode)) {
            sprintf(fdir, "%s/", ent->d_name);
        } else {
            sprintf(fdir, "%s", ent->d_name);
        }

        printf("--> %s\n", fdir);
        fprintf(flogwimic, "--> %s\n", fdir);
      }
      closedir(dirunix);
    } else {
      printf("Can't open dir %s\n", urus_path);
      fprintf(flogwimic, "Can't open dir %s\n", urus_path);
    }

	getcwd(buf, MAX_CHAR_PATH);
	fprintf(flogwimic, "\ncurrent working directory AFTER: %s\n", buf);

    fclose(flogwimic);
}

void CLWimicUtils::make_userdatadir()
{
    uint8_t *cbuff[1];
    uint32_t sizewimic_conf;
    char current_dir[MAX_CHAR_PATH] = {0};

    get_cw_dir(current_dir);

    init_fs_urus_cygmsys();

    sizewimic_conf = read_file_to_buff("wimic.conf", cbuff);

    get_cw_dir(current_dir);
    chdir("/system/urus/slotdata");

    write_buff_to_file(cbuff ,"wimic.conf");

#if DEBUG_WIMIC == 1
    log_wimic(current_dir, cbuff, sizewimic_conf);
#endif // DEBUG_WIMIC
}
