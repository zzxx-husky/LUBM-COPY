dir=$(pwd)/lubm_data/
mkdir ${dir}
remove_and_regen=False

if [ "$(ls ${dir} | head | wc -l)" -eq 0 ]; then
  if [ "${remove_and_regen}" == "True" ]; then
    rm ${dir}/*;

    mvn compile exec:java\
      -Dexec.mainClass="edu.lehigh.swat.bench.uba.Generator"\
      -Dexec.args="
        -univ 100000
        -index 0
        -seed 0
        -onto zzxx
        -dir ${dir}
      ";
  fi
fi

g++ lubm_to_labeled_graph.cpp -std=c++17 -O3 -o LUBM2LabeledGraph -lstdc++fs

./LUBM2LabeledGraph ${dir}
