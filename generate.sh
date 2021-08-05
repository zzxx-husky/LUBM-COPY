dir=$(pwd)/lubm_data/
mkdir ${dir}
rm ${dir}/*

mvn compile exec:java\
  -Dexec.mainClass="edu.lehigh.swat.bench.uba.Generator"\
  -Dexec.args="
    -univ 100
    -index 0
    -seed 0
    -onto zzxx
    -dir ${dir}
  "

g++ lubm_to_labeled_graph.cpp -std=c++17 -O3 -o LUBM2LabeledGraph

./LUBM2LabeledGraph ${dir}
