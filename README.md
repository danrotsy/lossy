# LOSSY
A lossy transmission line analysis program that uses a convolution algorithm to analyze the effect of skin depth and to precompensate signals for a microstrip setup.
Used for modeling of transmission lines in ATLAS Detector

# SYNOPSIS
First, it generates a 10GB library with spice for transmission line data according to any number of desired specs:
  - resistance/m, capacitance/m, inductance/m
  - number of dropoffs
  - length of wire
  - input logic (sequence of digital 1/0)
  - microstip characteristics (length, width, etc...)
Then, using convolution, it analyzes the following:
  - inverse wave (for precompensation)
  - transfer function and gain
  - skin depth effect
  - inverse wave accounting for skin depth
  - transfer function and gain accounting for skin depth

# LINKS
http://www.hep.upenn.edu/HEP_website_09/
http://atlas.cern/
https://home.cern/about/experiments/atlas
http://virtual-tours.web.cern.ch/virtual-tours/vtours/ATLAS/ATLAS.html

# AUTHORS
Daniel Rostovtsev
Will Watkins

# PENN (HEP 2016)
