## Description

This project provides an implementation of algorithms to efficiently calculate the erosion and dilation of runlength encoded binary images with arbitrary structuring elements as described in [2].

These algorithms are based on distance tables and hit/miss techniques and it was shown that the runtime complexity equals O(|X|M) for erosion where |X| denotes the number of pixels of the input image and M the number of runs of the structing element B (note that usually M << |B|). Since erosion and dilation are dual operators, also dilation has a similar runtime (check [2] for formal proofs / descriptions).

Experimental results show that runtime of these algorithms are a lot faster than the runtime of the OpenCV functions and other state of the art approaches. Please also note that a conversion of an input image X into a runlength image can be done in O(|X|).

## Included Files

The zip-file contains the following files and projects:					

- iplplus		this project contains the implementation of our erosion- and dilation-algorithms as follows:
				Region::erode1		erosion variant 1 as in [1]
				Region::erode2		erosion variant 2 as in [1]
				Region::erode3		erosion variant 3 as in [1]
				Region::erode2cut	erosion as in [2]
				Region::dilate		dilation as in [1]
				Region::dilatecut	dilation as in [2]
			
				examples on how to use the code can be found in ./examples/main.cc (when loading images with 
				the implemented function loadImage be aware that foreground pixels are black)
			
- machado		the runtime test environment
				execute "bash ./thesisTests" to generate tables with the test results (to test on other images, 
				simply edit this file - foreground pixels are white)

			this project is an adapted version of the runtime test environment that was implemented by Anderson Fraiha
			Machado [3] (http://score.ime.usp.br/~dandy/).

- machado.diff		lists the changes between the original machado test-environment and our adapted version
			

------------------------------------------------------------------------------------------------------------------

## Setup

To set up the projects, extract the zip-file and do the following:
mkdir build
cd build
cmake -G "Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=Release ../iplplus

Afterwards you are able to load the project with Eclipse by navigating to File -> Import -> General -> Existing Projects into Workspace and selecting the root directory "ehren". Select all projects ("iplplus-Release@build" and "machado") to finalize the import.

As a last step build both projects (this may take a while). Now you are able to run the projects:
iplplus:	run ipltest
machado:	execute "bash ./thesisTests" or "bash ./kick-mm -X KERNEL FILENAME" where "X" should be replaced 
		by "e" or "d" (erosion or dilation) and "KERNEL" by "square" or "diamond" to choose the shape of 
		the structuring element. "FILENAME" represents the filename of a picture (png) located in 
		"./datas/finais/".

--

my system: 	Linux Ubuntu 13.04 (64 Bit)	 Eclipse 3.7.2	OpenCV 2.4.2

## Licence

iplplus: Permission is hereby granted for research and educational use only. (check ./iplplus/LICENCE.txt)

machado: Credits go to [3]; licence unknown.

## Last Update
16 02 14

[1] G. Ehrensperger, “Schnelle Algorithmen zur Berechnung
von Erosion und Dilatation auf lauflängenkodierten
Binärbildern,” Master’s thesis, University of Innsbruck,
Dept. Math., 2012. [Online]. Available:
https://numerical-analysis.uibk.ac.at/g.ehrensperger

[2] G. Ehrensperger, A. Ostermann, and F. Schwitzer
“Fast algorithms for morphological operations using
run-length encoded binary images,” Submitted Feb '14
https://arxiv.org/abs/1504.01052

[3] A. F. Machado and R. F. Hashimoto, “Jump-miss binary
erosion algorithm,” in Proc. SIBGRAPI, Rio de Janeiro,
BR, 2009, pp. 149–155.