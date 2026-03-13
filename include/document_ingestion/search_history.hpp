#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

namespace document_ingestion {

struct FoundFile {
  std::string file_path;
  int matches = 0;
};

struct SearchRecord {
  std::string search_id;
  std::string file_type;
  std::string directory;
  std::string keyword;
  std::string timestamp;
  int files_found = 0;
  int files_stored = 0;
  std::vector<FoundFile> found_files;
};

class SearchHistory {
 public:
  explicit SearchHistory(const std::filesystem::path& history_file);
  void load();
  void save();
  SearchRecord add_search(const std::string& file_type, const std::string& directory,
                          const std::string& keyword, const std::vector<FoundFile>& found_files,
                          int files_stored = 0);
  std::vector<SearchRecord> get_all_searches() const;
  struct Stats {
    int total_searches = 0;
    int total_files_found = 0;
    int total_files_stored = 0;
  };
  Stats get_statistics() const;

 private:
  std::filesystem::path history_file_;
  std::vector<SearchRecord> searches_;
};

}  // namespace document_ingestion
