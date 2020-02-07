
awk '{print $1}' positions.8092-bead13 > temp

for line in $(< temp); do grep --color=never  -w $line bead13_oct10; done > positions.14222-bead13

diff positions.8092-bead13 positions.14222-bead13
