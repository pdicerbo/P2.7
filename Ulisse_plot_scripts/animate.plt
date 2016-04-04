set term png
set output "diffusivity.png"
minT=0
maxT=0.9
set cbrange [minT:maxT]
plot 'diffusivity_2.dat' matrix with image

set terminal png 
frames = 4 
minT=0
maxT=0.02
set cbrange [minT:maxT]
do for [i=1:frames] {
  set output "frame".i.'.png'
  plot 'concentration_'.i.'.dat' matrix  with image
}

