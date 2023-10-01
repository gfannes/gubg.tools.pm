#include <org/App.hpp>

#include <org/tree/Parser.hpp>
#include <org/tree/Prefix.hpp>
#include <org/tree/Writer.hpp>

#include <gubg/Strange.hpp>
#include <gubg/file/Filesystem.hpp>
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

    bool App::run_lsp_()
    {
        MSS_BEGIN(bool);

        std::string msg;
        MSS(read_json_message_(msg));
        log_.os(0) << C(msg) << std::endl;

        MSS_END();
    }

    bool App::read_json_message_(std::string &msg) const
    {
        MSS_BEGIN(bool);

        std::getline(std::cin, msg);
        gubg::Strange strange{msg};
        MSS(strange.pop_if("Content-Length: "));

        std::size_t size;
        MSS(strange.pop_decimal(size));

        std::getline(std::cin, msg);
        msg.resize(size);
        std::cin.read(msg.data(), size);

        MSS_END();
    }

} // namespace org
