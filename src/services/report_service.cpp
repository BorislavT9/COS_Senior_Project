#include "document_ingestion/report_service.hpp"
#include <fstream>
#include <algorithm>
#include <sstream>

namespace document_ingestion {

void generate_html_report(const ExtractionStore& store, const std::filesystem::path& output_path) {
  const auto& docs_map = store.documents();
  std::vector<std::reference_wrapper<const DocumentRecord>> sorted;
  for (const auto& [_, rec] : docs_map) sorted.push_back(std::cref(rec));
  std::sort(sorted.begin(), sorted.end(),
            [](const auto& a, const auto& b) { return a.get().processed_at > b.get().processed_at; });

  size_t recent_count = std::min(size_t(50), sorted.size());
  auto indexed = store.get_indexed_fields();
  std::ostringstream idx_ss;
  for (size_t i = 0; i < indexed.size(); ++i) {
    if (i > 0) idx_ss << ", ";
    idx_ss << indexed[i];
  }
  std::string indexed_fields = indexed.empty() ? "None" : idx_ss.str();

  std::ostringstream rows;
  if (sorted.empty()) {
    rows << "            <tr><td colspan='5'>No documents processed yet.</td></tr>";
  } else {
    for (size_t i = 0; i < recent_count; ++i) {
      const auto& doc = sorted[i].get();
      std::string status_class = (doc.status == "SUCCESS") ? "status-success" : "status-failed";
      std::ostringstream ext_ss;
      for (const auto& [k, v] : doc.extracted) {
        if (ext_ss.tellp() > 0) ext_ss << ", ";
        ext_ss << k << ": " << v;
      }
      std::string extracted_str = ext_ss.str();
      if (extracted_str.empty()) extracted_str = "None";
      if (extracted_str.size() > 100) extracted_str = extracted_str.substr(0, 100) + "...";

      rows << "            <tr>\n"
           << "                <td>" << doc.file_path << "</td>\n"
           << "                <td>" << doc.file_type << "</td>\n"
           << "                <td class=\"" << status_class << "\">" << doc.status << "</td>\n"
           << "                <td>" << doc.processed_at << "</td>\n"
           << "                <td class=\"extracted-values\">" << extracted_str << "</td>\n"
           << "            </tr>\n";
    }
  }

  std::string html = R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document Extraction Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5; }
        h1 { color: #333; }
        .summary { background-color: white; padding: 15px; border-radius: 5px; margin-bottom: 20px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        table { width: 100%; border-collapse: collapse; background-color: white; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        th, td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }
        th { background-color: #4CAF50; color: white; }
        tr:hover { background-color: #f5f5f5; }
        .status-success { color: green; font-weight: bold; }
        .status-failed { color: red; font-weight: bold; }
        .extracted-values { font-family: monospace; font-size: 0.9em; }
    </style>
</head>
<body>
    <h1>Document Extraction Report</h1>
    <div class="summary">
        <h2>Summary</h2>
        <p><strong>Total Documents:</strong> )";
  html += std::to_string(docs_map.size());
  html += R"(</p>
        <p><strong>Indexed Fields:</strong> )";
  html += indexed_fields;
  html += R"(</p>
        <p><strong>Recent Documents Shown:</strong> )";
  html += std::to_string(recent_count);
  html += R"(</p>
    </div>
    <h2>Recent Extractions</h2>
    <table>
        <thead>
            <tr>
                <th>File Path</th>
                <th>File Type</th>
                <th>Status</th>
                <th>Processed At</th>
                <th>Extracted Fields</th>
            </tr>
        </thead>
        <tbody>
)";
  html += rows.str();
  html += R"(
        </tbody>
    </table>
</body>
</html>)";

  std::filesystem::create_directories(output_path.parent_path());
  std::ofstream f(output_path);
  f << html;
}

}  // namespace document_ingestion
