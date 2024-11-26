#include "httplib.hpp"

#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <windows.h>
#include <wingdi.h>

#include "PromtCtlDocument.hpp"
#include "PromtFTManager.hpp"

static inline std::string random_filename(int len = 65) {
    static const char ASCII_PRINTABLE[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution distribution(0, static_cast<int>(strlen(ASCII_PRINTABLE) - 1));
    std::string random_string;
    for (std::size_t i = 0; i < len; ++i)
        random_string += ASCII_PRINTABLE[distribution(generator)];
    return random_string;
}

static const char TMP_FOLDER[] = "C:\\tmpfs\\";
static const auto TMP_IN = TMP_FOLDER + random_filename();
static const auto TMP_OUT = TMP_FOLDER + random_filename();

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
    void TranslateHandler(const httplib::Request &req, httplib::Response &res) {
        std::lock_guard _(m_global_lock);

        auto target_dir_it = req.headers.find("x-translation-direction");
        if (target_dir_it != req.headers.end()) {
            auto target_dir = target_dir_it->second;
            if (target_dir == "en-ru")
                m_doc.direction(PromtCtlDocument::Direction::kEngRus);
            else if (target_dir == "ru-en")
                m_doc.direction(PromtCtlDocument::Direction::kRusEng);
            else {
                res.set_content("X-Translation-Direction must be one of: [\"en-ru\", \"ru-en\"]", "text/plain");
                res.status = 400;
                return;
            }
        } else
            m_doc.direction(PromtCtlDocument::Direction::kEngRus);

        auto content_type_it = req.headers.find("content-type");
        if (content_type_it == req.headers.end()) {
            res.set_content("Missing Content-Type header", "text/plain");
            res.status = 400;
            return;
        }

        auto content_type = content_type_it->second;
        PromtFTManager::FileType ft;
        if (content_type == "text/html")
            ft = PromtFTManager::FileType::kHTML;
        else if (content_type == "text/rtf")
            ft = PromtFTManager::FileType::kRTF;
        else {
            res.set_content("Can't translate this content type", "text/plain");
            res.status = 400;
            return;
        }

        auto direction = m_doc.direction();
        auto translator = m_ftman.Translator(ft, direction);

        auto hash = std::to_string(std::hash<std::string>{}(req.body));

        {
            std::ofstream ofs(TMP_IN);
            ofs << req.body;
        }

        translator.Translate(TMP_IN, TMP_OUT);
        res.set_content(read_file(TMP_OUT), content_type);
    }

  public:
    WebServer() {
        if (!std::filesystem::exists(TMP_FOLDER)) std::filesystem::create_directory(TMP_FOLDER);
        m_svr.Post("/translate", [this](const httplib::Request &req, httplib::Response &res) { TranslateHandler(req, res); });
    }

    ~WebServer() {
        stop();
    }

    void listen() {
        m_svr.listen("0.0.0.0", 80);
    }

    void stop() {
        m_svr.stop();
        std::remove(TMP_IN.c_str());
        std::remove(TMP_OUT.c_str());
    }
};

int main() {
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CoInitializeSecurity(0, -1, 0, 0, 1u, 2u, 0, 0, 0); // TODO: make this pretty

    WebServer ws;
    ws.listen();
}
