# Cinema Booking System

![build](https://github.com/Sph00b/reservation-cinema-hall/workflows/build/badge.svg)

Client/server (WinAPI/POSIX) remote cinema hall seats reservation system.

Term paper of Operating Systems course.

## Overview

### Server

The server (multithreaded) accepts and processes concurrently the booking 
requests from the clients via TCP/IP.

![Alt Text](doc/server.gif)

### Client

The client provides the user with the following functions:
- Show the hall map to identify available seats.
- Send to the server the list of seats to be reserved.
- Retrieve the booking confirmation and a unique booking code from the server.
- Cancel the reservation associated with the code held.

![Alt Text](doc/client.gif)

