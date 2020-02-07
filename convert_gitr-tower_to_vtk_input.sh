awk 'BEGIN{i=0; j=0} {print "gitr_tower",i, "elem", j,  $0; if(NR%3==0){i++;
j=0;}  else{j++;} }' gitr-mesh-pisces-tower.txt >  vtk_input.txt
