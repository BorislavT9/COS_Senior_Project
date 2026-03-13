#include "document_ingestion/search_history.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>

namespace document_ingestion {

static std::string generate_uuid() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 15);
  std::uniform_int_distribution<> dis2(8, 11);
  std::stringstream ss;
  ss << std::hex;
  for (int i = 0; i < 8; i++) ss << dis(gen);
  ss << "-";
  for (int i = 0; i < 4; i++) ss << dis(gen);
  ss << "-4";
  for (int i = 0; i < 3; i++) ss << dis(gen);
  ss << "-" << dis2(gen);
  for (int i = 0; i < 3; i++) ss << dis(gen);
  ss << "-";
  for (int i = 0; i < 12; i++) ss << dis(gen);
  return ss.str();
}

static std::string iso_timestamp() {
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);
  std::tm tm_buf;
#ifdef _WIN32
  localtime_s(&tm_buf, &time);
#else
  localtime_r(&time, &tm_buf);
#endif
  std::ostringstream oss;
  oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S");
  return oss.str();
}

SearchHistory::SearchHistory(const std::filesystem::path& history_file) : history_file_(history_file) {
  load();
}

void SearchHistory::load() {
  if (!std::filesystem::exists(history_file_)) {
    searches_.clear();
    return;
  }
  try {
    std::ifstream f(history_file_);
    auto data = nlohmann::json::parse(f);
    searches_.clear();
    for (const auto& item : data) {
      SearchRecord rec;
      rec.search_id = item["search_id"];
      rec.file_type = item["file_type"];
      rec.directory = item["directory"];
      rec.keyword = item["keyword"];
      rec.timestamp = item["timestamp"];
      rec.files_found = item.value("files_found", 0);
      rec.files_stored = item.value("files_stored", 0);
      if (item.contains("found_files")) {
        for (const auto& ff : item["found_files"]) {
          FoundFile f;
          f.file_path = ff.value("file_path", "");
          f.matches = ff.value("matches", 0);
          if (!f.file_path.empty()) rec.found_files.push_back(f);
        }
      }
      searches_.push_back(rec);
    }
  } catch (...) {
    searches_.clear();
  }
}

void SearchHistory::save() {
  std::filesystem::create_directories(history_file_.parent_path());
  nlohmann::json data;
  for (const auto& s : searches_) {
    nlohmann::json files_j = nlohmann::json::array();
    for (const auto& f : s.found_files) {
      files_j.push_back({{"file_path", f.file_path}, {"matches", f.matches}});
    }
    data.push_back({
      {"search_id", s.search_id},
      {"file_type", s.file_type},
      {"directory", s.directory},
      {"keyword", s.keyword},
      {"timestamp", s.timestamp},
      {"files_found", s.files_found},
      {"files_stored", s.files_stored},
      {"found_files", files_j},
    });
  }
  std::ofstream f(history_file_);
  f << data.dump(2);
}

SearchRecord SearchHistory::add_search(const std::string& file_type, const std::string& directory,
                                       const std::string& keyword, const std::vector<FoundFile>& found_files,
                                       int files_stored) {
  SearchRecord rec;
  rec.search_id = generate_uuid();
  rec.file_type = file_type;
  rec.directory = directory;
  rec.keyword = keyword;
  rec.timestamp = iso_timestamp();
  rec.found_files = found_files;
  rec.files_found = static_cast<int>(found_files.size());
  rec.files_stored = files_stored;
  searches_.push_back(rec);
  save();
  return rec;
}

std::vector<SearchRecord> SearchHistory::get_all_searches() const {
  auto copy = searches_;
  std::sort(copy.begin(), copy.end(),
            [](const SearchRecord& a, const SearchRecord& b) { return a.timestamp > b.timestamp; });
  return copy;
}

SearchHistory::Stats SearchHistory::get_statistics() const {
  Stats s;
  s.total_searches = static_cast<int>(searches_.size());
  for (const auto& rec : searches_) {
    s.total_files_found += rec.files_found;
    s.total_files_stored += rec.files_stored;
  }
  return s;
}

}  // namespace document_ingestion
