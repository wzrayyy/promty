#include "httplib.hpp"

#include <filesystem>
#include <fstream>
#include <functional>
#include <mutex>
#include <csignal>
#include <string>
#include <windows.h>
#include <wingdi.h>

#include "PromtCtlDocument.hpp"
#include "PromtFTManager.hpp"


static inline std::string random_filename(int len = 65) {
    static const char ASCII_PRINTABLE[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'A', 'B', 'C', 'D', 'E', 'F',
                                           'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
                                           'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
                                           'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution distribution(0, 62);
    std::string random_string;
    for (std::size_t i = 0; i < len; ++i)
        random_string += ASCII_PRINTABLE[distribution(generator)];
    return random_string;
}

static const char TMP_FOLDER[] = "C:\\tmpfs\\";
static const auto TMP_IN = TMP_FOLDER + random_filename();
static const auto TMP_OUT = TMP_FOLDER + random_filename();

// https://www.linux.org.ru/forum/development/3968525?cid=3971135
static inline size_t cp1251_to_utf8(const char *in, char *out = nullptr) {
    static const int table[128] = {
        0x82D0, 0x83D0,   0x9A80E2, 0x93D1, 0x9E80E2, 0xA680E2, 0xA080E2, 0xA180E2, 0xAC82E2, 0xB080E2, 0x89D0,   0xB980E2, 0x8AD0,
        0x8CD0, 0x8BD0,   0x8FD0,   0x92D1, 0x9880E2, 0x9980E2, 0x9C80E2, 0x9D80E2, 0xA280E2, 0x9380E2, 0x9480E2, 0,        0xA284E2,
        0x99D1, 0xBA80E2, 0x9AD1,   0x9CD1, 0x9BD1,   0x9FD1,   0xA0C2,   0x8ED0,   0x9ED1,   0x88D0,   0xA4C2,   0x90D2,   0xA6C2,
        0xA7C2, 0x81D0,   0xA9C2,   0x84D0, 0xABC2,   0xACC2,   0xADC2,   0xAEC2,   0x87D0,   0xB0C2,   0xB1C2,   0x86D0,   0x96D1,
        0x91D2, 0xB5C2,   0xB6C2,   0xB7C2, 0x91D1,   0x9684E2, 0x94D1,   0xBBC2,   0x98D1,   0x85D0,   0x95D1,   0x97D1,   0x90D0,
        0x91D0, 0x92D0,   0x93D0,   0x94D0, 0x95D0,   0x96D0,   0x97D0,   0x98D0,   0x99D0,   0x9AD0,   0x9BD0,   0x9CD0,   0x9DD0,
        0x9ED0, 0x9FD0,   0xA0D0,   0xA1D0, 0xA2D0,   0xA3D0,   0xA4D0,   0xA5D0,   0xA6D0,   0xA7D0,   0xA8D0,   0xA9D0,   0xAAD0,
        0xABD0, 0xACD0,   0xADD0,   0xAED0, 0xAFD0,   0xB0D0,   0xB1D0,   0xB2D0,   0xB3D0,   0xB4D0,   0xB5D0,   0xB6D0,   0xB7D0,
        0xB8D0, 0xB9D0,   0xBAD0,   0xBBD0, 0xBCD0,   0xBDD0,   0xBED0,   0xBFD0,   0x80D1,   0x81D1,   0x82D1,   0x83D1,   0x84D1,
        0x85D1, 0x86D1,   0x87D1,   0x88D1, 0x89D1,   0x8AD1,   0x8BD1,   0x8CD1,   0x8DD1,   0x8ED1,   0x8FD1};

    size_t size = 0;
    while (*in)
        if (*in & 0x80) {
            int v = table[(int) (0x7f & *in++)];
            if (!v) continue;
            if (out) {
                *out++ = (char) v;
                *out++ = (char) (v >> 8);
            }
            size += 2;
            if (v >>= 16) {
                if (out) *out++ = (char) v;
                ++size;
            }
        } else {
            if (out)
                *out++ = *in++;
            else
                ++in;
            ++size;
        }
    if (out) *out = 0;
    return ++size;
}

static inline void convert_file(const std::string &filename) {
    char *in_buf = nullptr;
    {
        std::ifstream ifs(filename);
        ifs.seekg(0, ifs.end);
        size_t size = ifs.tellg();
        ifs.seekg(0, ifs.beg);
        in_buf = new char[size];
        ifs.read(in_buf, size);
    }
    auto size = cp1251_to_utf8(in_buf);
    char *out_buf = new char[size]{'A'};
    cp1251_to_utf8(in_buf, out_buf);

    std::ofstream ofs(filename);
    ofs.write(out_buf, size);

    delete[] in_buf;
    delete[] out_buf;
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
        convert_file(TMP_OUT);
        res.set_file_content(TMP_OUT, content_type);
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
