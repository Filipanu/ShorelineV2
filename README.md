# ShorelineV2
GUID generation problem

# implementation
- The solution consists of a C++ class which acts as a "node" from the problem
- properties: [unsigned long long] nodeId, sessionId and extraCounter
- methods: getNodeId(), getTimestamp(), getId() 

The getId() method returns a 64 bit unsigned integer
 - first 10 bits consist of the nodeId (1024 nodes -> 2^10 -> 10 bits; in this way, different nodes can generate IDs at the same time)
 - 2 bits belong to sessionId (this is used so the application can restart in the same millisecond; details below)
 - 11 bits from internal counter (extraCounter)
 - 41 bits from timestamp in milliseconds (by using only these bits we can use the system until Wednesday, September 7, 2039)
 
 - access to the getId method is synchronised on the use and incrementation of extraCounter using a lock_guard on a mutex object
 
 # requirements
 ## 1. 100k req/sec
  - 100k req/sec = 100 req/ms
  - by using an internal counter we can provide up to 2048 different GUIDs/ms (the 11 bits from GUID generation)
  - ideally, in an environment in which the clients send their requests evenly distributed in the span of one second, we can provide ~2kk different GUIDs per second
  - however, this is not the case if the requests come too quickly (see tests below)

## 2. has to work with multiple clients
 - we simulate this by using threads and requesting GUIDs from the same Node object
 - as stated above, the access to the "extraCounter" member of Node is synchronised, so it can handle multiple requests in a thread safe manner
 
## 3. has to work after crash/restart in the same millisecond
 - this is done by using a sessionId (number from 0 to 3)
 - it is a number stored in a file
 - at start up it reads the file (if it doesn't exist it defaults to 0) and rewrites the file with the next sessionId (to be used at the next execution)
 - can handle 4 crashes/ms and give different GUIDs
 
 ## 4. has to work if the timestamp gives bad results (goes backwards)
  - assuming the timestamp eventually readjusts itself and that the number of requests/ms is close to the desired 100/ms
  - because we are using the extraCounter field, we can generate safely generate ~2k unique GUIDs (or ~20ms) untill duplicates start to appear
  - if a restart happens while the clock starts going backwards, because of sessionId, all the new GUIDs will be guaranteed different to the previous ones
  
# GUID bit length opinions
I came up with this solution because of the 64 bit constraint. A much better solution would be to use the processor clock tick (this is used in the standardised UUID -> RFC 4122.
This would solve the problems related to restarts and bad timestamp. However, the number of bits required is too large.

Because of the 64 bit length, a randomised solution is not feasible, because the rate of collisions is high.

# tests
## Generate GUIDs test

- tested with 100 threads, each thread making 1000 requests to one Node object (100k req total)
- generation done in ~0.074 seconds
- because it generates the GUIDs so fast collisions occur

Example case: 100k GUIDs generated in 0.054 sec with ~6k duplicates:
 - 100.000 req/0.054 sec = 1.851 req/ms average (real nr of req/ms is (1500; 2300)
 - because of certain factors (thread priority on CPU; access to critical section given to same thread multiple times, etc) some threads finish their requests faster than others
 - thus, it is possible that in the same ms to generate over 2048 GUIDs
 
 ## Conlusions
 - duplicates didn't happen anymore if the internal counter max was raised to 4096, but this lowers the sessionId bits to 1 (making it more vulnerable to restarts)
 - this solution meets the requirements (100k req/sec, thread-safe/multiple clients, resistant to restarts, resistant to bad clock)
 - however, if the requests are done too fast (10 to 20 times faster, like in my test example) the generation cannot work
 - this is why for real-world scenarios a 128 bit GUID is used
  
