#include <org/App.hpp>

#include <org/tree/Parser.hpp>
#include <org/tree/Prefix.hpp>
#include <org/tree/Writer.hpp>

#include <gubg/Signaled.hpp>
#include <gubg/Strange.hpp>
#include <gubg/file/Filesystem.hpp>
#include <gubg/map.hpp>
#include <gubg/mss.hpp>

#include <fstream>

namespace org {

    bool App::run()
    {
        MSS_BEGIN(bool);

        switch (options_.mode)
        {
            case Mode::Normal: MSS(run_normal_()); break;
            case Mode::LSP: MSS(run_lsp_()); break;
            default: MSS(false, log_.error() << "Unknown mode '" << (int)options_.mode << "'"); break;
        }

        log_.os(0) << "App.run() is done" << std::endl;

        MSS_END();
    }

    // Privates
    bool App::run_normal_()
    {
        MSS_BEGIN(bool);

        MSS(!options_.filepath.empty(), log_.error() << "Expected filepath to be set" << std::endl);

        std::string content;
        MSS(gubg::file::read(content, options_.filepath));

        tree::Parser parser;
        MSS(parser.parse(content));
        std::cout << parser.root << std::endl;

        tree::Node &root = parser.root;

        if (!options_.state.empty())
        {
            MSS(options_.primary < options_.ranges.size());
            const gubg::ix::Range &range = options_.ranges[options_.primary];
            tree::Node *node = root.find(range.start());
            MSS(!!node);
            std::cout << "Found node " << *node << std::endl;

            {
                tree::Content *content = std::get_if<tree::Content>(&node->data);
                MSS(!!content);
                tree::Prefix prefix;
                MSS(prefix.parse(content->content));
                const bool is_bullet = prefix.indent && prefix.indent->back() == '-';
                if (options_.state == "-")
                    prefix.state.reset();
                else if (is_bullet && options_.state == "TODO")
                    prefix.state = "[ ]";
                else if (is_bullet && options_.state == "DONE")
                    prefix.state = "[X]";
                else if (is_bullet && options_.state == "QUESTION")
                    prefix.state = "[?]";
                else
                    prefix.state = options_.state;
                if (options_.tag)
                    prefix.tags = *options_.tag;
                std::string tmp;
                MSS(prefix.serialize(tmp));
                content->content = tmp;
            }

            tree::Writer writer;
            content.resize(0);
            MSS(writer.write(content, parser.root));

            MSS(gubg::file::write(content, options_.filepath));
        }

        MSS_END();
    }

    bool App::read_(std::string &value, const std::string &name, const nlohmann::json &jobj)
    {
        MSS_BEGIN(bool);
        auto it = jobj.find(name);
        MSS(it != jobj.end(), log_.error() << "Could not find '" << name << "'" << std::endl);
        MSS(it.value().is_string());
        value = it.value();
        log_.os(0) << C(name) C(value) << std::endl;
        MSS_END();
    }

    bool App::read_(int &value, const std::string &name, const nlohmann::json &jobj)
    {
        MSS_BEGIN(bool);
        auto it = jobj.find(name);
        MSS(it != jobj.end(), log_.error() << "Could not find '" << name << "'" << std::endl);
        MSS(it.value().is_number_integer());
        value = it.value();
        log_.os(0) << C(name) C(value) << std::endl;
        MSS_END();
    }

    bool App::run_lsp_()
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

    bool App::read_json_message_(nlohmann::json &msg) const
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

} // namespace org
