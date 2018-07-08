# ACM SIGMOD 2017 Contest

Implementation done within the course of the subject "Software Development for Information Systems" during the winter of 2017-2018 by our team:

**Panagiotis Kokkinakos** , **Georgios Rouvalis** , **Theodoros Stefou** 

# Summary:

An N-gram of words is a contiguous sequence of N words (https://en.wikipedia.org/wiki/N-gram). For this year's contest, the task is to filter a stream of documents using a set of N-grams of interest and, for each document, return where in it one of the N-grams is found The input to the task will have two parts: first, an initial set of N-grams, which may be processed and indexed; second, a series of queries (documents) and N-gram updates (insertions or deletions), arbitrarily interleaved. For each N-gram insertion or deletion, the set of N-grams of interest is updated accordingly. For each new query (document) arriving, the task is to return as fast as possible the N-grams of the currently up-to-date set that are found in the document. These should be presented in order of their first appearance in the document. If one N-gram is a prefix of another and the larger one is in the document, then the shorter one is presented first. Note that in answering each query, all insertions/deletions preceding the query must be taken into account.
