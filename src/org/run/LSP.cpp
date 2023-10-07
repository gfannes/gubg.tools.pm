#include <org/run/LSP.hpp>
#include <org/tree/Content.hpp>
#include <org/tree/Node.hpp>

#include <gubg/file/Filesystem.hpp>
#include <gubg/mss.hpp>
#include <gubg/Signaled.hpp>
#include <gubg/string/concat.hpp>

#include <cstdlib>

namespace org { namespace run {

    LSP::LSP(const Options &options)
        : options_(options), log_({.filename = options.log_filepath}) {}

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

            std::optional<Id> id;
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

                Pos pos;
                MSS(read_(pos, request));

                std::string document_fp;
                {
                    const nlohmann::json *ptr = &request;
                    MSS(read_(ptr, "params", *ptr));
                    MSS(read_(ptr, "textDocument", *ptr));
                    std::string uri;
                    MSS(read_(uri, "uri", *ptr));
                    gubg::Strange uri_se{uri};
                    MSS(uri_se.pop_if("file://"));
                    uri_se.pop_all(document_fp);
                }

                MSS(gubg::file::read(tmp_str_, document_fp));

                const auto markup_type = markup::guess_from_filepath(document_fp);
                MSS(!!markup_type);

                MSS(parser_.init(*markup_type));
                MSS(parser_.parse(tmp_str_));

                const auto node = parser_.root.find_line(pos.line);
                MSS(!!node);
                MSS(node->type == tree::Type::Line);
                const auto &line = node->line();

                markup::Parser parser;
                MSS(parser.init(*markup_type));
                gubg::Strange text, link;
                if (parser.extract_link(text, link, line.content))
                {
                    if (link.starts_with("https://") || link.starts_with("http://"))
                    {
                        auto command = gubg::string::concat("xdg-open", ' ', link.str());

                        const auto code = std::system(command.c_str());
                        if (code != 0)
                            log_.warning() << "Running command '" << command << "' failed with code " << code << std::endl;

                        response.set(true).ref() = {
                            {"jsonrpc", jsonrpc},
                            {"id", *id},
                            {"result", nullptr},
                        };
                    }
                    else
                    {
                        std::filesystem::path link_fp = link.str();
                        if (!link_fp.is_absolute())
                        {
                            auto dir = std::filesystem::path{document_fp}.parent_path();
                            link_fp = dir / link_fp;
                        }

                        response.set(true).ref() = {
                            {"jsonrpc", jsonrpc},
                            {"id", *id},
                            {"result", {{{"range", {{"start", {{"character", 0}, {"line", 0}}}, {"end", {{"character", 0}, {"line", 0}}}}}, {"uri", gubg::string::concat("file://", link_fp.string())}}}},
                        };
                    }
                }
                else
                {
                    response.set(true).ref() = {
                        {"jsonrpc", jsonrpc},
                        {"id", *id},
                        {"result", nullptr},
                    };
                }
            }
            else if (method == "textDocument/didOpen")
            {
            }
            else if (method == "textDocument/didClose")
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

    bool LSP::read_(std::size_t &value, const std::string &name, const nlohmann::json &jobj)
    {
        MSS_BEGIN(bool);
        auto it = jobj.find(name);
        MSS(it != jobj.end(), log_.error() << "Could not find '" << name << "'" << std::endl);
        MSS(it.value().is_number_integer());
        value = it.value();
        log_.os(0) << C(name) C(value) << std::endl;
        MSS_END();
    }

    bool LSP::read_(const nlohmann::json *&ptr, const std::string &name, const nlohmann::json &jobj)
    {
        MSS_BEGIN(bool);
        auto it = jobj.find(name);
        MSS(it != jobj.end(), log_.error() << "Could not find '" << name << "'" << std::endl);
        MSS(it.value().is_object());
        ptr = &it.value();
        MSS_END();
    }

    bool LSP::read_(Pos &pos, const nlohmann::json &jobj)
    {
        MSS_BEGIN(bool);

        const nlohmann::json *ptr;

        MSS(read_(ptr, "params", jobj));
        MSS(read_(ptr, "position", *ptr));

        MSS(read_(pos.line, "line", *ptr));
        MSS(read_(pos.col, "character", *ptr));

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
