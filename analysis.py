from sklearn.metrics import silhouette_score
import numpy as np
import pandas as pd
import sys
from kmeans import kMeans
import math
import symnmfmodule
from symnmf import parse_goal_and_calc
from symnmf import parse_cmdline_arg
from symnmf import parse_points
from symnmf import parse_k
max_iter = 300
epsilon=0.0001
beta = 0.5

def main():
    k=sys.argv[1]
    text_name = sys.argv[2]
    points=parse_points(text_name)
    k=parse_k(k, len(points))
    score_for_nmf(points,k)
    score_for_kmeans(points,k)
   
# calculate score from kmeans clustering from ex1
def score_for_kmeans(points, k):
    kms_lables=kMeans(points,k)
    score = silhouette_score(points, kms_lables, metric='euclidean')
    print(f"kmeans: {score:.4f}")
    
# calculate score from symnmf clustering
def score_for_nmf(points,k):
    final_H=parse_goal_and_calc(k,"symnmf",points)
    labels=cluster_association(final_H)
    score = silhouette_score(points, labels, metric='euclidean')
    print(f"nmf: {score:.4f}")

# determine clusters according to provided algorithm
def cluster_association(final_H):
    labels=[0 for i in range(len(final_H))]
    for i in range(len(final_H)):
        cluster=0
        score=0
        for j in range(len(final_H[i])):
            if final_H[i][j]>score:
                cluster=j
                score=final_H[i][j]
        labels[i]=cluster
    return labels



main()
