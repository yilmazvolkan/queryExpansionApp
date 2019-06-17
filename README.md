# Query Expansion Application


# Introduction


In this project, I implemented an MPI application specifically a query expansion application by using word embeddings.


Word embedding is one of the most popular representation of document vocabulary. It is capable of capturing context of a word in a document, semantic and syntactic similarity, relation with other words, etc. Word embeddings are in fact a
class of techniques where individual words are represented as real-valued vectors in a predefined vector space. Each word is mapped to one vector and the vector values are learned in a way that resembles a neural network, and hence the technique is often lumped into the field of deep learning. Each word is represented by a real-valued vector, often tens or hundreds of dimensions. This is contrasted to the thousands or millions of dimensions required for sparse word representations, such as a one-hot encoding.
In order to evaluate similarities between these words we use cosine similarity.
It is commonly used approach to match similar documents is based on counting the
maximum number of common words between the documents. Mathematically, it
measures the cosine of the angle between two vectors projected in a
multi-dimensional space. In this context, the two vectors I am talking about are arrays
containing the word counts of two documents.
We use open MPI environment to implement this algorithm. In open MPI
processes have ranks and 0 rank means master node whereas any rank rather than 0
means slave nodes. I create a communication channel between these processes in
order to use parallel programming concepts efficiently. I distribute the input word
dictionary to slave nodes and for each slave I also send the query word in order to find
the most similar words from their part of this dictionary. Therefore, the programme
runs very efficiently. Moreover I used pseudocode given in the documentation of the
project.
