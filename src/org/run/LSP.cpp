#include <org/run/LSP.hpp>

#include <gubg/Signaled.hpp>
#include <gubg/mss.hpp>
#include <org/tree/Node.hpp>

namespace org { namespace run {

    LSP::LSP(const Options &options)
        : options_(options) {}

    bool LSP::run()
    {
        MSS_BEGIN(bool);

        nlohmann::json request;
        gubg::Signaled<nlohmann::json> response;

        for (bool do_continue = true; do_continue;)
        {
            log_.os(0) << std::endl;
            MSS(read_json_message_(request));
            log_.os(0) << C(request) << std::endl;

            std::string jsonrpc;
            MSS(read_(jsonrpc, "jsonrpc", request));
            MSS(jsonrpc == "2.0");

            std::string method;
            MSS(read_(method, "method", request));

            std::optional<int> id;
            if (request.count("id"))
                MSS(read_(id.emplace(), "id", request));

            response.set(false);
            if (method == "initialize")
            {
                MSS(!!id);
                response.set(true).ref() = {
                    {"jsonrpc", jsonrpc},
                    {"id", *id},
                    {"result", {{"capabilities", {{"definitionProvider", true}}}, {"serverInfo", {{"name", "org"}, {"version", "v1"}}}}},
                };
            }
            else if (method == "initialized")
            {
                log_.os(0) << "We are initialized" << std::endl;
            }
            else if (method == "shutdown")
            {
                MSS(!!id);
                response.set(true).ref() = {
                    {"jsonrpc", jsonrpc},
                    {"id", *id},
                    {"result", nullptr},
                };
            }
            else if (method == "exit")
            {
                log_.os(0) << "Exit" << std::endl;
                do_continue = false;
            }
            else if (method == "textDocument/definition")
            {
                MSS(!!id);
                response.set(true).ref() = {
                    {"jsonrpc", jsonrpc},
                    {"id", *id},
                    {"result", {{{"range", {{"start", {{"character", 0}, {"line", 0}}}, {"end", {{"character", 0}, {"line", 0}}}}}, {"uri", "file:///home/geertf/tmp/a.txt"}}}},
                };
            }
            else if (method == "textDocument/didOpen")
            {
            }
            else
            {
                MSS(false, log_.error() << "Unsupported method '" << method << "'" << std::endl);
            }

            if (response)
            {
                log_.os(0) << C(*response) << std::endl;
                const std::string msg = response->dump();
                std::cout << "Content-Length: " << msg.size() << "\r\n\r\n"
                          << msg << std::flush;
            }
        }

        MSS_END();
    }

    // Privates
    bool LSP::read_(std::string &value, const std::string &name, const nlohmann::json &jobj)
    {
        MSS_BEGIN(bool);
        auto it = jobj.find(name);
        MSS(it != jobj.end(), log_.error() << "Could not find '" << name << "'" << std::endl);
        MSS(it.value().is_string());
        value = it.value();
        log_.os(0) << C(name) C(value) << std::endl;
        MSS_END();
    }

    bool LSP::read_(int &value, const std::string &name, const nlohmann::json &jobj)
    {
        MSS_BEGIN(bool);
        auto it = jobj.find(name);
        MSS(it != jobj.end(), log_.error() << "Could not find '" << name << "'" << std::endl);
        MSS(it.value().is_number_integer());
        value = it.value();
        log_.os(0) << C(name) C(value) << std::endl;
        MSS_END();
    }

    bool LSP::read_json_message_(nlohmann::json &msg) const
    {
        MSS_BEGIN(bool);

        std::getline(std::cin, tmp_str_);
        gubg::Strange strange{tmp_str_};
        MSS(strange.pop_if("Content-Length: "));

        std::size_t size;
        MSS(strange.pop_decimal(size));

        std::getline(std::cin, tmp_str_);
        tmp_str_.resize(size);
        std::cin.read(tmp_str_.data(), size);

        msg = nlohmann::json::parse(tmp_str_);

        MSS_END();
    }

}} // namespace org::run
