// Windows desktop GUI: tabbed search, history, DB view, and pie chart for keyword results.
#define UNICODE
#define _UNICODE
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "document_ingestion/config.hpp"
#include "document_ingestion/logger.hpp"
#include "document_ingestion/search_history.hpp"
#include "document_ingestion/keyword_search_service.hpp"
#include "document_ingestion/ingestion_service.hpp"
#include "document_ingestion/store.hpp"
#include "document_ingestion/database.hpp"
#include "document_ingestion/rules_repo.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <cstring>

#pragma comment(lib, "comctl32.lib")

namespace {

constexpr int IDC_TAB = 1000;
constexpr int IDC_PANEL_NEW = 1001;
constexpr int IDC_PANEL_PREV = 1002;
constexpr int IDC_PANEL_DB = 1003;

constexpr int IDC_EDIT_DIR = 1010;
constexpr int IDC_EDIT_KEY = 1011;
constexpr int IDC_BTN_SEARCH = 1012;
constexpr int IDC_LV_RESULTS = 1013;
constexpr int IDC_STATIC_SCANNED = 1014;
constexpr int IDC_PIE_HOST = 1015;

constexpr int IDC_LV_HISTORY = 1020;

constexpr int IDC_STATIC_DBINFO = 1030;
constexpr int IDC_LV_RULES = 1031;
constexpr int IDC_BTN_REFRESH_DB = 1032;

struct PieSlice {
  std::wstring label;
  int value = 0;
  COLORREF color = RGB(80, 120, 200);
};

struct PieChartData {
  int total_scanned = 0;
  std::vector<PieSlice> slices;
};

HWND g_hwnd_main = nullptr;
HWND g_hwnd_tab = nullptr;
HWND g_panel_new = nullptr;
HWND g_panel_prev = nullptr;
HWND g_panel_db = nullptr;
HWND g_hwnd_pie = nullptr;
HWND g_lbl_dir = nullptr;
HWND g_lbl_key = nullptr;
HFONT g_font_ui = nullptr;
HFONT g_font_bold = nullptr;

PieChartData g_pie_data;
WCHAR g_pie_class_name[] = L"DocIngestPieHost";

// Grey theme (cool neutral)
constexpr COLORREF CLR_WINDOW_BG = RGB(228, 228, 232);
constexpr COLORREF CLR_PANEL_BG = RGB(242, 242, 245);
constexpr COLORREF CLR_EDIT_BG = RGB(255, 255, 255);
constexpr COLORREF CLR_TEXT_DIM = RGB(52, 52, 58);
constexpr COLORREF CLR_TEXT_EDIT = RGB(32, 32, 40);

HBRUSH g_brush_window = nullptr;
HBRUSH g_brush_panel = nullptr;
HBRUSH g_brush_edit = nullptr;

std::wstring Utf8ToWide(const std::string& s) {
  if (s.empty()) return {};
  int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
  if (n <= 0) return {};
  std::wstring w(static_cast<size_t>(n - 1), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), n);
  return w;
}

std::string WideToUtf8(const std::wstring& w) {
  if (w.empty()) return {};
  int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (n <= 0) return {};
  std::string s(static_cast<size_t>(n - 1), '\0');
  WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), n, nullptr, nullptr);
  return s;
}

std::wstring GetEditTextUtf16(HWND h) {
  int len = GetWindowTextLengthW(h);
  if (len <= 0) return {};
  std::wstring w(static_cast<size_t>(len) + 1, L'\0');
  GetWindowTextW(h, w.data(), len + 1);
  w.resize(static_cast<size_t>(len));
  return w;
}

void SetStaticText(HWND h, const std::wstring& t) { SetWindowTextW(h, t.c_str()); }

