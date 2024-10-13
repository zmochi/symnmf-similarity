
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


def d(p, q):
    dis = 0
    for i in range(len(p)):
        dis += (p[i]-q[i])**2
    return math.sqrt(dis)


def parse_cmdline_arg(arg_str):
    try:
        parsed_arg = float(arg_str)
    except ValueError:
        return -1  # indicate error with -1 since commandline args shouldn't be negative

    if parsed_arg - int(parsed_arg) != 0:
        return -1  # indicate error with -1 since commandline args shouldn't be negative
    return int(parsed_arg)


if (len(sys.argv) == 4):
    k = sys.argv[1]
    iter = sys.argv[2]
    text_name = sys.argv[3]
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
    k = parse_cmdline_arg(k)
    if not (k > 1 and k < len(points)):
        print("Invalid number of clusters!")
        exit(1)
    iter = parse_cmdline_arg(iter)
    if not (iter > 1 and iter < max_iter):
        print("Invalid maximum iteration!")
        exit(1)
elif (len(sys.argv) == 3):
    k = sys.argv[1]
    iter = 200
    text_name = sys.argv[2]
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
    k = parse_cmdline_arg(k)
    if not (k > 1 and k < len(points)):
        print("Invalid number of clusters!")
        exit(1)
else:
    if not (1 > 2):
        print("An Error Has Occurred")
        exit(1)

kms = kMeans(points, k, iter)
# line = ""
# for k in kms:
#     for i in range(len(k)):
#         formated_num = "%0.4f" % k[i]
#         if i != len(k)-1:
#             line += formated_num+","
#         else:
#             line += formated_num
#     print(line)
#     line = ""
