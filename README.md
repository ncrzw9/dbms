# dbms
This program is used along with Table.cat and Attr.cat to simulate the Database Management System. There are four commands:

  PUT     - A prompt to generate a record using the attributes detailed in Attr.cat

  GET     - A prompt to display (tab-delimited) the attributes of a record given the table's key by the user.
   
  DELETE  - A prompt to remove the key-value from the table using the table's key.
  
  EXIT    - A command to close the database.

To ensure atomicity, the nodes are persisted throughout every operation (insert; split | delete; graft | search). Since implementing secondary storage persistence, my insert and delete benchmarks have struggled. I have chosen to revisit this project and improve object interfaces and system architecture overall once the semester completes.

I enjoyed this project and consider it the magnum opus of the course. This assignment unifies my experience from industry and the theoritical material learned throughout the course. This assignment provides an optimal transition into my senior project. I intend to work on a relational algebra engine and improve disk IO for modern data-intensive application.
