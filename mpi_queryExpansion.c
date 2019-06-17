#include <mpi.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Constant values and assumptions
const int MAX_LINE_LEN = 6000;
const int MAX_WORD_LEN = 20;
const int NUM_WORDS = 10;
const int VOCAB_SIZE = 1000;
const int EMBEDDING_DIMENSION= 300;
const char DELIMITER[2] = "\t";

const int COMMAND_EXIT= 0;
const int COMMAND_QUERY= 1;
const int COMMAND_CALCULATE_SIMILARITY=2;

void distributeEmbeddings(char *filename, int num_of_processes, int word_count_per_process){
  char line[MAX_LINE_LEN];
  printf( "Reading embedding file\n");
  
  FILE *file = fopen (filename, "r" );
  int wordIndex = 0;
  for(int p=1; p < num_of_processes; p++){
    printf("Sending the necessary data to process %d\n",p);
    float* embeddings_matrix = (float*)malloc(sizeof(float) * word_count_per_process * EMBEDDING_DIMENSION);

    char* words = (char*)malloc(sizeof(char) * word_count_per_process * MAX_WORD_LEN);
    
    for(int i = 0; i<word_count_per_process; i++){
      fgets(line, MAX_LINE_LEN, file);

      char *word;
      word = strtok(line, DELIMITER);
      strcpy(words+i*MAX_WORD_LEN, word);
      for(int embIndex = 0; embIndex<EMBEDDING_DIMENSION; embIndex++){
         char *field = strtok(NULL, DELIMITER);
         float emb = strtof(field,NULL);
         *(embeddings_matrix+i*EMBEDDING_DIMENSION+embIndex) = emb;
      }
    }
    printf("Sending words to process... %d\n",p);
    MPI_Send(
          /* data         = */ words, 
          /* count        = */ word_count_per_process * MAX_WORD_LEN, 
          /* datatype     = */ MPI_CHAR,
          /* destination  = */ p, 
          /* tag          = */ 0, 
          /* communicator = */ MPI_COMM_WORLD);

    printf("Sending embeddings to process %d\n",p);

    MPI_Send(
          /* data         = */ embeddings_matrix, 
          /* count        = */ word_count_per_process * EMBEDDING_DIMENSION, 
          /* datatype     = */ MPI_FLOAT,
          /* destination  = */ p,
          /* tag          = */ 0,
          /* communicator = */ MPI_COMM_WORLD);
   
   printf( "Embedding file.. has been distributed\n");
  }
}

int findWordIndex(char *words, char *query_word, int word_count_per_process){
   for(int wordIndex = 0; wordIndex<word_count_per_process; wordIndex++){
      if(strcmp((words+wordIndex*MAX_WORD_LEN), query_word)==0){
       return wordIndex;
      }
   }
 return -1;
}

