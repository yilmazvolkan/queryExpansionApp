# Query Expansion Application


# Introduction


In this project, I implemented an MPI application specifically a query expansion application by using word embeddings.


Word embedding is one of the most popular representation of document vocabulary. It is capable of capturing context of a word in a document, semantic and syntactic similarity, relation with other words, etc. 

Word embeddings are in fact a class of techniques where individual words are represented as real-valued vectors in a predefined vector space. Each word is mapped to one vector and the vector values are learned in a way that resembles a neural network, and hence the technique is often lumped into the field of deep learning. Each word is represented by a real-valued vector, often tens or hundreds of dimensions. This is contrasted to the thousands or millions of dimensions required for sparse word representations, such as a one-hot encoding.


In order to evaluate similarities between these words I use cosine similarity. It is commonly used approach to match similar documents is based on counting the maximum number of common words between the documents. Mathematically, it
measures the cosine of the angle between two vectors projected in a multi-dimensional space. In this context, the two vectors I am talking about are arrays containing the word counts of two documents.


I use open MPI environment to implement this algorithm. In open MPI processes have ranks and 0 rank means master node whereas any rank rather than 0 means slave nodes. I create a communication channel between these processes in order to use parallel programming concepts efficiently. I distribute the input word dictionary to slave nodes and for each slave I also send the query word in order to find the most similar words from their part of this dictionary. Therefore, the programme runs very efficiently. Moreover I used pseudocode given in the documentation of the project.


# Program Interface & Execution


Program is started from console. First you need to specify the direction of the project. Afterwards, user gives command
```
>mpicc mpi_ps.c -o mpi_ps.o

```
to console in order to build executable. Then, command 

```
>mpiexec -n 11 ./mpi_ps.o

```

in order to run. 11 means the number of processes including the master node. Therefore, if you want to N the most similar words from the dictionary, you need to give console command N+1 to define also the master node. After giving these
commands, programme runs and user should give a word for query. If this word is in the dictionary, user should see the most similar N words. If it is not, user shall get NOT FOUND in the console. After sequence of queries, if user wishes to terminate the programme, he/she simply do that by giving “EXIT” query word to the console. The
example output for query “telefon” input is ;

<p align="center">
<a href = "https://github.com/yilmazvolkan/queryExpansionApp"><img 
<img src="https://github.com/yilmazvolkan/queryExpansionApp/blob/master/example1.png" width="290" height="150"></a>
</p>


Some other examples:


<p align="left">
<a href = "https://github.com/yilmazvolkan/queryExpansionApp"><img 
<img src="https://github.com/yilmazvolkan/queryExpansionApp/blob/master/example2.png" width="410" height="330"></a>
</p>
