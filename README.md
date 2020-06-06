# AerospikeGetClient
A simple Aerospike client that allows you to receive binary data by namespace and key

# Used libraries
I tried to use standard libraries - **std** and **Boost**. However, when sending a request to the server, the key is encrypted with *RIPEMD-160*, so I connected the **OpenSSL** library.
