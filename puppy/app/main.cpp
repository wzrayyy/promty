#include "http/httplib.h"

#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <windows.h>
#include <winternl.h>

#include "PromtCtlDocument.hpp"
#include "PromtFTManager.hpp"

#define EPOCH_DIFF 116444736000000000LL
#define TICKS_PER_SEC 10000000LL

static LARGE_INTEGER fake_time;

static NTSTATUS WINAPI HookedNtQuerySystemTime(PLARGE_INTEGER time)
{
    if (time)
        time->QuadPart = fake_time.QuadPart;

    fake_time.QuadPart += TICKS_PER_SEC;
    return 0;
}

static void InstallTimeHook()
{
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll)
        return;

    auto target = (BYTE *)GetProcAddress(ntdll, "NtQuerySystemTime");
    if (!target)
        return;

    DWORD old;
    VirtualProtect(target, 5, PAGE_EXECUTE_READWRITE, &old);

    intptr_t rel = (BYTE *)HookedNtQuerySystemTime - target - 5;
    target[0] = 0xE9; // jmp rel32
    *(int32_t *)(target + 1) = (int32_t)rel;

    VirtualProtect(target, 5, old, &old);

    const char *env = std::getenv("FAKETIME");
    long long unix_ts = env ? std::strtoll(env, nullptr, 10) : 0;
    fake_time.QuadPart = unix_ts * TICKS_PER_SEC + EPOCH_DIFF;
}

static inline std::string random_filename(int len = 65) {
    static const char ASCII_PRINTABLE[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static std::random_device random_device;
    static std::mt19937 generator(random_device());
    static std::uniform_int_distribution distribution(0, static_cast<int>(strlen(ASCII_PRINTABLE) - 1));
    std::string random_string;
    random_string.reserve(len);
    for (int i = 0; i < len; ++i)
        random_string += ASCII_PRINTABLE[distribution(generator)];
    return random_string;
}

static constexpr std::string TMP_FOLDER = "C:\\tmpfs\\";
static const auto TMP_IN = TMP_FOLDER + random_filename();
static const auto TMP_OUT = TMP_FOLDER + random_filename();

template<typename... T>
static inline void print(T... args) {
    ((std::cout << args <<  ' '), ...) << std::endl;
}

static inline auto utf8_to_cp(const char *data) {
    int size = MultiByteToWideChar(CP_UTF8, 0, data, -1, 0, 0);
    auto wstr = new wchar_t[size];
    MultiByteToWideChar(CP_UTF8, 0, data, -1, wstr, size);

    size = WideCharToMultiByte(1251, 0, wstr, -1, NULL, 0, 0, 0);
    std::string out;
    out.resize(size);
    WideCharToMultiByte(1251, 0, wstr, -1, out.data(), size, 0, 0);
    delete[] wstr;
    return out;
}

static inline auto read_file(const std::string &filename) {
    char *in_buf = nullptr;
    {
        std::ifstream ifs(filename, std::ios::binary);
        ifs.seekg(0, ifs.end);
        size_t size = ifs.tellg();
        ifs.seekg(0, ifs.beg);
        in_buf = new char[size + 1];
        ifs.read(in_buf, size);
        in_buf[size] = 0;
    }

    int size = MultiByteToWideChar(1251, 0, in_buf, -1, NULL, 0);
    wchar_t *wstr = new wchar_t[size];
    MultiByteToWideChar(1251, 0, in_buf, -1, wstr, size);
    delete[] in_buf;

    size = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, 0, 0);
    std::string out;
    out.resize(size - 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, out.data(), size - 1, 0, 0);
    delete[] wstr;

    std::string out_lf;
    out_lf.reserve(out.size());

    for (auto c : out) {
        if (c == '\r')
            continue;
        out_lf.push_back(c);
    }

    return out_lf;
}

class WebServer {
  private:
    PromtCtlDocument m_doc{};
    PromtFTManager m_ftman{};
    httplib::Server m_svr;
    std::mutex m_global_lock;

  private:
    void TranslateHtml(const std::string &text, httplib::Response &res) {
        const auto ft = PromtFTManager::FileType::kHTML;
        auto direction = m_doc.direction();
        auto translator = m_ftman.Translator(ft, direction);
        {
            std::ofstream ofs(TMP_IN);
            ofs << utf8_to_cp(text.c_str());
        }
        translator.Translate(TMP_IN, TMP_OUT);
        res.set_content(std::move(read_file(TMP_OUT)), "text/html");
    }

    void TranslateText(const std::string &text, httplib::Response &res) {
        res.set_content("Plain text translation not implemented", "text/plain");
        res.status = 400;
    }

    bool SetDirection(const httplib::Headers &headers, httplib::Response &res) {
        const auto target_dir_it = headers.find("x-translation-direction");
        if (target_dir_it != headers.end()) {
            auto target_dir = target_dir_it->second;
            if (target_dir == "en-ru")
                m_doc.direction(PromtCtlDocument::Direction::kEngRus);
            else if (target_dir == "ru-en")
                m_doc.direction(PromtCtlDocument::Direction::kRusEng);
            else {
                print("Wrong X-Translation-Direction header:", target_dir);
                res.set_content("X-Translation-Direction must be one of: [\"en-ru\", \"ru-en\"]", "text/plain");
                res.status = 400;
                return false;
            }
        } else
            m_doc.direction(PromtCtlDocument::Direction::kEngRus);

        return true;
    }

    void TranslateHandler(const httplib::Request &req, httplib::Response &res) {
        const std::lock_guard lock(m_global_lock);

        if (!SetDirection(req.headers, res)) return;

        const auto &body = req.body;
        if (body.empty()) {
            print("Request with empty body");
            res.set_content("Request body must not be empty", "text/plain");
            res.status = 400;
            return;
        }

        const auto content_type_it = req.headers.find("content-type");
        if (content_type_it == req.headers.end()) {
            TranslateText(body, res);
            return;
        }

        auto content_type = content_type_it->second;

        if (content_type == "text/html")
            TranslateHtml(body, res);
        else
            TranslateText(body, res);
    }

  public:
    WebServer() {
        if (!std::filesystem::exists(TMP_FOLDER))
            std::filesystem::create_directory(TMP_FOLDER);

        m_svr.Post("/translate", [this](const httplib::Request &req, httplib::Response &res) {
            TranslateHandler(req, res);
        });

        m_svr.Get("/health", [](const httplib::Request &req, httplib::Response &res) {
            res.set_content("OK", "text/plain");
            res.status = 200;
        });
    }

    ~WebServer() {
        stop();
    }

    void listen(const char *host = "0.0.0.0", unsigned short port = 80) {
        print("started!");
        m_svr.listen(host, port);
    }

    void stop() {
        m_svr.stop();
        std::remove(TMP_IN.c_str());
        std::remove(TMP_OUT.c_str());
    }
};

static std::function<void(int)> signal_action;

void signal_handler(int signal) {
    signal_action(signal);
}

int main() {
    InstallTimeHook();

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IDENTIFY, nullptr, EOAC_NONE, nullptr);

    WebServer ws;

    signal_action = [&](int x) { ws.stop(); };
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);

    ws.listen();
}
