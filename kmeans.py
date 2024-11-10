
import math
import sys

max_iter = 300
epsilon=math.e**(-4)
k = ""
iter = ""
text_name = ""
type = ""
f = ""
points = []
pointstr = ""
kms = []
line = ""
formated_num = ""


def kMeans(points, k, iter=200):
    centroids = [[] for i in range(k)]
    labels=[0 for i in range(len(points))]
    for i in range(k):
        centroids[i] = points[i]

    for it in range(iter):
        clusters = [[] for i in range(k)]
        point_iter=0
        for point in points:
            distances_to_each_centroid = [
                d(point, centroid) for centroid in centroids]
            cluster_assignment = min(range(
                len(distances_to_each_centroid)), key=distances_to_each_centroid.__getitem__)
            clusters[cluster_assignment].append(point)
            labels[point_iter]=cluster_assignment
            point_iter+=1

        new_centroids = [calculate_centroid(cluster) for cluster in clusters]
        converge = True
        for i in range(k):
            if (d(centroids[i], new_centroids[i]) > epsilon):
                converge = False
        if (converge):
            return labels
        centroids = new_centroids
    print(labels)
    return labels

# determine new centroid for cluster
def calculate_centroid(cluster):
    size_of_cluster = len(cluster)
    new_centroid = []
    if len(cluster) > 0:
        d_of_point = len(cluster[0])
        for i in range(d_of_point):
            sum_i = 0
            for point in cluster:
                sum_i += point[i]
            new_centroid.append((sum_i/size_of_cluster))
    return new_centroid

# calculate distance between vectors
def d(p, q):
    dis = 0
    for i in range(len(p)):
        dis += (p[i]-q[i])**2
    return math.sqrt(dis)

