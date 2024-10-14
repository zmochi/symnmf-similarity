from sklearn.metrics import silhouette_score
import numpy as np
import pandas as pd
import sys
from kmeans import kMeans
import math
import symnmfmodule

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
   

def score_for_kmeans(points, k):
    kms_lables=kMeans(points,k)
    score = silhouette_score(points, kms_lables, metric='euclidean')
    print(f"kmeans: {score:.4f}")

def score_for_nmf(points,k):
    normalized_matrix = symnmfmodule.norm(points)
    iH = init_H(k, points)
    final_H = symnmfmodule.symnmf( iH, normalized_matrix, beta, epsilon, max_iter)
    labels=cluster_association(final_H)
    score = silhouette_score(points, labels, metric='euclidean')
    print(f"nmf: {score:.4f}")

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


def init_H(k, points):
    np.random.seed(1234)
    normalized_matrix = symnmfmodule.norm(points)
    sum = 0
    for i in range(len(normalized_matrix)):
        for j in range(len(normalized_matrix[0])):
            sum += normalized_matrix[i][j]
    m = sum/(len(normalized_matrix)*len(normalized_matrix[0]))
    iH = [[] for i in range(len(points))]
    for i in range(len(points)):
        for j in range(k):
            iH[i].append(np.random.uniform(0, 2*math.sqrt(m/k)))
    return iH

def parse_cmdline_arg(arg_str):
    try:
        parsed_arg = float(arg_str)
    except ValueError:
        return -1  # indicate error with -1 since commandline args shouldn't be negative

    if parsed_arg - int(parsed_arg) != 0:
        return -1  # indicate error with -1 since commandline args shouldn't be negative
    return int(parsed_arg)

def parse_points(text_name):
    points=[]
    type = text_name[-4:]
    if not (type == ".txt"):
        print("An Error Has Occurred")
        exit(1)
    f = open(text_name, "r")
    for x in f:
        x = x.replace("\n", "")
        pointstr = x.split(",")
        point = []
        for str in pointstr:
            point.append(float(str))
        points.append(point)
    return points

def parse_k(k, len_points):
    k = parse_cmdline_arg(k)
    if not (k > 1 and k < len_points):
        print("Invalid number of clusters!")
        exit(1)
    return k


main()