void runMasterNode(int world_rank, int num_of_processes, int word_count_per_process){

    // If we are rank 0, set the number to -1 and send it to process 1

    distributeEmbeddings("./word_embeddings_small.txt", num_of_processes, word_count_per_process);

    while(1==1){
            printf("Please type a query word:\n");
            char queryWord[256];
            scanf( "%s" , queryWord);
            printf("Query word:%s\n",queryWord);
            if(strcmp(queryWord, "EXIT") == 0){
                for(int p=1; p < num_of_processes; p++){
                      MPI_Send(
                      /* data         = */ (void *)&COMMAND_EXIT, 
                      /* count        = */ 1, 
                      /* datatype     = */ MPI_INT, 
                      /* destination  = */ p,
                      /* tag          = */ 0, 
                      /* communicator = */ MPI_COMM_WORLD);
                }
                break;
            }
            else{
              for(int p=1; p < num_of_processes; p++){
                  MPI_Send(
                            /* data         = */ (void *)&COMMAND_QUERY, 
                            /* count        = */ 1, 
                            /* datatype     = */ MPI_INT, 
                            /* destination  = */ p,
                            /* tag          = */ 0, 
                            /* communicator = */ MPI_COMM_WORLD);

                  printf("Command %d sent to process %d:\n", COMMAND_QUERY, p);
                        
                  MPI_Send(
                            /* data         = */ queryWord, 
                            /* count        = */ MAX_WORD_LEN, 
                            /* datatype     = */ MPI_CHAR, 
                            /* destination  = */ p,
                            /* tag          = */ 0, 
                            /* communicator = */ MPI_COMM_WORLD);

                  printf("Query %s sent to process %d:\n", queryWord, p);
              }

              int target_process_id = -1;
              int* wordIndex = (int*)malloc(sizeof(int));
              for(int p=1; p < num_of_processes; p++){
                  MPI_Recv(
                            /* data         = */ wordIndex, 
                            /* count        = */ 1, 
                            /* datatype     = */ MPI_INT, 
                            /* source       = */ p, 
                            /* tag          = */ 0, 
                            /* communicator = */ MPI_COMM_WORLD, 
                            /* status       = */ MPI_STATUS_IGNORE);
                  if(*wordIndex >= 0){
                      target_process_id = p;
                  }
                  printf("XXX Word index:%d and p: %d\n", *wordIndex,target_process_id);
              }

              if(target_process_id > 0){
                  float* query_embeddings = (float*)malloc(sizeof(float) * EMBEDDING_DIMENSION);
                  printf("Query embeddings received\n");
                  
                  MPI_Recv(
                          /* data         = */ query_embeddings, 
                          /* count        = */ EMBEDDING_DIMENSION, 
                          /* datatype     = */ MPI_FLOAT, 
                          /* source       = */ target_process_id, 
                          /* tag          = */ 0, 
                          /* communicator = */ MPI_COMM_WORLD, 
                          /* status       = */ MPI_STATUS_IGNORE);

                  for(int p=1; p < num_of_processes; p++){
                      MPI_Send(
                      /* data         = */ (void *)&COMMAND_CALCULATE_SIMILARITY, 
                      /* count        = */ 1, 
                      /* datatype     = */ MPI_INT, 
                      /* destination  = */ p,
                      /* tag          = */ 0, 
                      /* communicator = */ MPI_COMM_WORLD);

                      printf("Calculate similarity was sent to process %d:\n", p);

                      MPI_Send(
                                /* data         = */ query_embeddings, 
                                /* count        = */ EMBEDDING_DIMENSION, 
                                /* datatype     = */ MPI_FLOAT, 
                                /* destination  = */ p,
                                /* tag          = */ 0, 
                                /* communicator = */ MPI_COMM_WORLD);
                  }
                  char similar_words[(num_of_processes-1)][MAX_WORD_LEN];
                  float* similarityScores = (float*)malloc(sizeof(float) * num_of_processes);
                      for(int p=1; p < num_of_processes; p++){
                        char* words = (char*)malloc(sizeof(char) * MAX_WORD_LEN);
                        printf("Similar word receiving\n");
                        printf("Target process:%d\n", target_process_id);
                        MPI_Recv(
                                /* data         = */ words, 
                                /* count        = */ MAX_WORD_LEN, 
                                /* datatype     = */ MPI_CHAR, 
                                /* source       = */ p, 
                                /* tag          = */ 0, 
                                /* communicator = */ MPI_COMM_WORLD, 
                                /* status       = */ MPI_STATUS_IGNORE);
                        strcpy(similar_words[p], words);  
                        printf("Similar word received\n");
                        float* similarityScore = (float*)malloc(sizeof(float));
                        MPI_Recv(
                                /* data         = */ similarityScore, 
                                /* count        = */ 1, 
                                /* datatype     = */ MPI_FLOAT, 
                                /* source       = */ p, 
                                /* tag          = */ 0, 
                                /* communicator = */ MPI_COMM_WORLD, 
                                /* status       = */ MPI_STATUS_IGNORE);
                        *(similarityScores + p) = *similarityScore;
                        printf("Similarity score received\n");
                        printf("Master similarity score %f\n", *(similarityScores + p));
                      }
                      printf("Query results: ===========================\n");
                      for(int p=1; p < num_of_processes; p++){
                          printf("*** word: %s, similarity: %f\n", similar_words[p], *(similarityScores + p));
                      }
                      printf("==========================================\n");
              }
              else{
                  for(int p=1; p < num_of_processes; p++){
                      printf("*** word: %s, similarity: %f\n", "NOT FOUND", -1.0);
                  }
              }
            }
    }
}

