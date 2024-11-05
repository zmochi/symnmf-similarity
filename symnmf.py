import numpy as np
import pandas as pd
import sys
import symnmfmodule
import math

max_iter = 300
epsilon = 0.0001
beta = 0.5


def main():
    k = sys.argv[1]
    goal = sys.argv[2]
    text_name = sys.argv[3]
    points = parse_points(text_name)
    k = parse_k(k, len(points))
    matrix = parse_goal_and_calc(k, goal, points)
    output_matrix(matrix)


def parse_cmdline_arg(arg_str):
    try:
        parsed_arg = float(arg_str)
    except ValueError:
        return -1  # indicate error with -1 since commandline args shouldn't be negative

    if parsed_arg - int(parsed_arg) != 0:
        return -1  # indicate error with -1 since commandline args shouldn't be negative
    return int(parsed_arg)


def parse_points(text_name):
    points = []
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


def parse_goal_and_calc(k, goal, points):
    matrix = []
    if goal == "symnmf":
        normalized_matrix = symnmfmodule.norm(points)
        iH = init_H(k, points)
        matrix = symnmfmodule.symnmf(
            iH, normalized_matrix, beta, epsilon, max_iter)
    elif goal == "sym":
        matrix = symnmfmodule.sym(points)
    elif goal == "ddg":
        matrix = symnmfmodule.ddg(points)
    elif goal == "norm":
        matrix = symnmfmodule.norm(points)
    return matrix


def output_matrix(matrix):
    line = ""
    for k in matrix:
        for i in range(len(k)):
            formated_num = "%0.4f" % k[i]
            if i != len(k)-1:
                line += formated_num+","
            else:
                line += formated_num
        print(line)
        line = ""


if __name__ == "__main__":
    main()
