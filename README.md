# ECE-478-Project3
ECE 478-Operating Systems Project 3

Traffic Light

As of now Cars are arriving with usleep and arriving at the set time.

Elapsed timer not yet set up. Need to have it working concurrently with cars arriving, car actions and traffic lights.

Traffic light not yet implemented. 

For some reason car 0 and car 1 print out before car 0 trys to acquire lock.


When I add a line in CrossIntersection() to see if a car reaches this point, the line doesn't print out. When i add a additional line to see if the car is trying to release the lock it prints that the car reaches this point, then trys printing currentAction with garbage values and gives an error.