void runSlaveNode(int world_rank, int word_count_per_process){

  printf("Receiving words in process #: %d\n", world_rank);

  char* words = (char*)malloc(sizeof(char) * word_count_per_process*MAX_WORD_LEN);
  
  MPI_Recv(words, word_count_per_process*MAX_WORD_LEN, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  printf("Words received in process #: %d\n", world_rank);

  float* embeddings_matrix = (float*)malloc(sizeof(float) * word_count_per_process*EMBEDDING_DIMENSION);

  printf("Process %d started to receive embedding part\n", world_rank);
  
  // Now receive the message with the allocated buffer
  MPI_Recv(embeddings_matrix, word_count_per_process*EMBEDDING_DIMENSION, MPI_FLOAT, 0, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);

  printf("Process %d received embedding part\n", world_rank);

  while(1==1){

      printf("Process %d is waiting for command\n", world_rank);

      int command;

      MPI_Recv(
          /* data         = */ &command, 
          /* count        = */ 1, 
          /* datatype     = */ MPI_INT, 
          /* source       = */ 0, 
          /* tag          = */ 0, 
          /* communicator = */ MPI_COMM_WORLD, 
          /* status       = */ MPI_STATUS_IGNORE);
      printf("Command received:%d for %d\n",command,world_rank);

      if(command == COMMAND_EXIT){
          break;
      }
      else if(command == COMMAND_QUERY)
      {
          char* query_word = (char*)malloc(sizeof(char) * MAX_WORD_LEN);

          MPI_Recv(
          /* data         = */ query_word, 
          /* count        = */ MAX_WORD_LEN, 
          /* datatype     = */ MPI_CHAR, 
          /* source       = */ 0, 
          /* tag          = */ 0, 
          /* communicator = */ MPI_COMM_WORLD, 
          /* status       = */ MPI_STATUS_IGNORE);

          printf("Query word received:%s\n",query_word);
          int wordIndex = findWordIndex(words, query_word, word_count_per_process);
          MPI_Send(
          /* data         = */ &wordIndex, 
          /* count        = */ 1, 
          /* datatype     = */ MPI_INT,
          /* destination  = */ 0, 
          /* tag          = */ 0, 
          /* communicator = */ MPI_COMM_WORLD);
          printf("XXX wordIndex slave is %d\n", wordIndex); 
          if(wordIndex >= 0){
              float* query_embeddings = (embeddings_matrix+wordIndex*EMBEDDING_DIMENSION);
              MPI_Send(
              /* data         = */ query_embeddings, 
              /* count        = */ EMBEDDING_DIMENSION, 
              /* datatype     = */ MPI_FLOAT,
              /* destination  = */ 0, 
              /* tag          = */ 0, 
              /* communicator = */ MPI_COMM_WORLD);
              printf("Query embeddings is sent\n"); 
          }
      }
      else if(command == COMMAND_CALCULATE_SIMILARITY){
          printf("Calculating similarities in process :%d\n", world_rank);
          float* query_embeddings = (float*)malloc(sizeof(float) * EMBEDDING_DIMENSION);
          MPI_Recv(
          /* data         = */ query_embeddings, 
          /* count        = */ EMBEDDING_DIMENSION, 
          /* datatype     = */ MPI_FLOAT, 
          /* source       = */ 0, 
          /* tag          = */ 0, 
          /* communicator = */ MPI_COMM_WORLD, 
          /* status       = */ MPI_STATUS_IGNORE);

          int mostSimilarWordIndex = -1;
          float maxSimilarityScore = -1;

          for(int wordIndex = 0; wordIndex<word_count_per_process; wordIndex++){
              float similarity = 0.0;
              for(int embIndex = 0; embIndex<EMBEDDING_DIMENSION; embIndex++){
                 float emb1 = *(query_embeddings + embIndex);
                 float emb2 = *(embeddings_matrix + wordIndex*EMBEDDING_DIMENSION + embIndex);
                 similarity +=(emb1*emb2);
              }
              if(maxSimilarityScore < similarity){
                  maxSimilarityScore = similarity;
                  mostSimilarWordIndex = wordIndex;
              }
          }
          // Get and send the word from pth part of dictionary 
          char* vocable = (char*)malloc(sizeof(char) * MAX_WORD_LEN);
          vocable =  words + MAX_WORD_LEN * mostSimilarWordIndex;
          printf("Vocablary %s for process %d\n", vocable,world_rank);
          printf("similarities was calculated in process :%d\n", world_rank);
          printf("word is being sent back to master node in process:%d\n", world_rank);
          MPI_Send(
          /* data         = */ vocable, 
          /* count        = */ MAX_WORD_LEN, 
          /* datatype     = */ MPI_CHAR,
          /* destination  = */ 0, 
          /* tag          = */ 0, 
          /* communicator = */ MPI_COMM_WORLD);
          printf("maxSimilarityScore %f for process %d\n", maxSimilarityScore,world_rank);
          printf("similarity score is sent to master node in process:%d\n", world_rank);
          MPI_Send(
          /* data         = */ &maxSimilarityScore, 
          /* count        = */ 1, 
          /* datatype     = */ MPI_FLOAT,
          /* destination  = */ 0, 
          /* tag          = */ 0, 
          /* communicator = */ MPI_COMM_WORLD);
      }
  }
  free(embeddings_matrix);
}

int main(int argc, char** argv) {
    // Initialize the MPI environment
  MPI_Init(NULL, NULL);
    // Get the number of processes
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
  printf("Hello world from processor %s, rank %d out of %d processors\n",
   processor_name, world_rank, world_size);


    // We are assuming at least 2 processes for this task
  if (world_size < 2) {
      fprintf(stderr, "Process size must be greater than 1 for %s\n", argv[0]);
      MPI_Abort(MPI_COMM_WORLD, 1);
  }
  int wordIndex;
  int num_of_processes = world_size;
  int word_count_per_process = VOCAB_SIZE / (num_of_processes-1);

  if (world_rank == 0) {
    runMasterNode(world_rank, num_of_processes, word_count_per_process);
  } else {
    runSlaveNode(world_rank, word_count_per_process);
  }
    MPI_Finalize();
    printf("Process %d stopped\n", world_rank);
  }