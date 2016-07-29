# LOSSY
Used for modeling of transmission lines in ATLAS Detector, "lossy" is transmission line analysis program that analyzes the effect of skin depth and precompensates signals for a microstrip setup using a convolution algorithm.


# Synopsis
First, it generates a 10GB library with LTSpice for transmission line data according to any number of desired specs:
  - resistance/m, capacitance/m, inductance/m
  - number of dropoffs
  - length of wire
  - input logic (sequence of digital 1/0)
  - microstip characteristics (length, width, etc...)

Then, using convolution, it analyzes the following:
  - inverse wave (for precompensation)
  - transfer function and gain
  - effect of skin depth
  - inverse wave accounting for skin depth
  - transfer function and gain accounting for skin depth

# Links
http://www.hep.upenn.edu/HEP_website_09/

http://atlas.cern/

https://home.cern/about/experiments/atlas

http://virtual-tours.web.cern.ch/virtual-tours/vtours/ATLAS/ATLAS.html

# Authors
Daniel Rostovtsev, Will Watkins

PENN (HEP 2016)
