# Aerospike Get Client
A simple **Aerospike** client that allows you to receive binary data by namespace and key

# Used libraries
I tried to use standard libraries - **std** and **Boost**. However, when sending a request to the server, the key is encrypted with *RIPEMD-160*, so I connected the **OpenSSL** library.

# Possibilities
This project can be used to connect to an existing **Aerospike** server for subsequent receipt of binary data from it (strings, as well as integer and fractional numbers). You can also use connection pooling to maximize performance.