void DrawPieLegend(HDC hdc, RECT legend_rc, const PieChartData& data) {
  HFONT prev_font = (HFONT)SelectObject(hdc, g_font_bold ? g_font_bold : g_font_ui);
  SetBkMode(hdc, TRANSPARENT);
  SetTextColor(hdc, CLR_TEXT_DIM);
  RECT title_rc = {legend_rc.left, legend_rc.top, legend_rc.right, legend_rc.top + 20};
  DrawTextW(hdc, L"Color key", -1, &title_rc, DT_LEFT | DT_TOP | DT_SINGLELINE);
  if (g_font_ui) SelectObject(hdc, g_font_ui);

  const int swatch = 16;
  const int row_gap = 10;
  int y = legend_rc.top + 24;
  HPEN frame_pen = CreatePen(PS_SOLID, 1, RGB(190, 192, 198));
  HPEN old_pen = (HPEN)SelectObject(hdc, frame_pen);
  HBRUSH hollow = (HBRUSH)GetStockObject(HOLLOW_BRUSH);

  for (size_t i = 0; i < data.slices.size(); ++i) {
    HBRUSH lb = CreateSolidBrush(data.slices[i].color);
    RECT sw = {legend_rc.left, y, legend_rc.left + swatch, y + swatch};
    FillRect(hdc, &sw, lb);
    DeleteObject(lb);
    SelectObject(hdc, hollow);
    Rectangle(hdc, sw.left, sw.top, sw.right, sw.bottom);

    std::wstring line = data.slices[i].label + L": " + std::to_wstring(data.slices[i].value) + L" file(s)";
    RECT tr = {legend_rc.left + swatch + 10, y - 2, legend_rc.right, y + swatch + 2};
    DrawTextW(hdc, line.c_str(), -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    y += swatch + row_gap;
  }
  SelectObject(hdc, old_pen);
  DeleteObject(frame_pen);
  SelectObject(hdc, prev_font);
}

// Fills a circular sector without GDI Pie() arc-direction quirks (device vs math coords).
void FillCircularWedge(HDC hdc, int cx, int cy, int r, double start_deg, double sweep_deg,
                       COLORREF fill) {
  if (sweep_deg <= 0.0) return;
  constexpr double kPi = 3.14159265358979323846;
  if (sweep_deg >= 359.5) {
    RECT ell = {cx - r, cy - r, cx + r, cy + r};
    HBRUSH br = CreateSolidBrush(fill);
    HBRUSH old_br = (HBRUSH)SelectObject(hdc, br);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    Ellipse(hdc, ell.left, ell.top, ell.right, ell.bottom);
    SelectObject(hdc, old_br);
    DeleteObject(br);
    return;
  }
  const int steps = (std::max)(8, (std::min)(96, static_cast<int>(std::ceil(sweep_deg / 4.0))));
  std::vector<POINT> poly;
  poly.reserve(static_cast<size_t>(steps) + 2);
  poly.push_back({static_cast<LONG>(cx), static_cast<LONG>(cy)});
  for (int i = 0; i <= steps; ++i) {
    double deg = start_deg + sweep_deg * (static_cast<double>(i) / static_cast<double>(steps));
    double rad = deg * kPi / 180.0;
    poly.push_back({static_cast<LONG>(cx + std::lround(std::cos(rad) * r)),
                    static_cast<LONG>(cy + std::lround(std::sin(rad) * r))});
  }
  HBRUSH br = CreateSolidBrush(fill);
  HBRUSH old_br = (HBRUSH)SelectObject(hdc, br);
  SelectObject(hdc, GetStockObject(NULL_PEN));
  Polygon(hdc, poly.data(), static_cast<int>(poly.size()));
  SelectObject(hdc, old_br);
  DeleteObject(br);
}

void DrawPieChart(HDC hdc, RECT rc, const PieChartData& data) {
  HBRUSH bg = CreateSolidBrush(CLR_PANEL_BG);
  FillRect(hdc, &rc, bg);
  DeleteObject(bg);

  int w = rc.right - rc.left;
  int h = rc.bottom - rc.top;
  if (w < 20 || h < 20) return;

  int total_val = 0;
  for (const auto& s : data.slices) total_val += s.value;
  if (total_val <= 0) {
    HBRUSH br = CreateSolidBrush(RGB(210, 210, 214));
    int cx = rc.left + w / 2;
    int cy = rc.top + h / 2;
    int rr = (std::min)(w, h) / 2 - 16;
    if (rr < 20) rr = (std::min)(w, h) / 4;
    RECT ell_rect = {cx - rr, cy - rr, cx + rr, cy + rr};
    SelectObject(hdc, GetStockObject(NULL_PEN));
    SelectObject(hdc, br);
    Ellipse(hdc, ell_rect.left, ell_rect.top, ell_rect.right, ell_rect.bottom);
    DeleteObject(br);
    SetBkMode(hdc, TRANSPARENT);
    HFONT old = (HFONT)SelectObject(hdc, g_font_ui);
    const wchar_t* msg =
        data.total_scanned == 0 ? L"No supported files to scan" : L"No data for chart";
    DrawTextW(hdc, msg, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, old);
    return;
  }

  constexpr int kLegendGap = 12;
  int legend_w = 168;
  if (w < 260) legend_w = (std::max)(100, w / 3);
  int pie_area_w = w - legend_w - kLegendGap;
  if (pie_area_w < 64) {
    pie_area_w = w / 2;
    legend_w = w - pie_area_w - kLegendGap;
    if (legend_w < 80) legend_w = 80;
    pie_area_w = w - legend_w - kLegendGap;
  }

  RECT pie_area = {rc.left, rc.top, rc.left + pie_area_w, rc.bottom};
  RECT legend_area = {rc.left + pie_area_w + kLegendGap, rc.top + 8, rc.right - 8, rc.bottom - 8};

  int pw = pie_area.right - pie_area.left;
  int ph = pie_area.bottom - pie_area.top;
  int margin = 16;
  int size = (std::min)(pw, ph) - margin * 2;
  if (size < 24) size = (std::min)(pw, ph) - 8;
  int cx = pie_area.left + pw / 2;
  int cy = pie_area.top + ph / 2;
  int r = size / 2;
  RECT ell_rect = {cx - r, cy - r, cx + r, cy + r};

  double angle0 = -90.0;  // degrees, top; increasing angle moves clockwise on screen
  for (size_t i = 0; i < data.slices.size(); ++i) {
    double sweep = 360.0 * static_cast<double>(data.slices[i].value) / static_cast<double>(total_val);
    FillCircularWedge(hdc, cx, cy, r, angle0, sweep, data.slices[i].color);
    angle0 += sweep;
  }
  HPEN rim = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
  HPEN old_pen = (HPEN)SelectObject(hdc, rim);
  SelectObject(hdc, GetStockObject(NULL_BRUSH));
  Ellipse(hdc, ell_rect.left, ell_rect.top, ell_rect.right, ell_rect.bottom);
  SelectObject(hdc, old_pen);
  DeleteObject(rim);

  DrawPieLegend(hdc, legend_area, data);
}

LRESULT CALLBACK PieHostWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_ERASEBKGND: {
      RECT rc;
      GetClientRect(hwnd, &rc);
      HBRUSH b = CreateSolidBrush(CLR_PANEL_BG);
      FillRect((HDC)wParam, &rc, b);
      DeleteObject(b);
      return 1;
    }
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      RECT rc;
      GetClientRect(hwnd, &rc);
      DrawPieChart(hdc, rc, g_pie_data);
      EndPaint(hwnd, &ps);
      return 0;
    }
    default:
      return DefWindowProcW(hwnd, msg, wParam, lParam);
  }
}

