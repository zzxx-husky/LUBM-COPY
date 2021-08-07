#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

std::string dir;
std::vector<size_t> vtx_labels;
std::unordered_map<std::string, size_t> vtype2id = {
  {"propertyValue", 0}
};
// reverse mapping of vtype2id
std::vector<std::string> id2vtype = {
  "propertyValue"
};
std::unordered_map<std::string, size_t> etype2id = {
  {"specifiedValue", 0}
};

size_t num_edges = 0;
std::ofstream edges_out;
size_t vertex_id_cnt = 0;
std::ofstream desc2ids_out;

const size_t PropertyValueType = vtype2id.at("propertyValue");
const size_t SpecifiedValueType = etype2id.at("specifiedValue");

size_t get_vtx_type_id(const std::string& type) {
  auto i = vtype2id.find(type);
  if (i == vtype2id.end()) {
    i = vtype2id.emplace(type, vtype2id.size()).first;
    id2vtype.push_back(type);
  }
  return i->second;
}

struct Desc {
  int univ;
  short dept = -1;
  short type = -1;
  short type_idx = -1;
  short pub = -1;
  short pub_idx = -1;

  friend bool operator==(const Desc& a, const Desc& b) {
    return a.univ == b.univ
      && a.dept == b.dept
      && a.type == b.type
      && a.type_idx == b.type_idx
      && a.pub == b.pub
      && a.pub_idx == b.pub_idx;
  }

  friend std::ostream& operator<<(std::ostream& out, const Desc& a) {
    out << "http://www.";
    if (a.dept != -1) {
      out << "Department" << a.dept << '.';
    }
    out << "University" << a.univ << ".edu";
    if (a.type != -1) {
      out << '/' << id2vtype.at(a.type) << a.type_idx;
    }
    if (a.pub != -1) {
      out << '/' << id2vtype.at(a.pub) << a.pub_idx;
    }
    return out;
  }

  static Desc from_string(const std::string& desc) {
    try {
      auto scan_num = [&](int i) {
        for (; '0' <= desc[i] && desc[i] <= '9'; i--);
        return i + 1;
      };
      auto s = desc.find('.', 11 + 1); // http://www.
      // 1. "http://www.University0.edu"
      if (desc[11] == 'U') {
        auto x = scan_num(s - 1);
        return {std::stoi(desc.substr(x, s - x))};
      }
      auto x = scan_num(s - 1);
      short dept = std::stoi(desc.substr(x, s - x));
      s = desc.find('.', s + 1);
      x = scan_num(s - 1);
      int univ = std::stoi(desc.substr(x, s - x));
      s = desc.find('/', s + 1);
      // 2. "http://www.Department0.University0.edu"
      if (s == std::string::npos) {
        return {univ, dept};
      }
      auto t = desc.find('/', s + 1);
      // 3. "http://www.Department0.University0.edu/Course0"
      if (t == std::string::npos) {
        x = scan_num(desc.length() - 1);
        short type_idx = std::stoi(desc.substr(x, desc.length() - x));
        short type = get_vtx_type_id(desc.substr(s + 1, x - s - 1));
        return {univ, dept, type, type_idx};
      }
      x = scan_num(t - 1);
      short type_idx = std::stoi(desc.substr(x, t - x));
      short type = get_vtx_type_id(desc.substr(s + 1, x - s - 1));
      x = scan_num(desc.length() - 1);
      short pub_idx = std::stoi(desc.substr(x, desc.length() - x));
      short pub = get_vtx_type_id(desc.substr(t + 1, x - t - 1));
      // 4. "http://www.Department0.University0.edu/AssistantProfessor3/Publication4"
      return {univ, dept, type, type_idx, pub, pub_idx};
    } catch (...) {
      std::cerr << "Error when parsing `" << desc << "` to Desc." << std::endl;
      throw;
    }
  }
};

namespace std {
template<>
struct hash<Desc> {
  inline void hash_combine(size_t& v, size_t n) const {
    v ^= n + 0x9e3779b9 + (v<<6) + (v>>2);
  }

  inline size_t operator()(const Desc& a) const {
    size_t v = a.univ;
    hash_combine(v, a.dept);
    hash_combine(v, a.type);
    hash_combine(v, a.type_idx);
    return v;
  }
};
}

std::unordered_map<std::variant<Desc, std::string>, size_t> desc2id;

