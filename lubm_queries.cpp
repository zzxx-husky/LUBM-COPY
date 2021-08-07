/**
 * This cpp translates the 14 queries of LUBM
 * Query6 and Query14 are not translated because they query vertices only.
 **/

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

struct Query {
  std::vector<std::string> vlabels;
  std::vector<std::tuple<int, int, std::string>> edges;
};

std::string dir;
std::unordered_map<std::string, size_t> vtype2tid;
std::unordered_map<std::string, size_t> etype2tid;
std::vector<Query> queries;

void prepare_query(
    std::vector<std::string> vlabels,
    std::vector<std::tuple<int, int, std::string>> edges) {
  for (auto& s : vlabels) {
    vtype2tid[s] = 0;
  }
  for (auto& s : edges) {
    etype2tid[std::get<2>(s)] = 0;
  }
  queries.emplace_back(Query{
    std::move(vlabels),
    std::move(edges)
  });
}

void prepare_type_ids() {
  {
    std::ifstream in(dir + "/vertex_types_to_ids.lubm.txt");
    auto cnt = vtype2tid.size();
    for (std::string line; std::getline(in, line);) {
      auto s = line.find(' ');
      auto t = line.substr(0, s);
      auto i = vtype2tid.find(t);
      if (i != vtype2tid.end()) {
        i->second = std::stoi(line.substr(s + 1, line.size() - (s + 1)));
        if (--cnt == 0) {
          break;
        }
      }
    }
    in.close();
    // for (auto& i : vtype2tid) {
    //   std::cout << i.first << ' ' << i.second << std::endl;
    // }
    if (cnt != 0) {
      std::cerr << cnt << " vertex types not found in " << (dir + "/vertex_types_to_ids.lubm.txt") << std::endl;
      exit(0);
    }
  }
  {
    std::ifstream in(dir + "/edge_types_to_ids.lubm.txt");
    auto cnt = etype2tid.size();
    for (std::string line; std::getline(in, line);) {
      auto s = line.find(' ');
      auto t = line.substr(0, s);
      auto i = etype2tid.find(t);
      if (i != etype2tid.end()) {
        i->second = std::stoi(line.substr(s + 1, line.size() - (s + 1)));
        if (--cnt == 0) {
          break;
        }
      }
    }
    in.close();
    // for (auto& i : etype2tid) {
    //   std::cout << i.first << ' ' << i.second << std::endl;
    // }
    if (cnt != 0) {
      std::cerr << cnt << " edge types not found in " << (dir + "/edge_types_to_ids.lubm.txt") << std::endl;
      exit(0);
    }
  }
}

void output_queries(std::string query_file) {
  std::ofstream out(query_file);
  out << queries.size() << std::endl;
  for (auto& q : queries) {
    out << q.vlabels.size() << ' ' << q.edges.size() << std::endl;
    for (auto& s : q.vlabels) {
      out << vtype2tid.at(s) << std::endl;
    }
    for (auto& s : q.edges) {
      out << std::get<0>(s) << ' ' << std::get<1>(s) << ' ' << etype2tid.at(std::get<2>(s)) << std::endl;
    }
  }
  out.close();
}