void RegisterPieHostClass(HINSTANCE hi) {
  static bool done = false;
  if (done) return;
  WNDCLASSW wc = {};
  wc.lpfnWndProc = PieHostWndProc;
  wc.hInstance = hi;
  wc.lpszClassName = g_pie_class_name;
  wc.hbrBackground = nullptr;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassW(&wc);
  done = true;
}

LRESULT CALLBACK PanelContainerProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_ERASEBKGND: {
      if (!g_brush_panel) return DefWindowProcW(hwnd, msg, wParam, lParam);
      RECT rc;
      GetClientRect(hwnd, &rc);
      FillRect((HDC)wParam, &rc, g_brush_panel);
      return 1;
    }
    case WM_CTLCOLORSTATIC: {
      HDC hdc = (HDC)wParam;
      SetBkColor(hdc, CLR_PANEL_BG);
      SetTextColor(hdc, CLR_TEXT_DIM);
      return g_brush_panel ? (LRESULT)g_brush_panel : DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    case WM_CTLCOLOREDIT: {
      HDC hdc = (HDC)wParam;
      SetBkColor(hdc, CLR_EDIT_BG);
      SetTextColor(hdc, CLR_TEXT_EDIT);
      return g_brush_edit ? (LRESULT)g_brush_edit : DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    default:
      return DefWindowProcW(hwnd, msg, wParam, lParam);
  }
}

void RegisterPanelClass(HINSTANCE hi) {
  static bool done = false;
  if (done) return;
  WNDCLASSW wc = {};
  wc.lpfnWndProc = PanelContainerProc;
  wc.hInstance = hi;
  wc.lpszClassName = L"DocIngestPanel";
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = nullptr;
  RegisterClassW(&wc);
  done = true;
}

