#ifndef EDITOR_BUFFER_HPP
#define EDITOR_BUFFER_HPP

#include "event.hpp"

#include <boost/text/rope.hpp>
#include <boost/text/segmented_vector.hpp>
#include <boost/filesystem/fstream.hpp>

#include <vector>


struct snapshot_t
{
    boost::text::rope content_;
    boost::text::segmented_vector<int> line_sizes_;
    int first_row_ = 0;
    screen_pos_t cursor_pos_;
};

struct buffer_t
{
    snapshot_t snapshot_;
    boost::filesystem::path path_;
    std::vector<snapshot_t> history_;
};

inline bool dirty (buffer_t const & b)
{ return !b.snapshot_.content_.equal_root(b.history_.back().content_); }

inline buffer_t load (boost::filesystem::path path)
{
    boost::filesystem::ifstream ifs(path);

    snapshot_t snapshot;
    int line_size = 0;
    while (ifs.good()) {
        boost::text::text chunk;
        int const chunk_size = 1 << 16;
        chunk.resize(chunk_size, ' ');
        ifs.read(chunk.begin(), chunk_size);
        if (!ifs.good())
            chunk.resize(ifs.gcount(), ' ');

        auto it = std::find(chunk.begin(), chunk.end(), '\n');
        line_size += chunk.begin() - it;
        while (it != chunk.end()) {
            snapshot.line_sizes_.push_back(line_size);
            line_size = 0;
            it = std::find(it + 1, chunk.end(), '\n');
        }
        snapshot.content_ += std::move(chunk);
    }

    if (line_size)
        snapshot.line_sizes_.push_back(line_size);

    return buffer_t{snapshot, path, {1, snapshot}};
}

#endif
