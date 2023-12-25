## Components on Devices - Concurrent Programming assignment in Java
Language: Java

Class: Programowanie współbieżne (Concurrent Programming)

Template provided by prof. Konrad Iwanicki

My work can be found in the folder 'solution'

Task description: https://www.mimuw.edu.pl/~iwanicki/courses/cp/2023/ 

This project manages the concurrent transfer of components between devices where each device has a number of slots, which are distinguishable. It manages cycles that can occur during such transfers and ensures that no transfer that can be executed will be "starved", ie. if a transfer of component A to device 1 is waiting top be executed, another transfer of a component to device 1 cannot be scheduled to be executed before the first transfer unless the second transfer is part of a cycle. 

On a personal note, I am very proud of this project, not only because it received a high mark, but also because I put a lot of thought and effort into it.  