void ShowTabPanels(int sel) {
  HWND panes[3] = {g_panel_new, g_panel_prev, g_panel_db};
  for (int i = 0; i < 3; ++i) {
    ShowWindow(panes[i], (i == sel) ? SW_SHOW : SW_HIDE);
  }
  if (sel >= 0 && sel < 3 && panes[sel]) {
    SetWindowPos(panes[sel], HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
  }
  if (g_hwnd_main) {
    InvalidateRect(g_hwnd_main, nullptr, TRUE);
    UpdateWindow(g_hwnd_main);
  }
}

void StyleListView(HWND lv) {
  ListView_SetBkColor(lv, RGB(252, 252, 254));
  ListView_SetTextBkColor(lv, RGB(252, 252, 254));
  ListView_SetTextColor(lv, RGB(38, 42, 52));
  ListView_SetExtendedListViewStyle(lv, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
}

LRESULT CALLBACK ButtonHandProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
  if (msg == WM_SETCURSOR && LOWORD(lParam) == HTCLIENT) {
    SetCursor(LoadCursor(nullptr, IDC_HAND));
    return TRUE;
  }
  return DefSubclassProc(hwnd, msg, wParam, lParam);
}

void CreateUIFonts() {
  LOGFONTW lf = {};
  lf.lfHeight = -15;
  lf.lfWeight = FW_NORMAL;
  wcscpy_s(lf.lfFaceName, L"Segoe UI");
  g_font_ui = CreateFontIndirectW(&lf);
  if (!g_font_ui) {
    HFONT sys = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    LOGFONTW fb = {};
    GetObjectW(sys, sizeof(fb), &fb);
    fb.lfHeight = -15;
    g_font_ui = CreateFontIndirectW(&fb);
  }
  lf.lfWeight = FW_SEMIBOLD;
  g_font_bold = CreateFontIndirectW(&lf);
  if (!g_font_bold) {
    HFONT sys = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    LOGFONTW fb = {};
    GetObjectW(sys, sizeof(fb), &fb);
    fb.lfHeight = -15;
    fb.lfWeight = FW_SEMIBOLD;
    g_font_bold = CreateFontIndirectW(&fb);
  }
}

document_ingestion::ExtractionStore LoadStore() {
  document_ingestion::ExtractionStore store;
  auto dir = document_ingestion::get_data_store_dir();
  if (std::filesystem::exists(dir / "store.json")) {
    store.load(dir);
  }
  return store;
}

void RefreshHistoryList(HWND lv) {
  ListView_DeleteAllItems(lv);
  document_ingestion::SearchHistory history(document_ingestion::get_search_history_path());
  auto searches = history.get_all_searches();
  int row = 0;
  for (const auto& s : searches) {
    LVITEMW it = {};
    it.mask = LVIF_TEXT;
    it.iItem = row;
    std::wstring ts = Utf8ToWide(s.timestamp);
    it.pszText = ts.data();
    ListView_InsertItem(lv, &it);
    std::wstring wdir = Utf8ToWide(s.directory);
    std::wstring wkw = Utf8ToWide(s.keyword);
    ListView_SetItemText(lv, row, 1, wdir.data());
    ListView_SetItemText(lv, row, 2, wkw.data());
    std::wstring nf = std::to_wstring(s.files_found);
    ListView_SetItemText(lv, row, 3, nf.data());
    std::wstring paths;
    for (size_t i = 0; i < s.found_files.size(); ++i) {
      if (i) paths += L"; ";
      paths += Utf8ToWide(s.found_files[i].file_path);
      paths += L" (";
      paths += std::to_wstring(s.found_files[i].matches);
      paths += L")";
    }
    ListView_SetItemText(lv, row, 4, paths.data());
    row++;
  }
}

void RefreshRulesList(HWND lv) {
  ListView_DeleteAllItems(lv);
  try {
    document_ingestion::init_db(document_ingestion::get_db_path().string(),
                                document_ingestion::get_schema_path().string());
    auto conn = document_ingestion::get_connection(document_ingestion::get_db_path().string());
    auto rules = document_ingestion::rules_list_all(conn.get(), std::nullopt, std::nullopt);
    int row = 0;
    for (const auto& r : rules) {
      LVITEMW it = {};
      it.mask = LVIF_TEXT;
      it.iItem = row;
      std::wstring id = std::to_wstring(r.id);
      it.pszText = id.data();
      ListView_InsertItem(lv, &it);
      std::wstring wname = Utf8ToWide(r.name);
      std::wstring wpat = Utf8ToWide(r.regex_pattern);
      ListView_SetItemText(lv, row, 1, wname.data());
      ListView_SetItemText(lv, row, 2, wpat.data());
      std::wstring ft = r.file_type ? Utf8ToWide(*r.file_type) : L"*";
      ListView_SetItemText(lv, row, 3, ft.data());
      std::wstring act = r.active ? L"yes" : L"no";
      ListView_SetItemText(lv, row, 4, act.data());
      row++;
    }
  } catch (...) {
    LVITEMW it = {};
    it.mask = LVIF_TEXT;
    it.iItem = 0;
    std::wstring err = L"(could not read database)";
    it.pszText = err.data();
    ListView_InsertItem(lv, &it);
  }
}

void AddListViewColumns(HWND lv, const std::vector<std::pair<std::wstring, int>>& cols) {
  for (size_t i = 0; i < cols.size(); ++i) {
    LVCOLUMNW c = {};
    c.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    c.pszText = const_cast<LPWSTR>(cols[i].first.c_str());
    c.cx = cols[i].second;
    c.iSubItem = static_cast<int>(i);
    ListView_InsertColumn(lv, static_cast<int>(i), &c);
  }
}

void RunSearch(HWND hwnd) {
  HWND ed_dir = GetDlgItem(g_panel_new, IDC_EDIT_DIR);
  HWND ed_key = GetDlgItem(g_panel_new, IDC_EDIT_KEY);
  HWND lv = GetDlgItem(g_panel_new, IDC_LV_RESULTS);
  HWND st = GetDlgItem(g_panel_new, IDC_STATIC_SCANNED);

  std::wstring wdir = GetEditTextUtf16(ed_dir);
  std::wstring wkey = GetEditTextUtf16(ed_key);
  std::string dir = WideToUtf8(wdir);
  std::string key = WideToUtf8(wkey);

  if (dir.empty() || key.empty()) {
    MessageBoxW(hwnd, L"Enter both directory and keyword.", L"Search", MB_OK | MB_ICONINFORMATION);
    return;
  }

  HCURSOR wait = LoadCursor(nullptr, IDC_WAIT);
  SetCursor(wait);
  ListView_DeleteAllItems(lv);

  auto results = document_ingestion::search_keyword_in_directory(dir, key, std::nullopt);
  int scanned = static_cast<int>(results.size());
  int total_matches = 0;
  std::vector<document_ingestion::FoundFile> found_files;

  g_pie_data = {};
  g_pie_data.total_scanned = scanned;

  int row = 0;
  for (const auto& r : results) {
    LVITEMW it = {};
    it.mask = LVIF_TEXT;
    it.iItem = row;
    std::wstring fp = Utf8ToWide(r.file_path);
    it.pszText = fp.data();
    ListView_InsertItem(lv, &it);
    std::wstring mt = std::to_wstring(r.matches);
    ListView_SetItemText(lv, row, 1, mt.data());
    std::wstring st8 = Utf8ToWide(r.status);
    ListView_SetItemText(lv, row, 2, st8.data());
    row++;

    if (r.matches > 0) {
      total_matches += r.matches;
      found_files.push_back({r.file_path, r.matches});
    }
  }

  int files_with_keyword = 0;
  int files_without_keyword = 0;
  for (const auto& r : results) {
    if (r.matches > 0)
      files_with_keyword++;
    else
      files_without_keyword++;
  }

  g_pie_data = {};
  g_pie_data.total_scanned = scanned;
  if (scanned > 0) {
    if (files_with_keyword > 0) {
      PieSlice sl;
      sl.label = L"Files with keyword";
      sl.value = files_with_keyword;
      sl.color = RGB(78, 148, 125);
      g_pie_data.slices.push_back(sl);
    }
    if (files_without_keyword > 0) {
      PieSlice sl;
      sl.label = L"Files without keyword";
      sl.value = files_without_keyword;
      sl.color = RGB(172, 174, 182);
      g_pie_data.slices.push_back(sl);
    }
  }

  std::wostringstream oss;
  oss << L"Files scanned: " << scanned << L"   |   With keyword: " << files_with_keyword << L"   |   Without: "
      << files_without_keyword << L"   |   Match count (total): " << total_matches;
  SetWindowTextW(st, oss.str().c_str());

  if (g_hwnd_pie) InvalidateRect(g_hwnd_pie, nullptr, TRUE);

  document_ingestion::SearchHistory history(document_ingestion::get_search_history_path());
  int stored = 0;
  try {
    auto store = LoadStore();
    auto summary = document_ingestion::run_scan(dir, &store);
    store.save(document_ingestion::get_data_store_dir());
    stored = summary.processed;
  } catch (const std::exception& e) {
    std::wstring w = Utf8ToWide(std::string("Ingestion after search: ") + e.what());
    MessageBoxW(hwnd, w.c_str(), L"Ingest", MB_OK | MB_ICONWARNING);
  } catch (...) {
    MessageBoxW(hwnd, L"Ingestion after search failed.", L"Ingest", MB_OK | MB_ICONWARNING);
  }

  history.add_search("ALL", dir, key, found_files, stored);

  RefreshHistoryList(GetDlgItem(g_panel_prev, IDC_LV_HISTORY));
  RefreshRulesList(GetDlgItem(g_panel_db, IDC_LV_RULES));

  SetCursor(LoadCursor(nullptr, IDC_ARROW));
}

void LayoutNewSearchPanel() {
  RECT rc;
  GetClientRect(g_panel_new, &rc);
  int w = rc.right - rc.left;
  int h = rc.bottom - rc.top;
  int pad = 12;
  int y = pad;
  int label_h = 18;
  int edit_h = 26;
  int btn_h = 30;

  if (g_lbl_dir) MoveWindow(g_lbl_dir, pad, y, 220, label_h, TRUE);
  MoveWindow(GetDlgItem(g_panel_new, IDC_EDIT_DIR), pad, y + label_h, w - pad * 2, edit_h, TRUE);
  y += label_h + edit_h + 8;

  if (g_lbl_key) MoveWindow(g_lbl_key, pad, y, 200, label_h, TRUE);
  MoveWindow(GetDlgItem(g_panel_new, IDC_EDIT_KEY), pad, y + label_h, 220, edit_h, TRUE);
  MoveWindow(GetDlgItem(g_panel_new, IDC_BTN_SEARCH), pad + 240, y + label_h - 1, 120, btn_h, TRUE);
  y += label_h + edit_h + 12;

  int list_h = (h - y - pad * 3) / 2;
  if (list_h < 120) list_h = 120;
  HWND lv = GetDlgItem(g_panel_new, IDC_LV_RESULTS);
  MoveWindow(lv, pad, y, w - pad * 2, list_h, TRUE);
  y += list_h + 8;

  HWND st = GetDlgItem(g_panel_new, IDC_STATIC_SCANNED);
  MoveWindow(st, pad, y, w - pad * 2, 22, TRUE);
  y += 26;

  int pie_h = h - y - pad;
  if (pie_h < 100) pie_h = 100;
  if (g_hwnd_pie) MoveWindow(g_hwnd_pie, pad, y, w - pad * 2, pie_h, TRUE);
}

LRESULT CALLBACK PanelNewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
  if (msg == WM_SIZE) {
    LayoutNewSearchPanel();
    return 0;
  }
  if (msg == WM_COMMAND && LOWORD(wParam) == IDC_BTN_SEARCH && HIWORD(wParam) == BN_CLICKED) {
    RunSearch(g_hwnd_main);
    return 0;
  }
  return DefSubclassProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_CREATE: {
      g_hwnd_main = hwnd;
      g_brush_window = CreateSolidBrush(CLR_WINDOW_BG);
      g_brush_panel = CreateSolidBrush(CLR_PANEL_BG);
      g_brush_edit = CreateSolidBrush(CLR_EDIT_BG);

      INITCOMMONCONTROLSEX icex = {sizeof(icex), ICC_TAB_CLASSES | ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES};
      InitCommonControlsEx(&icex);

      RegisterPieHostClass(GetModuleHandleW(nullptr));

      RECT rc;
      GetClientRect(hwnd, &rc);
      constexpr int kMargin = 10;
      constexpr int kTabH = 42;
      const int content_y = kMargin + kTabH + 4;
      const int content_h = (rc.bottom > content_y + kMargin) ? (rc.bottom - content_y - kMargin) : 100;

      g_hwnd_tab =
          CreateWindowExW(0, WC_TABCONTROL, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TCS_HOTTRACK,
                          kMargin, kMargin, rc.right - 2 * kMargin, kTabH, hwnd, (HMENU)(UINT_PTR)IDC_TAB,
                          GetModuleHandleW(nullptr), nullptr);

      TCITEMW tie = {};
      tie.mask = TCIF_TEXT;
      tie.pszText = L"New search";
      TabCtrl_InsertItem(g_hwnd_tab, 0, &tie);
      tie.pszText = L"Previous searches";
      TabCtrl_InsertItem(g_hwnd_tab, 1, &tie);
      tie.pszText = L"Database";
      TabCtrl_InsertItem(g_hwnd_tab, 2, &tie);

      int pw = rc.right - 2 * kMargin;
      int ph = content_h;
      int px = kMargin;
      int py = content_y;

      g_panel_new = CreateWindowExW(
          WS_EX_CONTROLPARENT, L"DocIngestPanel", L"",
          WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, px, py, pw, ph, hwnd,
          (HMENU)(UINT_PTR)IDC_PANEL_NEW, GetModuleHandleW(nullptr), nullptr);
      g_panel_prev =
          CreateWindowExW(WS_EX_CONTROLPARENT, L"DocIngestPanel", L"",
                          WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, px, py, pw, ph, hwnd,
                          (HMENU)(UINT_PTR)IDC_PANEL_PREV, GetModuleHandleW(nullptr), nullptr);
      g_panel_db =
          CreateWindowExW(WS_EX_CONTROLPARENT, L"DocIngestPanel", L"",
                          WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, px, py, pw, ph, hwnd,
                          (HMENU)(UINT_PTR)IDC_PANEL_DB, GetModuleHandleW(nullptr), nullptr);

      SetWindowSubclass(g_panel_new, PanelNewProc, 1, 0);

      g_lbl_dir = CreateWindowExW(0, L"STATIC", L"Directory to scan", WS_CHILD | WS_VISIBLE, 12, 12, 200, 18,
                                  g_panel_new, nullptr, GetModuleHandleW(nullptr), nullptr);
      CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 12, 30, 400, 24,
                      g_panel_new, (HMENU)(UINT_PTR)IDC_EDIT_DIR, GetModuleHandleW(nullptr), nullptr);

      g_lbl_key = CreateWindowExW(0, L"STATIC", L"Keyword", WS_CHILD | WS_VISIBLE, 12, 62, 200, 18, g_panel_new,
                                  nullptr, GetModuleHandleW(nullptr), nullptr);
      CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 12, 80, 200, 24,
                      g_panel_new, (HMENU)(UINT_PTR)IDC_EDIT_KEY, GetModuleHandleW(nullptr), nullptr);
      CreateWindowExW(0, L"BUTTON", L"Search", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 230, 78, 120, 28,
                      g_panel_new, (HMENU)(UINT_PTR)IDC_BTN_SEARCH, GetModuleHandleW(nullptr), nullptr);

      HWND lv = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 12, 120,
                                pw - 24, 200, g_panel_new, (HMENU)(UINT_PTR)IDC_LV_RESULTS, GetModuleHandleW(nullptr),
                                nullptr);
      StyleListView(lv);
      AddListViewColumns(lv, {{L"File", 320}, {L"Matches", 70}, {L"Status", 80}});

      CreateWindowExW(0, L"STATIC", L"Files scanned: —", WS_CHILD | WS_VISIBLE, 12, 330, pw - 24, 22,
                      g_panel_new, (HMENU)(UINT_PTR)IDC_STATIC_SCANNED, GetModuleHandleW(nullptr), nullptr);

      g_hwnd_pie = CreateWindowExW(WS_EX_CLIENTEDGE, g_pie_class_name, L"", WS_CHILD | WS_VISIBLE, 12, 360,
                                   pw - 24, 200, g_panel_new, (HMENU)(UINT_PTR)IDC_PIE_HOST,
                                   GetModuleHandleW(nullptr), nullptr);

      CreateUIFonts();
      SendMessageW(g_lbl_dir, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
      SendMessageW(g_lbl_key, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
      SendMessageW(lv, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
      SendMessageW(GetDlgItem(g_panel_new, IDC_STATIC_SCANNED), WM_SETFONT, (WPARAM)g_font_bold, TRUE);
      SendMessageW(GetDlgItem(g_panel_new, IDC_EDIT_DIR), WM_SETFONT, (WPARAM)g_font_ui, TRUE);
      SendMessageW(GetDlgItem(g_panel_new, IDC_EDIT_KEY), WM_SETFONT, (WPARAM)g_font_ui, TRUE);
      SendMessageW(GetDlgItem(g_panel_new, IDC_BTN_SEARCH), WM_SETFONT, (WPARAM)g_font_ui, TRUE);
      SetWindowSubclass(GetDlgItem(g_panel_new, IDC_BTN_SEARCH), ButtonHandProc, 10, 0);

      LayoutNewSearchPanel();

      HWND lv_h = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                  WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 12,
                                  12, pw - 24, ph - 24, g_panel_prev, (HMENU)(UINT_PTR)IDC_LV_HISTORY,
                                  GetModuleHandleW(nullptr), nullptr);
      StyleListView(lv_h);
      AddListViewColumns(lv_h, {{L"Time", 140}, {L"Directory", 180}, {L"Keyword", 100}, {L"Files w/ hits", 90},
                                {L"Paths (matches)", 400}});
      SendMessageW(lv_h, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
      RefreshHistoryList(lv_h);

      CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE, 12, 12, pw - 24, 40, g_panel_db,
                      (HMENU)(UINT_PTR)IDC_STATIC_DBINFO, GetModuleHandleW(nullptr), nullptr);
      std::wstring dbpath = L"Database: " + Utf8ToWide(document_ingestion::get_db_path().string());
      SetWindowTextW(GetDlgItem(g_panel_db, IDC_STATIC_DBINFO), dbpath.c_str());
      SendMessageW(GetDlgItem(g_panel_db, IDC_STATIC_DBINFO), WM_SETFONT, (WPARAM)g_font_ui, TRUE);

      HWND lv_r = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"",
                                  WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS, 12,
                                  56, pw - 24, ph - 68, g_panel_db, (HMENU)(UINT_PTR)IDC_LV_RULES,
                                  GetModuleHandleW(nullptr), nullptr);
      StyleListView(lv_r);
      AddListViewColumns(lv_r, {{L"ID", 40}, {L"Name", 120}, {L"Regex", 280}, {L"Type", 60}, {L"Active", 50}});
      SendMessageW(lv_r, WM_SETFONT, (WPARAM)g_font_ui, TRUE);
      CreateWindowExW(0, L"BUTTON", L"Refresh rules", WS_CHILD | WS_VISIBLE, pw - 140, 12, 120, 28, g_panel_db,
                      (HMENU)(UINT_PTR)IDC_BTN_REFRESH_DB, GetModuleHandleW(nullptr), nullptr);
      SendMessageW(GetDlgItem(g_panel_db, IDC_BTN_REFRESH_DB), WM_SETFONT, (WPARAM)g_font_ui, TRUE);
      SetWindowSubclass(GetDlgItem(g_panel_db, IDC_BTN_REFRESH_DB), ButtonHandProc, 11, 0);
      RefreshRulesList(lv_r);

      return 0;
    }
    case WM_ERASEBKGND: {
      if (!g_brush_window) return DefWindowProcW(hwnd, msg, wParam, lParam);
      RECT rc;
      GetClientRect(hwnd, &rc);
      FillRect((HDC)wParam, &rc, g_brush_window);
      return 1;
    }
    case WM_NOTIFY: {
      LPNMHDR nh = reinterpret_cast<LPNMHDR>(lParam);
      if (nh->hwndFrom == g_hwnd_tab && nh->code == TCN_SELCHANGE) {
        int sel = TabCtrl_GetCurSel(g_hwnd_tab);
        ShowTabPanels(sel);
      }
      return 0;
    }
    case WM_COMMAND: {
      if (LOWORD(wParam) == IDC_BTN_REFRESH_DB && HIWORD(wParam) == BN_CLICKED) {
        RefreshRulesList(GetDlgItem(g_panel_db, IDC_LV_RULES));
        std::wstring dbpath = L"Database: " + Utf8ToWide(document_ingestion::get_db_path().string());
        SetWindowTextW(GetDlgItem(g_panel_db, IDC_STATIC_DBINFO), dbpath.c_str());
      }
      return 0;
    }
    case WM_SIZE: {
      RECT r;
      GetClientRect(hwnd, &r);
      constexpr int kMargin = 10;
      constexpr int kTabH = 42;
      const int content_y = kMargin + kTabH + 4;
      const int ph = (r.bottom > content_y + kMargin) ? (r.bottom - content_y - kMargin) : 100;
      const int pw = r.right - 2 * kMargin;
      MoveWindow(g_hwnd_tab, kMargin, kMargin, pw, kTabH, TRUE);
      MoveWindow(g_panel_new, kMargin, content_y, pw, ph, TRUE);
      MoveWindow(g_panel_prev, kMargin, content_y, pw, ph, TRUE);
      MoveWindow(g_panel_db, kMargin, content_y, pw, ph, TRUE);
      LayoutNewSearchPanel();

      HWND lv_h = GetDlgItem(g_panel_prev, IDC_LV_HISTORY);
      MoveWindow(lv_h, 12, 12, pw - 24, ph - 24, TRUE);

      MoveWindow(GetDlgItem(g_panel_db, IDC_STATIC_DBINFO), 12, 12, pw - 24, 40, TRUE);
      MoveWindow(GetDlgItem(g_panel_db, IDC_BTN_REFRESH_DB), pw - 140, 12, 120, 28, TRUE);
      MoveWindow(GetDlgItem(g_panel_db, IDC_LV_RULES), 12, 56, pw - 24, ph - 68, TRUE);
      return 0;
    }
    case WM_DESTROY:
      if (g_font_ui) {
        DeleteObject(g_font_ui);
        g_font_ui = nullptr;
      }
      if (g_font_bold) {
        DeleteObject(g_font_bold);
        g_font_bold = nullptr;
      }
      if (g_brush_window) {
        DeleteObject(g_brush_window);
        g_brush_window = nullptr;
      }
      if (g_brush_panel) {
        DeleteObject(g_brush_panel);
        g_brush_panel = nullptr;
      }
      if (g_brush_edit) {
        DeleteObject(g_brush_edit);
        g_brush_edit = nullptr;
      }
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProcW(hwnd, msg, wParam, lParam);
  }
}

}  // namespace

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int show) {
  char exe_path[MAX_PATH * 4];
  GetModuleFileNameA(nullptr, exe_path, static_cast<DWORD>(sizeof(exe_path)));
  document_ingestion::init_project_root_from_exe(exe_path);
  document_ingestion::ensure_dirs();
  document_ingestion::set_default_log_path(document_ingestion::get_log_path().string());
  try {
    document_ingestion::init_db(document_ingestion::get_db_path().string(),
                                document_ingestion::get_schema_path().string());
  } catch (...) {
    // GUI still works for keyword search; DB tab may show error
  }

  WNDCLASSW wc = {};
  wc.lpfnWndProc = MainWndProc;
  wc.hInstance = hInst;
  wc.lpszClassName = L"DocumentIngestionMain";
  wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = nullptr;
  RegisterClassW(&wc);

  RegisterPanelClass(hInst);

  HWND hwnd = CreateWindowExW(0, L"DocumentIngestionMain",
                              L"Document search & extraction", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT, 1000, 760, nullptr, nullptr, hInst, nullptr);
  ShowWindow(hwnd, show);
  UpdateWindow(hwnd);

  MSG msg;
  while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }
  return static_cast<int>(msg.wParam);
}
