#include <org/run/Normal.hpp>

#include <org/tree/Content.hpp>
#include <org/tree/Parser.hpp>
#include <org/tree/Writer.hpp>

#include <gubg/Strange.hpp>
#include <gubg/file/Filesystem.hpp>
#include <gubg/map.hpp>
#include <gubg/mss.hpp>
#include <gubg/string/concat.hpp>

#include <fstream>

namespace org { namespace run {

    bool Normal::run()
    {
        MSS_BEGIN(bool);

        MSS(!options_.filepath.empty(), log_.error() << "Expected filepath to be set" << std::endl);

        MSS(parse_(options_.filepath));

        if (options_.state || options_.tag)
        {
            // Find node that corresponds with start of the primary range
            tree::Node *node = nullptr;
            {
                MSS(options_.primary < options_.ranges.size());
                const gubg::ix::Range &range = options_.ranges[options_.primary];
                node = root_.find(range.start());
                MSS(!!node);
                std::cout << "Found node " << *node << std::endl;
            }

            {
                tree::Line *line = std::get_if<tree::Line>(&node->data);
                MSS(!!line);

                tree::Content content;
                MSS(content.parse(line->content));

                if (options_.state)
                {
                    if (options_.state == "-")
                        content.state.reset();
                    else if (line->is_bullet && options_.state == "TODO")
                        content.state = "[ ]";
                    else if (line->is_bullet && options_.state == "DONE")
                        content.state = "[X]";
                    else if (line->is_bullet && options_.state == "QUESTION")
                        content.state = "[?]";
                    else
                        content.state = options_.state;
                }
                if (options_.tag)
                {
                    content.tags = gubg::string::concat(':', *options_.tag, ':');
                }

                {
                    MSS(content.serialize(tmp_str_));
                    line->content = tmp_str_;
                }
            }

            MSS(write_(options_.filepath));
        }

        MSS_END();
    }

    // Privates
    bool Normal::parse_(const std::string &fp)
    {
        MSS_BEGIN(bool);

        MSS(gubg::file::read(tmp_str_, options_.filepath));

        auto markup_type = markup::guess_from_filepath(options_.filepath);
        MSS(!!markup_type, log_.error() << "Could not guess markup type for '" << options_.filepath << "'" << std::endl);

        tree::Parser parser{*markup_type};
        MSS(parser.parse(tmp_str_));

        std::swap(parser.root, root_);

        MSS_END();
    }

    bool Normal::write_(const std::string &fp)
    {
        MSS_BEGIN(bool);

        tree::Writer writer;
        tmp_str_.resize(0);
        MSS(writer.write(tmp_str_, root_));

        MSS(gubg::file::write(tmp_str_, options_.filepath));

        MSS_END();
    }

}} // namespace org::run