void output() {
  {
    // std::ofstream out(dir + "descriptions_to_ids.lumb.txt");
    for (auto& d2i : desc2id) {
      if (d2i.first.index()) {
        desc2ids_out << std::get<std::string>(d2i.first);
      } else {
        desc2ids_out << std::get<Desc>(d2i.first);
      }
      desc2ids_out << ' ' << d2i.second << std::endl;
    }
    desc2ids_out.close();
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
    // std::ofstream out(dir + "edges.lumb.txt");
    // for (auto& e : edges) {
    //   out << std::get<0>(e) << ' ' << std::get<1>(e) << ' ' << std::get<2>(e) << std::endl;
    // }
    // out.close();
    // std::cout << "Total number of edges is " << edges.size() << std::endl;
    edges_out.close();
    std::cout << "Total number of edges is " << num_edges << std::endl;
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

const std::unordered_set<std::string> special_vertices = {
  "http://www.Department0.University0.edu/GraduateCourse0",
  "http://www.Department0.University0.edu/AssistantProfessor0",
  "http://www.Department0.University0.edu",
  "http://www.Department0.University0.edu/AssociateProfessor0",
  "http://www.University0.edu",
};

size_t get_vtx_id(const std::string& desc_str) {
  auto desc = [&]() -> std::variant<Desc, std::string> {
    if (desc_str[0] == 'h') {
      return Desc::from_string(desc_str);
    } else {
      return desc_str;
    }
  }();
  auto i = desc2id.find(desc);
  return i == desc2id.end()
    ? desc2id.emplace(desc, vertex_id_cnt++).first->second
    : i->second;
}

size_t get_edge_type_id(const std::string& type) {
  auto i = etype2id.find(type);
  return i == etype2id.end()
    ? etype2id.emplace(type, etype2id.size()).first->second
    : i->second;
}

void add_edge(size_t src_id, size_t dst_id, size_t label) {
  // edges.emplace_back(src_id, dst_id, label);
  edges_out << src_id << ' ' << dst_id << ' ' << label << std::endl;
  ++num_edges;
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
    if (special_vertices.count(vtx_desc)) {
      auto v = vertex_id_cnt++;
      auto t = get_vtx_type_id(vtx_desc);
      set_vtx_label(v, t);
      add_edge(vtx_id, v, SpecifiedValueType);
    }
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
        auto id = [&]() {
          if (tag_name[0] != 'e') {
            // researchInterest or telephone or name of different objects may be the same
            // store inside the map for later reuse
            return get_vtx_id(desc);
          } else {
            // emailAddress of different people must be different
            // directly write to the file
            desc2ids_out << desc << ' ' << vertex_id_cnt++ << std::endl;
            return vertex_id_cnt;
          }
        }();
        set_vtx_label(id, PropertyValueType); // the vertex label for property value is 0
        add_edge(vtx_id, id, get_edge_type_id(tag_name));
      } else {
        std::cout << "Unknown tag name: " << tag_name << std::endl;
      }
    }
  }
  in.close();
}

void test() {
  {
    auto desc = "http://www.Department0.University0.edu/AssociateProfessor9";
    std::cout << desc << std::endl << Desc::from_string(desc) << std::endl;
  }
  {
    auto desc = "http://www.University715.edu";
    std::cout << desc << std::endl << Desc::from_string(desc) << std::endl;
  }
  {
    auto desc = "http://www.Department0.University0.edu/GraduateStudent93";
    std::cout << desc << std::endl << Desc::from_string(desc) << std::endl;
  }
  {
    auto desc = "http://www.Department0.University0.edu";
    std::cout << desc << std::endl << Desc::from_string(desc) << std::endl;
  }
  {
    auto desc = "http://www.Department0.University0.edu/AssociateProfessor12/Publication10";
    std::cout << desc << std::endl << Desc::from_string(desc) << std::endl;
  }
}

int main(int argc, char** argv) {
  // test(); return 0;

  dir = argv[1];
  edges_out.open(dir + "edges.lumb.txt", std::ofstream::out);
  desc2ids_out.open(dir + "descriptions_to_ids.lumb.txt", std::ofstream::out);

  for (const auto& e : std::filesystem::directory_iterator(dir)) {
    if (e.path().filename().string().find(".owl") != std::string::npos) {
      process_owl(e);
    } else {
      std::cout << "Ignore " << e << std::endl;
    }
  }

  output();
}