int main(int argc, char** argv) {
  dir = argv[1];
  /**
   * # Query1
   * # This query bears large input and high selectivity. It queries about just one class and
   * # one property and does not assume any hierarchy information or inference.
   * PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
   * PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
   * SELECT ?X	
   * WHERE
   * {?X rdf:type ub:GraduateStudent .
   *   ?X ub:takesCourse
   * http://www.Department0.University0.edu/GraduateCourse0}
   *
   * GraduateStudent:X --takesCourse--> GraduateCourse:http://www.Department0.University0.edu/GraduateCourse0
   **/
  prepare_query(
    {"GraduateStudent", "GraduateCourse", "http://www.Department0.University0.edu/GraduateCourse0"},
    {{0, 1, "takesCourse"}, {1, 2, "specifiedValue"}}
  );
  /**
   * # Query2
   * # This query increases in complexity: 3 classes and 3 properties are involved. Additionally,
   * # there is a triangular pattern of relationships between the objects involved.
   * PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
   * PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
   * SELECT ?X, ?Y, ?Z
   * WHERE
   * {?X rdf:type ub:GraduateStudent .
   *   ?Y rdf:type ub:University .
   *   ?Z rdf:type ub:Department .
   *   ?X ub:memberOf ?Z .
   *   ?Z ub:subOrganizationOf ?Y .
   *   ?X ub:undergraduateDegreeFrom ?Y}
   *
   *   GraduateStudent:X  --------memberOf-------->  Department:Z
   *           |                                          |
   * undergraduateDegreeFrom --> University:Y <-- subOrganizationOf
   **/
  prepare_query(
    {"GraduateStudent", "University", "Department"},
    {{0, 2, "memberOf"}, {2, 1, "subOrganizationOf"}, {0, 1, "undergraduateDegreeFrom"}}
  );
  /**
   * # Query3
   * # This query is similar to Query 1 but class Publication has a wide hierarchy.
   * PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
   * PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
   * SELECT ?X
   * WHERE
   * {?X rdf:type ub:Publication .
   *   ?X ub:publicationAuthor
   *         http://www.Department0.University0.edu/AssistantProfessor0}
   *
   * Publication:X --publicationAuthor--> AssistantProfessor:http://www.Department0.University0.edu/AssistantProfessor0
   **/
  prepare_query(
    {"Publication", "AssistantProfessor", "http://www.Department0.University0.edu/AssistantProfessor0"},
    {{0, 1, "publicationAuthor"}, {1, 2, "specifiedValue"}}
  );
  /**
   * # Query4
   * # This query has small input and high selectivity. It assumes subClassOf relationship
   * # between Professor and its subclasses. Class Professor has a wide hierarchy. Another
   * # feature is that it queries about multiple properties of a single class.
   * PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
   * PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
   * SELECT ?X, ?Y1, ?Y2, ?Y3
   * WHERE
   * {?X rdf:type ub:Professor .
   *   ?X ub:worksFor <http://www.Department0.University0.edu> .
   *   ?X ub:name ?Y1 .
   *   ?X ub:emailAddress ?Y2 .
   *   ?X ub:telephone ?Y3}
   *
   * Professor:X --worksFor--> Department:http://www.Department0.University0.edu
   *           +---name--> propertyValue:Y1
   *           +---emailAddress--> propertyValue:Y2
   *           +---telephone--> propertyValue:Y3
   **/
  for (auto Professor : {"FullProfessor", "AssociateProfessor", "AssistantProfessor"}) {
    prepare_query(
      {Professor, "Department", "http://www.Department0.University0.edu", "propertyValue", "propertyValue", "propertyValue"},
      {{0, 1, "worksFor"}, {1, 2, "specifiedValue"}, {0, 3, "name"}, {0, 4, "emailAddress"}, {0, 5, "telephone"}}
    );
  }
  /**
   * # Query5
   * # This query assumes subClassOf relationship between Person and its subclasses
   * # and subPropertyOf relationship between memberOf and its subproperties.
   * # Moreover, class Person features a deep and wide hierarchy.
   * PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
   * PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
   * SELECT ?X
   * WHERE
   * {?X rdf:type ub:Person .
   *   ?X ub:memberOf <http://www.Department0.University0.edu>}
   *
   * Person:X --memberOf--> Department:http://www.Department0.University0.edu
   **/
  for (auto Person : {"UndergraduateStudent", "GraduateStudent"}) {
    prepare_query(
      {Person, "Department", "http://www.Department0.University0.edu"},
      {{0, 1, "memberOf"}, {1, 2, "specifiedValue"}}
    );
  }
  /**
   * # Query7
   * # This query is similar to Query 6 in terms of class Student but it increases in the
   * # number of classes and properties and its selectivity is high.
   * PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
   * PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
   * SELECT ?X, ?Y
   * WHERE
   * {?X rdf:type ub:Student .
   *   ?Y rdf:type ub:Course .
   *   ?X ub:takesCourse ?Y .
   *   <http://www.Department0.University0.edu/AssociateProfessor0>,
   *   	ub:teacherOf, ?Y}
   *
   * Student:X --takesCourse--> Course:Y <--teacherOf-- AssociateProfessor:http://www.Department0.University0.edu/AssociateProfessor0
   **/
  for (auto Student : {"UndergraduateStudent"}) {
    prepare_query(
      {Student, "Course", "AssociateProfessor", "http://www.Department0.University0.edu/AssociateProfessor0"},
      {{0, 1, "takesCourse"}, {2, 1, "teacherOf"}, {2, 3, "specifiedValue"}}
    );
  }
  /**
   * # Query8
   * # This query is further more complex than Query 7 by including one more property.
   * PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
   * PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
   * SELECT ?X, ?Y, ?Z
   * WHERE
   * {?X rdf:type ub:Student .
   *   ?Y rdf:type ub:Department .
   *   ?X ub:memberOf ?Y .
   *   ?Y ub:subOrganizationOf <http://www.University0.edu> .
   *   ?X ub:emailAddress ?Z}
   *
   * Student:X --memberOf--> Department --subOrganizationOf--> University:http://www.University0.edu
   *         +---emailAddress--> propertyValue:Z
   **/
  for (auto Student : {"UndergraduateStudent", "GraduateStudent"}) {
    prepare_query(
      {Student, "Department", "University", "http://www.University0.edu", "propertyValue"},
      {{0, 1, "memberOf"}, {1, 2, "subOrganizationOf"}, {2, 3, "specifiedValue"}, {0, 4, "emailAddress"}}
    );
  }
  /**
   * # Query9
   * # Besides the aforementioned features of class Student and the wide hierarchy of
   * # class Faculty, like Query 2, this query is characterized by the most classes and
   * # properties in the query set and there is a triangular pattern of relationships.
   * PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
   * PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
   * SELECT ?X, ?Y, ?Z
   * WHERE
   * {?X rdf:type ub:Student .
   *   ?Y rdf:type ub:Faculty .
   *   ?Z rdf:type ub:Course .
   *   ?X ub:advisor ?Y .
   *   ?Y ub:teacherOf ?Z .
   *   ?X ub:takesCourse ?Z}
   *
   * Student:X --advisor--> Faculty:Y --teacherOf--> Course:Z
   *         |                                       ^
   *         +--------------takesCourse--------------+
   **/
  for (auto Student : {"UndergraduateStudent"}) {
    for (auto Faculty : {"FullProfessor", "AssociateProfessor", "AssistantProfessor", "Lecturer"}) {
      prepare_query(
        {Student, Faculty, "Course"},
        {{0, 1, "advisor"}, {1, 2, "teacherOf"}, {0, 2, "takesCourse"}}
      );
    }
  }
  /**
   * # Query10
   * # This query differs from Query 6, 7, 8 and 9 in that it only requires the
   * # (implicit) subClassOf relationship between GraduateStudent and Student, i.e., 
   * #subClassOf rela-tionship between UndergraduateStudent and Student does not add
   * # to the results.
   * PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
   * PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
   * SELECT ?X
   * WHERE
   * {?X rdf:type ub:Student .
   *   ?X ub:takesCourse
   * <http://www.Department0.University0.edu/GraduateCourse0>}
   *
   * Student:X --takesCourse--> GraduateCourse:http://www.Department0.University0.edu/GraduateCourse0
   **/
  prepare_query(
    {"GraduateStudent", "GraduateCourse", "http://www.Department0.University0.edu/GraduateCourse0"},
    {{0, 1, "takesCourse"}, {1, 2, "specifiedValue"}}
  );
  /**
   * # Query11
   * # Query 11, 12 and 13 are intended to verify the presence of certain OWL reasoning
   * # capabilities in the system. In this query, property subOrganizationOf is defined
   * # as transitive. Since in the benchmark data, instances of ResearchGroup are stated
   * # as a sub-organization of a Department individual and the later suborganization of 
   * # a University individual, inference about the subOrgnizationOf relationship between
   * # instances of ResearchGroup and University is required to answer this query. 
   * # Additionally, its input is small.
   * PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
   * PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
   * SELECT ?X
   * WHERE
   * {?X rdf:type ub:ResearchGroup .
   *   ?X ub:subOrganizationOf <http://www.University0.edu>}
   *
   * ResearchGroup:X --subOrganizationOf--> Department:Y --subOrganizationOf-->University:http://www.University0.edu
   **/
  prepare_query(
    {"ResearchGroup", "Department", "University", "http://www.University0.edu"},
    {{0, 1, "subOrganizationOf"}, {1, 2, "subOrganizationOf"}, {2, 3, "specifiedValue"}}
  );
  /**
   * # Query12
   * # The benchmark data do not produce any instances of class Chair. Instead, each
   * # Department individual is linked to the chair professor of that department by 
   * # property headOf. Hence this query requires realization, i.e., inference that
   * # that professor is an instance of class Chair because he or she is the head of a
   * # department. Input of this query is small as well.
   * PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
   * PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
   * SELECT ?X, ?Y
   * WHERE
   * {?X rdf:type ub:Chair .
   *   ?Y rdf:type ub:Department .
   *   ?X ub:worksFor ?Y .
   *   ?Y ub:subOrganizationOf <http://www.University0.edu>}
   *
   * FullProfessor:X --headOf--> Department:Y --subOrganizationOf--> University:http://www.University0.edu
   **/
  prepare_query(
    {"FullProfessor", "Department", "University", "http://www.University0.edu"},
    {{0, 1, "headOf"}, {1, 2, "subOrganizationOf"}, {2, 3, "specifiedValue"}}
  );
  /**
   * # Query13
   * # Property hasAlumnus is defined in the benchmark ontology as the inverse of
   * # property degreeFrom, which has three subproperties: undergraduateDegreeFrom, 
   * # mastersDegreeFrom, and doctoralDegreeFrom. The benchmark data state a person as
   * # an alumnus of a university using one of these three subproperties instead of
   * # hasAlumnus. Therefore, this query assumes subPropertyOf relationships between 
   * # degreeFrom and its subproperties, and also requires inference about inverseOf.
   * PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
   * PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
   * SELECT ?X
   * WHERE
   * {?X rdf:type ub:Person .
   *   <http://www.University0.edu> ub:hasAlumnus ?X}
   *
   * Person:X --degreeFrom--> University:http://www.University0.edu
   **/
  for (auto Person : {"FullProfessor", "AssociateProfessor", "AssistantProfessor", "Lecturer"}) {
    for (auto Degree : {"undergraduateDegreeFrom", "mastersDegreeFrom", "doctoralDegreeFrom"}) {
      prepare_query(
        {Person, "University", "http://www.University0.edu"},
        {{0, 1, Degree}, {1, 2, "specifiedValue"}}
      );
    }
  }
  prepare_query(
    {"GraduateStudent", "University", "http://www.University0.edu"},
    {{0, 1, "undergraduateDegreeFrom"}, {1, 2, "specifiedValue"}}
  );
  prepare_type_ids();
  output_queries(dir + "lubm.queries");
}

