#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_ask.H>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#ifdef _WIN32
#include <windows.h>
#endif
#ifndef _WIN32
#include <unistd.h>
#endif

using json = nlohmann::json;

struct BrowserEntry {
    std::string name;
    std::string path;
    std::vector<std::string> args;
};

static std::vector<BrowserEntry> g_browsers;
static size_t g_last_used = 0;
static bool g_verbose = false;

static Fl_Input* g_url_input = nullptr;
static Fl_Hold_Browser* g_browser_list = nullptr;

static std::filesystem::path config_path() {
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    std::filesystem::path dir = appdata ? appdata : ".";
    dir /= "BrowserSelector";
#else
    const char* xdg = std::getenv("XDG_CONFIG_HOME");
    std::filesystem::path dir;
    if (xdg) dir = xdg; else {
        const char* home = std::getenv("HOME");
        dir = home ? std::filesystem::path(home) / ".config" : ".";
    }
    dir /= "browserselector";
#endif
    std::filesystem::create_directories(dir);
    return dir / "config.json";
}

static void save_config() {
    json j;
    j["last_used"] = g_last_used;
    j["browsers"] = json::array();
    for (auto& b : g_browsers) {
        j["browsers"].push_back({{"name", b.name}, {"path", b.path}, {"args", b.args}});
    }
    std::ofstream ofs(config_path());
    ofs << j.dump(2);
}

static void load_config() {
    std::ifstream ifs(config_path());
    if (!ifs) return;
    json j; ifs >> j;
    g_last_used = j.value("last_used", 0);
    for (auto& item : j["browsers"]) {
        BrowserEntry b;
        b.name = item.value("name", "");
        b.path = item.value("path", "");
        b.args = item.value("args", std::vector<std::string>{});
        g_browsers.push_back(b);
    }
}

static void update_browser_list() {
    g_browser_list->clear();
    for (auto& b : g_browsers) {
        g_browser_list->add(b.name.c_str());
    }
    if (!g_browsers.empty()) {
        size_t sel = std::min(g_last_used, g_browsers.size()-1);
        g_browser_list->select(sel+1);
    }
}

static void launch_browser(const BrowserEntry& b, const std::string& url) {
#ifdef _WIN32
    std::string cmd = "\"" + b.path + "\"";
    for (auto& a : b.args) {
        cmd += " \"" + a + "\"";
    }
    if (!url.empty()) cmd += " \"" + url + "\"";
    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    if (CreateProcessA(NULL, cmd.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    }
#else
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(b.path.c_str()));
        for (auto& a : b.args) argv.push_back(const_cast<char*>(a.c_str()));
        if (!url.empty()) argv.push_back(const_cast<char*>(url.c_str()));
        argv.push_back(nullptr);
        execvp(b.path.c_str(), argv.data());
        _exit(1);
    }
#endif
}

static void on_go() {
    int index = g_browser_list->value();
    if (index <= 0 || index > (int)g_browsers.size()) return;
    std::string url = g_url_input->value();
    if (g_verbose) std::cout << "Launching " << g_browsers[index-1].name << " with URL " << url << std::endl;
    launch_browser(g_browsers[index-1], url);
    g_last_used = index-1;
    save_config();
    std::exit(0);
}

static void go_cb(Fl_Widget*, void*) { on_go(); }

static void add_browser_cb(Fl_Widget*, void*) {
    Fl_Native_File_Chooser chooser(Fl_Native_File_Chooser::BROWSE_FILE);
    chooser.title("Select Browser");
    if (chooser.show() != 0) return;
    std::string path = chooser.filename();
    const char* name = fl_input("Browser name:", std::filesystem::path(path).stem().string().c_str());
    BrowserEntry b;
    b.path = path;
    b.name = name ? name : std::filesystem::path(path).stem().string();
    g_browsers.push_back(b);
    g_last_used = g_browsers.size()-1;
    save_config();
    update_browser_list();
}

static void remove_browser_cb(Fl_Widget*, void*) {
    int index = g_browser_list->value();
    if (index <= 0 || index > (int)g_browsers.size()) return;
    g_browsers.erase(g_browsers.begin()+index-1);
    g_last_used = 0;
    save_config();
    update_browser_list();
}

static void browser_cb(Fl_Widget* w, void*) {
    if (Fl::event_clicks() && g_url_input->size()>0 && std::strlen(g_url_input->value())>0) {
        on_go();
    }
}

static void menu_cb(Fl_Widget*, void* data) {
    const char* label = (const char*)data;
    if (!std::strcmp(label, "exit")) {
        std::exit(0);
    } else if (!std::strcmp(label, "openconfig")) {
        auto path = config_path().parent_path().string();
#ifdef _WIN32
        ShellExecuteA(NULL, "open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
        pid_t pid = fork();
        if (pid == 0) {
            execlp("xdg-open", "xdg-open", path.c_str(), (char*)NULL);
            _exit(1);
        }
#endif
    } else if (!std::strcmp(label, "setdefault")) {
        fl_message("Set as default browser is not implemented in this demo.");
    }
}

int main(int argc, char** argv) {
    std::string url;
    for (int i=1;i<argc;i++) {
        std::string arg = argv[i];
        if (arg == "--verbose" || arg == "-v") { g_verbose = true; }
        else if (url.empty()) url = arg; // first non-flag argument
    }
    load_config();
    Fl_Window win(600, 400, "BrowserSelector");
    Fl_Menu_Bar menubar(0,0,600,25);
    menubar.add("File/Set as Default Browser", 0, menu_cb, (void*)"setdefault");
    menubar.add("File/Open Config Directory", 0, menu_cb, (void*)"openconfig");
    menubar.add("File/Exit", 0, menu_cb, (void*)"exit");
    g_url_input = new Fl_Input(0,25,550,25);
    g_url_input->value(url.c_str());
    g_url_input->take_focus();
    g_url_input->position(0); g_url_input->mark(g_url_input->size()); // select all
    g_url_input->when(FL_WHEN_ENTER_KEY);
    g_url_input->callback(go_cb);
    Fl_Button go(550,25,50,25,"Go");
    go.callback(go_cb);
    g_browser_list = new Fl_Hold_Browser(0,50,600,300);
    g_browser_list->callback(browser_cb);
    g_browser_list->when(FL_WHEN_RELEASE);
    Fl_Button addBtn(0,350,100,25,"Add Browser");
    addBtn.callback(add_browser_cb);
    Fl_Button remBtn(100,350,100,25,"Remove");
    remBtn.callback(remove_browser_cb);
    update_browser_list();
    win.end();
    win.show(argc, argv);
    return Fl::run();
}

