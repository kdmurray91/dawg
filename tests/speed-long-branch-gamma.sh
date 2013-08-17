#!/bin/sh

dawg=$1

${dawg} - <<EOF > /dev/null
Root.Length = 10000000
Tree.Tree = (A:10)B;
Sim.Reps = 10
Subst.Rate.Model = "gamma"
Subst.Rate.Params = 0.5, 0.0, 16
EOF
