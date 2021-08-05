#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

std::string dir;
std::vector<size_t> vtx_labels;
std::vector<std::tuple<size_t, size_t, size_t>> edges;
std::unordered_map<std::string, size_t> desc2id;
std::unordered_map<std::string, size_t> vtype2id;
std::unordered_map<std::string, size_t> etype2id;

void output() {
  {
    std::ofstream out(dir + "descriptions_to_ids.lumb.txt");
    for (auto& d2i : desc2id) {
      out << d2i.first << ' ' << d2i.second << std::endl;
    }
    out.close();
    std::cout << "Total number of vertices is " << desc2id.size() << std::endl;
  }
  {
    std::ofstream out(dir + "vertex_types_to_ids.lumb.txt");
    for (auto& v2i : vtype2id) {
      out << v2i.first << ' ' << v2i.second << std::endl;
    }
    out.close();
    std::cout << "Total number of vertex types is " << vtype2id.size() << std::endl;
  }
  {
    std::ofstream out(dir + "edge_types_to_ids.lumb.txt");
    for (auto& e2i : etype2id) {
      out << e2i.first << ' ' << e2i.second << std::endl;
    }
    out.close();
    std::cout << "Total number of edge types is " << etype2id.size() << std::endl;
  }
  {
    std::ofstream out(dir + "edges.lumb.txt");
    for (auto& e : edges) {
      out << std::get<0>(e) << ' ' << std::get<1>(e) << ' ' << std::get<2>(e) << std::endl;
    }
    out.close();
    std::cout << "Total number of edges is " << edges.size() << std::endl;
  }
  {
    std::ofstream out(dir + "vertex_labels.lumb.txt");
    for (int i = 0, n = vtx_labels.size(); i < n; i++) {
      out << i << ' ' << vtx_labels[i] << std::endl;
    }
    out.close();
    std::cout << "Total number of vertices is " << desc2id.size() << std::endl;
  }
}

const std::unordered_set<std::string> edge_tags = {
  "teacherOf",
  "worksFor",
  "memberOf",
  "takesCourse",
  "advisor",
  "subOrganizationOf",
  "publicationAuthor",
  "teachingAssistantOf",
  "undergraduateDegreeFrom",
  "mastersDegreeFrom",
  "doctoralDegreeFrom",
  "headOf"
};

const std::unordered_set<std::string> property_tags = {
  "emailAddress",
  "name",
  "telephone",
  "researchInterest"
};

size_t get_vtx_id(const std::string& desc) {
  auto i = desc2id.find(desc);
  return i == desc2id.end()
    ? desc2id.emplace(desc, desc2id.size()).first->second
    : i->second;
}

size_t get_vtx_type_id(const std::string& type) {
  auto i = vtype2id.find(type);
  return i == vtype2id.end()
    ? vtype2id.emplace(type, vtype2id.size()).first->second
    : i->second;
}

size_t get_edge_type_id(const std::string& type) {
  auto i = etype2id.find(type);
  return i == etype2id.end()
    ? etype2id.emplace(type, etype2id.size()).first->second
    : i->second;
}

void add_edge(size_t src_id, size_t dst_id, size_t label) {
  edges.emplace_back(src_id, dst_id, label);
}

void set_vtx_label(size_t vtx_id, size_t label) {
  if (vtx_labels.size() <= vtx_id) {
    if (vtx_labels.capacity() <= vtx_id) {
      auto cap = vtx_labels.capacity();
      cap = cap == 0 ? 2 : cap;
      while (cap <= vtx_id) {
        cap <<= 1;
      }
      vtx_labels.reserve(cap);
    }
    vtx_labels.resize(vtx_id + 1);
  }
  vtx_labels[vtx_id] = label;
}

void process_owl(const std::filesystem::directory_entry& entry) {
  std::cout << "Processing file " << entry.path() << std::endl;
  std::ifstream in(entry.path());
  for (std::string line; std::getline(in, line);) {
    if (line.find("<ub:") != 0) {
      continue;
    }
    // std::cout << "1:" << line << std::endl;
    auto s = line.find(' ', 4);
    auto vtx_type = line.substr(4, s - 4);
    auto a = line.find('\"', s) + 1;
    auto b = line.find('\"', a);
    auto vtx_desc = line.substr(a, b - a);
    auto vtx_id = get_vtx_id(vtx_desc);
    set_vtx_label(vtx_id, get_vtx_type_id(vtx_type));
    while (std::getline(in, line)) {
      if (line.find("</ub:") == 0) {
        break;
      }
      // std::cout << "2:" << line << std::endl;
      auto s = line.find("<ub:");
      auto tag_name = [&]() {
        for (auto i = s + 4;; i++) {
          if (line[i] == '>' || line[i] == ' ') {
            return line.substr(s + 4, i - (s + 4));
          }
        }
      }();
      // single line edge
      if (edge_tags.count(tag_name)) {
        if (line.find(":resource") == std::string::npos) {
          std::getline(in, line);
        }
        auto a = line.find('\"', s) + 1;
        auto b = line.find('\"', a);
        auto desc = line.substr(a, b - a);
        auto id = get_vtx_id(desc);
        add_edge(vtx_id, id, get_edge_type_id(tag_name));
      } else if (property_tags.count(tag_name)) {
        auto a = line.find('>', s) + 1;
        auto b = line.find('<', a);
        auto desc = line.substr(a, b - a);
        auto id = get_vtx_id(desc);
        add_edge(vtx_id, id, get_edge_type_id(tag_name));
      } else {
        std::cout << "Unknown tag name: " << tag_name << std::endl;
      }
    }
  }
  in.close();
}

int main(int argc, char** argv) {
  dir = argv[1];
  for (const auto& e : std::filesystem::directory_iterator(dir)) {
    if (e.path().filename().string().find(".owl") != std::string::npos) {
      process_owl(e);
    } else {
      std::cout << "Ignore " << e << std::endl;
    }
  }

  output();
}
