Compile with g++ -g server.cpp -o server -pthread

Run without arguments: ./server

This program follows the below approximate line of execution:
    1. Parse the config file for parameters to use
    2. Create a socket and set it to listen at the port specified in the config file
    3. Create a queue. When the socket accepts an incoming connection, push the message and the client socket returned by accept() to the queue
    4. Create a thread pool of 10 threads to read from the queue.
    5. Each thread parses the client's message, sends an error response if one is required, and if not, sends the file requested. Then it sleeps for a short amount of time before checking the queue again.
    6. When an interrupt signal is sent, the server closes down all threads, closes the socket, destroys all mutexes and exits gracefully.
