#include "httplib.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>

#include "PromtCtlDocument.hpp"
#include "PromtFTManager.hpp"

template<typename... T>
static inline void print(T... args) {
    ((std::cout << args <<  ' '), ...) << std::endl;
}

static inline std::string random_filename(int len = 65) {
    static const char ASCII_PRINTABLE[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution distribution(0, static_cast<int>(strlen(ASCII_PRINTABLE) - 1));
    std::string random_string;
    random_string.reserve(len);
    for (int i = 0; i < len; ++i)
        random_string += ASCII_PRINTABLE[distribution(generator)];
    return random_string;
}

static const char TMP_FOLDER[] = "C:\\tmpfs\\";
static const auto TMP_IN = TMP_FOLDER + random_filename();
static const auto TMP_OUT = TMP_FOLDER + random_filename();

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
    out.resize(size);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, out.data(), size, 0, 0);
    delete[] wstr;

    return out;
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
        res.set_content(read_file(TMP_OUT), "text/html");
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
        if (!std::filesystem::exists(TMP_FOLDER)) std::filesystem::create_directory(TMP_FOLDER);

        m_svr.Post("/translate", [this](const httplib::Request &req, httplib::Response &res) {
            print("Got request!");
            TranslateHandler(req, res);
        });
    }

    ~WebServer() {
        stop();
    }

    void listen(const char *host = "0.0.0.0", unsigned short port = 80) {
        m_svr.listen(host, port);
    }

    void stop() {
        m_svr.stop();
        std::remove(TMP_IN.c_str());
        std::remove(TMP_OUT.c_str());
    }
};

int main() {
    print("Starting...");

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IDENTIFY, nullptr, EOAC_NONE, nullptr);

    WebServer ws;
    ws.listen();
}
