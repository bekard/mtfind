#pragma once

#include <fstream>
#include <istream>
#include <memory>
#include <sstream>

using stream_ptr = std::unique_ptr<std::istream>;

class stream_provider_iface {
public:
  virtual ~stream_provider_iface() = default;

  virtual stream_ptr create_stream() const = 0;
  virtual size_t get_stream_size() const = 0;
};

class istringstream_provider final : public stream_provider_iface {
public:
  explicit istringstream_provider(const std::string &text);
  ~istringstream_provider() override = default;

  stream_ptr create_stream() const override;
  size_t get_stream_size() const override;

private:
  std::string text;
  size_t stream_size;
};

class ifstream_provider final : public stream_provider_iface {
public:
  ifstream_provider(const std::string &file_path);
  ~ifstream_provider() override = default;

  stream_ptr create_stream() const override;
  size_t get_stream_size() const override;

private:
  std::string file_path;
  size_t stream_size;
};

using stream_provider_ptr = std::shared_ptr<stream_provider_iface>;
